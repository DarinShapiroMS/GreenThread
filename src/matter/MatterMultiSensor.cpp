#include "MatterMultiSensor.h"
#include "../hardware/PowerManager.h"

void MatterMultiSensor::begin() {
    // Initialize Matter first
    matter.begin();
    
    // Initialize all endpoints
    soilMoisture.begin();
    batteryLevel.begin(); 
    sleepIntervalNormal.begin();
    sleepIntervalExt.begin();
    powerManagementControl.begin();
    
    setupDeviceNames();
    updateMatterCallbacks();
}

bool MatterMultiSensor::isOnline() {
    return matter.isDeviceCommissioned() && matter.isDeviceThreadConnected();
}

void MatterMultiSensor::setSoilMoisture(float percent) {
    soilMoisture.set_measured_value(percent);
}

void MatterMultiSensor::setBatteryVoltage(float voltage) {
    // Convert voltage (3.0-4.2V) to percentage for display
    float percent = constrain((voltage - 3.0) / 1.2 * 100.0, 0, 100);
    setBatteryPercent(percent);
}

void MatterMultiSensor::setBatteryPercent(float percent) {
    // Use temperature sensor to report battery percentage
    // Scale: 0-100% becomes 0-100°C for easy interpretation
    batteryLevel.set_measured_value_celsius(percent);
}

void MatterMultiSensor::setPowerManager(PowerManager* manager) {
    powerManager = manager;
    
    // Sync current values
    if (powerManager) {
        // Convert milliseconds to "temperature" scale for Matter
        float normalTemp = convertSecondsToTempScale(powerManager->getNormalSleepInterval() / 1000);
        float extTemp = convertSecondsToTempScale(powerManager->getExtendedSleepInterval() / 1000);
        
        sleepIntervalNormal.set_measured_value_celsius(normalTemp);
        sleepIntervalExt.set_measured_value_celsius(extTemp);
        
        // Set power management state
        PowerConfiguration config = powerManager->getConfiguration();
        powerManagementControl.set_state(config.enablePowerManagement);
    }
}

bool MatterMultiSensor::getPowerManagementEnabled() {
    return powerManagementControl.get_state();
}

void MatterMultiSensor::setPowerManagementEnabled(bool enabled) {
    powerManagementControl.set_state(enabled);
    if (powerManager) {
        PowerConfiguration config = powerManager->getConfiguration();
        config.enablePowerManagement = enabled;
        powerManager->setConfiguration(config);
    }
}

uint32_t MatterMultiSensor::getNormalSleepInterval() {
    float tempValue = sleepIntervalNormal.get_measured_value();
    return convertTempScaleToSeconds(tempValue);
}

void MatterMultiSensor::setNormalSleepInterval(uint32_t intervalSec) {
    float tempValue = convertSecondsToTempScale(intervalSec);
    sleepIntervalNormal.set_measured_value_celsius(tempValue);
    
    if (powerManager) {
        powerManager->setNormalSleepInterval(intervalSec * 1000); // Convert to ms
    }
}

uint32_t MatterMultiSensor::getExtendedSleepInterval() {
    float tempValue = sleepIntervalExt.get_measured_value();
    return convertTempScaleToSeconds(tempValue);
}

void MatterMultiSensor::setExtendedSleepInterval(uint32_t intervalSec) {
    float tempValue = convertSecondsToTempScale(intervalSec);
    sleepIntervalExt.set_measured_value_celsius(tempValue);
    
    if (powerManager) {
        powerManager->setExtendedSleepInterval(intervalSec * 1000); // Convert to ms
    }
}

uint8_t MatterMultiSensor::getCurrentPowerState() {
    return powerManager ? static_cast<uint8_t>(powerManager->getCurrentState()) : 0;
}

float MatterMultiSensor::getCurrentSleepInterval() {
    return powerManager ? powerManager->getCurrentSleepInterval() / 1000.0 : 0; // Convert to seconds
}

void MatterMultiSensor::setupDeviceNames() {
    // Set meaningful names for Home Assistant discovery
    soilMoisture.set_device_name("Soil Moisture Sensor");
    soilMoisture.set_product_name("Soil Moisture %");
    
    batteryLevel.set_device_name("Battery Level");  
    batteryLevel.set_product_name("Battery %");
    
    sleepIntervalNormal.set_device_name("Normal Sleep Interval");
    sleepIntervalNormal.set_product_name("Normal Sleep (sec)");
    
    sleepIntervalExt.set_device_name("Extended Sleep Interval");
    sleepIntervalExt.set_product_name("Extended Sleep (sec)");
    
    powerManagementControl.set_device_name("Power Management");
    powerManagementControl.set_product_name("Enable Power Mgmt");
}

void MatterMultiSensor::updateMatterCallbacks() {
    // Set callbacks for when Home Assistant changes values
    powerManagementControl.set_device_change_callback([this]() {
        this->onPowerManagementToggled();
    });
    
    // Note: Matter library callbacks are limited, but we can poll for changes
}

void MatterMultiSensor::onNormalSleepIntervalChanged() {
    // Called when Home Assistant changes the normal sleep interval
    uint32_t newInterval = getNormalSleepInterval();
    if (powerManager) {
        powerManager->setNormalSleepInterval(newInterval * 1000);
    }
}

void MatterMultiSensor::onExtendedSleepIntervalChanged() {
    // Called when Home Assistant changes the extended sleep interval  
    uint32_t newInterval = getExtendedSleepInterval();
    if (powerManager) {
        powerManager->setExtendedSleepInterval(newInterval * 1000);
    }
}

void MatterMultiSensor::onPowerManagementToggled() {
    // Called when Home Assistant toggles power management
    bool enabled = getPowerManagementEnabled();
    if (powerManager) {
        PowerConfiguration config = powerManager->getConfiguration();
        config.enablePowerManagement = enabled;
        powerManager->setConfiguration(config);
    }
}

float MatterMultiSensor::convertSecondsToTempScale(uint32_t seconds) {
    // Convert seconds to a temperature scale for Matter compatibility
    // Scale: 5-900 seconds becomes 5-90°C (divide by 10 for readability)
    return constrain(seconds / 10.0, 0.5, 90.0);
}

uint32_t MatterMultiSensor::convertTempScaleToSeconds(float tempValue) {
    // Convert temperature scale back to seconds
    // Scale: 5-90°C becomes 50-900 seconds (multiply by 10)
    return constrain(tempValue * 10, 5, 900);
}
