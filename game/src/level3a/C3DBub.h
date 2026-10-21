/*  C3DBub ('3BUU') - one bubble of the mini sub's trail (C3DMiniSub makes 10 at run time and
    lets one go every 0.25 s, its +0x670 set to 0.2): a C3DSpriteType billboard ("sprites.omt"
    canvas 26, size 20) that rises - gravity cancelled (its y acceleration set to minus the
    world's gravity y), pushed up by "UpAccel" per second (100) and its upward speed kept at
    least "MaxUpRate" (200) - and fades out (f_670 += 0.4 per second; alpha 1 - f_670).
    Reconstructed from Neutron2.exe: constructor 0x422cf0 (factory '3BUU', 0x67c bytes),
    destructor 0x422ed0 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x422ea0 /
    0x4230f0 / 0x423100). Vtables: element 0x4d0cbc? (C3DSprite's), game object (+0xc8) (0, 7,
    240 overridden). Own members 0x668..0x674, vtordisp 0x674, streamer 0x678.               */
#pragma once
#include "../C3DSpriteType.h"

class C3DBub : public C3DSpriteType
{
	public:
	C3DBub();																// 0x422cf0
	virtual ~C3DBub();														// 0x422ed0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x423070 UpAccel, MaxUpRate
	virtual void Process(float dt);											// SLOT 240 0x422f10 rise and fade

	// ---- data members (object offsets)
	float				f_668;			// 0x668 "UpAccel" (100)
	float				f_66c;			// 0x66c "MaxUpRate" (200): the least upward speed
	float				f_670;			// 0x670 faded (0..1; the mini sub starts it at 0.2)
};
