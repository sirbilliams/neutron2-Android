#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT
"${CXX:-g++}" -std=c++98 -Wall -Wextra -Werror -I"$ROOT/engine/sdl" \
    "$ROOT/engine/sdl/OMediaHDPng.cpp" "$ROOT/tools/test/native/hd_png_probe.cpp" -lz -o "$BUILD/png"
python3 "$ROOT/tools/test/native/png_test.py" "$BUILD/png"
"${CXX:-g++}" -std=c++98 -Wall -Wextra -Werror -I"$ROOT/game/src/port" \
    "$ROOT/tools/test/native/hud_layout_test.cpp" -o "$BUILD/hud"
"$BUILD/hud"

"${CXX:-g++}" -std=c++98 -Wall -Wextra -Werror -I"$ROOT/engine/sdl" \
    "$ROOT/tools/test/native/control_settings_test.cpp" -o "$BUILD/controls"
"$BUILD/controls"
