#include "config.h"
#include "pixels.h"

RgbColor ledMatrix[NUMPIXELS];
// setAll(RgbColor)
// setOne(pos, RgbColor)
// rotateLeft() rotateRight()

/**
 * blink2: toggle between color c1 and c2.
 * Each color lights for duration ms, in total loops repetitions
 *
 */
void
blink2(const RgbColor c1, const RgbColor c2,const int duration, const int loops){
  for(int i=0; i< duration; i++){
    strip.ClearTo(c1);
    strip.Show();
    for(int j=0; j<duration; j++) delay(1);

    strip.ClearTo(c2);
    strip.Show();
    for(int j=0; j<duration; j++) delay(1);
  }
}
/**
**
*/
void
ledSetTo(const RgbColor color){
  strip.ClearTo(color);
  strip.Show();
}
