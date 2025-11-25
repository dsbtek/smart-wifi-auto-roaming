# ZHR – Zero-Handoff Roaming

A real-time, speed-aware WiFi auto-roaming daemon with a clean Qt6 GUI. ZHR continuously monitors signal strength and actual internet performance, then instantly and seamlessly switches to the best available network — zero interruptions, zero manual intervention.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)
![ZHR](https://img.shields.io/badge/feature-ZHR-brightgreen)

## Overview

ZHR eliminates weak or congested WiFi connections by combining fast signal scanning with real download-speed testing. The moment a noticeably better network is available, ZHR performs a zero-handoff switch so your applications never notice the change.

### Key Features

- **Zero-Handoff Roaming (ZHR)** – Instant, seamless network transitions  
- **Speed-Based Decision Engine** – Switches only when real throughput improves  
- **Real-Time Scanning & Monitoring** – Live view of all networks and signal quality  
- **Stability Verification** – Requires consistent improvement over multiple scans  
- **Connectivity Validation** – Ping + speed checks before every switch  
- **Preferred Network Priority** – Always favor your chosen SSIDs  
- **Full Event Logging** – Every decision stored in SQLite  
- **Modern Qt6 GUI** – Intuitive control and diagnostics  

## How ZHR Works (the smart way)

1. Scan visible networks (default every 30 s)  
2. Measure current download speed & latency  
3. Identify candidate with significantly better signal + potential speed gain  
4. Confirm improvement is stable across 3 scans  
5. Execute zero-handoff switch via NetworkManager  
6. Verify new connection is actually faster  
7. Log everything  

→ Because signal bars lie — real Mbps don’t.

## Requirements

- Linux with NetworkManager (`nmcli`)  
- Qt6 (Widgets, Core, Gui, Network, Sql)  
- C++17 compiler + CMake ≥ 3.16  

## Quick Install (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev network-manager

git clone https://github.com/dsbtek/smart-wifi-auto-roaming.git
cd smart-wifi-auto-roaming
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## Usage

```bash
ZHR        # or ./build/ZHR if not installed
```

## Configuration (persisted automatically)

- Scan interval (5–600 s)  
- Minimum speed improvement (default 20 %)  
- Minimum acceptable speed (default 5 Mbps)  
- Signal stability scans (default 3)  
- Preferred SSIDs (comma-separated)  

All settings adjustable from the GUI and stored in `~/.local/share/ZHR/zhr.db`

## Platform Support

- Supported: **Linux** (NetworkManager)  
- Planned: macOS · Windows  

## Troubleshooting

- No networks shown → `nmcli radio wifi on`  
- No auto-switch → check logs tab for thresholds/reasons  
- Permission error → `sudo usermod -aG netdev $USER` (then reboot)

## Contributing

Issues, forks, and PRs very welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) when added.

## License

MIT – see [LICENSE](LICENSE)

## Author

**Muhammad Salihu** ([@dsbtek](https://github.com/dsbtek))

Enjoy truly seamless WiFi with **ZHR – Zero-Handoff Roaming**.
