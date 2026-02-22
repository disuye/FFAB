#!/bin/bash
# FFAB Release Build Script (Ninja)
# ./scripts/release.sh              <- ARM release + DMG
# ./scripts/release.sh universal    <- Universal (arm64+x86_64) release + DMG
# RUN:
# ./scripts/ninja-release.sh && ./scripts/ninja-release.sh universal 

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
QT_VERSION="6.10.1"
QT_PREFIX="$HOME/Qt/${QT_VERSION}/macos"
MACDEPLOYQT="$QT_PREFIX/bin/macdeployqt"

# Parse arguments
ARCH_FLAGS=""
ARCH_LABEL="ARM"
BUILD_DIR="$PROJECT_DIR/build-mac-arm-release"

case "$1" in
    universal)
        ARCH_FLAGS="-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0"
        ARCH_LABEL="Universal (arm64+x86_64)"
        BUILD_DIR="$PROJECT_DIR/build-mac-universal-release"
        ;;
    "")
        ;;
    *)
        echo "Usage: $0 [universal]"
        exit 1
        ;;
esac

echo "Building $ARCH_LABEL Release..."
echo "Using Qt from: $QT_PREFIX"
echo ""

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure
echo "⚙️  Configuring with CMake (Ninja)..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
    $ARCH_FLAGS
echo ""

# Build
echo "🔨 Building FFAB ($ARCH_LABEL Release)..."
cd "$BUILD_DIR"
time ninja
echo ""

# Package DMG
echo "📦 Packaging DMG..."
"$MACDEPLOYQT" FFAB.app -dmg
echo ""
VERSION=$(sed -n 's/.*VERSION_STR "\([^"]*\)".*/\1/p' "$PROJECT_DIR/src/Core/AppConfig.h")
mv "$BUILD_DIR/FFAB.dmg" "$BUILD_DIR/FFABv${VERSION}.dmg"
echo "  DMG ready:  $BUILD_DIR/FFABv${VERSION}.dmg"

echo "✓ macOS $ARCH_LABEL release build complete!"
echo "  App bundle: $BUILD_DIR/FFAB.app"
echo "  DMG ready:  $BUILD_DIR/FFAB.dmg"
