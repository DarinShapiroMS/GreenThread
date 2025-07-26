#include "MatterStandardClusters.h"

// Silicon Labs Matter library for Arduino Nano Matter
// #include <Matter.h>  // Temporarily commented out for compilation test
// #include <MatterHumidity.h>  // Temporarily commented out for compilation test

void MatterStandardClusters::begin() {
    Serial.println(F("[Matter] Initializing standard clusters..."));
    
    // Initialize Basic Information (this provides device name during commissioning)
    Serial.print(F("[Matter] Device: "));
    Serial.print(basicInfoAttrs.vendorName);
    Serial.print(F(" "));
    Serial.println(basicInfoAttrs.productName);
    
    Serial.print(F("[Matter] Vendor ID: 0x"));
    // Serial.print(basicInfoAttrs.vendorId, HEX);  // Temporarily commented out - may cause String issue
    Serial.print(F(", Product ID: 0x"));
    // Serial.println(basicInfoAttrs.productId, HEX);  // Temporarily commented out - may cause String issue
    
    // Set default humidity values
    humidityAttrs.measuredValue = 0;
    humidityAttrs.minMeasuredValue = 0;
    humidityAttrs.maxMeasuredValue = 10000;  // 100.00%
    humidityAttrs.tolerance = 100;  // 1% tolerance
    
    // Set default power values
    powerAttrs.status = 1;  // Active
    powerAttrs.order = 1;
    powerAttrs.description = 0;
    powerAttrs.batVoltage = 3300;
    powerAttrs.batPercentRemaining = 200;  // 100% (0-200 scale)
    powerAttrs.batChargeLevel = 0;  // OK
    
    // TODO: Register clusters with Matter SDK when available
    // Initialize the Matter humidity sensor
    // matterHumidity.begin();
    
    Serial.println(F("[Matter] Standard clusters ready"));
}

void MatterStandardClusters::updateMoisture(float moisturePercent) {
    // Convert 0-100% to Matter's 0-10000 scale (0.01% resolution)
    humidityAttrs.measuredValue = (uint16_t)(moisturePercent * 100);
    
    // Clamp to valid range
    if (humidityAttrs.measuredValue > 10000) {
        humidityAttrs.measuredValue = 10000;
    }
    
    // Report updated value to Matter network using the actual API
    // matterHumidity.set_percent(moisturePercent);
    
    Serial.print("Standard cluster - Humidity updated: ");
    Serial.print(moisturePercent);
    Serial.println("%");
}

void MatterStandardClusters::updateBattery(float voltage, uint8_t percent) {
    // Update voltage in millivolts
    powerAttrs.batVoltage = (uint32_t)(voltage * 1000);
    
    // Convert percentage to Matter's 0-200 scale (0.5% resolution)
    powerAttrs.batPercentRemaining = percent * 2;
    if (powerAttrs.batPercentRemaining > 200) {
        powerAttrs.batPercentRemaining = 200;
    }
    
    // Set charge level based on percentage
    if (percent < 15) {
        powerAttrs.batChargeLevel = 2;  // Critical
    } else if (percent < 25) {
        powerAttrs.batChargeLevel = 1;  // Warning
    } else {
        powerAttrs.batChargeLevel = 0;  // OK
    }
    
    // TODO: Report battery information when Matter battery API is available
    // Need to research MatterBattery or similar class
    
    Serial.print("Standard cluster - Battery updated: ");
    Serial.print(percent);
    Serial.print("% (");
    Serial.print(voltage);
    Serial.println("V)");
}

void MatterStandardClusters::setDeviceInfo(const char* serialNumber, const char* location) {
    // Update serial number if provided (make each device unique)
    if (serialNumber) {
        strncpy(basicInfoAttrs.serialNumber, serialNumber, sizeof(basicInfoAttrs.serialNumber) - 1);
        basicInfoAttrs.serialNumber[sizeof(basicInfoAttrs.serialNumber) - 1] = '\0';
        
        Serial.print(F("[Matter] Serial number set: "));
        Serial.println(basicInfoAttrs.serialNumber);
    }
    
    // Update location if provided
    if (location) {
        strncpy(basicInfoAttrs.location, location, sizeof(basicInfoAttrs.location) - 1);
        basicInfoAttrs.location[sizeof(basicInfoAttrs.location) - 1] = '\0';
        
        Serial.print(F("[Matter] Location set: "));
        Serial.println(basicInfoAttrs.location);
    }
    
    // TODO: Report basic information changes when Matter basic info API is available
    // The Matter library may handle device info automatically through configuration
}
