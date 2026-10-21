/*  C3DShadow ('3SHA') - Jimmy's blob shadow: sprite 9 of "retainedsprites.omt", multiplied onto
    the ground (blend Dst_Color, Zero), w x h world units (70 x 30; 150 x 50 in the rocket ship).
    C3DJimmy's constructor makes the one shadow (C3DJimmy f_cc4) and places it under him every
    frame; hidden in windowed mode.
    Reconstructed from Neutron2.exe: factory '3SHA', constructor 0x46e9f0 (0x60c bytes),
    destructor 0x46eb80 (only resets the vtables, then ~C3DPermanentSprite; deleting thunks
    0x46ec90 / 0x46eb50 / 0x46eca0).

    Original layout: as C3DSprite (element vtable 0x4f716c, 55 slots; listener 0x4f7158 at +0x30;
    CLocalGameObject vtable 0x4f6cdc at +0xc8), own members 0x600..0x604, vtordisp 0x604,
    streamer 0x608.                                                                            */
#pragma once
#include "C3DPermanentSprite.h"

class C3DShadow : public C3DPermanentSprite
{
	public:
	C3DShadow();															// 0x46e9f0
	virtual ~C3DShadow();													// 0x46eb80

	// ---- CGameObject-family overrides
	virtual void v260(void);												// SLOT 260 0x46ebc0 sprite, size, blend; hidden when windowed

	// ---- non-virtual methods
	void SetSize(short w, short h);											// 0x46ec30

	// ---- data members (object offsets)
	short				f_600;			// 0x600  width (70)
	short				f_602;			// 0x602  height (30)
};
