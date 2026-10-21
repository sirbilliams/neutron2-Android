/*  C3DElectron ('3ELE') - an electron: a small spinning pickup (sprites.omt canvas 219, size
    80), additive, facing the viewport. It tumbles about x and y at random rates. Touched by the
    camera (Jimmy) it bursts into 13 of Jimmy's sparkles, gives 1 neutron energy and 2 points,
    counts towards the electron indicator (every 3 a step, every 6 steps an extra life), plays
    "blip4" and is hidden for good (state saved).
    Reconstructed from Neutron2.exe: constructor 0x42df20 (factory '3ELE', 0x680 bytes),
    destructor 0x42e150 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x42e120
    / 0x42e5f0 / 0x42e600). Vtables 0x4d8aa0 (element, 55 slots: C3DSprite's) / 0x4d8a8c
    (listener) / 0x4d8610 (game object) / 0x4d8600 (streamer). Slots 231, 284 and 285 are code
    shared with C3DGreenThingie (231 also with C3DHelpSound / C3DNeutron). Own members
    0x668..0x678, vtordisp 0x678, the streamer at 0x67c.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DElectron : public C3DSpriteType
{
	public:
	C3DElectron();															// 0x42df20
	virtual ~C3DElectron();													// 0x42e150

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x42e190 collected by the camera
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x42e460 tumble
	virtual void v260(void);												// SLOT 260 0x42e4e0 additive, facing the viewport
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x43d6b0 + collected
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x42e570 + collected (hidden / shown)

	// FIELD 0x668 0x10
	float			m_spin;				// 0x668  spin speed, degrees / s (900)
	unsigned char	m_collected;		// 0x66c  taken (saved)
	float			m_rate_x;			// 0x670  x tumble factor (0.5..1.5, random)
	float			m_rate_y;			// 0x674  y tumble factor (0.5..1.5, random)
};
