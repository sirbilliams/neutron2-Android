#!/bin/sh
# Compare game/granny/granny_compat.cpp with the real granny.dll: every script in tools/granny/tests (or the
# ones named) is run on the DLL under Wine and natively on granny_compat, and the recorded results are
# compared (result codes and handles exactly, vertex / bone data as max abs error; 0 = bit-identical).
#
#   tools/granny/verify.sh <game>/Program_Executable_Files/grn <dir with the .grc files> [work dir] [test...]
#
# Needs i686-w64-mingw32-gcc, wine (32-bit prefix: WINEPREFIX, default /root/.wine32), g++, python3 + numpy.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
GRN=$(cd "$1" && pwd)
GRC=$(cd "$2" && pwd)
WORK=${3:-${TMPDIR:-/tmp}/grnverify}
[ $# -ge 3 ] && shift 3 || shift $#
mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)

i686-w64-mingw32-gcc -O2 -o "$WORK/grntest.exe" "$HERE/grntest.c"
cp "$GRN/../granny.dll" "$WORK/granny.dll"
g++ -O2 -ffp-contract=off -c "$ROOT/game/granny/granny_compat.cpp" -o "$WORK/granny_compat.o"
gcc -O2 -DGRNTEST_COMPAT -I"$ROOT/game/granny" -c "$HERE/grntest.c" -o "$WORK/grntest.o"
g++ -o "$WORK/grntest" "$WORK/grntest.o" "$WORK/granny_compat.o" -lm

winpath() { echo "Z:$1" | tr / '\\'; }
TESTS="$*"
[ -n "$TESTS" ] || TESTS=$(cd "$HERE/tests" && ls *.txt | sed 's/\.txt$//')
export WINEPREFIX=${WINEPREFIX:-/root/.wine32} WINEARCH=win32 WINEDEBUG=-all
status=0
for t in $TESTS; do
	(cd "$WORK" && wine grntest.exe "$(winpath "$HERE/tests/$t.txt")" "$(winpath "$WORK/$t.dll.gtst")" "$(winpath "$GRN")")
	"$WORK/grntest" "$HERE/tests/$t.txt" "$WORK/$t.compat.gtst" "$GRC"
	echo "== $t"
	python3 "$HERE/grntest.py" cmp "$WORK/$t.dll.gtst" "$WORK/$t.compat.gtst" || status=1
done
exit $status
