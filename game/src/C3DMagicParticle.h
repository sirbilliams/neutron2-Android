/*  C3DMagicParticle - one sparkle of Jimmy's gadgets (burp gun / bubble): a spinning additive
    sprite from "RetainedSprites.omt" (8 units; Jimmy picks sprite 14 / 12 / 17 or 27 / 28 / 29
    at random), shown at a point by Burst and faded out over f_610 seconds in the colour
    f_600..f_608. C3DJimmy's constructor makes 21 (f_4d00[20], f_4d50[1]). ObjectID '3MAG',
    no factory ID.
    Reconstructed from Neutron2.exe: constructor 0x451700 (0x620 bytes), destructor 0x4518c0
    (only resets the vtables, then ~C3DPermanentSprite; deleting thunks 0x451ad0 / 0x451890 /
    0x451ae0).

    Original layout: as C3DSprite (element vtable 0x4e8858, 56 slots: C3DSprite's 55 + 55;
    listener 0x4e8844 at +0x30; CLocalGameObject vtable 0x4e83c8 at +0xc8), own members
    0x600..0x618, vtordisp 0x618, streamer 0x61c.                                              */
#pragma once
#include "C3DPermanentSprite.h"
#include "OMedia3DVector.h"

class C3DMagicParticle : public C3DPermanentSprite
{
	public:
	C3DMagicParticle();														// 0x451700
	virtual ~C3DMagicParticle();											// 0x4518c0

	// ---- element-family game slots
	virtual void Burst(OMedia3DVector pos);									// ESLOT 55 0x451900 at pos, time left = f_610, shown

	// ---- CGameObject-family overrides
	virtual void Process(float dt);										// SLOT 240 0x451950 fade out and spin
	virtual void v260(void);												// SLOT 260 0x451a40 sprite, physics, additive, second pass

	// ---- data members (object offsets)
	float				f_600;			// 0x600  red (0.5)
	float				f_604;			// 0x604  green (0.4)
	float				f_608;			// 0x608  blue (1)
	float				f_60c;			// 0x60c  spin, degrees / s (500)
	float				f_610;			// 0x610  fade time (1)
	float				f_614;			// 0x614  time left (1)
};
