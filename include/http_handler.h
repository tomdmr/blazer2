#ifndef _http_handler_h_
#define _http_handler_h_
void onRequest(AsyncWebServerRequest *request);
void onConfig(AsyncWebServerRequest *request);
void onScan(AsyncWebServerRequest *request);
void onDevHome(AsyncWebServerRequest *request);
void onDevHall(AsyncWebServerRequest *request);
#endif
