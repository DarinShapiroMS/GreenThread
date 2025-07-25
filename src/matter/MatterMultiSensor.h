#pragma once
#include <Arduino.h>

// Forward declarations
class PowerManager;
class CalibrationManager;
class DisplayFactory;

// Stub for Matter device classes - replace with actual Silicon Labs Matter library classes
class MatterDevice {
public:
    virtual void begin() = 0;
    virtual void set_device_name(const char* name) = 0;
    virtual void set_product_name(const char* name) = 0;
    virtual void set_device_change_callback(void (*callback)()) = 0;
};

class MatterHumidityStub : public MatterDevice {
public:
    void begin() override {}
    void set_device_name(const char* name) override {}
    void set_product_name(const char* name) override {}
    void set_device_change_callback(void (*callback)()) override {}
    void set_measured_value(float value) { measured = value; }
    float get_measured_value() const { return measured; }
private:
    float measured = 0.0;
};

class MatterTemperatureStub : public MatterDevice {
public:
    void begin() override {}
    void set_device_name(const char* name) override {}
    void set_product_name(const char* name) override {}
    void set_device_change_callback(void (*callback)()) override {}
    void set_measured_value_celsius(float value) { measured = value; }
    float get_measured_value() const { return measured; }
private:
    float measured = 0.0;
};

class MatterSwitchStub : public MatterDevice {
public:
    void begin() override {}
    void set_device_name(const char* name) override {}
    void set_product_name(const char* name) override {}
    void set_device_change_callback(void (*callback)()) override {}
    void set_state(bool state) { currentState = state; }
    bool get_state() const { return currentState; }
private:
    bool currentState = false;
};

class MatterStub {
public:
    void begin() {}
    bool isDeviceCommissioned() { return true; }
    bool isDeviceThreadConnected() { return true; }
};

class MatterMultiSensor {
public:
    void begin();
    bool isOnline();
    
    // Main sensor data
    void setSoilMoisture(float percent);
    void setBatteryVoltage(float voltage);
    void setBatteryPercent(float percent);
    
    // Power management controls
    void setPowerManager(PowerManager* manager);
    void setCalibrationManager(CalibrationManager* manager);
    // Note: DisplayFactory is static, so we'll access it directly
    
    // Power management controls (via switches)
    bool getPowerManagementEnabled();
    void setPowerManagementEnabled(bool enabled);
    bool getUsbConnected();
    void setUsbConnected(bool connected);
    bool getCalibrationMode();
    void setCalibrationMode(bool active);
    
    // Sleep interval controls (using temperature sensors as proxy)
    uint32_t getNormalSleepInterval();
    void setNormalSleepInterval(uint32_t intervalSec);
    uint32_t getExtendedSleepInterval();
    void setExtendedSleepInterval(uint32_t intervalSec);
    uint32_t getLowPowerSleepInterval();
    void setLowPowerSleepInterval(uint32_t intervalSec);
    uint32_t getUsbSleepInterval();
    void setUsbSleepInterval(uint32_t intervalSec);
    
    // Calibration data access (read-only via temperature sensors)
    void updateCalibrationValues();
    float getMoistureDryValue();
    float getMoistureWetValue();
    float getBatteryDivider();
    
    // Status reporting
    uint8_t getCurrentPowerState();
    float getCurrentSleepInterval();
    uint8_t getDisplayType(); // 0=None, 1=Serial, 2=LED, 3=OLED
    
    // Matter device callbacks
    void onNormalSleepIntervalChanged();
    void onExtendedSleepIntervalChanged();
    void onLowPowerSleepIntervalChanged();
    void onUsbSleepIntervalChanged();
    void onPowerManagementToggled();
    void onCalibrationModeToggled();
    
private:
    // Multiple Matter endpoints for different data types
    MatterHumidityStub soilMoisture;           // Primary sensor
    MatterTemperatureStub batteryLevel;        // Battery % (using temp sensor)
    MatterTemperatureStub batteryVoltage;      // Battery voltage (using temp sensor)
    
    // Sleep interval configuration (using temperature sensors)
    MatterTemperatureStub sleepIntervalNormal; // Normal sleep config
    MatterTemperatureStub sleepIntervalExt;    // Extended sleep config  
    MatterTemperatureStub sleepIntervalLow;    // Low power sleep config
    MatterTemperatureStub sleepIntervalUsb;    // USB sleep config
    
    // Status reporting (using temperature sensors)
    MatterTemperatureStub powerStateReport;    // Current power state (0-4)
    MatterTemperatureStub displayTypeReport;   // Display type (0-3)
    MatterTemperatureStub currentSleepReport;  // Current sleep interval
    
    // Calibration values (read-only, using temperature sensors)
    MatterTemperatureStub calibDryValue;       // Moisture dry calibration
    MatterTemperatureStub calibWetValue;       // Moisture wet calibration
    MatterTemperatureStub calibBatteryDiv;     // Battery voltage divider
    
    // Control switches
    MatterSwitchStub powerManagementControl;   // Enable/disable power mgmt
    MatterSwitchStub usbConnectionStatus;      // USB connected status
    MatterSwitchStub calibrationModeControl;   // Calibration mode active
    
    PowerManager* powerManager = nullptr;
    CalibrationManager* calibrationManager = nullptr;
    MatterStub matter;
    
    // Helper methods
    void setupDeviceNames();
    void updateMatterCallbacks();
    float convertSecondsToTempScale(uint32_t seconds);
    uint32_t convertTempScaleToSeconds(float tempValue);
};
