#!/bin/bash
# Wrapper script to run SmartAutoRoam without snap library conflicts

# Clear problematic environment variables
unset GTK_PATH
unset LD_LIBRARY_PATH
unset LD_PRELOAD

# Change to build directory
cd "$(dirname "$0")/build" || exit 1

# Run the application
exec ./SmartAutoRoam "$@"

