/*  C3DSheen ('3SHE') - Sheen: a Granny friend (model "sheenbase.grn", one animation IDLE
    (sheenidle) used for running, waiting and sitting too), patrolling (AIState 2) at 160,
    VisibleRange 500. Reset puts him back at his start position.
    Reconstructed from Neutron2.exe: constructor 0x46ecd0 (factory '3SHE', 0x1d7c bytes),
    destructor 0x46eea0 (deleting thunks 0x46ee70 / 0x46f0f0 / 0x46f100 / 0x46f110).

    Original layout: as C3DFriends (element vtable 0x4f7730, 147 slots; listener 0x4f771c;
    OMediaAnim 0x4f76e0; CLocalGameObject vtable 0x4f7264 at +0xc0), no members of its own.
    Slot 260 is 0x46e3c0, a jump to C3DAI's (so not overridden here).                        */
#pragma once
#include "C3DFriends.h"

class C3DSheen : public C3DFriends
{
	public:
	C3DSheen();																// 0x46ecd0
	virtual ~C3DSheen();													// 0x46eea0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69  0x46eef0 load the model and the animation (once)
	virtual void ev145(void);												// ESLOT 145 0x44fa60

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void ResetObject(void);											// SLOT 10  0x46f020 back to the start position
	virtual void InitPhysics(void);											// SLOT 11  0x43ba20 (unless f_4aa)
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x471950
};
