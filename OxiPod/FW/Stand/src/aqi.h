#ifndef AQI_H
#define AQI_H

#include <Arduino.h>

// AQI Calculation Configuration
#define AQI_USE_PM25 true      // Use PM2.5 for AQI calculation
#define AQI_USE_PM10 true      // Use PM10 for AQI calculation  
#define AQI_USE_TEMP false     // Temperature doesn't directly affect AQI
#define AQI_USE_HUMIDITY false // Humidity doesn't directly affect AQI

// AQI Color Structure
struct AQIColor {
    int r;
    int g;
    int b;
};

// Function declarations
int calculateAQI(float concentration, const float* breakpoints, const int* aqiValues, int size);
int calculatePM25_AQI(float pm25);
int calculatePM10_AQI(float pm10);
int getFinalAQI(float pm25, float pm10, float temperature = NAN, float humidity = NAN);
String getAQICategory(int aqi);
AQIColor getAQIColorRGB(int aqi);
void printAQIDebugInfo(float pm25, float pm10, float temperature = NAN, float humidity = NAN);

#endif