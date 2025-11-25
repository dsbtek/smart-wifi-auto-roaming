#!/bin/bash
#
# ZHR (Zero-Handoff Roaming) Uninstaller
#

set -e

APP_NAME="ZHR"
BIN_PATH="/usr/local/bin/$APP_NAME"
DESKTOP_FILE="/usr/share/applications/$APP_NAME.desktop"
ICON_PATH="/usr/share/icons/hicolor/64x64/apps/zhr.png"
DATA_DIR="/var/lib/zhr"
CONF_DIR="/etc/zhr"

echo "🧹 Starting ZHR Uninstallation..."

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

    # Update desktop database
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database /usr/share/applications 2>/dev/null || true
    fi
fi

# ---------------------------------------
# 3. Remove icons
# ---------------------------------------
echo "❌ Removing icons..."
ICON_DIR="/usr/share/icons/hicolor"

# Remove PNG icons in various sizes
for size in 16 22 24 32 48 64; do
    rm -f "$ICON_DIR/${size}x${size}/apps/zhr.png"
done

# Remove SVG icon
rm -f "$ICON_DIR/scalable/apps/zhr.svg"

# Update icon cache
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$ICON_DIR" 2>/dev/null || true
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
