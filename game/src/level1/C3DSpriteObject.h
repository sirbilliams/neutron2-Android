/*  C3DSpriteObject ('3SRO') - a plain sprite (sprites.omt canvas 50, size 100) tinted "Red" /
    "Green" / "Blue" (1, 0.1, 0.1) at "Opacity" (0.8), shown or hidden by "InitiallyVisible" (1).
    With a "LinkedTag" it keeps that object at "LinkedOffsetX/Y/Z" (absolute; -1 = its own
    position + (10, 10, -30)) turned to "LinkedRotX/Y/Z".
    Tagged GEMH, LEFTGEM or RIGHTGEM it is a gem holder: Jimmy ('JIM1') touching it while he
    carries a gem, on the floor, faces it and puts the gem in it (0x437410; the holder links the
    gem); with empty hands he takes a "C3DGEM" back out of it (0x437340 for LEFTGEM / RIGHTGEM).
    Element slot 55 (new here) counts a timer down; the timers f_680 / f_684 hide / show it.
    Reconstructed from Neutron2.exe: constructor 0x472870 (factory '3SRO', 0x718 bytes),
    destructor 0x472aa0 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x4733e0
    / 0x472a70 / 0x4733f0). Vtables 0x4fc1bc (element, 56 slots: C3DSpriteType's 55 + 55) /
    0x4fc1a8 (listener) / 0x4fbd2c (game object: 0, 7, 10, 16, 240, 260, 284, 285 overridden)
    / 0x4fbd1c (streamer). Own members 0x668..0x710, vtordisp 0x710, the streamer at 0x714.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DSpriteObject : public C3DSpriteType
{
	public:
	C3DSpriteObject();														// 0x472870
	virtual ~C3DSpriteObject();												// 0x472aa0

	// ---- element-family slots it adds
	virtual unsigned char ev55(float *t, float dt);							// ESLOT 55 0x48fcb0 (+0xdc) count *t down: 1 when it reaches 0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x472ae0 properties
	virtual void ResetObject(void);											// SLOT 10  0x473010 + v176 / v157 (0, 0, 0)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x473040 gem holder
	virtual void Process(float dt);											// SLOT 240 0x472d60 timers, gem put down, the linked object
	virtual void v260(void);												// SLOT 260 0x472c60 shown / hidden, colour, the linked object
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x464ee0 (empty)
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x473340 the linked object again

	// FIELD 0x668 0xa8
	int				f_668;				// 0x668  "SpecialCase" (0, unused)
	float			f_66c;				// 0x66c  "Red" (1)
	float			f_670;				// 0x670  "Green" (0.1)
	float			f_674;				// 0x674  "Blue" (0.1)
	float			f_678;				// 0x678  "Opacity" (0.8)
	int				f_67c;				// 0x67c  "InitiallyVisible" (1)
	float			f_680;				// 0x680  hide timer
	float			f_684;				// 0x684  show timer
	float			f_688[3];			// 0x688  "LinkedRotX/Y/Z" (0)
	float			f_694[3];			// 0x694  "LinkedOffsetX/Y/Z" (-1)
	char			f_6a0[100];			// 0x6a0  "LinkedTag" ("none")
	CGameObject		*f_704;				// 0x704  the linked object (or the gem it holds)
	CGameObject		*f_708;				// 0x708  gem to put down next frame: this (0: none)
	CGameObject		*f_70c;				// 0x70c  ... by this Jimmy
};
