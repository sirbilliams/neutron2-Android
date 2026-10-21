/*  C3DCursor - a 3D arrow ("3darrow.ase" scaled x5, "3darrow.png", additive, facing the
    viewport) that points the way; C3DGoddard keeps two (f_1b80, f_1b84: towards the target
    the AI trigger "ShowTarget" gives Jimmy, and his marker in the rocket ship). ObjectID '3CUR',
    no factory ID.
    Reconstructed from Neutron2.exe: constructor 0x429c70 (0x92c bytes), destructor 0x429dd0
    (only resets the vtables, then ~C3DAnimated; deleting thunks 0x429f10 / 0x429da0 / 0x429f20
    / 0x429f00).

    Original layout: as C3DAnimated (element vtable 0x4d7120, 94 slots; OMediaAnim vtable
    0x4d70d0 at +0x90; listener 0x4d710c at +0x30; CLocalGameObject vtable 0x4d6c54 at +0xc0),
    no members of its own (vtordisp 0x924, streamer 0x928).                                    */
#pragma once
#include "C3DAnimated.h"

class C3DCursor : public C3DAnimated
{
	public:
	C3DCursor();															// 0x429c70
	virtual ~C3DCursor();													// 0x429dd0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7  0x429e20 the arrow animation and texture
};
