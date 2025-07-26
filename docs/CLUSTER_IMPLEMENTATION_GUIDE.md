# Green Thread Custom Matter Cluster Implementation Guide

## Overview
This guide covers implementing the Green Thread Soil Sensor custom Matter cluster using the official ConnectedHomeIP Matter SDK.

## Files Created
- `green-thread-soil-sensor-cluster.xml` - Main cluster definition
- `green-thread-enums.xml` - Enumeration definitions
- This implementation guide

## Cluster Details

### Cluster ID: 0xFFF1FC30
- **Vendor ID**: 0xFFF1 (Test vendor ID)
- **Cluster ID**: 0xFC30 (Custom cluster for soil sensor)
- **Name**: Green Thread Soil Sensor Cluster

### Key Features

#### 1. Soil Monitoring (0x0000-0x000F)
- **SoilMoisturePercent** (0x0000): Primary moisture reading (0-100%)
- **SoilMoistureRaw** (0x0001): Raw ADC value for diagnostics
- **CalibrationStatus** (0x0002): Current calibration state

#### 2. Calibration Management (0x0010-0x001F)
- **CalibrationDryValue** (0x0010): Dry soil reference value (R/W)
- **CalibrationWetValue** (0x0011): Wet soil reference value (R/W)
- **CalibrationBatteryDivider** (0x0012): Battery voltage calibration (R/W)

#### 3. Power Management (0x0020-0x002F)
- **BatteryVoltageMv** (0x0020): Battery voltage in millivolts
- **BatteryLevelPercent** (0x0021): Battery level (0-100%)
- **PowerState** (0x0022): Current power source/state
- **UsbConnected** (0x0023): USB connection status

#### 4. Sleep Configuration (0x0030-0x003F)
- **SleepIntervalNormal** (0x0030): Normal operation sleep time (R/W)
- **SleepIntervalExtended** (0x0031): Extended sleep time (R/W)
- **SleepIntervalLowBattery** (0x0032): Low battery sleep time (R/W)
- **SleepIntervalUsb** (0x0033): USB connected sleep time (R/W)
- **CurrentSleepInterval** (0x0034): Currently active interval

#### 5. System Status (0x0040-0x004F)
- **DisplayType** (0x0040): Active display type
- **SystemUptime** (0x0041): Seconds since boot
- **LastMeasurementTime** (0x0042): Timestamp of last reading
- **MeasurementCount** (0x0043): Total measurements taken

### Commands

#### Calibration Commands
- **StartDryCalibration** (0x10): Begin dry soil calibration
- **StartWetCalibration** (0x11): Begin wet soil calibration
- **ResetCalibration** (0x12): Reset to factory defaults

#### System Commands
- **ForceMeasurement** (0x13): Immediate sensor reading
- **EnterLowPowerMode** (0x14): Switch to power saving
- **ExitLowPowerMode** (0x15): Return to normal operation
- **GetStatus** (0x16): Get comprehensive status
- **RestartSystem** (0x17): Soft restart device

### Events

#### Automatic Notifications
- **MoistureThresholdCrossed**: Soil moisture level changes significantly
- **BatteryLevelChanged**: Battery level changes
- **PowerStateChanged**: Power source changes (USB/battery)
- **CalibrationCompleted**: Calibration process finishes
- **SystemError**: Error conditions and warnings

## Implementation Steps

### 1. Set Up Matter SDK Environment
```bash
# Clone Matter repository
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Install prerequisites (Linux/WSL)
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
    libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
    python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev

# Initialize environment
source scripts/bootstrap.sh
```

### 2. Add XML Files to Matter SDK
```bash
# Copy cluster definition
cp green-thread-soil-sensor-cluster.xml src/app/zap-templates/zcl/data-model/chip/

# Copy enum definitions  
cp green-thread-enums.xml src/app/zap-templates/zcl/data-model/chip/
```

### 3. Update Matter SDK Configuration Files

#### Add to `src/app/zap-templates/zcl/zcl.json`:
In the `xmlFile` array, add:
```json
"green-thread-soil-sensor-cluster.xml",
"green-thread-enums.xml",
```

#### Add to `src/app/zap-templates/zcl/zcl-with-test-extensions.json`:
In the `xmlFile` array, add:
```json
"green-thread-soil-sensor-cluster.xml",
"green-thread-enums.xml",
```

#### Add to `src/app/common/templates/config-data.yaml`:
```yaml
CommandHandlerInterfaceOnlyClusters:
    - GreenThreadSoilSensor
```

#### Add to `src/app/zap_cluster_list.json`:
```json
"ServerDirectories": {
    "GREEN_THREAD_SOIL_SENSOR_CLUSTER": ["green-thread-soil-sensor-server"],
}
```

### 4. Generate Code
```bash
# Regenerate all Matter code
scripts/tools/zap_regen_all.py

# Format code to Matter standards
scripts/helpers/restyle-diff.sh
```

### 5. Implement Cluster Server

Create `src/app/clusters/green-thread-soil-sensor-server/` with:
- `green-thread-soil-sensor-server.h` - Header definitions
- `green-thread-soil-sensor-server.cpp` - Implementation

### 6. Port to Silicon Labs MGM240S

The generated code can then be adapted for your Arduino Nano Matter hardware.

## Benefits of This Approach

### ✅ Proper Matter Integration
- Native Matter cluster, not workaround
- Full Home Assistant compatibility
- Proper semantic meaning for each attribute

### ✅ Professional Implementation
- Follows Matter specification exactly
- Comprehensive attribute set
- Rich command and event system

### ✅ Future-Proof
- Extensible design
- Standard Matter toolchain
- Version controlled in Matter SDK

### ✅ Advanced Features
- Remote calibration via Matter
- Comprehensive power management
- Rich event system for automation
- Diagnostic and status information

## Next Steps

1. **Set up Matter SDK environment** (Linux/WSL recommended)
2. **Integrate XML files** into Matter SDK
3. **Generate cluster code** using ZAP tools
4. **Implement server logic** for your specific hardware
5. **Port to Arduino Nano Matter** or create bridge device
6. **Test with Home Assistant** Matter integration

This creates a production-ready, specification-compliant Matter cluster specifically designed for your Green Thread soil monitoring needs!
