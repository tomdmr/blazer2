/**
 * ~/.platformio/penv/bin/pio project init --board esp32dev  # Init project
 * ~/.platformio/penv/bin/pio lib install AsyncTCP           # Install al ib
 * ~/.platformio/penv/bin/pio lib install "me-no-dev/ESP Async WebServer"
 * ~/.platformio/penv/bin/pio run --target clean
 * ~/.platformio/penv/bin/pio run
 * ~/.platformio/penv/bin/pio run --target upload
 * ~/.platformio/penv/bin/pio run -t upload --upload-port 192.168.13.171
 * ~/.platformio/penv/bin/pio device monitor -b 115200
 * python3 -m websockets ws://esp00/ws
 */
#include "Arduino.h"
#include "config.h"
#include "main.h"

#include "http_handler.h"

static bool touchDetected = false;

#ifdef WITH_TOUCH
int  touchSens     = 23;
void
gotTouch(){
  touchDetected = true;
}
#endif

#ifdef WITH_BUTTON
static long lastMillis = 0;
void IRAM_ATTR
gotButton(){
  if(millis() - lastMillis >10){
    //ets_printf("Trigger!\n");
    touchDetected = true;
  }
  lastMillis = millis();
}
#endif

#ifdef WITH_SPIFFS
char *
fgets(char *str, int n, File f){
  char *p=str;
  while(n-- && f.available()){
    *p = f.read();
    if( *p=='\n' ){
      *p=0;
      return str;
    }
    p++;
  }
  *p='\0';
  return str;
}
#endif
/* *************************************************************************  */
/* *** W i F i   C o n n e c t i o n   H a n d l i n g *********************  */
/* *************************************************************************  */
/**
 *  - Reset WiFi to a clean state
 *  - Get Network info from /APConfig.txt
 *  - Add fallback network
 *  - Try to connect via WiFiMulti
 *  - Start MDNS
 */
static WiFiMulti wifiMulti;
int
tryConnectWiFi(){
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(myName); //define hostname
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF); 

  WiFi.mode(WIFI_STA);
  File f = SPIFFS.open("/APConfig.txt", FILE_READ);
  // FIXME: Handle failure
  while(f && f.available()){
    char ssid[256];
    char pass[256];
    char *s = fgets(ssid, 255, f);
    char *p = fgets(pass, 255, f);
    Serial.printf("SSID: %s, PASWD: %s\n", s, p);
    wifiMulti.addAP(s, p);
  }
  if(f) f.close();
  // Fallback, if file is missing
  // Bad_ WiFiMulti does not check if a pair is entered
  // more than once...
  wifiMulti.addAP(HOMESSID, HOMEPASSWD);
  //
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
#endif
  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() != WL_CONNECTED) {
#ifdef WITH_NEOPIXELBUS
    for(int i=0; i<5; i++){
      strip.ClearTo(RgbColor(25,0,10));
      strip.Show();
      delay(125);
      strip.ClearTo(0);
      strip.Show();
      delay(120);
    }
#endif
    return 1;
  }
#ifdef WITH_MDNS
    if(!MDNS.begin(myName)) {
      Serial.println("Error starting mDNS");
    }
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("debug", "udp", 3333);
#  endif
  return 0;
}
/* ************************************************************************** */
/* ** S l e e p   a n d   W a k e   U p ************************************* */
/* ************************************************************************** */
 void
goToSleep(){
  DEBUG_MSG("Going to sleep now\n");
  ws.textAll("CLOSE");
  savePreferences();
#ifdef WITH_NEOPIXELBUS
    for(int i=0; i<5; i++){
      strip.ClearTo(RgbColor(0,0,35));
      strip.Show();
      delay(125);
      strip.ClearTo(0);
      strip.Show();
      delay(125);
    }
  strip.ClearTo(0);
  strip.Show();
  while (!strip.CanShow()) // wait until the last show call completes and the LEDs are off
    {
      yield();
    }
#endif
  server.end();
  esp_deep_sleep_start();
}
#if 0
/* interesting for debugging, but not needed here. */
void
print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
#endif
/******************************************************************************/
/** P r e f e r e n c e   H a n d l i n g *************************************/
/******************************************************************************/
void
loadPreferences(){
  // false: read/write
  preferences.begin(STORAGE_SPACE, false);
  preferences.getString("hname", myName,   sizeof(myName) );
  preferences.getString("ssid",  extSSID,   sizeof(extSSID));
  preferences.getString("pswd",  extPasswd, sizeof(extPasswd));
#ifdef WITH_TOUCH
  touchSens = preferences.getInt   ("tsns", 23);
  DEBUG_MSG("Load preferences: myName: <%s>, connect to %s/%s, touch: %d\n",
            myName, extSSID, extPasswd, touchSens);
#else
  DEBUG_MSG("Load preferences: myName: <%s>, connect to %s/%s\n",
            myName, extSSID, extPasswd);
#endif
  preferences.end();
}
/******************************************************************************/
void
savePreferences(){
  preferences.begin(STORAGE_SPACE, false);
  preferences.putString("hname",  myName);
  preferences.putString("ssid", extSSID);
  preferences.putString("pswd", extPasswd);
#ifdef WITH_TOUCH
  preferences.putInt   ("tsns", touchSens);
  DEBUG_MSG("Save preferences: myName: <%s>, connect to %s/%s, touch: %d\n",
            myName, extSSID, extPasswd, touchSens);
#else
  DEBUG_MSG("Save preferences: myName: <%s>, connect to %s/%s\n",
            myName, extSSID, extPasswd);
#endif
  preferences.end();
}
/* ************************************************************************** */
/* ** G e n e r a l   S e t u p ********************************************* */
/* ************************************************************************** */

