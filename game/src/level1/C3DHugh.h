/*  C3DHugh ('3HUG') - Hugh Neutron, Jimmy's dad: an OMT morph friend (hughtalk / hughwalk /
    hughstop .ase, texture "hugh.png"), patrolling (AIState 2) at 160, VisibleRange 500. His
    animations are loaded by InitObject (his ev69 is the empty one), and he is always shown in
    the story scenes (v267, the same code as C3DCarl's).
    Reconstructed from Neutron2.exe: constructor 0x4419f0 (factory '3HUG', 0x1d7c bytes),
    destructor 0x441b80 (deleting thunks 0x441b50 / 0x441cc0 / 0x441cd0 / 0x441ce0).

    Original layout: as C3DFriends (element vtable: 145 is 0x456d10, a jump to the empty
    0x463890; CLocalGameObject vtable at +0xc0: 7, 266, 267 overridden), no members of its own. */
#pragma once
#include "../friends/C3DFriends.h"

class C3DHugh : public C3DFriends
{
	public:
	C3DHugh();																// 0x4419f0
	virtual ~C3DHugh();														// 0x441b80

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x441bd0 load the morph animations and the texture
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x425e60 shown
};
