#!/bin/bash
# Qt/mac ARM Debug build script

set -e

BUILD_DIR=build-mac-arm-debug
QT_VERSION="6.10.1"
QT_PREFIX="$HOME/Qt/${QT_VERSION}/macos"

rm -rf $BUILD_DIR

echo "Using Qt from: $QT_PREFIX"

mkdir -p $BUILD_DIR
cd $BUILD_DIR
rm -rf *

cmake .. \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
    -DCMAKE_BUILD_TYPE=Debug

cmake --build .

echo "Debug build complete!"
echo "App bundle: $(pwd)/FFAB.app"
echo "Run with: open FFAB.app"