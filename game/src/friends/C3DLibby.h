/*  C3DLibby ('3LIB') - Libby: a morph friend (.ase animations TALK, WALK, RUN, DIGITIZE
    (libbyvox), FREEZE (libyvoxstop), STOP; textures "libby.png" (0) and "libbywire.png" (1)),
    patrolling (AIState 3) at 200, VisibleRange 500. The AI triggers DIGITIZE / UNDIGITIZE
    switch her to the wireframe texture and back (element slots 147 / 148). Reset puts her back
    at her start position.
    Reconstructed from Neutron2.exe: constructor 0x44f720 (factory '3LIB', 0x1d7c bytes),
    destructor 0x44f8e0 (deleting thunks 0x44f8b0 / 0x44fb80 / 0x44fb90 / 0x44fba0).

    Original layout: as C3DFriends (element vtable 0x4e6e34, 149 slots: C3DFriends' 147 +
    147, 148; listener 0x4e6e20; OMediaAnim 0x4e6de4; CLocalGameObject vtable 0x4e6968 at
    +0xc0), no members of its own. Slot 126 is 0x45b110 (a jump to the empty 0x456d10).      */
#pragma once
#include "C3DFriends.h"

class C3DLibby : public C3DFriends
{
	public:
	C3DLibby();																// 0x44f720
	virtual ~C3DLibby();													// 0x44f8e0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69  0x44f930 load the animations and the textures (once)
	virtual void ev145(void);												// ESLOT 145 0x44fa60
	virtual void ev147(void);												// ESLOT 147 0x44fb20 DIGITIZE: wireframe texture (1)
	virtual void ev148(void);												// ESLOT 148 0x44fb40 UNDIGITIZE: her own texture (0)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void ResetObject(void);											// SLOT 10  0x44fa80 back to the start position
	virtual void v260(void);												// SLOT 260 0x44fa30 her own texture
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x471950
};
