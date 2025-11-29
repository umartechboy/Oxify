#include <Arduino.h>
#include <FastLED.h>
#include <PMserial.h> // Arduino library for PM sensors with serial interface
#include <DHT.h>
#include "..\..\FW\Stand\src\aqi.h"
#include "..\..\FW\Stand\src\aqi.cpp"

#define DHTPin 22
#define PMTX 18
#define PMRX 19
#define LEDPin 5
#define LEDStrips 2
#define LedCount (300 * LEDStrips)
#define aqiAveraging 50

// Define the array of leds
CRGB leds[LedCount];

SerialPM pms(PMS5003, PMRX, PMTX); // PMSx003, RX, TX
DHT dht(DHTPin, DHT11);

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, 5>(leds, LedCount);  // GRB ordering is assumed
  pms.init();
}

int lastAQI = 0;
int lastAqiUpdate = 0;
void loop() {
  for (int i = 0; i < LedCount; i++){
    leds[i] = CRGB::Red;
    FastLED.show();
  }
  for (int i = 0; i < LedCount; i++){
    leds[i] = CRGB::Black;
    FastLED.show();
  }

  
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
