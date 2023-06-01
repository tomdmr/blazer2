#include "config.h"


bool touchDetected = false;
int  touchSens     = 23;

#ifdef WITH_TOUCH
void
gotTouch(){
  touchDetected = true;
}

void setupTouch(){
  touchAttachInterrupt(T3, gotTouch, touchSens);
  esp_sleep_enable_touchpad_wakeup();
  Serial.println("Touch attach");
}
#endif
