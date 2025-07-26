#include "GreenThreadSoilSensorCluster.h"
#include "../hardware/SensorManager.h"
#include "../hardware/BatteryMonitor.h"
#include "../hardware/CalibrationManager.h"
#include "../hardware/PowerManager.h"
#include "../config/Config.h"

GreenThreadSoilSensorCluster::GreenThreadSoilSensorCluster(SensorManager* sm, BatteryMonitor* bm, 
                                                          CalibrationManager* cm, PowerManager* pm)
    : sensorManager(sm), batteryMonitor(bm), calibrationManager(cm), powerManager(pm) {
    // Initialize attribute values to defaults
    memset(&attributes, 0, sizeof(attributes));
    
    // Set sensible defaults
    attributes.calibrationDryValue = 1023;
    attributes.calibrationWetValue = 0;
    attributes.moistureThresholdLow = 20;
    attributes.moistureThresholdHigh = 80;
    attributes.sleepIntervalSeconds = 300;  // 5 minutes
    attributes.measurementIntervalSeconds = 60;  // 1 minute
    attributes.batteryVoltageMv = 3300;
    attributes.batteryLevelPercent = 100;
    attributes.powerState = POWER_ACTIVE;
    attributes.sensorStatus = SENSOR_OK;
    attributes.firmwareVersion = 0x010000;  // v1.0.0
}

bool GreenThreadSoilSensorCluster::begin() {
    Serial.println("=== Green Thread Soil Sensor Cluster Initialization ===");
    
    if (!sensorManager || !batteryMonitor || !calibrationManager || !powerManager) {
        Serial.println("ERROR: Missing hardware abstraction references");
        attributes.sensorStatus = SENSOR_ERROR;
        attributes.errorCode = 1;  // Missing dependencies
        return false;
    }
    
    // Initialize with current hardware state
    updateSensorReadings();
    updateBatteryStatus();
    updateCalibrationStatus();
    updatePowerStatus();
    updateSystemStatus();
    
    clusterInitialized = true;
    attributes.lastMeasurementTime = millis() / 1000;
    
    Serial.print("Cluster ID: 0x");
    Serial.println(FULL_CLUSTER_ID, HEX);
    Serial.print("Vendor ID: 0x");
    Serial.println(VENDOR_ID, HEX);
    Serial.println("Green Thread Soil Sensor Cluster initialized successfully!");
    
    printClusterInfo();
    
    return true;
}

bool GreenThreadSoilSensorCluster::isOnline() const {
    // For now, return true during development
    // TODO: Implement real Matter/Thread connection checking when Silicon Labs APIs are integrated
    return true;
}

void GreenThreadSoilSensorCluster::update(bool forceUpdate) {
    if (!clusterInitialized) {
        return;
    }
    
    uint32_t currentTime = millis();
    
    // Update at regular intervals or when forced
    if (forceUpdate || (currentTime - lastAttributeUpdate) >= 5000) {  // Update every 5 seconds
        updateSensorReadings();
        updateBatteryStatus();
        updateCalibrationStatus();
        updatePowerStatus();
        updateSystemStatus();
        
        // Check for threshold crossings and send events
        checkThresholdCrossings();
        
        lastAttributeUpdate = currentTime;
        attributes.measurementCount++;
        attributes.lastMeasurementTime = currentTime / 1000;
    }
}

// === Internal Update Methods ===

void GreenThreadSoilSensorCluster::updateSensorReadings() {
    if (!sensorManager) return;
    
    // Get raw sensor value
    float moistureFloat = sensorManager->readMoisture();
    uint16_t rawMoisture = (uint16_t)(moistureFloat * 1023.0 / 100.0);  // Convert back to raw for compatibility
    attributes.soilMoistureRaw = rawMoisture;
    
    // Store the processed percentage
    attributes.soilMoisturePercent = (uint8_t)moistureFloat;
    
    // Set calibration status based on calibration manager
    if (calibrationManager && calibrationManager->isCalibrationValid()) {
        attributes.calibrationStatus = CALIBRATION_FULLY_CALIBRATED;
        // Get calibration values
        int dryVal, wetVal;
        calibrationManager->getMoistureCalibration(dryVal, wetVal);
        attributes.calibrationDryValue = (uint16_t)dryVal;
        attributes.calibrationWetValue = (uint16_t)wetVal;
    } else {
        attributes.calibrationStatus = CALIBRATION_NOT_CALIBRATED;
    }
    
    // For now, set soil temperature to 0 (no dedicated soil temp sensor)
    attributes.soilTemperatureCelsius = 0;
    
    // For now, set humidity to 0 (no dedicated humidity sensor) 
    attributes.humidityPercent = 0;
    
    // Update sensor status - assume healthy if we can read values
    attributes.sensorStatus = SENSOR_OK;
    attributes.errorCode = 0;
}

