/*  C3DSkateBoard - Nick's skateboard: an OMT morph object ("board.ase", texture "skate.png",
    animation BOARD) without collisions or physics, created by C3DNick's constructor and moved
    under him while he patrols. No factory ID ('3BOA' is set as its ObjectID).
    Reconstructed from Neutron2.exe: constructor 0x46ff50 (0x92c bytes), destructor 0x470120
    (C3DAnimated's, inlined; deleting thunks 0x4700f0 / 0x470220 / 0x470230 / 0x470240).

    Original layout: as C3DAnimated (element vtable 0x4f8fd4, 94 slots; listener 0x4f8fc0;
    OMediaAnim 0x4f8f84; CLocalGameObject vtable 0x4f8b08 at +0xc0, only slot 7 overridden),
    no members of its own: vtordisp 0x924, streamer 0x928.                                   */
#pragma once
#include "../C3DAnimated.h"

class C3DSkateBoard : public C3DAnimated
{
	public:
	C3DSkateBoard();														// 0x46ff50
	virtual ~C3DSkateBoard();												// 0x470120

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x470170 the animation and the texture
};
