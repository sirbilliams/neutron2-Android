/*  C3DFlyingBot - the base of the VR level's flying enemies (C3DFlyingBug '3BUG', C3DGiantWasp
    '3GWA'); no factory ID of its own. A hovering C3DAI that wanders and attacks (AIState 11) at
    400 within 500 (animations FLY / RUN = "RUN", WAG / SIT = "STOP", WALK = "SHOOT").
    Hit by a water balloon or an energy projectile ("vrzap", the shot is used up): the first hit
    shows the second texture (the wire grid) and lit blending for 10 s; a second hit within those
    10 s knocks it out: it stops hovering, stands (state 1, STOP) and spins for f_1b60 seconds
    (135; the bug: 60000, for good), 20 points. Then it flies again (state AIState). Touching the
    camera makes it flee (state 5) for 20 s / 3000. Every 5 s a ray 600 down: near the ground it
    is lifted (dt * 90) and pushed up (v162(0, 200, 0)).
    Reconstructed from Neutron2.exe: constructor 0x432b20, destructor 0x432d40 (only resets the
    vtables, then ~C3DAI; deleting thunks 0x432d10 / 0x4332f0 / 0x433300 / 0x433310). Vtables:
    element 0x4db6e0 (C3DAI's 144), game object (+0xc0) 0x4db214 (0, 7, 16, 231, 240, 260
    overridden), streamer 0x4db204. Own members 0x1b50..0x1b68, vtordisp 0x1b68, streamer 0x1b6c.
    Element-family slots are `ev<n>`; members are f_<offset in the original object> (CGameObject-
    family code sees them at offset - 0xc0).                                                   */
#pragma once
#include "../C3DAI.h"

class C3DFlyingBot : public C3DAI
{
	public:
	C3DFlyingBot();															// 0x432b20
	virtual ~C3DFlyingBot();												// 0x432d40

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x467a80 C3DAI's properties
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x433050 hit by a shot: lit, then knocked out
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x433280 collides with the camera, '3JIM', '3EPR', '3GOD', '3WBL'
	virtual void Process(float dt);											// SLOT 240 0x432d90 lit / knocked-out timers, the ground check
	virtual void v260(void);												// SLOT 260 0x433220 properties loaded: ranges, opaque

	// ---- data members (object offsets)
	float				f_1b50;			// 0x1b50 knocked out: time left (spinning)
	float				f_1b54;			// 0x1b54 time to the next ground check (5 s)
	int					f_1b58;			// 0x1b58 (never set)
	float				f_1b5c;			// 0x1b5c the speed to fly on with (400)
	float				f_1b60;			// 0x1b60 how long a knock-out lasts (135; the bug 60000)
	float				f_1b64;			// 0x1b64 lit after a hit: time left (10 s per hit)
};
