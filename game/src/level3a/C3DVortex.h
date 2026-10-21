/*  C3DVortex ('3VOR') - a whirlpool in Level3a: an invisible sprite ("icons.omt" canvas 1, size
    150; hidden on load) that pulls the mini sub towards it. Within 5000 of the sub it sets the
    sub's impulse to the direction to the vortex times (5000 - distance) * (0.1, 0.008, 0.1), and
    that of the 10 objects the sub keeps at +0xa04 to a tenth of it (y a fortieth).
    Reconstructed from Neutron2.exe: constructor 0x47a6f0 (factory '3VOR', 0x60c bytes),
    destructor 0x47a870 (only resets the vtables, then ~C3DSprite; deleting thunks 0x47aad0 /
    0x47a840 / 0x47aae0). Vtables 0x50353c (element, 55 slots, all C3DSprite's) / 0x503528
    (listener) / 0x5030ac (game object at +0xc8: 0, 240, 260 overridden) / 0x50309c (streamer:
    C3DSprite's read_class / write_class). Own member 0x600, vtordisp 0x604, streamer 0x608.
    Members are named after their offset from the object start (the CGameObject-family methods
    of the original see them 0xc8 lower).                                                     */
#pragma once
#include "../C3DSprite.h"

class C3DVortex : public C3DSprite
{
	public:
	C3DVortex();															// 0x47a6f0
	virtual ~C3DVortex();													// 0x47a870

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x47a8e0 pull the sub
	virtual void v260(void);												// SLOT 260 0x47a8b0 (re)load: find the sub, hide

	// FIELD 0x600 0x4
	CGameObject	*m_sub;					// 0x600  the object tagged "C3DMINISUB" (game object view)
};
