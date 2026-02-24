#!/bin/bash
# FFAB Release Build Script (Ninja) — signed + notarized
# ./scripts/ninja-release.sh              <- ARM release + signed DMG
# ./scripts/ninja-release.sh universal    <- Universal (arm64+x86_64) release + signed DMG
# RUN:
# ./scripts/ninja-release.sh && ./scripts/ninja-release.sh universal

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
QT_VERSION="6.10.1"
QT_PREFIX="$HOME/Qt/${QT_VERSION}/macos"
MACDEPLOYQT="$QT_PREFIX/bin/macdeployqt"

# Code signing & notarization
SIGNING_ID="Developer ID Application: Daniel Findlay (Y9MKGP7V9D)"
NOTARY_PROFILE="Notary-Password"

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

VERSION=$(sed -n 's/.*VERSION_STR "\([^"]*\)".*/\1/p' "$PROJECT_DIR/src/Core/AppConfig.h")

# Deploy Qt frameworks (must happen before codesign)
echo "📦 Deploying Qt frameworks..."
"$MACDEPLOYQT" FFAB.app
echo ""

# Sign with hardened runtime (required for notarization)
echo "🔏 Code signing FFAB.app..."
codesign --deep --force --verify --verbose \
    --sign "$SIGNING_ID" \
    --options runtime \
    FFAB.app
codesign --verify --deep --strict FFAB.app
echo ""

# Create DMG
echo "📀 Creating DMG..."
hdiutil create -volname "FFAB" \
    -srcfolder FFAB.app \
    -ov -format UDZO \
    "FFABv${VERSION}.dmg"
codesign --force --sign "$SIGNING_ID" "FFABv${VERSION}.dmg"
echo ""

# Notarize (blocks while Apple scans, typically 2-5 min)
echo "🍎 Notarizing FFABv${VERSION}.dmg..."
xcrun notarytool submit "FFABv${VERSION}.dmg" \
    --keychain-profile "$NOTARY_PROFILE" \
    --wait
echo ""

# Staple ticket (embeds approval for offline verification)
echo "📎 Stapling notarization ticket..."
xcrun stapler staple "FFABv${VERSION}.dmg"
echo ""

echo "✅ macOS $ARCH_LABEL release build complete!"
echo "  App bundle: $BUILD_DIR/FFAB.app (signed)"
echo "  DMG ready:  $BUILD_DIR/FFABv${VERSION}.dmg (signed + notarized)"
