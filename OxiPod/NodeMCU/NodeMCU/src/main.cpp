
// Example sketch which shows how to display some patterns
// on a 64x32 LED matrix
//

#include "dma/ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include "BufferedDisplay/BufferedDisplay.h"
#include "BufferedDisplay\drawStringHelpers.h"
#include <DHT.h>


#define PANEL_RES_X 32      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 16     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another
 
  HUB75_I2S_CFG::i2s_pins my_pin_map = {
      // R1, G1, B1 all connect to your DR pin (21)
      4, 12, 13,
      // R2, G2, B2 (unused)
      14, 15, 21,
      // A, B (your row address pins)
      16, 17,
      // C, D, E (unused)
      25, -1, -1,
      // LAT, OE, CLK (your control pins)
      32, 33, 27
  };
  HUB75_I2S_CFG mxconfig(
      PANEL_RES_X,
      PANEL_RES_Y,
      PANEL_CHAIN,
      my_pin_map // <--- Custom pin map passed here
  );

//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA dma_display = MatrixPanel_I2S_DMA(mxconfig);
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// Brightness parameter controls overall intensity (0-255)
uint16_t colorWheel(uint8_t pos, uint8_t brightness = 255) {
  uint32_t r, g, b;
  if (brightness == 0)
  return 0;
  if(pos < 85) {
    // Red to Green transition
    r = 255 - pos * 3;
    g = pos * 3;
    b = 0;
  } else if(pos < 170) {
    // Green to Blue transition
    pos -= 85;
    r = 0;
    g = 255 - pos * 3;
    b = pos * 3;
  } else {
    // Blue to Red transition
    pos -= 170;
    r = pos * 3;
    g = 0;
    b = 255 - pos * 3;
  }
  
  // Apply brightness scaling
  if (brightness < 255) {
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;
  }
  
  return dma_display.color565(r, g, b);
}

int addrX, addrY, addrW, addrH;
void dma_display_startWrite(){addrX = 0; addrY = 0; addrW = 0; addrH = 0;}
void dma_display_setAddressWindow(int x, int y, int width, int height){
  addrX = x;
  addrY = y;
  addrW = width;
  addrH = height;
}
void dma_display_writePixels(uint16_t *colors, uint16_t count){
  for (int y = 0; y < addrH; y++)
    for (int x = 0; x < addrW; x++){
      dma_display.drawPixel(addrX + x, addrY + y, colors[0]);
      colors++;
    }
}
void dma_display_endWrite(){}

BufferedDisplay display(dma_display, dma_display_startWrite, dma_display_setAddressWindow, dma_display_writePixels, dma_display_endWrite);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-HUB75-MatrixPanel-I2S-DMA Example");
  // Display Setup
  dma_display.begin();
  display.setWidth(32);
  display.setHeight(16);
}

#include <Fonts/FreeSerif9pt7b.h>
uint8_t wheelval = 0;
long lastGlow = 0;
long lastGlowExpand = 0;
int focusR = 0;
void loop() {
  int col = colorWheel(wheelval);
  int w = 6;
  if (millis() - lastGlow > 3000){
    focusR = 0;
    lastGlowExpand  = millis();
  }
  display.fillScreen(0);
  if (focusR < 25 + w){
    for (int r = 25; r >= 0 ; r--){
      int dist = abs(r - focusR);
      int o = ((w - dist) * 255) / w;
      if (o < 0) o = 0; else if (o > 255) o = 255;
      display.fillEllipse(16, 8, r, r / 2, colorWheel(wheelval, o));
    }
    focusR++;
    lastGlow = millis();
    if (millis() - lastGlowExpand > 20){
      focusR++;
      lastGlowExpand = millis();
    }
  }
  display.setFont(&FreeSerif9pt7b);
  display.setTextColor(0);
  display.setCursor(5, 5);
  String str = "128";

  int period = 10000; // 10 second period
  long currentTime = millis() % period;
  int opacity = map(currentTime, 0, period, 0, 510); // 0-510 for full cycle

  // Convert triangular wave: 0->255->0
  if (opacity > 255) {
      opacity = 510 - opacity; // Reverse from 255 back down to 0
  }

  // Use the color position for your color wheel
  display.setTextColor(colorWheel(0, opacity)); // Full brightness
  //Serial.println(opacity);

  centerString(&display, str.c_str(), 16, 8);
  display.update();
}
