#include "config.h"

// TODO: Clean-up
#ifdef WITH_TOUCH
extern int  touchSens;
#endif

void notifyClients() {
  char sString[256];

#ifdef WITH_NEOPIXELBUS
  strip.ClearTo(0);
  for(int i=0; i<NUMPIXELS; i++){
    strip.SetPixelColor
      (i,
       RgbColor(30*ledState[0],
                25*ledState[1],
                40*ledState[2]));
  }
  strip.Show();
#endif
  
  sprintf(sString, "STATE%d%d%d %lu %d", ledState[0], ledState[1], ledState[2], millis(), touchRead(T3));
  ws.textAll(sString);
  DEBUG_MSG(sString);
  msLastEvent = millis();
}

/**
 */
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    DEBUG_MSG("Got cmd %s\n", data);
    char *req = (char *)data;
    if     (!strncmp(req, "STATE", 4)){
      notifyClients();
    }
    else if(!strncmp(req, "set", 3)){
      req += 3;
      int v = atoi(req) & 0x03;
      ledState[v] = 1;
      notifyClients();
    }
    else if(!strncmp(req, "clr", 3)){
      req += 3;
      int v = atoi(req) & 0x03;
      ledState[v] = 0;
      notifyClients();
    }
    else if(!strncmp(req, "SET", 3)){
      req += 3;
      for(int i=0; i<3; i++){
        int s = (byte)req[i]-(byte)'0';
        ledState[i] = s ? 1 : 0;
      }
      notifyClients();
    }
    else if(!strncmp(req, "CLR", 3)){
      ledState[0] = ledState[1] = ledState[2]=0;
      notifyClients();
    }
#ifdef WITH_TOUCH
    else if(!strncmp(req, "GTT", 3)){
      char rsp[256];
      sprintf(rsp, "%d", touchSens);
      ws.textAll(rsp);
      msLastEvent = millis();      
    }
    else if(!strncmp(req, "STT", 3)){
      req +=3;
      int newT = atoi(req);
      if((newT>0) && (newT<256)){
        touchSens = newT;
        savePreferences();
      }
      msLastEvent = millis();      
    }
#endif
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      DEBUG_MSG("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      DEBUG_MSG("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
      DEBUG_MSG("PONG\n");
      break;
    case WS_EVT_ERROR:
      DEBUG_MSG("ERROR\n");
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  DEBUG_MSG("initWebSocket: Done\n");
}
