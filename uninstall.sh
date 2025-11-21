#!/bin/bash
#
# SmartAutoRoam Uninstaller
#

set -e

APP_NAME="SmartAutoRoam"
BIN_PATH="/usr/local/bin/$APP_NAME"
DESKTOP_FILE="/usr/share/applications/$APP_NAME.desktop"
ICON_PATH="/usr/share/icons/hicolor/64x64/apps/smartautoroam.png"
DATA_DIR="/var/lib/smartautoroam"
CONF_DIR="/etc/smartautoroam"

echo "🧹 Starting SmartAutoRoam Uninstallation..."

# ---------------------------------------
# 1. Remove binary
# ---------------------------------------
if [ -f "$BIN_PATH" ]; then
    echo "❌ Removing binary..."
    rm -f "$BIN_PATH"
fi

# ---------------------------------------
# 2. Remove desktop file
# ---------------------------------------
if [ -f "$DESKTOP_FILE" ]; then
    echo "❌ Removing desktop entry..."
    rm -f "$DESKTOP_FILE"
fi

# ---------------------------------------
# 3. Remove icon
# ---------------------------------------
if [ -f "$ICON_PATH" ]; then
    echo "❌ Removing icon..."
    rm -f "$ICON_PATH"
fi

# ---------------------------------------
# 4. Offer to remove app data
# ---------------------------------------
echo "📁 Optional: Remove configuration and database? (y/n)"
read -r choice

if [ "$choice" = "y" ]; then
    rm -rf "$DATA_DIR"
    rm -rf "$CONF_DIR"
    echo "🗑 App data removed."
else
    echo "⚠️ App data preserved."
fi

echo "🎯 Uninstallation Complete."
