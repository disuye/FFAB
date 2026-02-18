#!/bin/bash

set -e

SH_DIR=scripts

# ./${SH_DIR}/build-mac-arm-debug.sh

# echo "───────────────── macOS ARM debug complete ─────────────────"

./${SH_DIR}/build-mac-arm-release.sh

echo "───────────────── macOS ARM release complete ─────────────────"

# ./${SH_DIR}/build-mac-universal-debug.sh

# echo "───────────────── macOS Universal debug complete ─────────────────"

./${SH_DIR}/build-mac-universal-release.sh

echo "───────────────── macOS Universal release complete ─────────────────"
echo "All macOS builds complete."