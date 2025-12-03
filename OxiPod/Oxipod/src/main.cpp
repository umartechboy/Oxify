#include <Arduino.h>
#include <FastLED.h>
#include <PMserial.h> // Arduino library for PM sensors with serial interface
#include <DHT.h>
#include "..\..\FW\Stand\src\aqi.h"
#include "..\..\FW\Stand\src\aqi.cpp"
#include "..\..\FW\Stand\src\BufferedDisplay\Bitmap.h"
#include "..\..\FW\Stand\src\BufferedDisplay\BufferedDisplay.h"
#include "..\..\FW\Stand\src\BufferedDisplay\BufferedDisplay.cpp"
#include "..\..\FW\Stand\src\BufferedDisplay\color.h"
#include "..\..\FW\Stand\src\BufferedDisplay\color.cpp"
#include "..\..\FW\Stand\src\BufferedDisplay\drawStringHelpers.h"
#include "FastLEDDisplay.h"

#define DHTPin 22
#define PMTX 18
#define PMRX 19
#define LEDPin 5
#define LEDStrips 35
#define LEDsPerStrip 300
#define LedCount (LEDsPerStrip * LEDStrips)
#define aqiAveraging 50

// Define the array of leds
CRGB leds[LedCount];
FastLEDDisplay fDisplay(leds, LEDStrips, LEDsPerStrip);

SerialPM pms(PMS5003, PMRX, PMTX); // PMSx003, RX, TX
DHT dht(DHTPin, DHT11);

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
      //dma_display.drawPixel(addrX + x, addrY + y, colors[0]);
      colors++;
    }
}
void dma_display_endWrite(){}

BufferedDisplay display(fDisplay, dma_display_startWrite, dma_display_setAddressWindow, dma_display_writePixels, dma_display_endWrite);


void setup2() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, 5>(leds, LedCount);  // GRB ordering is assumed
  pms.init();
}

int lastAQI = 0;
int lastAqiUpdate = 0;
void loop2() {
  
  return;
  fDisplay.fillScreen(0);
  FastLED.show();
  fDisplay.fillScreen(Color(255,0,0));
  FastLED.show();
  
  return;
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
}
