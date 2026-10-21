/*  C3DBalloon ('3BAL') - a balloon (Level5c): a sprite (sprites.omt canvas 298, size 100)
    tinted "Red" / "Green" / "Blue" (alpha 0.8). It starts tied: Jimmy (the camera) touching it
    unties it, and it then rises (vertical speed capped at 70, no sideways drift, gravity
    cancelled with an acceleration of 10). A baseball or a rocket pops it: hidden, "balloon
    pop", 10 points (200 while still tied) plus (distance from the camera / 30 - 33) when the
    shot came from farther than 1000.
    Reconstructed from Neutron2.exe: constructor 0x421300 (factory '3BAL', 0x680 bytes),
    destructor 0x4214e0 (vtable resets only, then ~C3DSpriteType; deleting thunks 0x4214b0 /
    0x421810 / 0x421820).

    Original layout: as C3DSpriteType (element vtable 0x4cf15c, 55 slots: C3DSprite's;
    CLocalGameObject vtable 0x4ceccc at +0xc8), own members 0x668..0x678, vtordisp 0x678,
    streamer 0x67c. Members are named after their 2002 offset from the object start (the
    CGameObject-family methods of the original see them 0xc8 lower).                         */
#pragma once
#include "../C3DSpriteType.h"

class C3DBalloon : public C3DSpriteType
{
	public:
	C3DBalloon();															// 0x421300
	virtual ~C3DBalloon();													// 0x4214e0

	// ---- CGameObject-family overrides
	virtual void ResetObject(void);											// SLOT 10  0x421520 tied again, still, tinted
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x4215a0 untied by Jimmy, popped by a shot
	virtual void Process(float dt);											// SLOT 240 0x421720 rise once untied

	// FIELD 0x668 0x10
	float			m_red;				// 0x668  "Red" (1)
	float			m_green;			// 0x66c  "Green" (1)
	float			m_blue;				// 0x670  "Blue" (1)
	unsigned char	m_popped;			// 0x674  (0)
	unsigned char	m_tied;				// 0x675  (1)
};
