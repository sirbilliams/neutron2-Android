/*  C3DTargetCursor - the aiming cross of Jimmy's gadgets: sprite 8 of "retainedsprites.omt",
    110 x 110 world units (ObjectID '3TAR'). A copy of C3DShadow's code (it even names itself
    "C3DSHADOW"). C3DJimmy's constructor makes the one cursor (C3DJimmy f_4c8c), hidden.
    No factory ID.
    Reconstructed from Neutron2.exe: constructor 0x475b20 (0x60c bytes), destructor 0x475cb0
    (only resets the vtables, then ~C3DPermanentSprite; deleting thunks 0x475d40 / 0x475c80 /
    0x475d50).

    Original layout: as C3DSprite (element vtable 0x4febfc, 55 slots; listener 0x4febe8 at +0x30;
    CLocalGameObject vtable 0x4fe76c at +0xc8), own members 0x600..0x604, vtordisp 0x604,
    streamer 0x608.                                                                            */
#pragma once
#include "C3DPermanentSprite.h"

class C3DTargetCursor : public C3DPermanentSprite
{
	public:
	C3DTargetCursor();														// 0x475b20
	virtual ~C3DTargetCursor();												// 0x475cb0

	// ---- CGameObject-family overrides
	virtual void v260(void);												// SLOT 260 0x475cf0 sprite and size

	// ---- data members (object offsets)
	short				f_600;			// 0x600  width (110)
	short				f_602;			// 0x602  height (110)
};