#define DELAYVAL 50
extern void initWebSocket();

void
setup(void){
  Serial.begin(230400);
  while( !Serial ) { ; }
  Serial.println("Start setup");
#ifdef WITH_NEOPIXELBUS  
  // this resets all the neopixels to an off state
  strip.Begin();
  //strip.Show();
  strip.SetPixelColor(1, RgbColor(0,25,0));
  strip.Show();
  DEBUG_MSG("Neopixels\n");
  delay(DELAYVAL);
#endif

#ifdef WITH_TOUCH
  // Touch sensor
  touchAttachInterrupt(T3, gotTouch, touchSens);
  esp_sleep_enable_touchpad_wakeup();
  DEBUG_MSG("Touch attach\n");
#endif

#ifdef WITH_BUTTON
  pinMode(GPIO_NUM_33, INPUT_PULLUP);
  // enable wakeup
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //1 = High, 0 = Low
  // enable Interrupt
  attachInterrupt(GPIO_NUM_33, gotButton, FALLING );
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
  delay(DELAYVAL);
#endif
#endif
  // Preferences
  loadPreferences();
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
  delay(DELAYVAL);
#endif
  // File sytem
#ifdef WITH_SPIFFS
  SPIFFS.begin(true);
#endif
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
  delay(DELAYVAL);
#endif
  // WiFi
  tryConnectWiFi();
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
  delay(DELAYVAL);
#endif
#ifdef DEBUG_UDP  
  udpOK = true;
  DEBUG_MSG("Turn on UDP\n");
#endif
  DEBUG_MSG("Build date: %s/%s, ", __DATE__, __TIME__);
  DEBUG_MSG("WiFi OK, MyIp=%s\n", WiFi.localIP().toString().c_str());

  
  // Websockets
  initWebSocket();
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
  delay(DELAYVAL);
#endif
  // Web sserver
  server.onNotFound(onRequest);
  server.on("/scan", HTTP_ANY, onScan);
#if 0
  server.on("/devhome", HTTP_ANY, onDevHome);
  server.on("/devhall", HTTP_ANY, onDevHall);
#endif
  server.on("/config", HTTP_ANY, onConfig);
  server.begin();
#ifdef WITH_NEOPIXELBUS
  strip.RotateLeft(1);
  strip.Show();
#endif

#ifdef WITH_OTA
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH){
        type = "sketch";
      }
      else{ // U_SPIFFS
        type = "filesystem";
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
#ifdef WITH_SPIFFS
        SPIFFS.end();
#endif
      }
      DEBUG_MSG("Start updating %s\n", type.c_str());
      })
    .onEnd([]() {
      DEBUG_MSG("\nEnd\n");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
                  DEBUG_MSG("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      DEBUG_MSG("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) DEBUG_MSG("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) DEBUG_MSG("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) DEBUG_MSG("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) DEBUG_MSG("Receive Failed");
      else if (error == OTA_END_ERROR) DEBUG_MSG("End Failed");
    });
  ArduinoOTA.setHostname(myName);
  ArduinoOTA.begin();
#endif
#ifdef WITH_NEOPIXELBUS
  for(int i=0; i<5; i++){
    strip.ClearTo(RgbColor(0,25,10));
    strip.Show();
    delay(250);
    strip.ClearTo(0);
    strip.Show();
    delay(250);
  }
#endif
  msLastTouch = msLastEvent = millis();
  Serial.println("Done init");
}

/* ************************************************************************** */
/* ** L o o p   A c t i v i t i e s ***************************************** */
/* ************************************************************************** */
//static int loopCtr=100000;
void
loop(void){
  /*
  if(loopCtr == 100000){
    Serial.println(millis());
    loopCtr=0;
  }
  ++loopCtr;
  */
  // Always
  ws.cleanupClients();
  // TODO: Often, change to, like, every 1000ms.
  if( WiFi.status() != WL_CONNECTED){
    //Serial.println("WiFi: No connection");
    if (wifiMulti.run( 1000 ) == WL_CONNECTED) {
      Serial.print("WiFi connected: ");
      Serial.print(WiFi.SSID());
      Serial.print(" ");
      Serial.println(WiFi.RSSI());
      Serial.println(WiFi.localIP());
    }
  }
#ifdef WITH_OTA
  ArduinoOTA.handle();
#endif

  if(touchDetected){
    touchDetected = false;
    if(( millis() > msLastTouch + 50)){
      char sString[255];
      sprintf(sString, "TOUCH%d%d%d %lu %d", ledState[0], ledState[1], ledState[2], millis(), touchRead(T3));
      ws.textAll(sString);
      //DEBUG_MSG("TOUCH: %d %lu\n", touchRead(T3), millis());
    }
    msLastTouch = msLastEvent = millis();
  }
  if( millis() > msLastEvent + IDLE_TO_SLEEP ){
    //DEBUG_MSG("Loop: going to sleep, millis=%ld, msLastEvent=%ld\n", millis(), msLastEvent);
    goToSleep();
  }
}

