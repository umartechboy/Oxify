#include "FastLEDDisplay.h"
#include "..\..\FW\Stand\src\BufferedDisplay\color.h"

FastLEDDisplay::FastLEDDisplay(CRGB* leds, int stripCount, int ledsPerStrip):Adafruit_GFX(stripCount * 2, ledsPerStrip / 2){
    this->leds = leds;
}
void FastLEDDisplay::drawPixel(int16_t x, int16_t y, uint16_t color){
    if (x < 0 || y < 0 || x >= width() || y > height())
    return;

    if (x % 2 == 0) 
        y = (height() - y - 1); // swap direction

    Color c = color;
    leds[x * height() + y] = CRGB(c.R(), c.G(), c.B());
}