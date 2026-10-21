/*  C3DSporky ('3SPK') - Sporky, who delivers food (Level5c / Level5f): a C3DAI (visible range
    500, speed 200, state 1) with the morph animations TALK, BAGIN, BAGOUT, BAGLEAVE, RUN and STOP
    (texture "sporky.png", radius 150, v64 0.5, loaded once by ev69).
    "DeliverFood" 1 (default): he stands still playing BAGLEAVE (no AI, radius 700 / 500 / 300 by
    difficulty); Jimmy (the camera) touching him while the bag is there gets food: "jimmystore09",
    a green sparkle (10 s), JIM1's f_4d94 + 20 (at most 100); the bag goes in (BAGIN, then hidden)
    and comes back out (shown, BAGOUT, then BAGLEAVE again) after "RespawnTime" (30 s).
    "DeliverFood" 2: he patrols (state 3); 0: a plain C3DAI. His visibility is saved.
    Reconstructed from Neutron2.exe: constructor 0x4719e0 (factory '3SPK', 0x1b64 bytes; it
    inlines InitObject), destructor 0x471bd0 (vtable resets only, then ~C3DAI; deleting thunks
    0x471ba0 / 0x4721c0 / 0x4721d0 / 0x4721e0).

    Original layout: as C3DAI (element vtable 0x4fb460, 144 slots; CLocalGameObject vtable
    0x4faf94 at +0xc0), own members 0x1b50..0x1b5c, vtordisp 0x1b5c, streamer 0x1b60.        */
#pragma once
#include "../C3DAI.h"

class C3DSporky : public C3DAI
{
	public:
	C3DSporky();															// 0x4719e0
	virtual ~C3DSporky();													// 0x471bd0

	// ---- element-family overrides
	virtual void ev67(void);												// ESLOT 67 0x4720a0 BAGIN ended: hide; BAGOUT ended: BAGLEAVE
	virtual void ev69(void);												// ESLOT 69 0x471c20 load the model once

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x471d10 + "DeliverFood", "RespawnTime"
	virtual void ResetObject(void);											// SLOT 10  0x471e60 back to the start point
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x471f00 food for Jimmy
	virtual void Process(float dt);											// SLOT 240 0x472000 the bag comes back
	virtual void v260(void);												// SLOT 260 0x471d80 by DeliverFood
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x472150 + visible
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x472100 + visible

	float				m_respawn_time;	// 0x1b50 "RespawnTime" (30)
	int					m_deliver_food;	// 0x1b54 "DeliverFood" (1)
	float				m_bag_time;		// 0x1b58 time until the bag is back (0: there)
};
