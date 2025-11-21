# SmartAutoRoam

A real-time WiFi auto-roaming system with a Qt-based GUI that continuously monitors network strength and automatically switches to the best available WiFi network for optimal internet performance.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)

## Overview

SmartAutoRoam is an intelligent WiFi management application that eliminates the frustration of staying connected to weak networks. It automatically detects when a stronger WiFi network is available and seamlessly switches to it, ensuring you always have the best possible connection.

### Key Features

- **🔄 Automatic Network Switching**: Intelligently switches to stronger networks based on configurable signal thresholds
- **📊 Real-time Network Monitoring**: Continuously scans and displays available WiFi networks with signal strength
- **⚙️ Configurable Settings**: Customize scan intervals, signal difference thresholds, and preferred networks
- **📝 Event Logging**: Comprehensive logging of all network events and switches to SQLite database
- **🎯 Preferred Networks**: Define priority networks that are favored during auto-roaming
- **🔒 Stability Checks**: Requires consistent signal improvement across multiple scans before switching
- **🌐 Connectivity Validation**: Performs ping checks to ensure network stability before switching
- **🖥️ User-Friendly GUI**: Clean Qt-based interface for monitoring and manual control

## Architecture

The application is built with a modular architecture using Qt6 and C++17:

### Core Components

- **MainWindow**: Qt-based GUI for network visualization and settings management
- **RoamingManager**: Background service running in a separate thread that handles automatic network switching
- **NetworkScanner**: Scans available WiFi networks using `nmcli` (NetworkManager CLI)
- **ConnectionManager**: Manages WiFi connections via NetworkManager
- **ConfigManager**: Handles application settings with persistent storage
- **Database**: SQLite database for storing network history, settings, and event logs

### How It Works

SmartAutoRoam uses an intelligent **speed-based switching algorithm** rather than relying solely on signal strength:

1. **Scanning**: The RoamingManager periodically scans for available networks at configurable intervals (default: 30 seconds)
2. **Signal Analysis**: Identifies networks with significantly better signal strength
3. **Speed Testing**: Measures actual WiFi performance (download speed, latency) of the current connection
4. **Smart Decision**: Switches to a better network if:
   - Current speed is below acceptable threshold (default: 5 Mbps), OR
   - Estimated speed improvement exceeds minimum threshold (default: 20%)
   - Signal improvement is stable across 3 consecutive scans
   - Candidate network has sufficient signal strength
5. **Validation**: Performs connectivity checks before switching
6. **Logging**: Records all events including speed measurements to the database

**Why Speed-Based?** Signal strength doesn't always correlate with actual performance. A network with excellent signal might be congested, while a network with moderate signal might offer better throughput. By measuring actual speed, SmartAutoRoam makes smarter decisions.

## Requirements

### System Requirements
- **OS**: Linux (uses NetworkManager)
- **Network Manager**: `nmcli` must be installed and configured
- **Compiler**: C++17 compatible compiler (GCC 7+, Clang 5+)
- **CMake**: Version 3.16 or higher

### Dependencies
- **Qt6** (required modules):
  - Qt6::Widgets
  - Qt6::Sql
  - Qt6::Core
  - Qt6::Gui
  - Qt6::Network (for speed testing)
- **Qt6::Charts** (optional, for future enhancements)

## Installation

### Building from Source

1. **Clone the repository**:
   ```bash
   git clone https://github.com/dsbtek/smart-wifi-auto-roaming.git
   cd smart-wifi-auto-roaming
   ```

2. **Install dependencies** (Ubuntu/Debian):
   ```bash
   sudo apt update
   sudo apt install build-essential cmake qt6-base-dev qt6-charts-dev network-manager
   ```

   For Fedora/RHEL:
   ```bash
   sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtcharts-devel NetworkManager
   ```

3. **Build the application**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **Install** (optional):
   ```bash
   sudo make install
   ```

## Usage

### Running the Application

```bash
./SmartAutoRoam
```

Or if installed:
```bash
SmartAutoRoam
```

### GUI Interface

The main window consists of three sections:

