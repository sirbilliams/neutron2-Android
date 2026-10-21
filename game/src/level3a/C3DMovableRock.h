/*  C3DMovableRock ('3MOR') - a rock that can be pushed about in Level3a: an unlit object whose
    shape is chunk 27 of "objects.omt" (no animation system, f_6d0 = 2), radius 350, a physics
    object (InitPhysics; flags v40/v42/v46/v52 on). Touching another movable rock turns its
    v132 flag off (f_13e, the one v46 sets). Its position, angles and two flags (v135, v131)
    go into a savegame. No properties of its own.
    Reconstructed from Neutron2.exe: constructor 0x454cf0 (factory '3MOR', 0x92c bytes),
    destructor 0x454eb0 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x4551b0 /
    0x4551a0 / 0x454e80 / 0x4551c0). Vtables 0x4ea100 (element, 94 slots, all C3DAnimated's) /
    0x4ea0ec (listener) / 0x4ea0b0 / 0x4e9c34 (game object at +0xc0: 0, 16, 220, 231, 240, 284,
    285 overridden; 231 is the shared 0x43d610) / 0x4e9c24 (streamer). No own members:
    vtordisp 0x924, streamer 0x928.                                                          */
#pragma once
#include "../C3DAnimated.h"

class C3DMovableRock : public C3DAnimated
{
	public:
	C3DMovableRock();														// 0x454cf0
	virtual ~C3DMovableRock();												// 0x454eb0

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x454f00 another rock: v132(0)
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x454f60 (only passes it on)
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x454f30
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x455070
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x454f90
};
