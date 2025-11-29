#include <Adafruit_GFX.h>
#include <FastLED.h>

class FastLEDDisplay: public Adafruit_GFX{
private:
CRGB* leds;
public:
    FastLEDDisplay(CRGB* leds, int stripCount, int ledsPerStrip);
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
};