/*  C3DPointCursor - the spinning 3D arrow ("pointarrow.ase", "3darrow.png", additive) that
    C3DJimmy shows over the nearest pickup (ev164, his f_d4c). No factory ID (ObjectID '3PCU').
    Reconstructed from Neutron2.exe: constructor 0x466a70 (0x92c bytes), destructor 0x466bc0
    (only resets the vtables, then ~C3DAnimated; deleting thunks 0x466ce0 / 0x466b90 / 0x466cf0
    / 0x466cd0).

    Original layout: as C3DAnimated (element vtable 0x4f2b14, 94 slots; OMediaAnim vtable
    0x4f2ac4 at +0x90; listener 0x4f2b00 at +0x30; CLocalGameObject vtable 0x4f2648 at +0xc0),
    no members of its own (vtordisp 0x924, streamer 0x928).                                    */
#pragma once
#include "C3DAnimated.h"

class C3DPointCursor : public C3DAnimated
{
	public:
	C3DPointCursor();														// 0x466a70
	virtual ~C3DPointCursor();												// 0x466bc0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7  0x466c10 the arrow animation and texture
};