void GreenThreadSoilSensorCluster::updateBatteryStatus() {
    if (!batteryMonitor) return;
    
    uint8_t oldBatteryLevel = attributes.batteryLevelPercent;
    
    // Read voltage and convert to millivolts
    float voltage = batteryMonitor->readVoltage();
    attributes.batteryVoltageMv = (uint16_t)(voltage * 1000.0);
    
    // Calculate battery percentage (assuming 3.3V nominal, 2.7V low)
    attributes.batteryLevelPercent = (uint8_t)constrain(
        ((voltage - 2.7) / (3.3 - 2.7)) * 100.0, 
        0, 100
    );
    
    // Send event if battery level changed significantly
    if (abs((int)attributes.batteryLevelPercent - (int)oldBatteryLevel) >= 5) {
        sendBatteryLevelChangedEvent(attributes.batteryLevelPercent);
    }
}

void GreenThreadSoilSensorCluster::updateCalibrationStatus() {
    if (!calibrationManager) return;
    
    if (calibrationManager->isCalibrationValid()) {
        attributes.calibrationStatus = CALIBRATION_FULLY_CALIBRATED;
        int dryVal, wetVal;
        calibrationManager->getMoistureCalibration(dryVal, wetVal);
        attributes.calibrationDryValue = (uint16_t)dryVal;
        attributes.calibrationWetValue = (uint16_t)wetVal;
        attributes.calibrationPointsCount = 2;  // Dry + Wet
    } else if (calibrationManager->isCalibrating()) {
        attributes.calibrationStatus = CALIBRATION_IN_PROGRESS;
        attributes.calibrationPointsCount = 0;
    } else {
        attributes.calibrationStatus = CALIBRATION_NOT_CALIBRATED;
        attributes.calibrationPointsCount = 0;
    }
}

void GreenThreadSoilSensorCluster::updatePowerStatus() {
    if (!powerManager) return;
    
    uint8_t oldPowerState = attributes.powerState;
    
    // Map PowerManager states to our custom cluster states
    PowerState currentPowerState = powerManager->getCurrentState();
    switch (currentPowerState) {
        case PowerState::Critical:
            attributes.powerState = POWER_CRITICAL_BATTERY;
            break;
        case PowerState::LowPower:
        case PowerState::Extended:
            attributes.powerState = POWER_SLEEP;
            break;
        case PowerState::Normal:
        case PowerState::UsbPowered:
        case PowerState::Booting:
        default:
            attributes.powerState = POWER_ACTIVE;
            break;
    }
    
    // Send event if power state changed
    if (attributes.powerState != oldPowerState) {
        sendPowerStateChangedEvent(attributes.powerState);
    }
    
    // Get current sleep interval
    attributes.sleepIntervalSeconds = powerManager->getCurrentSleepInterval() / 1000;  // Convert ms to seconds
}

void GreenThreadSoilSensorCluster::updateSystemStatus() {
    // Update system status based on overall health
    if (attributes.sensorStatus == SENSOR_ERROR || 
        attributes.powerState == POWER_CRITICAL_BATTERY) {
        // System has issues
        return;
    }
    
    // All good
    attributes.sensorStatus = SENSOR_OK;
    attributes.errorCode = 0;
}

// === Command Handlers ===

bool GreenThreadSoilSensorCluster::handleStartDryCalibration() {
    Serial.println("Command: Start Dry Calibration");
    
    if (!calibrationManager) {
        Serial.println("ERROR: CalibrationManager not available");
        return false;
    }
    
    attributes.calibrationStatus = CALIBRATION_IN_PROGRESS;
    
    // Start calibration process and then calibrate dry
    calibrationManager->startCalibration();
    calibrationManager->calibrateDry();
    
    Serial.println("Dry calibration started successfully");
    
    return true;
}

bool GreenThreadSoilSensorCluster::handleStartWetCalibration() {
    Serial.println("Command: Start Wet Calibration");
    
    if (!calibrationManager) {
        Serial.println("ERROR: CalibrationManager not available");
        return false;
    }
    
    attributes.calibrationStatus = CALIBRATION_IN_PROGRESS;
    
    // Start calibration process and then calibrate wet
    calibrationManager->startCalibration();
    calibrationManager->calibrateWet();
    
    Serial.println("Wet calibration started successfully");
    
    return true;
}

