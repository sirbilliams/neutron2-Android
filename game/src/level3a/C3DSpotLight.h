/*  C3DSpotLight ('3SPO') - the beam of the mini sub's headlight: a morph object (animation LIGHT
    from "subspotlight.ase", texture "subspotlight.png", additive at 0.8) with a C3DCorona
    (sprites.omt canvas 36, not pulsing) as the lit spot. While on (f_928, the sub turns it on),
    a ray from it to the point (0, -2250, 6000) in front of and below it finds what it lights:
    the corona goes there (pulled back towards the light by 0.45 / -1.2 of the distance d * 0.3),
    yellowish (0.9, 0.9, 0.1) with alpha (2000 - d) / 2000 and size 0.8 d; nothing hit, or off,
    hides it (off also hides the beam).
    Reconstructed from Neutron2.exe: constructor 0x4721f0 (factory '3SPO', 0x934 bytes; it
    repeats InitObject inline), destructor 0x472460 (only resets the vtables, then ~C3DAnimated;
    deleting thunks 0x472850 / 0x472840 / 0x472430 / 0x472860). Vtables 0x4fbb88 (element, 94
    slots, all C3DAnimated's) / 0x4fbb74 (listener) / 0x4fbb38 / 0x4fb6bc (game object at +0xc0:
    0, 7, 240 overridden) / 0x4fb6ac (streamer). Own members 0x924..0x92c, vtordisp 0x92c,
    streamer 0x930.                                                                          */
#pragma once
#include "../C3DAnimated.h"

class C3DCorona;

class C3DSpotLight : public C3DAnimated
{
	public:
	C3DSpotLight();															// 0x4721f0
	virtual ~C3DSpotLight();												// 0x472460

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4724b0 "C3DSpotLight::InitObject()"
	virtual void Process(float dt);											// SLOT 240 0x472540 the lit spot

	// FIELD 0x924 0x8
	C3DCorona			*f_924;			// 0x924  the lit spot (kept as its game object view)
	unsigned char		f_928;			// 0x928  on
};
