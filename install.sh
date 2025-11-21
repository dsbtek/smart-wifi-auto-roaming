#!/bin/bash
#
# SmartAutoRoam Universal Installer
# Installs dependencies, builds the app, registers icons,
# desktop file, and places the binary in /usr/local/bin.
#

set -e

APP_NAME="SmartAutoRoam"
INSTALL_PREFIX="/usr/local"
BIN_PATH="$INSTALL_PREFIX/bin/$APP_NAME"
DESKTOP_FILE="/usr/share/applications/$APP_NAME.desktop"
ICON_PATH="/usr/share/icons/hicolor/64x64/apps/smartautoroam.png"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "🚀 SmartAutoRoam Installation Starting..."

# ---------------------------------------
# 1. Install Dependencies
# ---------------------------------------
echo "📦 Installing dependencies..."

apt update
apt install -y \
    cmake \
    g++ \
    qtbase5-dev \
    qttools5-dev \
    qtcharts5-dev \
    libqt5charts5-dev \
    qt5-qmake \
    libsqlite3-dev

echo "✅ Dependencies installed."

# ---------------------------------------
# 2. Build Application
# ---------------------------------------
echo "🔧 Building SmartAutoRoam..."

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

# Optional: convert SVG to PNG
ICON_SRC="$SCRIPT_DIR/resources/icons/wifi.png"
if [ -f "$ICON_SRC" ]; then
    cp "$ICON_SRC" "$ICON_PATH"
else
    echo "⚠️ No PNG icon found. Install will continue."
fi

echo "✨ Icon installed."

# ---------------------------------------
# 5. Install Desktop Entry
# ---------------------------------------
echo "📄 Installing desktop entry..."

cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Name=Smart Auto Roam
Comment=Automatically connects to the strongest WiFi available
Exec=$BIN_PATH
Icon=smartautoroam
Terminal=false
Type=Application
Categories=Network;Utility;
EOF

chmod 644 "$DESKTOP_FILE"

echo "📎 Desktop entry installed."

# ---------------------------------------
# 6. Create App Data Directory
# ---------------------------------------
echo "📂 Creating configuration directory..."

mkdir -p /etc/smartautoroam
mkdir -p /var/lib/smartautoroam

echo "🌐 Installing default database (if missing)..."
if [ ! -f /var/lib/smartautoroam/data.db ]; then
    touch /var/lib/smartautoroam/data.db
fi

echo "🎉 Installation Complete!"
echo "You can now launch SmartAutoRoam from:"
echo "  → Application Menu"
echo "  → Or run: SmartAutoRoam"
