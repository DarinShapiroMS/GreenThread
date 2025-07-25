#include "MatterMultiSensor.h"
#include "../hardware/PowerManager.h"
#include "../hardware/CalibrationManager.h"
#include "../ui/DisplayFactory.h"

void MatterMultiSensor::begin() {
    // Initialize Matter first
    matter.begin();
    
    // Initialize all endpoints
    soilMoisture.begin();
    batteryLevel.begin(); 
    batteryVoltage.begin();
    
    // Sleep interval controls
    sleepIntervalNormal.begin();
    sleepIntervalExt.begin();
    sleepIntervalLow.begin();
    sleepIntervalUsb.begin();
    
    // Status reporting
    powerStateReport.begin();
    displayTypeReport.begin();
    currentSleepReport.begin();
    
    // Calibration values (read-only)
    calibDryValue.begin();
    calibWetValue.begin();
    calibBatteryDiv.begin();
    
    // Control switches
    powerManagementControl.begin();
    usbConnectionStatus.begin();
    calibrationModeControl.begin();
    
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
    // Store voltage directly (3.0-4.2V range)
    batteryVoltage.set_measured_value_celsius(voltage);
    
    // Convert voltage to percentage for display
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
        float lowTemp = convertSecondsToTempScale(powerManager->getLowPowerSleepInterval() / 1000);
        float usbTemp = convertSecondsToTempScale(powerManager->getUsbSleepInterval() / 1000);
        
        sleepIntervalNormal.set_measured_value_celsius(normalTemp);
        sleepIntervalExt.set_measured_value_celsius(extTemp);
        sleepIntervalLow.set_measured_value_celsius(lowTemp);
        sleepIntervalUsb.set_measured_value_celsius(usbTemp);
        
        // Set power management state
        PowerConfiguration config = powerManager->getConfiguration();
        powerManagementControl.set_state(config.enablePowerManagement);
        
        // Update status reports
        powerStateReport.set_measured_value_celsius(static_cast<float>(powerManager->getCurrentState()));
        currentSleepReport.set_measured_value_celsius(powerManager->getCurrentSleepInterval() / 1000.0);
    }
}

