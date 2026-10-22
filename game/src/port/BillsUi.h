/*  port: "Bill's UI" (not in the original) - an optional in-game UI polish, switchable with the
    original look ("Vanilla").

    The setting: NEG_UI=vanilla / bills in the environment (unset: Bill's UI). On Android the
    launcher's Settings item "In-game UI" sets it, and the in-game port menu (hold Back) switches
    it while playing (Companion.java -> SetBillsUi, applied from the next frame). Vanilla is the
    original's behaviour (plus bug fixes that apply to both, marked `port:` where they are).     */
#pragma once

namespace neg {

bool BillsUi(void);					// the current setting
void SetBillsUi(bool on);			// switch at run time (any thread; read once per frame)
void SetHoloDesign(int design);		// Bill's UI's gadget hologram: 0 off, 1 BAY, 2 CONSOLE, 3 WHEEL
void SetEnergyDesign(int design);	// Bill's UI's energy meter: 1 TUBE, 2 ORBIT, 3 DIAL (0 the original bar)

// Text metrics of the game's bitmap fonts ('SMAL' 'MED' 'LARG' 'BIG' 'TIME'), in 640x480 units.
int FontHeight(int font);
int TextWidth(int font, const char *s);

// The pieces (BillsUi.cpp; each is called only with Bill's UI on, the frame hook in both modes):
void BillsSonarReadout(int range);	// C3DRadar: the range readout, centred under the disc on a plate
void BillsScore(float dt, short x, short y);	// DrawScoreHud: the score, counting up, pop and glow
void BillsHealthBar(float dt, float health);	// instead of SetHealthBar: smooth, glow, flash, flicker
void BillsMirror(void *viewport);	// C3DVehicle::ev139: the rear-view mirror (an OMediaViewPort) is up
void BillsMirrorGone(void *viewport);	// C3DVehicle::ev140: it is about to be deleted
void BillsUiFrame(float dt);		// C2DInGameMenu::Process, at its end: shows / hides our pieces
void BillsUiShutdown(void);			// C2DInGameMenu's destructor: frees them

}
