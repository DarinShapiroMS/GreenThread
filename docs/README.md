# Green Thread - Arduino Nano Matter Soil Moisture Sensor

A professional-grade soil moisture monitoring system built on Arduino Nano Matter with Thread/Matter connectivity for Home Assistant integration. Monitor your garden's health with smart watering optimization through real-time soil monitoring and intuitive LED status indication.

## Features

### 🌱 **Core Functionality**
- **Smart Soil Monitoring**: Calibrated capacitive sensor with percentage readings
- **Intelligent LED Status**: Color-coded moisture levels with intuitive boot/error indication
- **Battery Management**: Long-lasting operation with intelligent power optimization  
- **Multi-Display Support**: Auto-detection of OLED, RGB LED, or Serial displays
- **Matter/Thread Integration**: Seamless Home Assistant compatibility

### ⚡ **Advanced Power Management**
- **Adaptive Sleep Intervals**: Battery-aware power conservation
- **USB Override**: More responsive operation when connected to USB
- **Configurable Thresholds**: Remote adjustment via Matter attributes
- **Power State Reporting**: Real-time status via Thread network

### � **Enhanced LED Status System**
- **Boot Guidance**: Steady green light during startup phases
- **Connection Status**: Slow red blink for network/Matter failures  
- **Moisture Display**: 7-color gradient (red=dry → blue=wet) with 5 slow blinks
- **Sleep Indication**: LED off during sleep cycles for power conservation

### �🔧 **Professional Features**
- **EEPROM Calibration**: Persistent sensor calibration storage
- **Factory Reset**: Easy recalibration workflow
- **Modular Architecture**: Clean separation of concerns for maintainability
- **Custom Matter Cluster**: Advanced soil sensor attributes and thresholds

### 🏠 **Home Assistant Integration**
- **Multiple Matter Endpoints**: Separate entities for all sensor data
- **Remote Configuration**: Adjust sleep intervals and power settings
- **Real-time Monitoring**: Battery level, power state, and soil moisture
- **Smart Automation**: Create watering schedules and plant health alerts

## Hardware Requirements

- **Arduino Nano Matter** (Silicon Labs MGM240S)
- **Soil Moisture Sensor** (Capacitive, connected to A0)
- **Battery Monitor** (Voltage divider on A1)
- **Optional OLED Display** (SSD1306, I2C 0x3C)
- **Built-in RGB LEDs** for status indication

## Quick Start

1. **Hardware Setup**: Connect soil sensor to A0, battery monitor to A1
2. **Flash Firmware**: Upload `Green_Thread.ino` to Arduino Nano Matter
3. **Matter Commissioning**: Add to Home Assistant via QR code
4. **Calibration**: Use dry/wet calibration workflow
5. **Configuration**: Adjust power settings via HA entities

## LED Behavior Guide

### Boot Sequence
- **Steady Green**: Normal startup progression through initialization phases
- **LED Off**: Boot complete, device ready

### Connection Status
- **Slow Red Blink**: Network connectivity issues (Thread/Matter unreachable)
- **LED Off**: Connections restored successfully

### Moisture Readings
- **5 Color Blinks**: Moisture level indication (400ms on/off pattern)
  - Red (0-14%): Driest - critical moisture
  - Orange (15-28%): Very dry - water needed
  - Yellow (29-42%): Slightly dry - monitor closely  
  - White (43-57%): Adequate moisture
  - Light Green (58-71%): Good moisture level
  - Dark Green (72-85%): Excellent moisture
  - Blue (86-100%): Maximum moisture
- **LED Off**: Data published, entering sleep cycle

## Project Structure

```
Green_Thread.ino           # Main Arduino sketch
src/
├── config/               # Configuration constants
│   └── Config.h
├── hardware/             # Hardware abstraction layer
│   ├── SensorManager.cpp/h
│   ├── BatteryMonitor.cpp/h
│   ├── CalibrationManager.cpp/h
│   └── PowerManager.cpp/h
├── matter/               # Matter/Thread integration
│   ├── MatterMultiSensor.cpp/h
│   └── GreenThreadSoilSensorCluster.cpp/h
└── ui/                   # Display implementations
    ├── StatusDisplay.h
    ├── OledStatusDisplay.cpp/h
    ├── RgbLedStatusDisplay.cpp/h
    ├── SerialStatusDisplay.cpp/h
    ├── CompositeStatusDisplay.cpp/h
    └── DisplayFactory.cpp/h
docs/                     # Documentation
examples/                 # Example sketches and tests
```

## Documentation

- **[LED Behavior Guide](LED_BEHAVIOR_GUIDE.md)**: Complete LED status system documentation
- **[Custom Cluster Guide](CUSTOM_CLUSTER_README.md)**: Matter cluster implementation details
- **[Integration Guide](CUSTOM_CLUSTER_INTEGRATION_GUIDE.md)**: Home Assistant setup and usage

## Development

### Build System
- **Arduino CLI** with VS Code tasks
- **Silicon Labs Arduino Core** v2.3.0+
- **Matter Protocol Stack** enabled

### Version Control
- Clean commit history with feature branches
- Automated testing of builds before merge
- Tagged releases for stable versions

## License

MIT License - see LICENSE file for details

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Submit a pull request

## Acknowledgments

- **Silicon Labs** for Arduino Matter support
- **Arduino Team** for the development platform
- **Home Assistant** community for Matter integration
- **Thread Group** for the mesh networking standard
