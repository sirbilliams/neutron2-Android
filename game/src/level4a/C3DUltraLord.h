/*  C3DUltraLord ('3ULT') - Ultra Lord (level4a): an OMT morph friend (ultrawalk, ultraflex 1..3,
    ultratalk, ultragive, ultrawhisper, ultrastop .ase; texture "ultralord.png"; radius 150,
    scale 0.5), patrolling (AIState 2) at 160, VisibleRange 500.
    Reconstructed from Neutron2.exe: constructor 0x47a330 (factory '3ULT', 0x1d7c bytes),
    destructor 0x47a4f0 (deleting thunks 0x47a6b0 / 0x47a6a0 / 0x47a4c0 / 0x47a6c0).

    Original layout: as C3DFriends (element vtable 0x502e44: 69, 126 (0x45b110, a jump to the
    empty 0x456d10) and 145 (0x44fa60) overridden; CLocalGameObject vtable 0x502978 at +0xc0: 7,
    266, 267 overridden, all shared code), no members of its own.                             */
#pragma once
#include "../friends/C3DFriends.h"

class C3DUltraLord : public C3DFriends
{
	public:
	C3DUltraLord();															// 0x47a330
	virtual ~C3DUltraLord();												// 0x47a4f0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69  0x47a540 load the morph animations and the texture (once)
	virtual void ev145(void);												// ESLOT 145 0x44fa60 talked to (nothing happens)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x47a660
};
