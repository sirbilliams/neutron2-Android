/*  C3DTextTrigger ('3TEX') - a trigger that shows (or, with "EnableTrigger" 0, clears) the
    in-game menu's typed caption: the TRIGTEXTSTRINGs of the group "TextTrigName", typed at
    "DrawSpeed" characters per second. With "DisableTime" > 0 the caption is cleared that many
    seconds later; a skipped cut scene clears it at once. Then it fires NextTrigger at once.
    Reconstructed from Neutron2.exe: constructor 0x4767d0 (factory '3TEX', 0x81c bytes),
    destructor 0x476a20 (only resets the vtables, then ~C3DTriggerType; deleting thunks 0x4769f0
    / 0x476ca0 / 0x476cb0). Vtables 0x4ffe4c (element, 57 slots: C3DTriggerType's 56 + 56) /
    0x4ffe38 (listener) / 0x4ff9bc (game object, slots 7 and 240 overridden) / 0x4ff9ac
    (streamer). Own members 0x7a0..0x814, vtordisp 0x814, the streamer at 0x818.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DTriggerType.h"

class C3DTextTrigger : public C3DTriggerType
{
	public:
	C3DTextTrigger();														// 0x4767d0
	virtual ~C3DTextTrigger();												// 0x476a20

	// ---- element-family slots it adds
	virtual void Trigger(void);												// ESLOT 56 0x476bd0 (+0xe0) fire: show / clear the caption

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x476b50
	virtual void Process(float dt);											// SLOT 240 0x476a60 DisableTime countdown

	// FIELD 0x7a0 0x74
	char		m_text_trig_name[100];	// 0x7a0  "TextTrigName" (= "none")
	float		m_draw_speed;			// 0x804  "DrawSpeed" (= 12)
	int			m_enable_trigger;		// 0x808  "EnableTrigger" (= 1; 0: clear the caption)
	float		m_disable_time;			// 0x80c  "DisableTime" (= -1: stays)
	float		m_time_left;			// 0x810  until the caption is cleared
};

namespace neg {
// Trigger (element slot 56) of a C3DTextTrigger known as a CGameObject (C3DTriggerType).
void TextTriggerTrigger(CGameObject *o);
}
