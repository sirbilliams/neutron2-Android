/*  C3DFadeObject ('3FAD') - a trigger that fades the screen: when fired it starts the in-game
    menu's fade "FadeType" (4: to black) over "FadeTime" seconds, holds it black "StayBlack"
    seconds, fades back in over "FadeInTime" and then fires "NextTrigger". When the cut scene is
    being skipped it fires NextTrigger at once. Level3's "fadecam2" (after the underwater flight
    of the C3DFlyingCamera "labflyin1") is one.
    Reconstructed from Neutron2.exe: constructor 0x431180 (factory '3FAD', 0x7b0 bytes),
    destructor 0x431390 (only resets the vtables, then ~C3DTriggerType; deleting thunks
    0x431360 / 0x431490 / 0x4314a0). Vtables 0x4d9d20 (element, 57 slots: C3DTriggerType's 56
    + 56) / 0x4d9d0c (listener) / 0x4d9890 (game object, slots 7 and 16 overridden; 16 is a
    copy of C3DSpriteType's) / 0x4d9880 (streamer). Own members 0x7a0..0x7a8, vtordisp 0x7a8,
    the streamer at 0x7ac.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DTriggerType.h"

class C3DFadeObject : public C3DTriggerType
{
	public:
	C3DFadeObject();														// 0x431180
	virtual ~C3DFadeObject();												// 0x431390

	// ---- element-family slots it adds
	virtual void Trigger(void);												// ESLOT 56 0x431420 (+0xe0) fire: start the fade

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4313e0

	// FIELD 0x7a0 0x8
	float		m_fade_in_time;			// 0x7a0  "FadeInTime" (= 1.0)
	float		m_stay_black;			// 0x7a4  "StayBlack" (= 0.5)
};

namespace neg {
// Trigger (element slot 56) of a C3DFadeObject known as a CGameObject (C3DTriggerType).
void FadeObjectTrigger(CGameObject *o);
// 0x406910: start the in-game menu's fade `mode` over `t` seconds, held `hold` seconds, then
// faded back over `in_time`, then `trigger` is activated (C2DInGameMenu::v341). With fades
// off the trigger is activated at once.
void StartTriggerFade(int mode, float t, float hold, float in_time, const char *trigger);
}
