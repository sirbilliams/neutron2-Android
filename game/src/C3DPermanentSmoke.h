/*  C3DPermanentSmoke - a puff of smoke from the permanent sprite set (sprite 7 of
    "retainedsprites.omt", 80 units, ObjectID '3BAS'). C3DRocketShip keeps a ring of 20 of them
    for its exhaust trail; no factory ID.
    Reconstructed from Neutron2.exe: constructor 0x45d220 (0x624 bytes), destructor 0x45d3f0
    (only resets the vtables, then ~C3DPermanentSprite; deleting thunks 0x45d3c0 / 0x45d5a0 /
    0x45d5b0).

    Original layout: as C3DSprite (element vtable 0x4f007c, 55 slots; listener 0x4f0068 at +0x30;
    CLocalGameObject vtable 0x4efbec at +0xc8), own members 0x600..0x61c, vtordisp 0x61c,
    streamer 0x620.

    While f_618 is set, Process grows it (SpriteSize += f_614 per second, back to 80 past 400) and
    fades it out over f_610 seconds in the colour f_604..f_60c, then hides it. (Nothing in the
    game sets f_618: the rocket ship places, shows and colours its puffs itself.)            */
#pragma once
#include "C3DPermanentSprite.h"

class C3DPermanentSmoke : public C3DPermanentSprite
{
	public:
	C3DPermanentSmoke();													// 0x45d220
	virtual ~C3DPermanentSmoke();											// 0x45d3f0

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x45d430 grow and fade (while f_618)
	virtual void v260(void);												// SLOT 260 0x45d540 sprite, physics, no collision, blending

	// ---- data members (object offsets)
	float				f_600;			// 0x600  time since the puff started (0)
	float				f_604;			// 0x604  red (1)
	float				f_608;			// 0x608  green (1)
	float				f_60c;			// 0x60c  blue (1)
	float				f_610;			// 0x610  fade time (0.5)
	float				f_614;			// 0x614  growth, units / s (120)
	unsigned char		f_618;			// 0x618  growing and fading (0)
};
