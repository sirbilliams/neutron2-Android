/*  C3DAIGranny ('3GRN') - a C3DAI animated with RAD Granny (f_6d0 = 1) instead of OMT morphs:
    the walking, talking characters (the default model is "nummey": nummeybase/walk/talk/stop.grn).
    Its AI animation names are WALK for running (f_ac4) and STOP for sitting and waiting (f_b14,
    f_a9c); the model and its animations come from the *Animation properties and are loaded once
    (ev69, from v260). Touched by the camera (Jimmy) it plays TouchSound and TouchAnimA (as its
    FLY / RUN animation for the moment), then TouchAnimB (ev67), and goes back to STOP after
    ResetToStop seconds.
    Reconstructed from Neutron2.exe: constructor 0x415e00 (factory '3GRN', 0x2144 bytes),
    destructor 0x416220 (deleting thunks 0x4161f0 / 0x416aa0 / 0x416ab0 / 0x416ac0).

    Original layout: as C3DAI (element vtable 0x4cbe44, 144 slots, 67 and 69 overridden;
    listener 0x4cbe30; CLocalGameObject vtable 0x4cb978 at +0xc0), own members 0x1b50..0x213c,
    vtordisp 0x213c, streamer 0x2140.

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code of the original sees them at offset - 0xc0).            */
#pragma once
#include "C3DAI.h"

class C3DAIGranny : public C3DAI
{
	public:
	C3DAIGranny();															// 0x415e00
	virtual ~C3DAIGranny();													// 0x416220

	// ---- element-family slots
	virtual void ev67(void);												// ESLOT 67 0x4169d0 animation ended: after TOUCHANIMA back to our own, then TOUCHANIMB
	virtual void ev69(void);												// ESLOT 69 0x4166e0 load the model and the animations (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4162c0 properties
	virtual void UnInitObject(void);										// SLOT 8   0x4162b0 (C3DAnimated's)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x416590 the camera: TouchSound, TouchAnimA
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x417f40 collides with the camera, '3DOG', '3JIM', '3LBW' only
	virtual void Process(float dt);											// SLOT 240 0x416510 ResetToStop countdown
	virtual void v260(void);												// SLOT 260 0x416480 properties loaded

	// ---- data members (object offsets)
	char				f_1b50[100];	// 0x1b50 "BASEAnimation" (model, "nummeybase.grn")
	char				f_1bb4[100];	// 0x1bb4 "WALKAnimation" ("nummeywalk.grn")
	char				f_1c18[100];	// 0x1c18 "TALKAnimation" ("nummeytalk.grn")
	char				f_1c7c[100];	// 0x1c7c "STOPAnimation" ("nummeystop.grn")
	char				f_1ce0[100];	// 0x1ce0 "RUNAnimation" ("none")
	char				f_1d44[100];	// 0x1d44 "FLYAnimation" ("none")
	char				f_1da8[100];	// 0x1da8 "ANIM1Animation" ("none")
	char				f_1e0c[100];	// 0x1e0c "ANIM2Animation" ("none")
	char				f_1e70[100];	// 0x1e70 "ANIM3Animation" ("none")
	char				f_1ed4[100];	// 0x1ed4 "ANIM4Animation" ("none")
	char				f_1f38[100];	// 0x1f38 "TouchSound" ("none")
	char				f_1f9c[100];	// 0x1f9c "TouchAnimA" ("none")
	char				f_2000[100];	// 0x2000 "TouchAnimB" ("none")
	char				f_2064[100];	// 0x2064 our FLY animation name while TOUCHANIMA plays
	char				f_20c8[100];	// 0x20c8 our RUN animation name while TOUCHANIMA plays
	int					f_212c;			// 0x212c 0
	float				f_2130;			// 0x2130 "ResetToStop" (-1: never)
	float				f_2134;			// 0x2134 time left until STOP
	int					f_2138;			// 0x2138 "PassThru" (-1; 1: v34(1))
};
