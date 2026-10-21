/*  C3DMovingTarget ('3TAR') - a shooting-gallery target: a sprite (sprites.omt canvas 98, size
    100) that glides from the position of "StartObj" towards "DestObj" at "Speed" (100; x 0.6 /
    0.9 / 1.2 by difficulty), jumping back to the start within 100 of the end. Hit by a water
    balloon or an energy projectile, both vanish ("vrzap", "pickup blip3") and Jimmy scores
    "NumPoints" (10); every "HitsRequired" (5) hits on targets of the same "TargetID" (0..2,
    counters shared by all targets and saved) fire "ActivateObject": an AI trigger, or anything
    else, which is dropped 10000 below where it is.
    Reconstructed from Neutron2.exe: constructor 0x456320 (factory '3TAR', 0x7dc bytes),
    destructor 0x456630 (deleting thunks 0x456600 / 0x456da0 / 0x456db0). Vtables 0x4eb574
    (element, 55 slots: C3DSprite's) / 0x4eb560 (listener) / 0x4eb0e4 (game object at +0xc8:
    0, 16, 231, 240, 260, 278, 284, 285 overridden; 277 is 0x456d10, a jump to the empty
    0x463890) / 0x4eb0d4 (streamer). Own members 0x668..0x7d4, vtordisp 0x7d4, streamer 0x7d8.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DMovingTarget : public C3DSpriteType
{
	public:
	C3DMovingTarget();														// 0x456320
	virtual ~C3DMovingTarget();												// 0x456630

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x456670 hit by a water balloon / energy projectile
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x456a10 collides with '3EPR' / '3WBL' only
	virtual void Process(float dt);											// SLOT 240 0x456a40 glide to DestObj, back to the start
	virtual void v260(void);												// SLOT 260 0x4568f0 properties loaded: the two ends, the speed
	virtual void v278(void);												// SLOT 278 0x456d20 the speed for the difficulty
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x456b80 hit counters, position, active
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x456c30

	// ---- data members (object offsets)
	int				f_668;				// 0x668  = 0 (unused)
	float			f_66c[4];			// 0x66c  the start: StartObj's position
	float			f_67c[4];			// 0x67c  the end: DestObj's position
	float			f_68c;				// 0x68c  "Speed" (100)
	int				f_690;				// 0x690  = 0 (unused)
	char			f_694[100];			// 0x694  "ActivateObject" ("none")
	int				f_6f8;				// 0x6f8  "HitsRequired" (5)
	int				f_6fc;				// 0x6fc  "NumPoints" (10)
	float			f_700;				// 0x700  = 15 (unused)
	char			f_704[100];			// 0x704  "StartObj" ("none")
	char			f_768[100];			// 0x768  "DestObj" ("none")
	int				f_7cc;				// 0x7cc  "TargetID" (0): which hit counter (g_545fdc / e0 / e4)
	float			f_7d0;				// 0x7d0  the speed now (Speed for the difficulty)
};
