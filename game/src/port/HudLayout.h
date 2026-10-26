// port: edge-aware score animation; keep the original 4:3 trajectory and timing.
#pragma once
namespace neg {
inline int ScorePopupX(bool aside, double timeLeft, float margin) {
    if (timeLeft > 0.2) return aside ? 450 + (int)margin : 300;
    double elapsed = 0.2 - timeLeft;
    if (elapsed < 0) elapsed = 0;
    if (elapsed > 0.2) elapsed = 0.2;
    const int original = aside ? (int)(elapsed * 400.0 + 450.0) : (int)(elapsed * 1500.0 + 300.0);
    return original + (int)(margin * (aside ? 1.0 : elapsed / 0.2));
}
}
