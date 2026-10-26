#!/usr/bin/env bash
# Compile the real installer against minimal Android type stubs, in a disposable directory.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT
mapfile -t SOURCES < <(find "$ROOT/tools/test/hdpack" -name '*.java' -print)
javac -d "$BUILD" "${SOURCES[@]}" "$ROOT/android/app/src/main/java/com/sirbilliams/negatron/HdPack.java"
java -cp "$BUILD" com.sirbilliams.negatron.HdPackTest
