#ifndef _mywifi_h_
#define _mywifi_h_

#ifdef WITH_WIFI_CLIENT
int setupWiFiClient(const char* ssid, const char *passwd, const int LED, const char *hostname);
#endif
#ifdef WITH_WIFI_AP
int setupWiFiAP(const char *ssid, const char *passwd);
#endif

#if defined(WITH_WIFI_CLIENT) && defined(WITH_WIFI_AP)
int setupWiFiBoth(const char *softSSID, const char *softPasswd, const char *extSSID, const char *extPasswd,
                  const char *hostname, 
                  const int LED);
#endif



#endif
