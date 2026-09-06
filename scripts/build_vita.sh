#!/usr/bin/env bash
set -euo pipefail

: "${VITASDK:?Set VITASDK to your VitaSDK installation}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-vita}"
JOBS="${JOBS:-2}"

python3 "$ROOT/scripts/verify_vita_package.py"

cmake -S "$ROOT" -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" \
  -DBUILD_VITA=ON \
  -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" -j"$JOBS"

VPK="$BUILD_DIR/sight_city_underworld.vpk"
if [ ! -f "$VPK" ]; then
  echo "ERROR: Vita build completed without producing $VPK" >&2
  exit 3
fi

echo "OK: $VPK"
