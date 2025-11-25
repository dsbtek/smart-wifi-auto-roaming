# ZHR Rebranding Summary

## Overview

The project has been successfully rebranded from **"SmartAutoRoam"** to **"ZHR – Zero-Handoff Roaming"**.

## Changes Made

### 1. Project Name & Binary
- **Old**: `SmartAutoRoam`
- **New**: `ZHR`
- **Version**: Updated from 0.1 to 1.0

### 2. Application Display Name
- **Full Name**: ZHR – Zero-Handoff Roaming
- **Short Name**: ZHR
- **Tagline**: Real-time, speed-aware WiFi auto-roaming with zero interruptions

### 3. Files Modified

#### Build Configuration
- **CMakeLists.txt**
  - Project name: `SmartAutoRoam` → `ZHR`
  - Version: `0.1` → `1.0`
  - Desktop file: `SmartAutoRoam.desktop` → `ZHR.desktop`
  - Icon name: `smartautoroam.svg` → `zhr.svg`

#### Source Code
- **src/main.cpp**
  - Application name: `SmartAutoRoam` → `ZHR`
  - Added display name: `ZHR – Zero-Handoff Roaming`

- **src/MainWindow.cpp**
  - Window title references updated
  - Tray icon tooltip: `SmartAutoRoam - WiFi Auto-Roaming` → `ZHR – Zero-Handoff Roaming`
  - Notification messages updated to use "ZHR"

- **src/Database.cpp**
  - Config directory: `~/.config/smart_auto_roam/` → `~/.config/zhr/`
  - Database file: `auto_roam.db` → `zhr.db`

- **ui/MainWindow.ui**
  - Window title: `SmartAutoRoam` → `ZHR – Zero-Handoff Roaming`

#### Desktop Integration
- **ZHR.desktop** (new file, replaces SmartAutoRoam.desktop)
  - Name: `ZHR`
  - GenericName: `Zero-Handoff Roaming`
  - Comment: Real-time, speed-aware WiFi auto-roaming with zero interruptions
  - Exec: `/usr/local/bin/ZHR`
  - Icon: `zhr`
  - Keywords: Added "zero-handoff" and "zhr"

#### Scripts
- **run.sh**
  - Binary name: `SmartAutoRoam` → `ZHR`
  - Messages updated to reference ZHR

- **install.sh**
  - App name: `SmartAutoRoam` → `ZHR`
  - Binary path: `/usr/local/bin/SmartAutoRoam` → `/usr/local/bin/ZHR`
  - Icon path: `smartautoroam.png` → `zhr.png`
  - Config directories: `/etc/smartautoroam/` → `/etc/zhr/`
  - Data directories: `/var/lib/smartautoroam/` → `/var/lib/zhr/`

- **uninstall.sh**
  - All paths updated to use `zhr` instead of `smartautoroam`

#### Documentation
- **CHANGELOG.md**
  - Header updated to reference "ZHR (Zero-Handoff Roaming)"

- **README.md**
  - Already updated with ZHR branding (no changes needed)

### 4. File System Paths

#### Old Paths
```
~/.config/smart_auto_roam/auto_roam.db
/usr/local/bin/SmartAutoRoam
/etc/smartautoroam/
/var/lib/smartautoroam/
/usr/share/applications/SmartAutoRoam.desktop
/usr/share/icons/hicolor/*/apps/smartautoroam.*
```

#### New Paths
```
~/.config/zhr/zhr.db
/usr/local/bin/ZHR
/etc/zhr/
/var/lib/zhr/
/usr/share/applications/ZHR.desktop
/usr/share/icons/hicolor/*/apps/zhr.*
```

### 5. Build Output
- **Executable**: `build/ZHR` (was `build/SmartAutoRoam`)
- **Auto-generated files**: All now use `ZHR_autogen/` prefix

## Migration Notes

### For Existing Users

If you have an existing installation, your old configuration will remain at:
- `~/.config/smart_auto_roam/auto_roam.db`

The new version will create a fresh configuration at:
- `~/.config/zhr/zhr.db`

To migrate your settings and logs:
```bash
# Backup old data
cp -r ~/.config/smart_auto_roam ~/.config/smart_auto_roam.backup

# Copy to new location
mkdir -p ~/.config/zhr
cp ~/.config/smart_auto_roam/auto_roam.db ~/.config/zhr/zhr.db
```

### Clean Installation

For a clean installation:
```bash
# Remove old installation (if exists)
sudo rm -f /usr/local/bin/SmartAutoRoam
sudo rm -f /usr/share/applications/SmartAutoRoam.desktop
sudo rm -f /usr/share/icons/hicolor/*/apps/smartautoroam.*

# Install new version
./install.sh
```

## Verification

After rebranding, verify:
1. ✅ Build completes successfully: `make` produces `build/ZHR`
2. ✅ Application launches with new name in title bar
3. ✅ Tray icon shows "ZHR – Zero-Handoff Roaming" tooltip
4. ✅ Configuration saved to `~/.config/zhr/zhr.db`
5. ✅ Desktop file appears in application menu as "ZHR"

## Branding Consistency

All user-facing text now uses:
- **Short form**: ZHR
- **Full form**: ZHR – Zero-Handoff Roaming
- **Description**: Real-time, speed-aware WiFi auto-roaming with zero interruptions

The em dash (–) is used consistently in the full name for professional appearance.