bool GreenThreadSoilSensorCluster::handleResetCalibration() {
    Serial.println("Command: Reset Calibration");
    
    if (!calibrationManager) {
        Serial.println("ERROR: CalibrationManager not available");
        return false;
    }
    
    calibrationManager->resetToDefaults();
    
    attributes.calibrationStatus = CALIBRATION_NOT_CALIBRATED;
    attributes.calibrationDryValue = 1023;
    attributes.calibrationWetValue = 0;
    attributes.calibrationPointsCount = 0;
    
    Serial.println("Calibration reset successfully");
    
    return true;
}

bool GreenThreadSoilSensorCluster::handleForceMeasurement() {
    Serial.println("Command: Force Measurement");
    
    // Force an immediate sensor reading
    update(true);
    
    Serial.print("Forced measurement - Soil Moisture: ");
    Serial.print(attributes.soilMoisturePercent);
    Serial.print("% (Raw: ");
    Serial.print(attributes.soilMoistureRaw);
    Serial.println(")");
    
    return true;
}

bool GreenThreadSoilSensorCluster::handleSetThresholds(uint8_t lowThreshold, uint8_t highThreshold) {
    Serial.print("Command: Set Thresholds - Low: ");
    Serial.print(lowThreshold);
    Serial.print("%, High: ");
    Serial.print(highThreshold);
    Serial.println("%");
    
    if (!validateThresholds(lowThreshold, highThreshold)) {
        Serial.println("ERROR: Invalid threshold values");
        return false;
    }
    
    attributes.moistureThresholdLow = lowThreshold;
    attributes.moistureThresholdHigh = highThreshold;
    
    Serial.println("Thresholds updated successfully");
    return true;
}

bool GreenThreadSoilSensorCluster::handleSetMeasurementInterval(uint16_t intervalSeconds) {
    Serial.print("Command: Set Measurement Interval - ");
    Serial.print(intervalSeconds);
    Serial.println(" seconds");
    
    if (!validateMeasurementInterval(intervalSeconds)) {
        Serial.println("ERROR: Invalid measurement interval");
        return false;
    }
    
    attributes.measurementIntervalSeconds = intervalSeconds;
    
    Serial.println("Measurement interval updated successfully");
    return true;
}

bool GreenThreadSoilSensorCluster::handleGetStatus() {
    Serial.println("Command: Get Status");
    printAttributeValues();
    return true;
}

bool GreenThreadSoilSensorCluster::handleEnterSleepMode() {
    Serial.println("Command: Enter Sleep Mode");
    
    if (!powerManager) {
        Serial.println("ERROR: PowerManager not available");
        return false;
    }
    
    // PowerManager enterSleepMode() returns void, so just call it
    powerManager->enterSleepMode();
    attributes.powerState = POWER_SLEEP;
    Serial.println("Entering sleep mode");
    
    return true;
}

// === Event Generation ===

void GreenThreadSoilSensorCluster::checkThresholdCrossings() {
    uint8_t currentMoisture = attributes.soilMoisturePercent;
    
    // Check for threshold crossings
    if (lastMoistureLevel != 0) {  // Skip first reading
        if (lastMoistureLevel <= attributes.moistureThresholdLow && 
            currentMoisture > attributes.moistureThresholdLow) {
            // Crossed low threshold upward
            sendMoistureThresholdCrossedEvent(currentMoisture, 0);  // 0 = low threshold
        } else if (lastMoistureLevel >= attributes.moistureThresholdHigh && 
                   currentMoisture < attributes.moistureThresholdHigh) {
            // Crossed high threshold downward
            sendMoistureThresholdCrossedEvent(currentMoisture, 1);  // 1 = high threshold
        }
    }
    
    lastMoistureLevel = currentMoisture;
}

void GreenThreadSoilSensorCluster::sendMoistureThresholdCrossedEvent(uint8_t newLevel, uint8_t thresholdType) {
    Serial.print("Event: Moisture threshold crossed - Level: ");
    Serial.print(newLevel);
    Serial.print("%, Threshold: ");
    Serial.println(thresholdType == 0 ? "LOW" : "HIGH");
    
    uint8_t eventData[2] = { newLevel, thresholdType };
    sendEvent(EVENT_MOISTURE_THRESHOLD_CROSSED, eventData, sizeof(eventData));
}

