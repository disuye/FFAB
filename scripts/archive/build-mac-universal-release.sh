#!/bin/bash
# Qt Universal Release build script

set -e

BUILD_DIR=build-mac-universal-release
QT_VERSION="6.10.1"
QT_PREFIX="$HOME/Qt/${QT_VERSION}/macos"
MACDEPLOYQT="$QT_PREFIX/bin/macdeployqt"

rm -rf $BUILD_DIR

echo "Using Qt from: $QT_PREFIX"

mkdir -p $BUILD_DIR
cd $BUILD_DIR
rm -rf *

cmake .. \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release

cmake --build .

"$MACDEPLOYQT" FFAB.app -dmg

echo "macOS Intel/ARM Universal release build complete!"
echo "DMG ready: FFAB.dmg"
echo "App bundle: FFAB.app"