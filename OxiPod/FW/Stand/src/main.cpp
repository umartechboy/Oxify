
// Example sketch which shows how to display some patterns
// on a 64x32 LED matrix
//

#include "dma/ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include "BufferedDisplay/BufferedDisplay.h"
#include "BufferedDisplay\drawStringHelpers.h"
#include <PMserial.h> // Arduino library for PM sensors with serial interface
#include <DHT.h>
#include "aqi.h"
#define RingDelay 3000
#define RingSpeed_ms 100
#define RingSize 8
#define TextGlowPeriod 10000
#define aqiAveraging 50
SerialPM pms(PMS5003, 34, 22); // PMSx003, RX, TX

DHT dht(5, DHT11);

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
  dht.begin();
  // Display Setup
  dma_display.begin();
  pms.init();
  display.setWidth(32);
  display.setHeight(16);
}

#include <Fonts/FreeSerif9pt7b.h>
uint8_t wheelval = 0;
long lastGlow = 0;
long lastGlowExpand = 0;
int focusR = 0;
long lastAqiUpdate = 0;
int lastAQI = 0;
void loop() {
  if (millis() - lastAqiUpdate > 1000)
  {
    if (pms.read() == 0){
      // In your loop function:
      lastAQI = (lastAQI * aqiAveraging + getFinalAQI(pms.pm25, pms.pm10, dht.readTemperature(), dht.readHumidity()) * (100 - aqiAveraging)) / 100;
      printAQIDebugInfo(pms.pm25, pms.pm10, dht.readTemperature(), dht.readHumidity());
      lastAqiUpdate = millis();
    }
    else{ // something went wrong
      switch (pms.status)
      {
      case pms.OK: // should never come here
        break;     // included to compile without warnings
      case pms.ERROR_TIMEOUT:
        Serial.println(F(PMS_ERROR_TIMEOUT));
        break;
      case pms.ERROR_MSG_UNKNOWN:
        Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
        break;
      case pms.ERROR_MSG_HEADER:
        Serial.println(F(PMS_ERROR_MSG_HEADER));
        break;
      case pms.ERROR_MSG_BODY:
        Serial.println(F(PMS_ERROR_MSG_BODY));
        break;
      case pms.ERROR_MSG_START:
        Serial.println(F(PMS_ERROR_MSG_START));
        break;
      case pms.ERROR_MSG_LENGTH:
        Serial.println(F(PMS_ERROR_MSG_LENGTH));
        break;
      case pms.ERROR_MSG_CKSUM:
        Serial.println(F(PMS_ERROR_MSG_CKSUM));
        break;
      case pms.ERROR_PMS_TYPE:
        Serial.println(F(PMS_ERROR_PMS_TYPE));
        break;
      }
    }
  }
  AQIColor _c = getAQIColorRGB(lastAQI);

  int w = RingSize;
  if (millis() - lastGlow > RingDelay){
    focusR = 0;
    lastGlowExpand  = millis();
  }
  display.fillScreen(0);
  if (focusR < 25 + w){
    for (int r = 25; r >= 0 ; r--){
      int dist = abs(r - focusR);
      int o = ((w - dist) * 255) / w;
      if (o < 0) o = 0; else if (o > 255) o = 255;
      
      uint16_t aqiColor2 = dma_display.color565((_c.r * o) / 255, (_c.g * o) / 255,(_c.b * o) / 255);
      display.fillEllipse(16, 8, r, r / 2, aqiColor2);
    }
    lastGlow = millis();
    if (millis() - lastGlowExpand > RingSpeed_ms){
      focusR++;
      lastGlowExpand = millis();
    }
  }

  
  display.setFont(&FreeSerif9pt7b);
  display.setTextColor(0);
  display.setCursor(5, 5);
  String str = String(lastAQI);
  if (lastAQI == 0)
  str = "-";

  int period = TextGlowPeriod; // 10 second period
  long currentTime = millis() % period;
  int opacity = map(currentTime, 0, period, 0, 510); // 0-510 for full cycle

  // Convert triangular wave: 0->255->0
  if (opacity > 255) {
      opacity = 510 - opacity; // Reverse from 255 back down to 0
  }

  uint16_t aqiColor = dma_display.color565((_c.r * opacity) / 255, (_c.g * opacity) / 255,(_c.b * opacity) / 255);
  // Use the color position for your color wheel
  display.setTextColor(aqiColor); // Full brightness
  //Serial.println(opacity);

  centerString(&display, str.c_str(), 16, 8);
  display.update();

}
