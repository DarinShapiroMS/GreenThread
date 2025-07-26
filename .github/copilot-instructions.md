# GitHub Copilot Instructions for Green Thread Project

## Project Context
This is the **Green Thread Soil Moisture Sensor** - an Arduino Nano Matter-based IoT device for monitoring soil moisture with advanced features:

- **Hardware**: Arduino Nano Matter with capacitive soil moisture sensor
- **Connectivity**: Thread/Matter protocol for Home Assistant integration  
- **Power**: Battery-powered with USB charging capability
- **Display**: RGB LED status indication, optional OLED display
- **Features**: Custom Matter cluster, calibration, power management

## Code Organization

### Core Files
- `Green_Thread.ino` - Main Arduino sketch
- `src/hardware/` - Hardware abstraction (sensors, battery, power)
- `src/ui/` - User interface (LED, OLED, Serial displays)
- `src/matter/` - Matter/Thread protocol implementation
- `src/config/` - Configuration and calibration management

### Key Classes
- `SensorManager` - Soil moisture sensor interface
- `BatteryMonitor` - Battery voltage and health monitoring
- `PowerManager` - Power saving and sleep management
- `RgbLedStatusDisplay` - Intuitive LED status indication
- `MatterMultiSensor` - Matter protocol integration
- `GreenThreadSoilSensorCluster` - Custom Matter cluster

## Development Guidelines

### LED Behavior System
The LED system provides intuitive status indication:
- **Green during boot** - Device initializing normally
- **Color-coded moisture blinks** - 7-color scale (red=dry → blue=wet)
- **Slow red blink** - Connection problems
- **LED off** - Normal operation (battery saving)

### Matter Integration
- Uses Thread for mesh networking
- Custom cluster ID: `0x001F` (Green Thread Soil Sensor)
- Standard humidity cluster for Home Assistant compatibility
- Battery status and calibration data reporting

### Power Management
- Sleep modes for battery conservation
- USB vs battery detection
- Low battery warnings and shutdown protection
- Adaptive measurement intervals based on power state

## When Assisting With This Project

1. **Prioritize battery life** - Always consider power consumption in suggestions
2. **Keep LED behavior simple** - Avoid complex blinking patterns
3. **Maintain Matter compatibility** - Ensure Thread/Home Assistant integration works
4. **Consider calibration** - Soil sensors need dry/wet calibration for accuracy
5. **Use hardware abstraction** - Don't directly access sensors, use manager classes

## Common Tasks
- **LED debugging**: Use serial commands `led_off`, `led_green`, `led_red`
- **Calibration**: Commands `calibrate_dry`, `calibrate_wet`, `reset`
- **Testing**: Command `measure` for immediate sensor reading
- **Status**: Command `status` for comprehensive device status

## Architecture Notes
- Non-blocking code design (no `delay()` calls in main loop)
- Event-driven status system with `StatusEvent` enum
- Factory pattern for display selection (RGB LED vs OLED vs Serial)
- Composite display support for multiple outputs simultaneously
