#include "config.h"

#ifdef WITH_SPIFFS
// TODO: check
// https://gist.github.com/amakukha/7854a3e910cb5866b53bf4b2af1af968
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
/**
 *  Handle general requests for the web-server. Basically:
 *  - split path from extension
 *  - hash the path and append the extension
 *  - serve the file HASH.EXT with the mime-type of EXT.
 *  - or fail, if it does not exist.
 */
void onRequest(AsyncWebServerRequest *request) {
  const char *url = request->url().c_str();
  msLastEvent = millis();
#ifdef WITH_SPIFFS
  if(!strcmp(url, "/"))
    url="/portal.html";
  // We need to grab the extension before hashing, because otherwise
  // the MIME-type cannot be set.
  char *ext = strrchr(url,'.');
  if( !ext ){
    // No extension, no service...
    request->send(404,"text/html", "Not found: " + request->url());
    return;
  }
  char buf[256];
  // Construct name of file in SPIFFS.
  snprintf(buf, sizeof(buf), "/%016llx%s", hash(url), ext);
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
#else  // Will we ever go back to no SPIFFS? this can be pruned?
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
}

/**
 * 
 */
void onConfig(AsyncWebServerRequest *request){
  msLastEvent = millis();
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
/**
 *
 */
void onScan(AsyncWebServerRequest *request) {
#define TEMPL "<tr><td><input type='checkbox' name='blazers' checked='true'/>"\
  "</td><td><input value='%s'/></td><td><input value='%s'/></td></tr>"
  char buf[256];

  msLastEvent = millis();
  DEBUG_MSG("onScan Request\n");
  // Find all peers that advertise debug on udp
  int nrOfServices = MDNS.queryService("debug", "udp");

  String rsp  = "<!DOCTYPE HTML><html lang='de'><head><title>Config Page</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<link href='blazer.css' rel='stylesheet'>"
    "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
    "<script src='blazer2.js'></script>"
    "</head>"
    "<body><h1>ESP WLAN Peers</h1><center>"
    "<table id='BlazerTable'>";
  // Put myself in the list
  snprintf(buf, 256, TEMPL, WiFi.localIP().toString().c_str(), myName);
  rsp += buf;
  // Add everyone found in local net
  for (int i = 0; i < nrOfServices; i=i+1) {
    snprintf(buf, 256, TEMPL, MDNS.IP(i).toString().c_str(), MDNS.hostname(i));
    rsp += buf;
  }
  // And send page
  rsp +=
    "</table><input type='submit' value='Submit by IP' onclick='launch(\"portal.html\")'>"
    "<br><input type='submit' value='Submit by name' onclick='launch2(\"portal.html\")'></center></body></html>"; 
  request->send(200, "text/html", rsp);
}
