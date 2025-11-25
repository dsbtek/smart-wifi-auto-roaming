#!/bin/bash
#
# ZHR (Zero-Handoff Roaming) Universal Installer
# Installs dependencies, builds the app, registers icons,
# desktop file, and places the binary in /usr/local/bin.
#

set -e

APP_NAME="ZHR"
INSTALL_PREFIX="/usr/local"
BIN_PATH="$INSTALL_PREFIX/bin/$APP_NAME"
DESKTOP_FILE="/usr/share/applications/$APP_NAME.desktop"
ICON_PATH="/usr/share/icons/hicolor/64x64/apps/zhr.png"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "🚀 ZHR (Zero-Handoff Roaming) Installation Starting..."

# ---------------------------------------
# 1. Install Dependencies
# ---------------------------------------
echo "📦 Installing dependencies..."

apt update
apt install -y \
    cmake \
    g++ \
    qt6-base-dev \
    qt6-tools-dev \
    libqt6charts6-dev \
    qmake6 \
    libsqlite3-dev

echo "✅ Dependencies installed."

# ---------------------------------------
# 2. Build Application
# ---------------------------------------
echo "🔧 Building ZHR..."

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. || { echo "❌ CMake failed"; exit 1; }
make -j"$(nproc)" || { echo "❌ Build failed"; exit 1; }

echo "✅ Build successful."

# ---------------------------------------
# 3. Install Binary
# ---------------------------------------
echo "📌 Installing binary to $BIN_PATH..."
cp "$BUILD_DIR/$APP_NAME" "$BIN_PATH"
chmod +x "$BIN_PATH"

echo "✅ Binary installed."

# ---------------------------------------
# 4. Install Icon
# ---------------------------------------
echo "🖼 Installing icon..."

# Install multiple icon sizes for better compatibility
ICON_DIR="/usr/share/icons/hicolor"

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
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$ICON_DIR" 2>/dev/null || true
    echo "  ✓ Icon cache updated"
fi

echo "✨ Icon installed."

# ---------------------------------------
# 5. Install Desktop Entry
# ---------------------------------------
echo "📄 Installing desktop entry..."

cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Version=1.0
Type=Application
Name=ZHR
GenericName=Zero-Handoff Roaming
Comment=Real-time, speed-aware WiFi auto-roaming with zero interruptions
Exec=$BIN_PATH
Icon=zhr
Terminal=false
Categories=Network;System;Settings;
Keywords=wifi;network;roaming;wireless;connection;zero-handoff;zhr;
StartupNotify=true
StartupWMClass=ZHR
EOF

chmod 644 "$DESKTOP_FILE"

# Update desktop database
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
    echo "  ✓ Desktop database updated"
fi

echo "📎 Desktop entry installed."

# ---------------------------------------
# 6. Create App Data Directory
# ---------------------------------------
echo "📂 Creating configuration directory..."

mkdir -p /etc/zhr
mkdir -p /var/lib/zhr

echo "🌐 Installing default database (if missing)..."
if [ ! -f /var/lib/zhr/data.db ]; then
    touch /var/lib/zhr/data.db
fi

echo "🎉 Installation Complete!"
echo "You can now launch ZHR from:"
echo "  → Application Menu"
echo "  → Or run: ZHR"
