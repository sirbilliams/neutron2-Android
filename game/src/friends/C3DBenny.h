/*  C3DBenny ('3BEN') - Benny (the museum janitor): a morph friend (.ase animations WIPE, STOP;
    texture "benny.png"), patrolling (AIState 2) at 160, VisibleRange 500, wiping from the
    start; shadow 5 below his origin.
    Reconstructed from Neutron2.exe: constructor 0x421be0 (factory '3BEN', 0x1d7c bytes),
    destructor 0x421dc0 (deleting thunks 0x421d90 / 0x421f40 / 0x421f50 / 0x421f60).

    Original layout: as C3DFriends (element vtable 0x4cfcb4, 147 slots; listener 0x4cfca0;
    OMediaAnim 0x4cfc64; CLocalGameObject vtable 0x4cf7e8 at +0xc0), no members of its own.  */
#pragma once
#include "C3DFriends.h"

class C3DBenny : public C3DFriends
{
	public:
	C3DBenny();																// 0x421be0
	virtual ~C3DBenny();													// 0x421dc0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69  0x421e80 load the animations and the texture (once)
	virtual void ev145(void);												// ESLOT 145 0x44fa60

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void UnInitObject(void);										// SLOT 8   0x421e50 "C3DBenny::UnInitObject()"
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x471950
};
