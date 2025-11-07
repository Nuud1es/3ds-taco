# 3DS-TACO

**OBD-II WiFi Telemetry for Nintendo 3DS**

A Nintendo 3DS homebrew application that displays real-time car telemetry data from an OBD-II WiFi adapter. Features a cassette futurism-inspired interface with analog dials for speed and RPM.

## Features

- **Real-time Telemetry**: Connect to ELM327-compatible OBD-II WiFi adapters
- **Cassette Futurism UI**: Retro-futuristic interface design with smooth 2D graphics
- **Analog Dials with Numeric Displays**: Top screen features dual analog gauges showing:
  - Speed (MPH) with large numeric readout
  - Engine RPM (x100) with large numeric readout
  - Animated needles that respond to real-time data
- **Comprehensive Telemetry Panels**: Bottom screen displays six data panels with values:
  - Throttle position (%)
  - Coolant temperature (°F)
  - Engine load (%)
  - Intake air temperature (°F)
  - Fuel level (%)
  - MAF sensor readings (g/s)
- **Professional Text Rendering**: Citro2D-powered UI with smooth fonts and labels

## Requirements

### Hardware
- Nintendo 3DS/2DS console with custom firmware (CFW)
- OBD-II WiFi adapter (ELM327-compatible recommended)
- Vehicle with OBD-II port (1996+ for most US vehicles)

### Software
- devkitARM/devkitPro toolchain
- libctru
- 3DS homebrew launcher access

## Building

### Quick Build (Automated)

The easiest way to build is using the automated build script that downloads and installs all required tools:

```bash
# Clone the repository
git clone https://github.com/yourusername/3ds-taco.git
cd 3ds-taco

# Run the automated build script
./build.sh
```

The script will:
- Detect your operating system (Linux/macOS/Windows)
- Download and install devkitPro if not already present
- Install the 3ds-dev package with all dependencies
- Compile the application
- Show you the next steps for installation

**Supported platforms:**
- Linux (Ubuntu/Debian/Fedora/Arch)
- macOS (requires Homebrew)
- Windows (requires MSYS2)

### Manual Build

If you prefer to install tools manually or already have devkitPro installed:

1. Install [devkitPro](https://devkitpro.org/wiki/Getting_Started) with 3DS development tools:
   ```bash
   # On Linux/macOS
   sudo (dkp-)pacman -S 3ds-dev

   # On Windows, use the graphical installer
   ```

2. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/3ds-taco.git
   cd 3ds-taco
   ```

3. Build the application:
   ```bash
   make
   ```

4. The compiled `.3dsx` file will be in the project root directory.

## Installation

1. Copy `3ds-taco.3dsx` to your SD card's `/3ds/` folder

2. Create the configuration directory:
   ```
   /3ds/3ds-taco/
   ```

3. Copy `config.ini.example` to `/3ds/3ds-taco/config.ini` on your SD card and edit it with your OBD-II adapter settings:
   ```ini
   ssid=OBDII
   password=
   ip=192.168.0.10
   port=35000
   ```

## Usage

1. **Setup your OBD-II WiFi adapter**:
   - Plug the adapter into your vehicle's OBD-II port
   - Turn on your vehicle's ignition (engine doesn't need to be running)
   - Wait for the adapter to create its WiFi network

2. **Connect your 3DS**:
   - Go to System Settings → Internet Settings
   - Set up a new connection to your OBD-II adapter's WiFi network
   - Use the SSID and password from your adapter (commonly "OBDII" with no password)

3. **Launch the app**:
   - Open the Homebrew Launcher on your 3DS
   - Navigate to and launch 3DS-TACO

4. **Connect to telemetry**:
   - Press **A** to connect to the OBD-II adapter
   - Wait for the connection indicator to turn green
   - Start your engine to see live data

5. **Controls**:
   - **A Button**: Connect to OBD-II adapter
   - **START**: Exit application

## Configuration

Edit `sdmc:/3ds/3ds-taco/config.ini`:

- `ssid`: Your OBD-II adapter's WiFi network name
- `password`: WiFi password (empty for open networks)
- `ip`: Adapter IP address (usually 192.168.0.10)
- `port`: Adapter port (usually 35000)

## Troubleshooting

### Can't connect to adapter
- Ensure your 3DS is connected to the adapter's WiFi network
- Verify the IP address and port in config.ini
- Try the default values: IP=192.168.0.10, Port=35000
- Check if your adapter is ELM327-compatible

### No data displayed
- Make sure your vehicle's ignition is on
- Try starting the engine
- Some vehicles require the engine to be running for certain sensors

### Incorrect readings
- Different vehicles may report data differently
- Some PIDs may not be supported by your vehicle
- Try resetting the adapter by unplugging it for 10 seconds

## OBD-II Adapter Compatibility

This application works with ELM327-compatible WiFi adapters. Recommended models:
- Generic ELM327 WiFi adapters (v1.5+)
- OBDLink MX+
- Vgate iCar series

**Note**: Avoid cheap Bluetooth-only adapters as the 3DS cannot connect to them.

## Technical Details

### Supported OBD-II PIDs
- `0x0C`: Engine RPM
- `0x0D`: Vehicle Speed
- `0x04`: Engine Load
- `0x05`: Coolant Temperature
- `0x0F`: Intake Air Temperature
- `0x11`: Throttle Position
- `0x2F`: Fuel Level
- `0x10`: MAF Sensor

### Display Specifications
- **Top Screen (400x240)**:
  - Two analog dials with cassette futurism styling
  - Left: Speed (0-140 MPH) with numeric display
  - Right: RPM (0-8000 RPM) with numeric display
  - Smooth animated needles using Citro2D
  - Title bar with connection status indicator
  - Decorative accent lines
- **Bottom Screen (320x240)**:
  - Six telemetry data panels with labeled values
  - Connection status and instructions when disconnected
  - Real-time updating numeric displays

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## License

This project is provided as-is for educational and personal use.

## Disclaimer

**Use at your own risk.** This software connects to your vehicle's OBD-II system. While it only reads data (does not write), ensure you:
- Only use while the vehicle is safely parked
- Do not use while driving
- Understand your local laws regarding vehicle modifications and diagnostics

The developers are not responsible for any damage to your vehicle or 3DS console.

## Credits

- Built with [libctru](https://github.com/devkitPro/libctru)
- Graphics powered by [Citro2D](https://github.com/devkitPro/citro2d) and [Citro3D](https://github.com/devkitPro/citro3d)
- ELM327 protocol documentation
- Cassette futurism design inspiration from retro computing aesthetics

## Support

For issues, questions, or contributions, please visit the [GitHub repository](https://github.com/yourusername/3ds-taco).
