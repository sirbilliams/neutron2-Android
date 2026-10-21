/*  C3DNumey ('3NUM') - Nummy (Negatron's lab, Level3): a Granny friend (model
    "nummeybase.grn"; TALK, WALK (FLAIL "NummyFlail.grn" instead in LV4B), STOP, SCHEME,
    SMALLSTOP, LAUGH, TYPE, TALKAROG, IDLEAROG, FLIP, TALKMAD, IDLEMAD, LAUGHEVIL, FLIPS,
    FLYAIR, IDLEMAD2, FOWLLAUGH, GIVE, DOOM), waiting (AIState 1) at 160, VisibleRange 1000.
    A cut scene that finds him stopped makes him SCHEME; so does a missing animation.
    Reconstructed from Neutron2.exe: constructor 0x45ab70 (factory '3NUM', 0x1d7c bytes),
    destructor 0x45ad60 (deleting thunks 0x45ad30 / 0x45b240 / 0x45b250 / 0x45b260).

    Original layout: as C3DFriends (element vtable 0x4ee724, 147 slots; listener 0x4ee710;
    OMediaAnim 0x4ee6d4; CLocalGameObject vtable 0x4ee258 at +0xc0), no members of its own.
    Slot 126 is 0x45b110 (a jump to the empty 0x456d10).                                     */
#pragma once
#include "C3DFriends.h"

class C3DNumey : public C3DFriends
{
	public:
	C3DNumey();																// 0x45ab70
	virtual ~C3DNumey();													// 0x45ad60

	// ---- element-family slots
	virtual void ev58(char *name, unsigned char loop);						// ESLOT 58 0x45b1f0 (calls C3DAnimated's)
	virtual void ev60(void);												// ESLOT 60 0x45b210 no such animation: SCHEME
	virtual void ev69(void);												// ESLOT 69 0x45adf0 load the model and the animations (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x45b0e0
	virtual void Process(float dt);											// SLOT 240 0x45b0d0
	virtual void v260(void);												// SLOT 260 0x417110
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x436900
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x45b120 "C3DNumey::ChangeState": SCHEME
};
