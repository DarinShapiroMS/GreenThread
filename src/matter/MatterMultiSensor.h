#pragma once
#include <Arduino.h>

// Arduino Matter library classes
// Note: These would need to be included individually based on Silicon Labs library structure
// For now, we'll stub them and implement based on available library classes

// Forward declaration for PowerManager
class PowerManager;

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
    
    // Power management controls (via switches)
    void setPowerManager(PowerManager* manager);
    bool getPowerManagementEnabled();
    void setPowerManagementEnabled(bool enabled);
    
    // Sleep interval controls (using temperature sensors as proxy)
    uint32_t getNormalSleepInterval();
    void setNormalSleepInterval(uint32_t intervalSec);
    uint32_t getExtendedSleepInterval();
    void setExtendedSleepInterval(uint32_t intervalSec);
    
    // Status reporting
    uint8_t getCurrentPowerState();
    float getCurrentSleepInterval();
    
    // Matter device callbacks
    void onNormalSleepIntervalChanged();
    void onExtendedSleepIntervalChanged();
    void onPowerManagementToggled();
    
private:
    // Multiple Matter endpoints for different data types
    MatterHumidityStub soilMoisture;           // Primary sensor
    MatterTemperatureStub batteryLevel;        // Battery % (using temp sensor)
    MatterTemperatureStub sleepIntervalNormal; // Sleep config (using temp sensor)
    MatterTemperatureStub sleepIntervalExt;    // Sleep config (using temp sensor)
    MatterSwitchStub powerManagementControl;   // Enable/disable power mgmt
    
    PowerManager* powerManager = nullptr;
    MatterStub matter;
    
    // Helper methods
    void setupDeviceNames();
    void updateMatterCallbacks();
    float convertSecondsToTempScale(uint32_t seconds);
    uint32_t convertTempScaleToSeconds(float tempValue);
};
