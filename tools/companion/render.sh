#!/bin/sh
# Desktop check of the bottom-screen companion panel: renders the real CompanionView.java (with a
# small Java2D stand-in for the android.graphics / org.json calls it makes, tools/companion/shim)
# from a NEG_COMPANION_DUMP folder.  Usage: tools/companion/render.sh <dump dir> <Level> <out.png> [far]
set -e
here=$(cd "$(dirname "$0")" && pwd)
src=$here/../../android/app/src/main/java/com/sirbilliams/negatron/CompanionView.java
build=${TMPDIR:-/tmp}/neg-companion-render
rm -rf "$build"; mkdir -p "$build"
javac -nowarn -d "$build" $(find "$here/shim" -name '*.java' ! -name RenderMenu.java) "$src"
NEG_PANEL_CLOCK=${NEG_PANEL_CLOCK:-1500} java -cp "$build" com.sirbilliams.negatron.RenderPanel \
  "$1/$2_snap.json" "$1/$2_map.png" "$1" "$3" $4
