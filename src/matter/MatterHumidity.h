#pragma once
#include <Arduino.h>

// Forward declaration for PowerManager
class PowerManager;

class MatterHumidity {
public:
    void begin() {}
    bool is_online() { return true; } // Stub: always online for now
    void set_measured_value(float value) { measured = value; }
    float get_measured_value() const { return measured; }
    
    // Power management attribute interface (for future Matter integration)
    void setPowerManager(PowerManager* manager) { powerManager = manager; }
    
    // Matter attribute getters/setters for power management
    uint32_t getNormalSleepInterval() const;
    void setNormalSleepInterval(uint32_t interval);
    
    uint32_t getExtendedSleepInterval() const;
    void setExtendedSleepInterval(uint32_t interval);
    
    uint32_t getLowPowerSleepInterval() const;
    void setLowPowerSleepInterval(uint32_t interval);
    
    uint32_t getUsbSleepInterval() const;
    void setUsbSleepInterval(uint32_t interval);
    
    float getBatteryNormalThresh() const;
    void setBatteryNormalThresh(float thresh);
    
    float getBatteryExtendedThresh() const;
    void setBatteryExtendedThresh(float thresh);
    
    float getBatteryCriticalThresh() const;
    void setBatteryCriticalThresh(float thresh);
    
    // Power state reporting
    uint8_t getPowerState() const;
    uint32_t getCurrentSleepInterval() const;
    
    // Optionally add min/max if needed
    // void set_min_measured_value(float value) {}
    // void set_max_measured_value(float value) {}
    
private:
    float measured = 0.0;
    PowerManager* powerManager = nullptr;
};
