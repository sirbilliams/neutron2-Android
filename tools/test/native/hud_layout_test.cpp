#include "HudLayout.h"
#include <stdio.h>
#include <stdlib.h>
static void check(bool ok) { if (!ok) { fputs("HUD layout regression\n", stderr); exit(1); } }
int main() {
    // Preserve the 4:3 trajectory exactly; a side pop-up stays anchored throughout its flight.
    check(neg::ScorePopupX(false, 1.0, 0) == 300);
    check(neg::ScorePopupX(true, 1.0, 0) == 450);
    check(neg::ScorePopupX(false, 0.1, 0) == 450);
    check(neg::ScorePopupX(true, 0.1, 0) == 490);
    check(neg::ScorePopupX(false, 1.0, 107) == 300);
    check(neg::ScorePopupX(true, 1.0, 107) == 557);
    check(neg::ScorePopupX(false, 0.1, 107) == 503);
    check(neg::ScorePopupX(true, 0.1, 107) == 597);
    check(neg::ScorePopupX(false, 0.0, 107) == 707);
    check(neg::ScorePopupX(true, 0.0, 107) == 637);
    puts("HUD 4:3 / widescreen regression checks passed");
}
