#ifndef _config_h
#define _config_h
#include <Arduino.h>
/**
 * credentials.h defines 2 macros:
 * #define HOMESSID    "myHomeWlan"
 * #define HOMEPASSWD  "password_for_home"
 */
#include "credentials.h"

// Either this
//#define WITH_TOUCH
// or that
#define WITH_BUTTON
//
// send debug messages to serial
#define DEBUG_SERIAL
// send debug messages to UDP
#define DEBUG_UDP
#define WITH_NEOPIXELBUS
// Enable WiFi Client
#define   WITH_WIFI_CLIENT
// Run MDNS to advertise services
#define   WITH_MDNS
// Enable OTA Updates
#define   WITH_OTA
// Mini file system
#define   WITH_SPIFFS
// Space for config settings
#define STORAGE_SPACE     "blazer"
// Go to sleep after 10 Minutes
#define IDLE_TO_SLEEP      (600  * 1000L)


/******************************************************************************/
#include <Preferences.h>
void loadPreferences();
void savePreferences();


#ifdef DEBUG_UDP
#define   WITH_UDP
#endif

#if defined(WITH_WIFI_CLIENT) || defined(WITH_WIFI_AP)
  // Import required libraries
#  include <WiFi.h>
#  include <WiFiMulti.h>
#  include <AsyncTCP.h>
#  include <ESPAsyncWebServer.h>
   extern AsyncWebServer server;
   extern AsyncWebSocket ws;
   extern char extSSID[32];
   extern char extPasswd[32];
   extern char myName[32];
#endif

#ifdef WITH_UDP
#  include <WiFi.h>
#  include <WiFiMulti.h>
#  include <WiFiUdp.h>
#  include "udpdbg.h"
#endif

#ifdef WITH_NEOPIXELBUS
#  include <NeoPixelBus.h>
#  include "pixels.h"
#endif


#ifdef WITH_MDNS
#  include <ESPmDNS.h>
#endif

#ifdef WITH_OTA
#  include <ArduinoOTA.h>
#endif
#ifdef WITH_SPIFFS
#  include <SPIFFS.h>
#endif

#ifdef WITH_TOUCH
extern int  touchSens;
extern unsigned long msLastTouch;    // millis() of the last event
void gotTouch();
void setupTouch();

#endif
#if defined(WITH_TOUCH) || defined(WITH_BUTTON)
#endif
extern unsigned long msLastEvent;    // millis() of the last event

#ifdef DEBUG_SERIAL
#define SERIAL_OUT(x) Serial.print(x)
#else
#define SERIAL_OUT(x)
#endif

#ifdef DEBUG_UDP
#define UDP_OUT(x) sendUDP(x)
#else
#define UDP_OUT(x)
#endif

#define DEBUG_MSG(...) \
  do{                                                           \
    char __es[706]; snprintf(__es, sizeof(__es), __VA_ARGS__);  \
    SERIAL_OUT(__es);                                           \
    UDP_OUT(__es);                                              \
  }while(0)
#endif
