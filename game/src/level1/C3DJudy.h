/*  C3DJudy ('3MOM') - Judy Neutron, Jimmy's mom: an OMT morph friend (judytalk / judywalk /
    judystop .ase, texture "judy.png"), patrolling (AIState 2) at 160, VisibleRange 500.
    Reconstructed from Neutron2.exe: constructor 0x44e8f0 (factory '3MOM', 0x1d7c bytes),
    destructor 0x44eab0 (deleting thunks 0x44ea80 / 0x44ec40 / 0x44ec50 / 0x44ec60).

    Original layout: as C3DFriends (element vtable: 69, 145 overridden; CLocalGameObject vtable
    0x4e5644 at +0xc0: 7, 266, 267 overridden), no members of its own.                       */
#pragma once
#include "../friends/C3DFriends.h"

class C3DJudy : public C3DFriends
{
	public:
	C3DJudy();																// 0x44e8f0
	virtual ~C3DJudy();														// 0x44eab0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69  0x44eb00 load the morph animations and the texture (once)
	virtual void ev145(void);												// ESLOT 145 0x44ec00 talked to (nothing happens)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x47a660
};
