#ifndef _main_h
#define _main_h

// Always required
Preferences preferences;
unsigned long msLastEvent;    // millis() of the last event
unsigned long msLastTouch;    // millis() of the last touch

#ifdef WITH_WIFI_CLIENT
char  extSSID[32]   = HOMESSID;
char  extPasswd[32] = HOMEPASSWD;
char  myName[32]    = "esp-default";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
#endif

#ifdef WITH_WIFI_AP
bool isAP   = true;
bool tryExt = false;
unsigned long msAPActivity;   // millis() of last AP event
#endif

#ifdef WITH_TOUCH
#endif

bool ledState[3] = {0, 0, 0};

#ifdef WITH_NEOPIXELBUS
//NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(NUMPIXELS, NEO_PIN);
NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod> strip(NUMPIXELS, NEO_PIN);
#endif

#endif
