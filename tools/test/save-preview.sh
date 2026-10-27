#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT
read -r -a SDL_FLAGS <<< "${SDL_CFLAGS:-$(pkg-config --cflags sdl2)}"
read -r -a SDL_LINK <<< "${SDL_LIBS:-$(pkg-config --libs sdl2)}"
"${CXX:-g++}" -std=c++98 -Wall -Wextra "${SDL_FLAGS[@]}" \
    "$ROOT/game/src/port/SavePreview.cpp" "$ROOT/tools/test/native/save_preview_test.cpp" \
    "${SDL_LINK[@]}" -lz -o "$BUILD/preview"
"$BUILD/preview"
