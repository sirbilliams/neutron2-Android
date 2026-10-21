/*  C3DFowl ('3FOW') - Miss Fowl, Jimmy's teacher (in the museum): a Granny friend (model
    "fowlbase.grn"; TALK, YELL (fowldino), STOP, IDLE, TALK2 (fowltalknorm)), patrolling
    (AIState 2) at 160, VisibleRange 500. A cut scene that finds her stopped makes her TALK.
    Reconstructed from Neutron2.exe: constructor 0x436540 (factory '3FOW', 0x1d7c bytes),
    destructor 0x436710 (deleting thunks 0x4366e0 / 0x436a20 / 0x436a30 / 0x436a40).

    Original layout: as C3DFriends (element vtable 0x4dd8d4, 147 slots; listener 0x4dd8c0;
    OMediaAnim 0x4dd884; CLocalGameObject vtable 0x4dd408 at +0xc0), no members of its own.
    Element slot 145 is 0x456d10 (empty, as C3DFriends').                                    */
#pragma once
#include "C3DFriends.h"

class C3DFowl : public C3DFriends
{
	public:
	C3DFowl();																// 0x436540
	virtual ~C3DFowl();														// 0x436710

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60 0x4369f0 no such animation: IDLE
	virtual void ev69(void);												// ESLOT 69 0x436760 load the model and the animations (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void v260(void);												// SLOT 260 0x417110
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x436900
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x436920 "C3DFowl::ChangeState": TALK
};
