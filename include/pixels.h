#ifndef _pixels_h
#define _pixels_h

#include <NeoPixelBus.h>
#define NEO_PIN    2
#define NUMPIXELS 16
//extern NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip;
extern NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod> strip;
extern bool ledState[3];

extern RgbColor ledMatrix[NUMPIXELS];

#endif