void MatterMultiSensor::setCalibrationManager(CalibrationManager* manager) {
    calibrationManager = manager;
    updateCalibrationValues();
    
    // Also update display type since we have access to DisplayFactory
    DisplayFactory::DisplayType type = DisplayFactory::detectBestDisplay();
    float displayTypeValue = static_cast<float>(type);
    displayTypeReport.set_measured_value_celsius(displayTypeValue);
    
    // Update USB connection status
    bool usbConnected = DisplayFactory::isUsbConnected();
    usbConnectionStatus.set_state(usbConnected);
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

uint32_t MatterMultiSensor::getLowPowerSleepInterval() {
    float tempValue = sleepIntervalLow.get_measured_value();
    return convertTempScaleToSeconds(tempValue);
}

void MatterMultiSensor::setLowPowerSleepInterval(uint32_t intervalSec) {
    float tempValue = convertSecondsToTempScale(intervalSec);
    sleepIntervalLow.set_measured_value_celsius(tempValue);
    
    if (powerManager) {
        powerManager->setLowPowerSleepInterval(intervalSec * 1000); // Convert to ms
    }
}

uint32_t MatterMultiSensor::getUsbSleepInterval() {
    float tempValue = sleepIntervalUsb.get_measured_value();
    return convertTempScaleToSeconds(tempValue);
}

void MatterMultiSensor::setUsbSleepInterval(uint32_t intervalSec) {
    float tempValue = convertSecondsToTempScale(intervalSec);
    sleepIntervalUsb.set_measured_value_celsius(tempValue);
    
    if (powerManager) {
        powerManager->setUsbSleepInterval(intervalSec * 1000); // Convert to ms
    }
}

void MatterMultiSensor::updateCalibrationValues() {
    if (calibrationManager) {
        int dryValue, wetValue;
        calibrationManager->getMoistureCalibration(dryValue, wetValue);
        float batteryDiv = calibrationManager->getBatteryDivider();
        
        // Scale ADC values (0-1023) to temperature range (0-102.3°C)
        calibDryValue.set_measured_value_celsius(dryValue / 10.0);
        calibWetValue.set_measured_value_celsius(wetValue / 10.0);
        calibBatteryDiv.set_measured_value_celsius(batteryDiv);
    }
}

float MatterMultiSensor::getMoistureDryValue() {
    return calibDryValue.get_measured_value() * 10.0; // Convert back to ADC value
}

float MatterMultiSensor::getMoistureWetValue() {
    return calibWetValue.get_measured_value() * 10.0; // Convert back to ADC value
}

float MatterMultiSensor::getBatteryDivider() {
    return calibBatteryDiv.get_measured_value();
}

bool MatterMultiSensor::getUsbConnected() {
    return usbConnectionStatus.get_state();
}

void MatterMultiSensor::setUsbConnected(bool connected) {
    usbConnectionStatus.set_state(connected);
}

bool MatterMultiSensor::getCalibrationMode() {
    return calibrationModeControl.get_state();
}

void MatterMultiSensor::setCalibrationMode(bool active) {
    calibrationModeControl.set_state(active);
    if (calibrationManager) {
        if (active) {
            calibrationManager->startCalibration();
        } else {
            calibrationManager->finishCalibration();
        }
    }
}

uint8_t MatterMultiSensor::getDisplayType() {
    return static_cast<uint8_t>(displayTypeReport.get_measured_value());
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
    
    batteryVoltage.set_device_name("Battery Voltage");
    batteryVoltage.set_product_name("Battery Volts");
    
    // Sleep interval controls
    sleepIntervalNormal.set_device_name("Normal Sleep Interval");
    sleepIntervalNormal.set_product_name("Normal Sleep (sec)");
    
    sleepIntervalExt.set_device_name("Extended Sleep Interval");
    sleepIntervalExt.set_product_name("Extended Sleep (sec)");
    
    sleepIntervalLow.set_device_name("Low Power Sleep Interval");
    sleepIntervalLow.set_product_name("Low Power Sleep (sec)");
    
    sleepIntervalUsb.set_device_name("USB Sleep Interval");
    sleepIntervalUsb.set_product_name("USB Sleep (sec)");
    
    // Status reporting
    powerStateReport.set_device_name("Power State");
    powerStateReport.set_product_name("Power State (0-4)");
    
    displayTypeReport.set_device_name("Display Type");
    displayTypeReport.set_product_name("Display (0=None,1=Serial,2=LED,3=OLED)");
    
    currentSleepReport.set_device_name("Current Sleep Interval");
    currentSleepReport.set_product_name("Active Sleep (sec)");
    
    // Calibration values (read-only)
    calibDryValue.set_device_name("Calibration Dry Value");
    calibDryValue.set_product_name("Dry ADC Value");
    
    calibWetValue.set_device_name("Calibration Wet Value");
    calibWetValue.set_product_name("Wet ADC Value");
    
    calibBatteryDiv.set_device_name("Battery Voltage Divider");
    calibBatteryDiv.set_product_name("Voltage Divider Ratio");
    
    // Controls
    powerManagementControl.set_device_name("Power Management");
    powerManagementControl.set_product_name("Enable Power Mgmt");
    
    usbConnectionStatus.set_device_name("USB Connected");
    usbConnectionStatus.set_product_name("USB Status");
    
    calibrationModeControl.set_device_name("Calibration Mode");
    calibrationModeControl.set_product_name("Calibration Active");
}

void MatterMultiSensor::updateMatterCallbacks() {
    // Set callbacks for when Home Assistant changes values
    // Note: Since we're using stubs, callbacks are not functional yet
    // In real implementation, these would be set up with actual Matter library callbacks
    
    // powerManagementControl.set_device_change_callback(staticPowerManagementCallback);
    // This will be implemented when we have the real Matter library
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

void MatterMultiSensor::onLowPowerSleepIntervalChanged() {
    // Called when Home Assistant changes the low power sleep interval  
    uint32_t newInterval = getLowPowerSleepInterval();
    if (powerManager) {
        powerManager->setLowPowerSleepInterval(newInterval * 1000);
    }
}

void MatterMultiSensor::onUsbSleepIntervalChanged() {
    // Called when Home Assistant changes the USB sleep interval  
    uint32_t newInterval = getUsbSleepInterval();
    if (powerManager) {
        powerManager->setUsbSleepInterval(newInterval * 1000);
    }
}

void MatterMultiSensor::onCalibrationModeToggled() {
    // Called when Home Assistant toggles calibration mode
    bool active = getCalibrationMode();
    if (calibrationManager) {
        if (active) {
            calibrationManager->startCalibration();
        } else {
            calibrationManager->finishCalibration();
            updateCalibrationValues(); // Refresh the values after calibration
        }
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
