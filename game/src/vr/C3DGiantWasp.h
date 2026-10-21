/*  C3DGiantWasp ('3GWA') - the VR level's boss: a C3DFlyingBot (animations MOVE from
    "waspmove.ase", STOP from "waspstop.ase"; textures "wasp.png" and the wire "wirewasp.png"),
    radius 800. Only energy projectiles hurt it: each costs 10 / 5 / 2 health (of 100) by skill,
    20 points and "vrzap", and from the first hit on its health bar shows over it (two interface
    elements, canvases 411 and 359 of screens.omt, 0.5 px per point). Hit again with no health
    left, it stops (state 1) and is gone: the wire texture, hidden, switched off, the object
    tagged BLOCKWASP switched off, and the C3DAITrigger tagged WASPDIE fired. Touching the camera
    hurts Jimmy (-10) and makes it flee for 15 s / 5000.
    Reconstructed from Neutron2.exe: constructor 0x437ad0 (factory '3GWA', 0x1d80 bytes),
    destructor 0x437ca0 (hides the bar, then ~C3DFlyingBot; deleting thunks 0x437c70 / 0x438260 /
    0x438270 / 0x438280). Vtables: element 0x4dedb4 (69 overridden), game object (+0xc0)
    0x4de8e8 (0, 16, 240 overridden), streamer 0x4de8d8. Own members 0x1b68..0x1d78, vtordisp
    0x1d78, streamer 0x1d7c.                                                                  */
#pragma once
#include "C3DFlyingBot.h"
#include "../InterfaceElement.h"

class C3DGiantWasp : public C3DFlyingBot
{
	public:
	C3DGiantWasp();															// 0x437ad0
	virtual ~C3DGiantWasp();												// 0x437ca0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69 0x437d60 load the animations and textures (once)

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x438100 energy projectiles hurt; the camera is hurt
	virtual void Process(float dt);											// SLOT 240 0x437e10 the health bar; out of health: gone, WASPDIE

	// ---- data members (object offsets)
	InterfaceElt		f_1b68;			// 0x1b68 health bar frame (screens.omt canvas 411)
	InterfaceElt		f_1c6c;			// 0x1c6c health bar (canvas 359, width f_1d70 * 0.5)
	float				f_1d70;			// 0x1d70 health (100)
	unsigned char		f_1d74;			// 0x1d74 hit with no health left: dead
	unsigned char		f_1d75;			// 0x1d75 hit once: the bar shows
};
