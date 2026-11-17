#!/bin/bash
# Build only; no acquisition, signing, publication or HID execution.
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo 'macOS host required' >&2; exit 2; }
: "${QTDIR:?Set QTDIR to the approved, provenance-recorded Qt SDK}"
: "${PERIVOLT_ARCH:?Set arm64 or x86_64 explicitly for this build}"
: "${PERIVOLT_MACOS_MIN:?Set deployment target from the selected SDK compatibility floor}"
case "$PERIVOLT_ARCH" in arm64|x86_64) ;; *) echo 'Unsupported architecture' >&2; exit 2;; esac
repo=$(cd "$(dirname "$0")/../.." && pwd)
build="${PERIVOLT_BUILD_DIR:-$repo/.local/macos-$PERIVOLT_ARCH}"
cmake -S "$repo/app" -B "$build" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$QTDIR" -DCMAKE_OSX_ARCHITECTURES="$PERIVOLT_ARCH" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$PERIVOLT_MACOS_MIN" \
  -DBUILD_TESTING=ON -DPERIVOLT_BUILD_UI=ON
cmake --build "$build" --parallel 2
ctest --test-dir "$build" --output-on-failure
printf 'Candidate bundle: %s/perivolt_monitor.app\n' "$build"

