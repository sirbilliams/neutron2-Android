/*  C3DDoorUpDown ('3DUD') - a door that slides up: a morph object (one .ase animation RAY,
    "ASEFile", default "doorfowl.ase"; texture "PNGFile", "doorfowl.png"). NextState (v268:
    0 open, 1 close, other toggles) moves it by "OpenAmount" (500) in y over "DoorSpeed" (5 s;
    reversed half-way if asked) with the "Door opening loop" sound, then "door closing"; open,
    it closes by itself after "OpenTime" (3 s). Each NextState also fires the "Next" object
    (an AI trigger, a cut-scene camera or a multi cut scene). Opened when touched by a Yokian,
    or by Jimmy while "TouchActivated" and not moving. "ItemClosed" (1) is its state on reset.
    Reconstructed from Neutron2.exe: constructor 0x42d660 (factory '3DUD', 0xa84 bytes),
    destructor 0x42d880 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x42d850 /
    0x42ded0 / 0x42dee0 / 0x42def0). Vtables 0x4d7fb0 (game object at +0xc0: 0, 7, 10, 16,
    231, 240, 260, 268 overridden; 231 is code shared at 0x47fc20) / 0x4d7fa0 (streamer).
    Own members 0x924..0xa7c, vtordisp 0xa7c, streamer 0xa80.                                */
#pragma once
#include "../C3DAnimated.h"

class C3DDoorUpDown : public C3DAnimated
{
	public:
	C3DDoorUpDown();														// 0x42d660
	virtual ~C3DDoorUpDown();												// 0x42d880

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x42d930 properties
	virtual void ResetObject(void);											// SLOT 10  0x42d8d0 back to ItemClosed
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x42da30 a Yokian / Jimmy: open
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x47fc20 collides with the camera, '3JIM', '3ENE', '3GOD', '3WBL' only
	virtual void Process(float dt);											// SLOT 240 0x42daa0 moving, closing by itself
	virtual void v260(void);												// SLOT 260 0x42de30 properties loaded: animation, texture
	virtual void v268(int state);											// SLOT 268 0x42dc60 "NextState": 0 open, 1 close, else toggle

	// ---- data members (object offsets)
	int					f_924;			// 0x924  "ItemClosed" (1): closed on reset
	int					f_928;			// 0x928  closed (or closing)
	char				f_92c[100];		// 0x92c  "Next" ("none")
	float				f_990;			// 0x990  moving time left
	float				f_994;			// 0x994  "DoorSpeed" (5 s for the whole way)
	float				f_998;			// 0x998  "OpenTime" (3 s)
	float				f_99c;			// 0x99c  time open
	int					f_9a0;			// 0x9a0  = 0 (unused)
	int					f_9a4;			// 0x9a4  (unused)
	float				f_9a8;			// 0x9a8  "OpenAmount" (500)
	char				f_9ac[100];		// 0x9ac  "ASEFile" ("doorfowl.ase")
	char				f_a10[100];		// 0xa10  "PNGFile" ("doorfowl.png")
	int					f_a74;			// 0xa74  "TouchActivated" (1)
	int					f_a78;			// 0xa78  the "Door opening loop" channel (-1: none)
};
