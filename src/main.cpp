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

#include "mywifi.h"
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
long lastMillis = 0;

void IRAM_ATTR
gotButton(){
  if(millis() - lastMillis >10){
    ets_printf("Trigger!\n");
    touchDetected = true;
  }
  lastMillis = millis();
}
#endif


int tryConnectWiFi(){
  DEBUG_MSG("WiFi: AP with %s, WiFi to %s\n", myName, extSSID);
  int res = setupWiFiBoth(myName, "", extSSID, extPasswd, myName, 0);
  if( res ){
    setupWiFiAP(myName, "");
    // FIXME: StartScanning
    DEBUG_MSG("Connect failed, keeping AP open\n");
    msAPActivity = millis();
    isAP = true;
  }
  else{
    // Connect OK. Shutdown AP

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
    DEBUG_MSG("Connect to %s as %s OK, shutting down AP\n", extSSID, myName);
    isAP = false;
    WiFi.enableAP(false);
    // And save preferences
    savePreferences();
# ifdef WITH_MDNS
    if(!MDNS.begin(myName)) {
      Serial.println("Error starting mDNS");
    }
    MDNS.addService("http", "tcp", 80);
#  endif
  }
  return res;
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void
goToSleep(){
  DEBUG_MSG("Going to sleep now\n");
  ws.textAll("CLOSE");
#ifdef WITH_NEOPIXELBUS
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
/******************************************************************************/
/******************************************************************************/
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
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

#define DELAYVAL 500

extern void initWebSocket();
void
setup(void){
  Serial.begin(230400);
  while( !Serial ) { ; }
  Serial.println("Start setup");
#ifdef WITH_TOUCH
  // Touch sensor
  touchAttachInterrupt(T3, gotTouch, touchSens);
  esp_sleep_enable_touchpad_wakeup();
  Serial.println("Touch attach");
#endif

#ifdef WITH_BUTTON
  pinMode(GPIO_NUM_33, INPUT_PULLUP);
  // enable wakeup
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //1 = High, 0 = Low
  // enable Interrupt
  attachInterrupt(GPIO_NUM_33, gotButton, FALLING );
  Serial.println("Button attach");
#endif
#ifdef WITH_NEOPIXELBUS  
  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();
#endif  
  // Preferences
  loadPreferences();
  // WiFi
  tryConnectWiFi();
#ifdef DEBUG_UDP  
  udpOK = true;
  DEBUG_MSG("Turn on UDP\n");
#endif  
  // File sytem
#ifdef WITH_SPIFFS
  SPIFFS.begin(true);
#endif  

  
  // Websockets
  initWebSocket();
  // Web sserver
  server.onNotFound(onRequest);
  server.on("/scan", HTTP_ANY, onScan);
  server.on("/devhome", HTTP_ANY, onDevHome);
  server.on("/devhall", HTTP_ANY, onDevHall);
  server.on("/config", HTTP_ANY, onConfig);
  server.begin();
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
      Serial.println("Start updating " + type);
      })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();
#endif
  msAPActivity = msLastTouch = msLastEvent = millis();
}
void
loop(void){

  // Always
  ws.cleanupClients();
#ifdef WITH_OTA
  ArduinoOTA.handle();
#endif
  // if /scan got submitted
  if( tryExt ){
    DEBUG_MSG("New test on extrnal WiFi\n");
    tryExt = false;
    tryConnectWiFi();
  }
  if(touchDetected){
    touchDetected = false;
    DEBUG_MSG("Raw Touch\n");
    if(( millis() > msLastTouch + 50)){
      char sString[255];
      sprintf(sString, "TOUCH%d%d%d %lu %d", ledState[0], ledState[1], ledState[2], millis(), touchRead(T3));
      ws.textAll(sString);
      DEBUG_MSG("TOUCH: %d %lu\n", touchRead(T3), millis());
    }
    msLastTouch = msLastEvent = millis();
  }
  if( isAP && (millis()  > msAPActivity + AP_GRACEPERIOD)){
    Serial.println("Shutting down AP");
    isAP = false;
    WiFi.enableAP(false);
    msLastEvent = millis();
  }

  if( !isAP && (millis() > msLastEvent + IDLE_TO_SLEEP)){
    DEBUG_MSG("Loop: going to sleep, millis=%ld, msLastEvent=%ld\n", millis(), msLastEvent);
    goToSleep();
  } 
}

