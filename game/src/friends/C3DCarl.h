/*  C3DCarl ('3CAR') - Carl, an OMT morph friend (.ase animations, texture "carl.png"). In the
    museum level LV5C he is the giant Carl (giantcarl*.ase) walking about: water balloons that
    hit him in the face feed him (EAT, a hunger bar over his head), the others bounce off; once
    fed enough (f_1f84 >= 100) he SHRINKs, and the end of SHRINK calls the CARLSHRINK trigger.
    His walk shakes Jimmy's camera at two steps (BOOM1 / BOOM2). In LV5F he is small Carl
    (SMALLSTOP / RUN).
    Reconstructed from Neutron2.exe: constructor 0x425840 (factory '3CAR', 0x1f98 bytes),
    destructor 0x425af0 (deleting thunks 0x425ac0 / 0x426a30 / 0x426a40 / 0x426a50).

    Original layout: as C3DFriends (element vtable 0x4d31b8, 147 slots: 58, 67, 69 overridden;
    listener 0x4d31a4; OMediaAnim 0x4d3168; CLocalGameObject vtable 0x4d2cec at +0xc0), own
    members 0x1d74..0x1f8c, vtordisp 0x1f90, streamer 0x1f94.

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code of the original sees them at offset - 0xc0).            */
#pragma once
#include "C3DFriends.h"
#include "../InterfaceElement.h"

class C3DCarl : public C3DFriends
{
	public:
	C3DCarl();																// 0x425840
	virtual ~C3DCarl();														// 0x425af0

	// ---- element-family slots
	virtual void ev58(char *name, unsigned char loop);						// ESLOT 58 0x4268f0 GROW / KONG / WALK move the shadow away
	virtual void ev67(void);												// ESLOT 67 0x4267e0 animation ended: WALK again, SHRINK -> CARLSHRINK
	virtual void ev69(void);												// ESLOT 69 0x425be0 load the morph animations and the texture (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43e920
	virtual void UnInitObject(void);										// SLOT 8   0x425bb0
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x425e90 water balloons: eat or bounce
	virtual void Process(float dt);											// SLOT 240 0x426380 steps, the hunger bar
	virtual void v260(void);												// SLOT 260 0x425da0 properties loaded
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x425e60 C3DAnimated's, then ev70(1)
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x4269e0 + f_1f84
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x4269b0 + f_1f84

	// ---- data members (object offsets)
	unsigned char		f_1d74;			// 0x1d74 BOOM1 done this walk cycle (shake at 0.35 s)
	unsigned char		f_1d75;			// 0x1d75 BOOM2 done this walk cycle (shake at 1.7 s)
	CGameObject			*f_1d78;		// 0x1d78 "JIM1" (v260): whose camera the steps shake
	InterfaceElt		f_1d7c;			// 0x1d7c hunger bar frame (screens canvas 411)
	InterfaceElt		f_1e80;			// 0x1e80 hunger bar (screens canvas 359, width f_1f84 / 2)
	float				f_1f84;			// 0x1f84 how much he has eaten (100: SHRINK)
	float				f_1f88;			// 0x1f88 time left eating (8 s), the bar shows meanwhile
};
