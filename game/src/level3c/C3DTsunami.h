/*  C3DTsunami ('3TSU') - the tidal wave (LV3C; a rolling boulder in L5B1): a morph C3DAI
    (animation TSUNAMI from "tsunami.ase" / "tsunami.png"; L5B1: "boulder.ase" / "boulder.png"
    and STOP "boulderStop.ase") that starts in state 3 (patrol only) at 700, VisibleRange 500,
    hovering. After a 3 s hold on (re)load it follows its patrol, with the "waterspout" loop
    while patrolling; its speed is InitialSpeed scaled by the distance to the camera and the
    difficulty (it slows down near Jimmy and catches up from far away). Touching Jimmy or the
    hover cycle hurts Jimmy badly (-300 energy, -10 points, 5 s). LV3C: alpha 0.85, second pass.
    Reconstructed from Neutron2.exe: constructor 0x479a10 (factory '3TSU', 0x1c28 bytes),
    destructor 0x479c50 (only resets the vtables, then ~C3DAI; deleting thunks 0x479c20 /
    0x47a2e0 / 0x47a2f0 / 0x47a300). Vtables: game object (+0xc0) 0x502244 (0, 16, 240, 260,
    277, 278, 284, 285 overridden), element ESLOT 69 0x479ca0, streamer 0x502234.
    Own members 0x1b50..0x1c20, vtordisp 0x1c20, streamer 0x1c24.                           */
#pragma once
#include "../C3DAI.h"

class C3DTsunami : public C3DAI
{
	public:
	C3DTsunami();															// 0x479a10
	virtual ~C3DTsunami();													// 0x479c50

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69 0x479ca0 load the animation and texture (once)

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x47a190 Jimmy / the hover cycle: hurt Jimmy
	virtual void Process(float dt);											// SLOT 240 0x479e50 hold, sound, speed by distance
	virtual void v260(void);												// SLOT 260 0x479de0 properties loaded
	virtual void v277(void);												// SLOT 277 0x47a240 pause: sound off
	virtual void v278(void);												// SLOT 278 0x47a260 resume: sound on again
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x47a230
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x47a200

	// ---- data members (object offsets)
	char				f_1b50[100];	// 0x1b50 the .ase file ("tsunami.ase"; L5B1 "boulder.ase")
	char				f_1bb4[100];	// 0x1bb4 the texture ("tsunami.png"; L5B1 "boulder.png")
	int					f_1c18;			// 0x1c18 the "waterspout" channel (-1: none)
	float				f_1c1c;			// 0x1c1c hold time left (3 s after a (re)load)
};
