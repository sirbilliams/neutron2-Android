/*  port: widescreen (not in the original).

    The original draws at 640x480 (4:3). With widescreen on, the logical window is as wide as the
    panel's aspect at the same 480 lines (854x480 on a 16:9 panel) and:
      - the 3D world fills the whole width with the original vertical field of view (Hor+: OMT's
        FOV projection keeps the vertical angle and derives the horizontal one from the viewport),
        so more of the world is visible at the sides and nothing is stretched;
      - the game's 2D screen (g_screen_w x g_screen_h, still 640x480) is the original 4:3 area,
        centred: the 2D layers' ortho projection is centred on the window, so every picture, menu
        and text keeps its 4:3 shape and position inside that area (4:3 pictures get side bars);
      - HUD pieces that belong at a screen edge move out to it by WideMargin() (see the callers),
        full-screen effects (letterbox, fades) are widened to cover the whole window, and window-
        space rectangles / mouse positions are shifted between the window and the 4:3 area.

    The setting: NEG_WIDESCREEN=1 / 0 in the environment; off by default (unfinished: only the
    3D Hor+ part and the centred 4:3 2D area are done - see the TODO list below).

    TODO: anchor the edge HUD pieces (health/neutrons top left, gadget bar and HelpMenu bottom,
    f_5b0 top right) by WideMargin(); widen the full-screen effects (LetterBoxTop/Bottom morphs
    in C2DInGameMenu_b.cpp CutsceneLetterbox, the fades); shift window-space rectangles (the cut
    scenes' picture-in-picture viewports in C3DCutSceneCamera / C3DMultiCutSceneCamera) and
    the mouse / touch positions (window -> 4:3 area: x - WideMargin()); an Android SetupActivity
    option that sets NEG_WIDESCREEN; check the cut scenes widened.                               */
#pragma once

namespace neg {

bool WidescreenOn(void);			// the setting (decided once, before the window is made)
void WidescreenSetup(void);			// CGfx::InitDisplay: size the logical window for the setting

// Window (logical) size and the 4:3 area's margin at each side, in the game's screen units
// (480 lines). WideMargin() is 0 with widescreen off (or on a 4:3 panel).
int WindowWidth(void);
int WindowHeight(void);
float WideMargin(void);

// HUD pieces that belong at a screen edge: registered once (the sprite or InterfaceElt pointer)
// with the side they hug; the placing functions (C2DInGameMenu::SetupSprite / PositionSprite,
// InterfaceElt's placement) add WideShift() to their x, so later moves keep the anchoring.
// Register before the first placement. WideAnchorClear forgets them all (menu destructor).
enum { WIDE_LEFT = -1, WIDE_RIGHT = 1 };
void WideAnchor(const void *piece, int side);
void WideAnchorClear(void);
float WideShift(const void *piece);			// -WideMargin(), +WideMargin() or 0
// An x in the 640x480 space moved to the window edge on `side` (DrawText positions etc.).
inline short WideX(short x, int side) { return (short)(x + side * (int)WideMargin()); }

}
