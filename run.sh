#!/bin/bash
#
# SmartAutoRoam - Universal Run Script
# ------------------------------------------------------------
# Features:
#  - Auto-detect missing build/ and trigger CMake + Make
#  - Checks Qt and QtCharts availability
#  - Prevents snap library conflicts
#  - Logs runtime crashes to logs/app.log
#  - Runs SmartAutoRoam cleanly on any Linux distro
#

set -e

# ---------------------------------------
# 1. FIX SNAP + GTK ENVIRONMENT CONFLICTS
# ---------------------------------------
unset GTK_PATH
unset LD_LIBRARY_PATH
unset LD_PRELOAD
export GTK_MODULES=""

# ---------------------------------------
# 2. DETECT PROJECT ROOT + BUILD DIR
# ---------------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BINARY="$BUILD_DIR/SmartAutoRoam"

# ---------------------------------------
# 3. ENSURE LOGS FOLDER EXISTS
# ---------------------------------------
mkdir -p "$SCRIPT_DIR/logs"

# ---------------------------------------
# 4. CHECK Qt INSTALLATION
# ---------------------------------------
check_qt() {
    if ! dpkg -s qtbase5-dev &>/dev/null; then
        echo "⚠️  Qt5 development package not found!"
        echo "Install using:"
        echo "  sudo apt install qtbase5-dev qttools5-dev qtcharts5-dev libqt5charts5-dev"
    fi
}

check_qt

# ---------------------------------------
# 5. BUILD IF MISSING
# ---------------------------------------
if [ ! -f "$BINARY" ]; then
    echo "🔧 No compiled binary found. Building..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    cmake .. || { echo "❌ CMake configuration failed"; exit 1; }
    make -j"$(nproc)" || { echo "❌ Build failed"; exit 1; }

    echo "✅ Build completed!"
fi

# ---------------------------------------
# 6. RUN THE APP WITH LOGGING
# ---------------------------------------
echo "🚀 Launching SmartAutoRoam..."
echo "📄 Logs: $SCRIPT_DIR/logs/app.log"

cd "$BUILD_DIR"

# Run and capture any crash logs
./SmartAutoRoam "$@" 2>> "$SCRIPT_DIR/logs/app.log"
