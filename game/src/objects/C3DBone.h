/*  C3DBone ('3BON') - a bone for Goddard: an unlit, spinning object, shape 0 of
    "permanentobjects.omt" scaled by 3 (the shared shape itself is scaled, each time one is made),
    radius 15, no animation system (f_6d0 = 2). When Goddard touches one he takes it: it is hidden,
    its f_924 cleared, and he goes to Jimmy (C3DGoddard AddTouching 0x43a830). No shipped .gam file
    places one, and nothing creates one at run time, so in the retail game this is never seen.
    Reconstructed from Neutron2.exe: constructor 0x421f70 (factory '3BON', 0x930 bytes),
    destructor 0x422140 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x422220 /
    0x422210 / 0x422110 / 0x422200). Game-object slots 240 (0x422190) and 260 (0x47e3a0)
    overridden; streamer 0x422230 / 0x422240 are OMedia3DShapeElement's. Own member 0x924,
    vtordisp 0x928, streamer 0x92c.                                                           */
#pragma once
#include "../C3DAnimated.h"

class C3DBone : public C3DAnimated
{
	public:
	C3DBone();																// 0x421f70
	virtual ~C3DBone();														// 0x422140

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x422190 spin
	virtual void v260(void);												// SLOT 260 0x47e3a0 properties loaded: InitPhysics

	// FIELD 0x924 0x4
	int					f_924;			// 0x924  0 (cleared again when Goddard takes it; never read)
};
