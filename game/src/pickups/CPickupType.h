/*  CPickupType - the base of the pickups (C3DPickupItem): a C3DTriggerType with a slot in the
    savegame's object states ("PickupIndex", handed out by the level as it loads: 0 = not saved)
    and a menu pickup number ("PIC_NUMBER"). Collected (state != 0) it is hidden and does not
    collide. With a "TaskName" the pickup is only there while the task's state is
    "ExactLevel", or at least "RequiredLevel".
    Reconstructed from Neutron2.exe: constructor 0x496c20, destructor 0x496d20 (only resets the
    vtables, then ~C3DTriggerType; deleting thunks 0x496cf0 / 0x496fa0 / 0x496fb0). Vtables
    0x516a00 (element, 56 slots: C3DTriggerType's) / 0x5169ec (listener) / 0x516570 (game
    object: 7, 242, 260, 266, 267 overridden; 266 is code shared with C3DArrow) / 0x516560
    (streamer). No factory ID. Own members 0x7a0..0x7b0 (vtordisp 0x7b0, streamer 0x7b4 when
    it stands alone; C3DPickupItem puts its members from 0x7b0).

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DTriggerType.h"

class CPickupType : public C3DTriggerType
{
	public:
	CPickupType();															// 0x496c20
	virtual ~CPickupType();													// 0x496d20

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x496d60
	virtual void v242(float t);												// SLOT 242 0x496e70 editor: shown
	virtual void v260(void);												// SLOT 260 0x496de0 PickupIndex from the level; shown unless collected
	virtual void v266(void);												// SLOT 266 0x4368e0 the task's state set to itself
	virtual void v267(int level);											// SLOT 267 0x496ea0 TaskName state: shown / hidden

	// FIELD 0x7a0 0x10
	int			m_pickup_index;			// 0x7a0  "PickupIndex" (= 0; v260: the next object state index)
	int			m_pic_number;			// 0x7a4  "PIC_NUMBER" (= -1: no menu item)
	int			m_required_level;		// 0x7a8  "RequiredLevel" (= 0)
	int			m_exact_level;			// 0x7ac  "ExactLevel" (= -1)
};

namespace neg {
// The PickupIndex of a pickup (C3DAnimated's PickupLink reads it at game object +0x6d8);
// -1 when o is not a CPickupType.
int PickupIndexOf(CGameObject *o);
}
