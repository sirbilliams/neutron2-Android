/*  C3DFireStrato - the exhaust flame of the rocket ship (the "Strato" XL): a morph-animated
    C3DAnimated with the animations DEFAULT ("firestrato.ase") and AFTERBURN ("firestrato2.ase")
    and the texture "fire.png", additive, no collision, ObjectID '3FST'. C3DRocketShip creates one
    and moves it with itself; no factory ID.
    Reconstructed from Neutron2.exe: constructor 0x431b60 (0x92c bytes), destructor 0x431d10
    (only resets the vtables, then ~C3DAnimated; deleting thunks 0x431ce0 / 0x431da0 / 0x431db0 /
    0x431dc0).

    Original layout: as C3DAnimated (element vtable 0x4da958, 94 slots; CLocalGameObject vtable
    0x4da48c at +0xc0), no members of its own: vtordisp 0x924, streamer 0x928.             */
#pragma once
#include "C3DAnimated.h"

class C3DFireStrato : public C3DAnimated
{
	public:
	C3DFireStrato();														// 0x431b60
	virtual ~C3DFireStrato();												// 0x431d10

	// ---- CGameObject-family overrides
	virtual void ResetObject(void);											// SLOT 10 0x431d60 + hidden
};
