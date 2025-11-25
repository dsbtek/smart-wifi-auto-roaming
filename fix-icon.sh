#!/bin/bash
#
# ZHR Icon Fix Script
# Run this if the icon is not showing after installation
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ICON_DIR="/usr/share/icons/hicolor"

echo "🔧 Fixing ZHR icon installation..."

# Install multiple icon sizes for better compatibility
echo "📦 Installing icon files..."

# Install PNG icons in various sizes
for size in 16 22 24 32 48; do
    ICON_SRC="$SCRIPT_DIR/resources/icons/wifi_${size}x${size}.png"
    ICON_DEST="$ICON_DIR/${size}x${size}/apps/zhr.png"
    if [ -f "$ICON_SRC" ]; then
        mkdir -p "$ICON_DIR/${size}x${size}/apps"
        cp "$ICON_SRC" "$ICON_DEST"
        echo "  ✓ Installed ${size}x${size} icon"
    fi
done

# Install 64x64 icon (main size)
ICON_SRC="$SCRIPT_DIR/resources/icons/wifi.png"
if [ -f "$ICON_SRC" ]; then
    mkdir -p "$ICON_DIR/64x64/apps"
    cp "$ICON_SRC" "$ICON_DIR/64x64/apps/zhr.png"
    echo "  ✓ Installed 64x64 icon"
fi

# Install SVG icon (scalable)
ICON_SVG="$SCRIPT_DIR/resources/icons/wifi.svg"
if [ -f "$ICON_SVG" ]; then
    mkdir -p "$ICON_DIR/scalable/apps"
    cp "$ICON_SVG" "$ICON_DIR/scalable/apps/zhr.svg"
    echo "  ✓ Installed scalable SVG icon"
fi

# Update icon cache
echo "🔄 Updating icon cache..."
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$ICON_DIR"
    echo "  ✓ Icon cache updated"
else
    echo "  ⚠️ gtk-update-icon-cache not found, skipping cache update"
fi

# Update desktop database
echo "🔄 Updating desktop database..."
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database /usr/share/applications
    echo "  ✓ Desktop database updated"
else
    echo "  ⚠️ update-desktop-database not found, skipping"
fi

echo ""
echo "✅ Icon fix complete!"
echo ""
echo "The icon should now appear. If it still doesn't show:"
echo "  1. Log out and log back in"
echo "  2. Or restart your desktop environment"
echo "  3. Or run: killall nautilus (for GNOME)"
echo ""

