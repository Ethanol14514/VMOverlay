#!/bin/bash
# Build script for VMOverlay

set -e  # Exit on error

echo "=== VMOverlay Build Script ==="

# Check for dependencies
echo "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "Error: cmake not found. Please install cmake."
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install g++."
    exit 1
fi

# Check for Qt6
echo "Checking for Qt6 components..."
for component in Qt6Core Qt6Widgets Qt6DBus Qt6Svg; do
    if ! pkg-config --exists $component 2>/dev/null; then
        echo "Warning: $component not found via pkg-config."
        echo "Please install Qt6 development packages:"
        echo "  Ubuntu/Debian: sudo apt install qt6-base-dev qt6-svg-dev"
        echo "  Arch Linux: sudo pacman -S qt6-base qt6-svg"
        echo "  Fedora: sudo dnf install qt6-qtbase-devel qt6-qtsvg-devel"
        break
    fi
done

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring project with CMake..."
cmake ..

# Build
echo "Building project..."
make -j$(nproc)

echo ""
echo "=== Build Complete ==="
echo "Executable: $BUILD_DIR/VMOverlay"
echo ""
echo "Usage: ./VMOverlay [VM_NAME] [--overlay OVERLAY_PATH] [--base BASE_PATH]"
echo "Example: ./VMOverlay win10 --overlay /var/lib/libvirt/images/overlay.qcow2 --base /var/lib/libvirt/images/base.qcow2"
