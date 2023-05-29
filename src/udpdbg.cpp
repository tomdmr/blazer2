#include <string.h>

#include "config.h"
#include <WiFi.h>
#include <WiFiUdp.h>

static const int udpPort = 3333;
static IPAddress broadcastIP(255,255,255,255);

//create UDP instance
WiFiUDP udp;
bool udpOK=false;


void
sendUDP(char *msg){
  if(!udpOK) return;
  udp.beginPacket(broadcastIP, udpPort);
  udp.write((const unsigned char*)msg, strlen(msg));
  udp.endPacket();
}

