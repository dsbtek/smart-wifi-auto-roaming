# Changelog

All notable changes to SmartAutoRoam will be documented in this file.

## [Unreleased] - Speed-Based Switching Refactoring

### Added
- **SpeedTester Component**: New class for measuring actual WiFi performance
  - Quick speed test (3-second timeout)
  - Full speed test with upload capability
  - Latency measurement via ping
  - Link speed detection from system
  
- **Speed-Based Decision Making**: Intelligent switching algorithm
  - Measures actual download speed before switching
  - Compares current speed vs. estimated candidate speed
  - Only switches when speed improvement justifies it
  - Falls back to signal-based decision if speed test fails
  
- **New Configuration Settings**:
  - `min_speed_improvement`: Minimum speed improvement percentage (default: 20%)
  - `min_acceptable_speed`: Minimum acceptable speed in Mbps (default: 5.0)
  
- **Enhanced Database Schema**:
  - Added speed tracking columns to `networks` table
  - New `speed_history` table for historical speed measurements
  - Stores download speed, upload speed, latency, and signal strength
  
- **UI Enhancements**:
  - Speed improvement threshold setting
  - Minimum acceptable speed setting
  - Better logging of speed test results

### Changed
- **RoamingManager Algorithm**: Refactored from signal-only to speed-based switching
  - Now measures actual speed before making switch decisions
  - Considers both signal strength and actual performance
  - Verifies speed improvement after switching
  
- **Switching Logic**: More intelligent decision making
  - Switches if current speed < minimum acceptable threshold
  - Switches if estimated improvement > configured percentage
  - Maintains 3-scan stability requirement
  
- **Dependencies**: Added Qt6::Network module for HTTP-based speed testing

### Improved
- **Reduced Unnecessary Switches**: Avoids switching to networks with good signal but poor performance
- **Better User Experience**: Maintains connection quality based on actual speed
- **Data Collection**: Builds historical speed data for future optimizations
- **Logging**: Enhanced logs with speed measurements and decision rationale

### Technical Details

**Files Added:**
- `src/SpeedTester.h` - Speed testing interface
- `src/SpeedTester.cpp` - Speed testing implementation
- `REFACTORING_NOTES.md` - Detailed refactoring documentation

**Files Modified:**
- `src/RoamingManager.h` - Added speed-based methods
- `src/RoamingManager.cpp` - Implemented speed-based switching logic
- `src/ConfigManager.cpp` - Added new configuration defaults
- `src/Database.cpp` - Updated schema with speed tracking
- `src/MainWindow.cpp` - Added UI handlers for new settings
- `ui/MainWindow.ui` - Added speed-related UI controls
- `CMakeLists.txt` - Added SpeedTester and Qt6::Network
- `README.md` - Updated documentation with speed-based algorithm

**Configuration Changes:**
```
Old Settings:
- scan_interval: 30s
- min_signal_diff: 10%
- switch_signal_threshold: -75 dBm

New Settings (added):
- min_speed_improvement: 20%
- min_acceptable_speed: 5.0 Mbps
```

**Database Schema Changes:**
```sql
-- networks table (added columns)
ALTER TABLE networks ADD COLUMN last_speed_mbps REAL;
ALTER TABLE networks ADD COLUMN avg_speed_mbps REAL;
ALTER TABLE networks ADD COLUMN speed_test_count INTEGER DEFAULT 0;

-- New table
CREATE TABLE speed_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ssid TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    download_speed_mbps REAL,
    upload_speed_mbps REAL,
    latency_ms REAL,
    signal_strength INTEGER
);
```

### Migration Notes

For existing users:
1. The database will automatically add new columns and tables on first run
2. New settings will use default values initially
3. Speed testing adds ~3 seconds to the switching decision process
4. Historical data will accumulate over time for better decisions

### Performance Impact

- Speed test adds 3-5 seconds to switching decision (configurable timeout)
- Network bandwidth usage: ~1-5 MB per speed test
- Speed tests are only performed when considering a switch (not every scan)
- Periodic speed checks on current connection (every 5 scans)

## [0.1.0] - Initial Release

### Added
- Basic WiFi network scanning
- Signal-based auto-roaming
- Qt6-based GUI
- SQLite database for settings and logs
- NetworkManager integration (Linux)
- Configurable scan intervals and thresholds
- Event logging system

