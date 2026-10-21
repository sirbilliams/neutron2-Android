/*  C3DLabScreen ('3SCR') - a monitor in Negatron's lab: a morph object ("screen.ase", DEFAULT)
    whose picture cycles through five textures on a 13 s loop: "screen1.png" up to 1 s,
    "screen0.png" to 3 s, "screen2.png" to 5 s, "screen3.png" to 7 s, "screen4.png" to 10 s,
    then "screen1.png" again (the clock restarts once it passes 10 s, at the first frame
    before 13 s).
    Reconstructed from Neutron2.exe: constructor 0x44ec70 (factory '3SCR', 0x930 bytes),
    destructor 0x44edc0 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x44ed90 /
    0x44f080 / 0x44f090 / 0x44f0a0).

    Original layout: as C3DAnimated (element vtable 0x4e6244, 94 slots; OMediaAnim vtable
    0x4e61f4 at +0x90; listener 0x4e6230 at +0x30; CLocalGameObject vtable 0x4e5d78 at +0xc0),
    own member 0x924, vtordisp 0x928, streamer 0x92c.                                         */
#pragma once
#include "../C3DAnimated.h"

class C3DLabScreen : public C3DAnimated
{
	public:
	C3DLabScreen();															// 0x44ec70
	virtual ~C3DLabScreen();												// 0x44edc0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ee10 animation and textures
	virtual void v23(CGameObject *o);										// SLOT 23  0x44f050 (calls the empty 0x464ee0)
	virtual void Process(float dt);											// SLOT 240 0x44eee0 the picture cycle

	// ---- data members (object offsets)
	float				f_924;			// 0x924  clock of the picture cycle (0)
};
