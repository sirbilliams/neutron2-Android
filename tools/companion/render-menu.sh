#!/bin/sh
# Desktop check of the launcher menu: renders the real LauncherView.java pages (main and bottom screen)
# through the Java2D shim (tools/companion/shim), reading the save slots of a game folder.
# Usage: tools/companion/render-menu.sh <game folder> <out dir> [picture.png from NEG_MENU_ART]
set -e
here=$(cd "$(dirname "$0")" && pwd)
src=$here/../../android/app/src/main/java/com/sirbilliams/negatron
build=${TMPDIR:-/tmp}/neg-menu-render
rm -rf "$build"; mkdir -p "$build"
javac -nowarn -d "$build" $(find "$here/shim" -name '*.java') "$src/LauncherModel.java" "$src/LauncherPages.java" "$src/LauncherView.java" "$src/CompanionView.java"
NEG_PANEL_CLOCK=${NEG_PANEL_CLOCK:-1500} java -cp "$build" com.sirbilliams.negatron.RenderMenu "$@"
