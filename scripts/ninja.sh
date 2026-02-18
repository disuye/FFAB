#!/bin/bash
# FFAB Ninja Build Script
# ./build.sh            <- Debug build (incremental)
# ./build.sh release    <- Release build
# ./build.sh runBuild   <- then launch app
# ./build.sh clean      <- Delete build directory

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build-ninja"
BUILD_TYPE="Debug"
#----------------------------
# Uncomment for latest macOS:
#----------------------------
QT_PATH="$HOME/Qt/6.10.1/macos"
#----------------------------
# Uncomment for legacy Linux:
# + run scripts/dummyAGL.sh
#----------------------------
# QT_PATH="$HOME/Qt/6.7.3/macos"


# Go... 
rm -rf $BUILD_DIR

# Parse arguments
case "$1" in
    clean)
        echo "🧹 Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        echo "✓ Clean complete"
        exit 0
        ;;
    release)
        BUILD_TYPE="Release"
        ;;
    run)
        "$0"
        echo ""
        echo "🚀 Running FFAB..."
        open "$BUILD_DIR/FFAB.app" 2>/dev/null || "$BUILD_DIR/FFAB"
        exit 0
        ;;
    "")
        ;;
    *)
        echo "Usage: $0 [clean|release|run]"
        exit 1
        ;;
esac

# Create build directory if needed
mkdir -p "$BUILD_DIR"

# Configure with CMake (only if needed)
if [ ! -f "$BUILD_DIR/build.ninja" ] || [ "$PROJECT_DIR/CMakeLists.txt" -nt "$BUILD_DIR/build.ninja" ]; then
    echo "⚙️  Configuring with CMake (Ninja)..."
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_PREFIX_PATH="$QT_PATH" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        $AGL_FIX
    echo ""
fi

# Build
echo "🔨 Building FFAB ($BUILD_TYPE)..."
cd "$BUILD_DIR"
time ninja

echo ""
echo "✓ Build complete: $BUILD_DIR/FFAB.app"