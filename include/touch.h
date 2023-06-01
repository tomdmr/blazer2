#ifndef _touch_h
#define _touch_h

extern bool touchDetected;
extern int  touchSens;

extern int  touchSens;
extern unsigned long msLastTouch;    // millis() of the last event
void gotTouch();
void setupTouch();


#endif
