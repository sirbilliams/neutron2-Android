/*  C3DCindy ('3CIN') - Cindy: a Granny friend (model "cindybase.grn"; WALK, TALK, STOP, LOVE,
    LOVETALK, LOVEIDLE, DISGTALK, IDLE, TALKFLURP, DRINKFLURP, HOLDFLURP), patrolling
    (AIState 2) at 160, VisibleRange 500, shadow 80 below her origin. A cut scene that finds her
    stopped makes her IDLE.
    Reconstructed from Neutron2.exe: constructor 0x427600 (factory '3CIN', 0x1d7c bytes),
    destructor 0x4277e0 (deleting thunks 0x4277b0 / 0x427b30 / 0x427b40 / 0x427b50).

    Original layout: as C3DFriends (element vtable 0x4d4500, 147 slots; listener 0x4d44ec;
    OMediaAnim 0x4d44b0; CLocalGameObject vtable 0x4d4034 at +0xc0), no members of its own.  */
#pragma once
#include "C3DFriends.h"

class C3DCindy : public C3DFriends
{
	public:
	C3DCindy();																// 0x427600
	virtual ~C3DCindy();													// 0x4277e0

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60 0x4369f0 no such animation: IDLE
	virtual void ev69(void);												// ESLOT 69 0x427830 load the model and the animations (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void v260(void);												// SLOT 260 0x417110
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x427a40 "C3DCindy::ChangeState": IDLE
};