void GreenThreadSoilSensorCluster::sendBatteryLevelChangedEvent(uint8_t newLevel) {
    Serial.print("Event: Battery level changed - ");
    Serial.print(newLevel);
    Serial.println("%");
    
    uint8_t eventData[1] = { newLevel };
    sendEvent(EVENT_BATTERY_LEVEL_CHANGED, eventData, sizeof(eventData));
}

void GreenThreadSoilSensorCluster::sendPowerStateChangedEvent(uint8_t newState) {
    Serial.print("Event: Power state changed - ");
    Serial.println(newState);
    
    uint8_t eventData[1] = { newState };
    sendEvent(EVENT_POWER_STATE_CHANGED, eventData, sizeof(eventData));
}

void GreenThreadSoilSensorCluster::sendCalibrationCompletedEvent(uint8_t status) {
    Serial.print("Event: Calibration completed - Status: ");
    Serial.println(status);
    
    uint8_t eventData[1] = { status };
    sendEvent(EVENT_CALIBRATION_COMPLETED, eventData, sizeof(eventData));
}

void GreenThreadSoilSensorCluster::sendSystemErrorEvent(uint8_t errorCode) {
    Serial.print("Event: System error - Code: ");
    Serial.println(errorCode);
    
    uint8_t eventData[1] = { errorCode };
    sendEvent(EVENT_SYSTEM_ERROR, eventData, sizeof(eventData));
}

void GreenThreadSoilSensorCluster::sendEvent(uint8_t eventId, const uint8_t* eventData, size_t dataLength) {
    // TODO: Implement actual Matter event sending when Matter library is integrated
    // For now, just log the event
    Serial.print("Matter Event - ID: 0x");
    Serial.print(eventId, HEX);
    Serial.print(", Data: ");
    for (size_t i = 0; i < dataLength; i++) {
        Serial.print("0x");
        Serial.print(eventData[i], HEX);
        if (i < dataLength - 1) Serial.print(" ");
    }
    Serial.println();
}

// === Validation Helpers ===

bool GreenThreadSoilSensorCluster::validateThresholds(uint8_t low, uint8_t high) const {
    return (low < high) && (low >= 0) && (high <= 100);
}

bool GreenThreadSoilSensorCluster::validateMeasurementInterval(uint16_t interval) const {
    return (interval >= 10) && (interval <= 3600);  // 10 seconds to 1 hour
}

// === Debug and Diagnostics ===

void GreenThreadSoilSensorCluster::printClusterInfo() const {
    // Optimized cluster info with single sprintf calls
    char buffer[80];
    
    Serial.println("=== Green Thread Soil Sensor Cluster Info ===");
    
    sprintf(buffer, "Cluster ID: 0x%08X, Vendor ID: 0x%04X", FULL_CLUSTER_ID, VENDOR_ID);
    Serial.println(buffer);
    
    sprintf(buffer, "Init: %s, Cal: %s, Health: %s, BatLow: %s", 
            clusterInitialized ? "YES" : "NO",
            isCalibrated() ? "YES" : "NO",
            isSensorHealthy() ? "OK" : "ERR",
            isBatteryLow() ? "YES" : "NO");
    Serial.println(buffer);
    
    Serial.println("============================================");
}

void GreenThreadSoilSensorCluster::printAttributeValues() const {
    // Use fewer Serial.print calls for faster output
    Serial.println("=== Current Attribute Values ===");
    
    // Format multiple values into single strings to reduce serial overhead
    char buffer[128];
    
    // Soil measurements
    sprintf(buffer, "Soil: %d%% (Raw: %d), Temp: %.1f°C", 
            attributes.soilMoisturePercent, 
            attributes.soilMoistureRaw,
            attributes.soilTemperatureCelsius / 100.0);
    Serial.println(buffer);
    
    // Battery status - show power state instead of USB status
    sprintf(buffer, "Battery: %d%% (%dmV), Power: %d", 
            attributes.batteryLevelPercent,
            attributes.batteryVoltageMv,
            attributes.powerState);
    Serial.println(buffer);
    
    // System status
    sprintf(buffer, "Status: %d, Count: %d, Last: %ds ago", 
            attributes.sensorStatus,
            attributes.measurementCount,
            (millis() / 1000) - attributes.lastMeasurementTime);
    Serial.println(buffer);
    
    // Configuration
    sprintf(buffer, "Thresholds: L=%d%%, H=%d%%, Cal: %d", 
            attributes.moistureThresholdLow,
            attributes.moistureThresholdHigh,
            attributes.calibrationStatus);
    Serial.println(buffer);
    
    Serial.println("==============================");
}
