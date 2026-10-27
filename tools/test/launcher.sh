#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT
SRC="$ROOT/android/app/src/main/java/com/sirbilliams/negatron"
javac -d "$BUILD" "$SRC/ControlProfile.java" "$SRC/SavePreviewInfo.java" "$SRC/LauncherModel.java" "$SRC/LauncherPages.java" \
    "$ROOT/tools/test/launcher/com/sirbilliams/negatron/LauncherFeatureTest.java"
java -cp "$BUILD" com.sirbilliams.negatron.LauncherFeatureTest
