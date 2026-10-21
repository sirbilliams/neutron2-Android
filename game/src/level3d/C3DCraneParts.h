/*  The two parts C3DCraneBoom ('3CRB') builds for itself (no factory IDs of their own):

    C3DChain ('3CHA' as its ObjectID) - the crane's cable ("cranecable.ase", "craneclaw.png"),
    hung from the boom at the trolley. Reconstructed from Neutron2.exe: constructor 0x426a80
    (0x92c bytes), destructor 0x426bd0 (only resets the vtables, then ~C3DAnimated; deleting
    thunks 0x426d30 / 0x426ba0 / 0x426d20 / 0x426d40). Original layout: as C3DAnimated
    (CLocalGameObject vtable at +0xc0), no members of its own (vtordisp 0x924, streamer 0x928).
    Slot 240 (0x45daf0) only calls C3DAnimated's, slot 260 is a jump to C3DAnimated's
    (0x46fc00): neither is overridden here. Its InitObject logs "C3DGraplingHook::InitObject()"
    (the code was copied from there).

    C3DCraneClaw ('3CLW' as its ObjectID) - the claw at the end of the cable ("craneclawopen.ase"
    OPEN / "craneclawclosed.ase" CLOSE), with a blob shadow (C3DShadow 140 x 60) on the ground
    below it. While active (the crane is being driven) it only collides with C3DOmtObj objects
    ('3OMT'): touching one not tagged PLATFORM it closes on it and carries it ("cranedrop", the
    boom winds up); touching a PLATFORM while carrying, it hands the crate to the platform (its
    ev94), plays "cranedrop" and one of Jimmy's "jimmyitem1..5", scores 100 and opens again.
    Reconstructed from Neutron2.exe: constructor 0x4293c0 (0x93c bytes), destructor 0x4295d0
    (only resets the vtables, then ~C3DAnimated; deleting thunks 0x429c30 / 0x4295a0 / 0x429c20 /
    0x429c40). Original layout: as C3DAnimated (CLocalGameObject vtable at +0xc0), own members
    0x924..0x934, vtordisp 0x934, streamer 0x938.                                           */
#pragma once
#include "../C3DAnimated.h"

class C3DCraneBoom;
class C3DShadow;

class C3DChain : public C3DAnimated
{
	public:
	C3DChain();																// 0x426a80
	virtual ~C3DChain();													// 0x426bd0

	virtual void InitObject(void);											// SLOT 7   0x426c20 the cable animation and texture
};

class C3DCraneClaw : public C3DAnimated
{
	public:
	C3DCraneClaw();															// 0x4293c0
	virtual ~C3DCraneClaw();												// 0x4295d0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x429620 animations, texture
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x4296d0 pick up / put down a crate
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x4299f0 while active: C3DOmtObj objects only
	virtual void Process(float dt);											// SLOT 240 0x429a20 the shadow, the crate carried along
	virtual void v260(void);												// SLOT 260 0x4299d0 + v152(95)

	void Drop(CGameObject *held);											// 0x429980 let go (the boom winds up)

	C3DCraneBoom		*m_boom;		// 0x924  the boom that built us
	C3DShadow			*m_shadow;		// 0x928  the blob shadow
	CGameObject			*m_held;		// 0x92c  the crate carried (= NULL)
	unsigned char		m_holding;		// 0x930  (= 0)
	unsigned char		m_active;		// 0x931  the crane is being driven (= 0; the boom sets it)
};
