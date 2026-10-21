/*  C3DFish ('3FIS') - a C3DAI animated with OMT morphs from .ase files (the dolphins of
    level 1: "dolphinswim.ase" / "dolphinstop.ase", texture "dolphin01.png"), hovering (no
    gravity) and patrolling (AIState 3) at 500. Biting the camera (Jimmy, or the mini sub he
    drives) while attacking hurts him and makes it flee. "SpecialCase" picks a variant:
      1  the dragonfly: buzzing sound, turns faster, no collisions with the walls; a water
         balloon knocks it out of the air (state 1, no hover)
      2  bites Jimmy whenever it touches him (v94(2))
      5  carries the rocket, Jimmy (DRIVE) and Goddard (ROCK) along with it, hidden itself;
         a C3DAITrigger tagged BARRELROLL rolls it once (AINewRotY seconds, 0.75 by default)
    Reconstructed from Neutron2.exe: constructor 0x431df0 (factory '3FIS', 0x1d64 bytes),
    destructor 0x4320f0 (deleting thunks 0x4320c0 / 0x432ad0 / 0x432ae0 / 0x432af0).

    Original layout: as C3DAI (element vtable 0x4dafb8, 144 slots, 69 overridden; listener
    0x4dafa4; CLocalGameObject vtable 0x4daaec at +0xc0), own members 0x1b50..0x1d5c,
    vtordisp 0x1d5c, streamer 0x1d60.

    Hooks into C3DJimmy: C3DFish hurts Jimmy through his element slot 134 (0x44bd90). It is
    declared below and defined weak (a TODO log) in C3DFish.cpp; C3DJimmy.cpp defines the real
    one. The mini sub (C3DMiniSub, not reconstructed yet) and Goddard's +0x1b60 flag are TODO
    stubs in C3DFish.cpp.

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code of the original sees them at offset - 0xc0).            */
#pragma once
#include "C3DAI.h"

class C3DFish : public C3DAI
{
	public:
	C3DFish();																// 0x431df0
	virtual ~C3DFish();														// 0x4320f0

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69 0x432140 load the morph animations and the texture (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x432240 properties
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x432820 bite / be hit / barrel roll
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x432a60 collides with the camera, '3JIM', '3AIT', '3DOG', '3LBW' only
	virtual void Process(float dt);											// SLOT 240 0x4324e0 SpecialCase 5 carrying, barrel roll
	virtual void v260(void);												// SLOT 260 0x432350 properties loaded: the SpecialCase set-up

	// ---- data members (object offsets)
	char				f_1b50[100];	// 0x1b50 "ASEWalk" ("dolphinswim.ase"): animation WALK
	char				f_1bb4[100];	// 0x1bb4 "ASEStop" ("dolphinstop.ase"): animation STOP
	char				f_1c18[100];	// 0x1c18 "ASEAnim2" ("none"): animation ANIM2
	char				f_1c7c[100];	// 0x1c7c "ASEAnim3" ("none"): animation ANIM3
	char				f_1ce0[100];	// 0x1ce0 "PNGFile" ("dolphin01.png")
	int					f_1d44;			// 0x1d44 "DoubleSided" (0)
	int					f_1d48;			// 0x1d48 "SpecialCase" (0)
	CGameObject			*f_1d4c;		// 0x1d4c SpecialCase 5: the "C3DROCKETSHIP" object
	CGameObject			*f_1d50;		// 0x1d50 SpecialCase 5: "JIM1"
	CGameObject			*f_1d54;		// 0x1d54 SpecialCase 5: the "C3DGODDARD" object
	float				f_1d58;			// 0x1d58 barrel roll time left
};

namespace neg {
// C3DJimmy hook (weak in C3DFish.cpp): his element slot 134 (0x44bd90), hurt by <damage> (loses
// <points>), <time> seconds of protection after; <a>: also checks his shield gadget.
void JimmyHurt(CGameObject *jimmy, float damage, int points, float time, unsigned char a);
}
