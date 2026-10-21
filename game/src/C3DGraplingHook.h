/*  C3DGraplingHook - the rope of Jimmy's grappling hook: a translucent .ase strip ("rope01.ase",
    "laser.png", its texture scrolling while shown), stretched along z (C3DJimmy scales its
    shape) from Jimmy's hand towards the anchor. C3DJimmy's constructor makes the one rope
    (f_d20). No factory ID (ObjectID '3GRA'; the name keeps the original's spelling).
    Reconstructed from Neutron2.exe: constructor 0x43cdb0 (0x92c bytes), destructor 0x43cf00
    (only resets the vtables, then ~C3DAnimated; deleting thunks 0x43d0d0 / 0x43ced0 / 0x43d0e0
    / 0x43d0c0).

    Original layout: as C3DAnimated (element vtable 0x4e02b4, 94 slots; OMediaAnim vtable
    0x4e0264 at +0x90; listener 0x4e02a0 at +0x30; CLocalGameObject vtable 0x4dfde8 at +0xc0),
    no members of its own (vtordisp 0x924, streamer 0x928).                                    */
#pragma once
#include "C3DAnimated.h"

class C3DGraplingHook : public C3DAnimated
{
	public:
	C3DGraplingHook();														// 0x43cdb0
	virtual ~C3DGraplingHook();												// 0x43cf00

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43cf50 the rope animation and texture, turned 180
	virtual void Process(float dt);											// SLOT 240 0x43d030 scroll the texture while shown
	virtual void v260(void);												// SLOT 260 0x43d070 translucent (0.4)
};
