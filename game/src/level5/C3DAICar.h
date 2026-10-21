/*  C3DAICar - a car that drives along its patrol points (a C3DAI in state 3, speed 400, turn
    rates 1, 2, 2). Hit by Jimmy, the jeep, Goddard or the rocket ship it honks ("horn") and
    stops where it is for 8 s (speed 0.0001, held at that point while touched), then drives on
    at 400. Something more than 480 above it is thrown off (velocity 700, 700, 200); Jimmy also
    loses his jump state. The base of C3DNeuCar ('3NCA'), C3DNeuCar2 ('3NC2') and C3DBus
    ('3SBU'); no factory ID of its own.
    Reconstructed from Neutron2.exe: constructor 0x4159b0, destructor 0x415b10 (deleting thunks
    0x415ae0 / 0x415d90 / 0x415da0 / 0x415db0).

    Original layout: as C3DAI (element vtable 0x4cb718, 144 slots, all C3DAI's; CLocalGameObject
    vtable 0x4cb24c at +0xc0), own members 0x1b50..0x1b6c, vtordisp 0x1b6c, streamer 0x1b70.
    Slot 260 (0x46e3c0) is a jump to C3DAI's, so it is not overridden.                         */
#pragma once
#include "../C3DAI.h"

class C3DAICar : public C3DAI
{
	public:
	C3DAICar();																// 0x4159b0
	virtual ~C3DAICar();													// 0x415b10

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x415be0 honk and stop; throw off what is on top
	virtual void RemoveTouching(CGameObject *other);						// SLOT 17  0x415d30 no longer held
	virtual void Process(float dt);											// SLOT 240 0x415b60 held in place; drive on when the time is up
	virtual void v277(void);												// SLOT 277 0x415d50 ev79(1)
	virtual void v278(void);												// SLOT 278 0x415d60 ev79(0)

	float				m_stop_time;	// 0x1b50 time left standing after a hit (= 0)
	OMedia3DVector		m_hold_pos;		// 0x1b54 where it was hit (0, 0, 0, 1)
	unsigned char		m_held;			// 0x1b64 touched: kept at m_hold_pos (= 0)
	int					f_1b68;			// 0x1b68 (= -1, not used)
};
