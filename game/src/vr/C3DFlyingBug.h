/*  C3DFlyingBug ('3BUG') - the bugs of the VR level: a C3DFlyingBot (animations RUN from
    "bugfly.ase", STOP from "bugstop.ase"; textures "bug01.png" and the wire grid
    "bug01grid.png"), radius 200. Knocked out (two hits within 10 s) it never comes back
    (60000 s): it fades out (alpha -1/3 per second) and is switched off.
    Reconstructed from Neutron2.exe: constructor 0x433340 (factory '3BUG', 0x1b74 bytes),
    destructor 0x433480 (only resets the vtables, then ~C3DFlyingBot; deleting thunks 0x433450 /
    0x4336f0 / 0x433700 / 0x433710). Vtables: element 0x4dbe0c (69 overridden), game object
    (+0xc0) 0x4db940 (0, 231, 240 overridden), streamer 0x4db930. Own member 0x1b68, vtordisp
    0x1b6c, streamer 0x1b70.                                                                  */
#pragma once
#include "C3DFlyingBot.h"

class C3DFlyingBug : public C3DFlyingBot
{
	public:
	C3DFlyingBug();															// 0x433340
	virtual ~C3DFlyingBug();												// 0x433480

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69 0x4334d0 load the animations and textures (once)

	// ---- CGameObject-family overrides
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x433590 collides with the camera, '3PRO', '3BUG', '3EPR', '3VRT'
	virtual void Process(float dt);											// SLOT 240 0x4335e0 knocked out: fade away

	// ---- data members (object offsets)
	float				f_1b68;			// 0x1b68 alpha while fading out (1.0)
};
