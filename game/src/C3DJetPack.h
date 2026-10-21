/*  C3DJetPack - Jimmy's jet pack: a C3DFlyingObject that never flies itself (Process only keeps
    its fire on Jimmy), holding the visible jet pack C3DJetpackFire (f_9f0: unfolds when Jimmy's
    control state f_a68 becomes 1, folds when it ends) and the "jetpack" sound loop (f_9f4).
    C3DJimmy's constructor makes the one jet pack (C3DJimmy f_ccc, parked: hidden, no physics).
    No factory ID (ObjectID '3FLY' from C3DFlyingObject).
    Reconstructed from Neutron2.exe: constructor 0x442f10 (0xa04 bytes), destructor 0x443090
    (only resets the vtables and logs "~C3DFlyingObject()", then ~C3DPlayerObject; deleting
    thunks 0x4434c0 / 0x443060 / 0x4434d0 / 0x4434b0).

    Original layout: as C3DFlyingObject (element vtable 0x4e463c, 110 slots; OMediaAnim vtable
    0x4e45ec at +0x90; listener 0x4e4628 at +0x30; CLocalGameObject vtable 0x4e4170 at +0xc0),
    own members 0x9f0..0x9fc, vtordisp 0x9fc, streamer 0xa00.                                  */
#pragma once
#include "C3DFlyingObject.h"

class C3DJetpackFire;

class C3DJetPack : public C3DFlyingObject
{
	public:
	C3DJetPack();															// 0x442f10
	virtual ~C3DJetPack();													// 0x443090

	// ---- element-family game slots
	virtual void ShowAgain(void);											// ESLOT 54 0x4430e0 the fire too
	virtual void HideAgain(void);											// ESLOT 55 0x443100 the fire too

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x443120 only UpdateFire (no flying)
	virtual void v260(void);												// SLOT 260 0x443310 the fire into the world, folded
	virtual void v277(void);												// SLOT 277 0x443430 pause: stop the sound
	virtual void v278(void);												// SLOT 278 0x443450 resume: the sound again

	// ---- non-virtual methods (thiscall on the element)
	void UpdateFire(void);													// 0x443140 open / close / follow Jimmy by his f_a68
	void Off(void);															// 0x443370 flames 0 (g_jimmy's jet pack's and ours), sound off
	void Sound(void);														// 0x4433f0 loop "jetpack" unless playing

	// ---- data members (object offsets)
	C3DJetpackFire		*f_9f0;			// 0x9f0  the jet pack with its flames
	int					f_9f4;			// 0x9f4  "jetpack" sound channel (-1)
	short				f_9f8;			// 0x9f8  Jimmy's f_a68 last frame
};
