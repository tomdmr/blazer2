#include "config.h"
#include "Arduino.h"

#ifdef WITH_SPIFFS
// Hash function from djb.
static unsigned long long
hash(const char* str){
  unsigned long long hash = 5831;
  int c;

  while((c = *str++)){
    hash = ((hash<<5)+hash) ^ c;
  }
  return hash;
}
#endif
void onRequest(AsyncWebServerRequest *request) {
  const char *url = request->url().c_str();
#ifdef WITH_SPIFFS
  if(!strcmp(url, "/"))
    url="/portal.html";
  //Serial.printf("request for %s\n", url);

  //unsigned long long hs = hash( url );
  char *ext = strrchr(url,'.');
  if( !ext ){
    request->send(404,"text/html", "Not found: " + request->url());
    return;
  }
  char buf[256];
  //sprintf(buf, "Request for %s, hash is %016llx%s\n", url, hs, ext);
  sprintf(buf, "/%016llx%s", hash(url), ext);
  Serial.println(buf);
  if(SPIFFS.exists(buf)){
    AsyncWebServerResponse *response;
    if(!strcmp(ext, ".html")){
      response = request->beginResponse(SPIFFS, buf, "text/html");
    }
    else if(!strcmp(ext, ".js")){
      response = request->beginResponse(SPIFFS, buf, "text/javascript");
    }
    else if(!strcmp(ext, ".css")){
      response = request->beginResponse(SPIFFS, buf, "text/css");
    }
    else{
      response = request->beginResponse(SPIFFS, buf, "text/plain");
    }
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  }
  else
    request->send(404,"text/html", "Not found: " + request->url());
#else
  if(!strcmp(url, "/demo01.html")){
    DEBUG_MSG("Request for /demo02");
    sendZipped(request, demo01_html_gz, demo01_html_gz_len);
  }
  else if(!strcmp(url, "/demo02.html")){
    DEBUG_MSG("Request for /demo02");
    sendZipped(request, demo02_html_gz, demo02_html_gz_len);
  }
  else if(!strcmp(url, "/demo03.html")){
    DEBUG_MSG("Request for /demo03");
    sendZipped(request, demo03_html_gz, demo03_html_gz_len);
  }
  else if(!strcmp(url, "/portal.html") || !strcmp(url, "/")){
    DEBUG_MSG("Request for /portal");
    sendZipped(request, portal_html_gz, portal_html_gz_len);
  }
  else if(!strcmp(url, "/blazer2.js")){
    DEBUG_MSG("Request for /blazer2.js");
    AsyncWebServerResponse *response = 
    request->beginResponse_P(200, "text/javascript", blazer2_js_gz, blazer2_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  }
  else if(!strcmp(url, "/blazer.css")){
    DEBUG_MSG("Request for /blazer.css");
    AsyncWebServerResponse *response = 
    request->beginResponse_P(200, "text/css", blazer_css_gz, blazer_css_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  }
  else{
    request->send(404,"text/html", "Not found: " + request->url());
  }
#endif
 //request->redirect("/");
}


void onConfig(AsyncWebServerRequest *request){
  DEBUG_MSG("onConfig request\n");
  if(request->hasParam("name")
    && request->hasParam("lan")
    && request->hasParam("pw") ){
    strncpy(extSSID,   request->getParam("lan")->value().c_str(), sizeof(extSSID));
    strncpy(extPasswd, request->getParam("pw")->value().c_str(), sizeof(extPasswd));
    strncpy(myName,    request->getParam("name")->value().c_str(), sizeof(myName));
    DEBUG_MSG("TryWiFi on %s/%s and hName %s\n", extSSID, extPasswd, myName);
    request->send(200, "text/html", "OK, try to connect" );
    // save preferences
    savePreferences();
  }
}
void onScan(AsyncWebServerRequest *request) {
  DEBUG_MSG("onScan Request\n");
  if( ON_AP_FILTER(request) ){ msAPActivity = millis(); }
  if ( (request->hasParam("NW", true)) && (request->hasParam("HNAME", true))) {
    DEBUG_MSG("Looks good\n");
    String NW, pw, hname;
    NW = "" + request->getParam("NW", true)->value();
    if( request->getParam("PASSWD", true) ){
      pw = "" + request->getParam("PASSWD", true)->value();
    }
    else{
      pw = "";
    }
    hname = "" + request->getParam("HNAME", true)->value();
    strncpy(extSSID,   request->getParam("NW", true)->value().c_str(), sizeof(extSSID));
    strncpy(extPasswd, request->getParam("PASSWD", true)->value().c_str(), sizeof(extPasswd));
    strncpy(myName,    request->getParam("HNAME", true)->value().c_str(), sizeof(myName));
    DEBUG_MSG("TryWiFi on %s/%s and hName %s\n", extSSID, extPasswd, myName);
    request->send(200, "text/html", "OK, try to connect" );
    tryExt = true;
  }
  else if ( (request->hasParam("HNAME", true)) ) {
    String  hname = "" + request->getParam("HNAME", true)->value();
    strncpy(myName, hname.c_str(), sizeof(myName));
    DEBUG_MSG("Changing name to %s\n", myName);
    request->send(200, "text/html", "OK, try to connect" );
    tryExt = true;
  }
  else {
    Serial.println("Request for host " + request->host() + ", path " + request->url() );
   String rsp  = "<!DOCTYPE HTML><html lang='de'><head><title>Config Page</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
        "<script src='blazer2.js'></script>"
        "</head>"
        "<body><h1>ESP WLAN Config</h1><form action='/scan' method='post'>"
        "<table>"
        "<tr><td>My Name</td><td><input type='text' name='HNAME' value='" + String(myName) + "' /></td></tr>";
    int n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true);
    } else if (n) {
      for (int i = 0; i < n; ++i) {
        rsp += "<tr><td>" + String(i) + "</td>"
               "<td>" + WiFi.SSID(i) + "</td>"
               "<td><input type='radio' name='NW' value='" + WiFi.SSID(i) + "'/></td>"
               "<td>" + String(WiFi.RSSI(i)) + "</td></tr>";
      }
      rsp += "<tr><td>Password</td><td><input type='password' name='PASSWD'/></td></tr>";
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) {
        WiFi.scanNetworks(true);
      }
    }
    rsp +=
      "</table><input type='submit' value='Submit'></form></body></html>";
    request->send(200, "text/html", rsp);
    rsp = String();
  }
}
/**
 * Set credentials to home-net
 */
void onDevHome(AsyncWebServerRequest *request) {
  DEBUG_MSG("Request for /devhome");
  strcpy(extSSID, HOMESSID);
  strcpy(extPasswd, HOMEPASSWD);
  request->send(200, "text/html", "OK, try to connect" );
  // Here we save preferences unconditionally
  savePreferences();
  tryExt = true;
}
void onDevHall(AsyncWebServerRequest *request) {
  DEBUG_MSG("Request for /devhall");
  strcpy(extSSID, HALLESSID);
  strcpy(extPasswd, HALLEPASSWD);
  request->send(200, "text/html", "OK, try to connect" );
  // Here we save preferences unconditionally
  savePreferences();
  tryExt = true;
}
