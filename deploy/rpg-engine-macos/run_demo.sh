#!/bin/bash

echo "🎮 RPG Engine - macOS Demo"
echo "=========================="
echo ""

if [[ -x "./CrossPlatformTest" ]]; then
    echo "Running Cross-Platform Test..."
    echo "=============================="
    ./CrossPlatformTest
    echo ""
    echo "Demo completed successfully!"
else
    echo " CrossPlatformTest not found or not executable"
    exit 1
fi

echo ""
echo "For more demos, check the other executable files in this directory."