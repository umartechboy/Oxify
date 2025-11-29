#include "aqi.h"
#include <Arduino.h>

// International AQI Breakpoints (based on WHO guidelines)
const float PM25_BREAKPOINTS[] = {0.0, 15.0, 30.0, 55.0, 110.0, 250.0};
const int PM25_AQI[] = {0, 50, 100, 150, 200, 300};
const int PM25_BREAKPOINTS_SIZE = 6;

const float PM10_BREAKPOINTS[] = {0.0, 45.0, 90.0, 180.0, 270.0, 400.0};
const int PM10_AQI[] = {0, 50, 100, 150, 200, 300};
const int PM10_BREAKPOINTS_SIZE = 6;

// AQI calculation function
int calculateAQI(float concentration, const float* breakpoints, const int* aqiValues, int size) {
  if (concentration < 0) concentration = 0;
  
  for (int i = 0; i < size - 1; i++) {
    if (concentration >= breakpoints[i] && concentration <= breakpoints[i + 1]) {
      return (int)((aqiValues[i + 1] - aqiValues[i]) / (breakpoints[i + 1] - breakpoints[i]) * (concentration - breakpoints[i]) + aqiValues[i]);
    }
  }
  return concentration > breakpoints[size - 1] ? 500 : 0;
}

// Calculate individual pollutant AQI
int calculatePM25_AQI(float pm25) {
  return calculateAQI(pm25, PM25_BREAKPOINTS, PM25_AQI, PM25_BREAKPOINTS_SIZE);
}

int calculatePM10_AQI(float pm10) {
  return calculateAQI(pm10, PM10_BREAKPOINTS, PM10_AQI, PM10_BREAKPOINTS_SIZE);
}

// Get final AQI (returns highest value from enabled pollutants)
int getFinalAQI(float pm25, float pm10, float temperature, float humidity) {
  int finalAQI = 0;
  
  #if AQI_USE_PM25
    int pm25_aqi = calculatePM25_AQI(pm25);
    finalAQI = max(finalAQI, pm25_aqi);
  #endif
  
  #if AQI_USE_PM10
    int pm10_aqi = calculatePM10_AQI(pm10);
    finalAQI = max(finalAQI, pm10_aqi);
  #endif
  
  return finalAQI;
}

// Get AQI category description
String getAQICategory(int aqi) {
  if (aqi <= 50) return "Good";
  else if (aqi <= 100) return "Moderate";
  else if (aqi <= 150) return "Unhealthy for Sensitive";
  else if (aqi <= 200) return "Unhealthy";
  else if (aqi <= 300) return "Very Unhealthy";
  else return "Hazardous";
}// Get AQI color as RGB values (0-255) with linear interpolation
// Blue (50) -> Green (100) -> Red (300)
AQIColor getAQIColorRGB(int aqi) {
  AQIColor color;
  
  if (aqi <= 50) {
    // Pure Blue for excellent air quality
    color = {0, 0, 255};
  } else if (aqi <= 100) {
    // Blue to Green transition (50-100)
    float ratio = (aqi - 50) / 50.0;
    color = {
      (uint8_t)(0),                    // Red stays 0
      (uint8_t)(255 * ratio),          // Green increases
      (uint8_t)(255 * (1.0 - ratio))   // Blue decreases
    };
  } else if (aqi <= 300) {
    // Green to Red transition (100-300)
    float ratio = (aqi - 100) / 200.0;
    color = {
      (uint8_t)(255 * ratio),          // Red increases
      (uint8_t)(255 * (1.0 - ratio)),  // Green decreases
      (uint8_t)(0)                     // Blue stays 0
    };
  } else {
    // Pure Red for hazardous (300+)
    color = {255, 0, 0};
  }
  
  return color;
}

// Print comprehensive debug information
void printAQIDebugInfo(float pm25, float pm10, float temperature, float humidity) {
  Serial.println(F("=== AIR QUALITY MONITOR ==="));
  
  // PM Data
  Serial.print(F("PM2.5: "));
  Serial.print(pm25);
  Serial.print(F(" μg/m³, PM10: "));
  Serial.print(pm10);
  Serial.println(F(" μg/m³"));
  
  // Environmental Data (if available)
  if (!isnan(temperature)) {
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.print(F("°C"));
    
    if (!isnan(humidity)) {
      Serial.print(F(", Humidity: "));
      Serial.print(humidity);
      Serial.print(F("%"));
    }
    Serial.println();
  }
  
  // AQI Calculations
  int pm25_aqi = calculatePM25_AQI(pm25);
  int pm10_aqi = calculatePM10_AQI(pm10);
  int final_aqi = getFinalAQI(pm25, pm10, temperature, humidity);
  AQIColor color = getAQIColorRGB(final_aqi);
  
  Serial.println(F("--- AQI Breakdown ---"));
  #if AQI_USE_PM25
    Serial.print(F("PM2.5 AQI: "));
    Serial.println(pm25_aqi);
  #endif
  
  #if AQI_USE_PM10
    Serial.print(F("PM10 AQI: "));
    Serial.println(pm10_aqi);
  #endif
  
  Serial.print(F("FINAL AQI: "));
  Serial.print(final_aqi);
  Serial.print(F(" - "));
  Serial.println(getAQICategory(final_aqi));
  
  Serial.print(F("AQI Color RGB: ("));
  Serial.print(color.r);
  Serial.print(F(", "));
  Serial.print(color.g);
  Serial.print(F(", "));
  Serial.print(color.b);
  Serial.println(F(")"));
  
  Serial.println(F("============================="));
  Serial.println();
}