/*  C3DAIOmtObj ('3AIO') - a C3DAI whose shape comes from an OMT database ("OmtDatabase", default
    "objects.omt"; 3D shape chunk "OmtIndex", default 4), unlit. "TerrainColl" -1 leaves the
    terrain collision as is, 0 turns it off (and hovers), 1 on. "SpecialCase": 1 a rolling
    hazard (spins, scrolls its texture, pushes the camera away within 2000 by the difficulty;
    touching it hurts Jimmy, autosaves the spot and costs 5 energy), 2 a vehicle that carries
    the rocket / Jimmy / Goddard along and barrel-rolls when it meets the "BARRELROLL"
    C3DAITrigger, 3 a train ("traintracks" sound; the camera lands on top of it), 5 a
    trampoline ("boing", throws the camera up at 500 with our velocity).
    Reconstructed from Neutron2.exe: constructor 0x4171a0 (factory '3AIO', 0x1bdc bytes),
    destructor 0x4173d0 (only resets the vtables, then ~C3DAI; deleting thunks 0x4173a0 /
    0x417fb0 / 0x417fc0 / 0x417fd0).

    Original layout: as C3DAI (element vtable 0x4ccca0, 144 slots, none new; OMediaAnim vtable
    0x4ccc50 at +0x90; listener 0x4ccc8c at +0x30; CLocalGameObject vtable 0x4cc7d4 at +0xc0),
    own members 0x1b50..0x1bd4, vtordisp 0x1bd4, streamer 0x1bd8.                        */
#pragma once
#include "../C3DAI.h"

class C3DAIOmtObj : public C3DAI
{
	public:
	C3DAIOmtObj();															// 0x4171a0
	virtual ~C3DAIOmtObj();													// 0x4173d0

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x417960 by special case
	virtual void Process(float dt);											// SLOT 240 0x417420 by special case
	virtual void RotateToDest(float dt);									// SLOT 246 0x417f80 (C3DAI's)
	virtual void v260(void);												// SLOT 260 0x417ca0 shape from the database, special case setup

	// ---- data members (object offsets)
	char				f_1b50[100];	// 0x1b50 "OmtDatabase" ("objects.omt")
	int					f_1bb4;			// 0x1bb4 "OmtIndex" (4): its 3D shape chunk
	int					f_1bb8;			// 0x1bb8 "TerrainColl" (-1)
	int					f_1bbc;			// 0x1bbc "SpecialCase" (0)
	CGameObject			*f_1bc0;		// 0x1bc0 case 2: the rocket ship (looked up only if already set)
	CGameObject			*f_1bc4;		// 0x1bc4 case 2: "JIM1" (likewise)
	CGameObject			*f_1bc8;		// 0x1bc8 case 2: Goddard (likewise)
	float				f_1bcc;			// 0x1bcc case 2: barrel roll time left
	int					f_1bd0;			// 0x1bd0 case 3: "traintracks" channel (-1)
};
