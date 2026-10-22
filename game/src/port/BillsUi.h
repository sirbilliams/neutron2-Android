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

// Text metrics of the game's bitmap fonts ('SMAL' 'MED' 'LARG' 'BIG' 'TIME'), in 640x480 units.
int FontHeight(int font);
int TextWidth(int font, const char *s);

// The pieces (BillsUi.cpp; each is called only with Bill's UI on, the frame hook in both modes):
void BillsSonarReadout(int range);	// C3DRadar: the range readout, centred under the disc on a plate
void BillsUiFrame(float dt);		// C2DInGameMenu::Process, at its end: shows / hides our pieces
void BillsUiShutdown(void);			// C2DInGameMenu's destructor: frees them

}