1. **Available Networks Panel**:
   - Displays all detected WiFi networks with signal strength
   - Allows manual connection to selected networks
   - Refresh button to trigger immediate scan

2. **Settings Panel**:
   - **Scan Interval**: How often to scan for networks (5-600 seconds)
   - **Min Signal Diff**: Minimum signal improvement required to consider a switch (1-100%)
   - **Switch Threshold**: Minimum acceptable signal strength for the target network (1-100%)
   - **Min Speed Improvement**: Minimum speed improvement percentage to trigger a switch (5-100%, default: 20%)
   - **Min Acceptable Speed**: Minimum acceptable speed in Mbps before considering a switch (0.5-1000 Mbps, default: 5)
   - **Preferred Networks**: Comma-separated list of SSIDs to prioritize

3. **Log Panel**:
   - Real-time display of network events
   - Connection attempts and results
   - Roaming decisions and status updates

### Configuration

Settings are persisted in an SQLite database located at:
- `~/.local/share/SmartAutoRoam/networks.db` (Linux)

Default settings:
- Scan interval: 30 seconds
- Minimum signal difference: 10%
- Switch signal threshold: -75 dBm
- Minimum speed improvement: 20%
- Minimum acceptable speed: 5 Mbps
- Preferred networks: (empty)

### Network Manager Setup

Ensure your WiFi networks are configured in NetworkManager:

```bash
# List available connections
nmcli connection show

# Add a new WiFi connection
nmcli device wifi connect "SSID" password "PASSWORD"

# View saved connections
nmcli connection show --active
```

## How Auto-Roaming Works

The speed-based roaming algorithm follows these steps:

1. **Scan**: Detect all visible WiFi networks and their signal strengths
2. **Identify Current**: Determine currently connected network and its signal
3. **Find Best**: Identify the strongest available network
4. **Signal Analysis**: Check if candidate network has significantly better signal (>10% improvement)
5. **Stability Check**: Verify signal improvement is consistent over 3 consecutive scans
6. **Speed Test**: Measure actual download speed and latency of current connection
7. **Decision Logic**:
   - If current speed < minimum acceptable (5 Mbps) AND candidate has better signal → **Switch**
   - If estimated speed improvement > threshold (20%) → **Switch**
   - Otherwise → **Stay connected**
8. **Ping Test**: Validate current connection stability before switching
9. **Switch**: Connect to the better network if all conditions are met
10. **Verify**: Measure speed on new connection to confirm improvement
11. **Log**: Record all events including speed measurements to database

## Database Schema

The application uses SQLite with four main tables:

- **networks**: Stores network information, connection history, and speed measurements
- **settings**: Persists user configuration
- **logs**: Records all events with timestamps
- **speed_history**: Tracks historical speed test results for each network

## Platform Support

Currently supported:
- ✅ **Linux** (via NetworkManager/nmcli)

Planned support:
- ⏳ **macOS** (via CoreWLAN framework)
- ⏳ **Windows** (via Native WiFi API)

## Troubleshooting

### Application doesn't detect networks
- Ensure NetworkManager is running: `systemctl status NetworkManager`
- Check `nmcli` is installed: `which nmcli`
- Verify WiFi is enabled: `nmcli radio wifi on`

### Auto-switching not working
- Check the roaming manager is started (should start automatically)
- Review logs in the GUI for error messages
- Verify signal thresholds are appropriate for your environment
- Ensure networks are saved in NetworkManager

### Permission issues
- The application uses `nmcli` which may require proper user permissions
- Add your user to the `netdev` group: `sudo usermod -aG netdev $USER`

## Contributing

Contributions are welcome! Please feel free to submit issues, fork the repository, and create pull requests.

### Development Setup

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Build and test: `cmake .. && make`
5. Commit your changes: `git commit -m 'Add amazing feature'`
6. Push to the branch: `git push origin feature/amazing-feature`
7. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

**Muhammad Salihu** ([@dsbtek](https://github.com/dsbtek))

## Acknowledgments

- Built with [Qt6](https://www.qt.io/) framework
- Uses NetworkManager for Linux WiFi management
- Inspired by the need for seamless WiFi connectivity in multi-AP environments
