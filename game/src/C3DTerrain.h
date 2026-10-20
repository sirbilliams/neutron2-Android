/*  C3DTerrain ('3TER') - one shape of a level's terrain database. The game type's CreateTerrain
    makes one per shape (CJimmyGame 0x482bc0 / 0x483040 / 0x483780, CGameType 0x4b0560 /
    0x4b07b0), gives it the shape, and InitPhysics puts the shape's polygons into the physics
    terrain grid (PhysicsAddTerrain). The element is drawn in the world layer.

    Reconstructed from Neutron2.exe: constructor 0x49d0e0, destructor 0x49d210 (thunks 0x49d350 /
    0x49d340 / 0x49d1e0 / 0x49d360). Same layout as C3DObject (0x590 bytes + the streamer,
    factory size 0x594), no members of its own. The element vtable (0x51a1d0) is C3DObject's
    unchanged; the CGameObject vtable (0x519d04) overrides slots 0, 7, 11 and 240.            */
#pragma once
#include "C3DObject.h"

class C3DTerrain : public C3DObject
{
	public:
	C3DTerrain();															// 0x49d0e0
	virtual ~C3DTerrain();													// SLOT 0  0x49d210

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7  0x49d2b0
	virtual void InitPhysics(void);											// SLOT 11 0x49d2f0
	virtual void Process(float dt);											// SLOT 240 0x464ee0 (empty)
};
