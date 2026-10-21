/*  C3DGreenThingie ('3GRT') - a green thingie: a small spinning pickup (sprites.omt canvas 295,
    size 30) facing the viewport. Touched by the camera (Jimmy) it gives him a green flash and
    a burst of 13 of his sparkles, 0.5 neutron energy and 10 points, plays "greenthingy" and is
    hidden for good (state saved).
    Reconstructed from Neutron2.exe: constructor 0x43d0f0 (factory '3GRT', 0x680 bytes),
    destructor (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x43d260 /
    0x43d700 / 0x43d710). Vtables 0x4e08d8 (element, 55 slots: C3DSprite's) / 0x4e08c4
    (listener) / 0x4e0448 (game object) / 0x4e0438 (streamer). Slots 231, 284 and 285 are code
    shared with C3DElectron (231 also with C3DHelpSound / C3DNeutron). Same layout as
    C3DElectron: own members 0x668..0x678 (0x670 / 0x674 not used), vtordisp 0x678, the
    streamer at 0x67c.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DGreenThingie : public C3DSpriteType
{
	public:
	C3DGreenThingie();														// 0x43d0f0
	virtual ~C3DGreenThingie();

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43d2d0 collected by the camera
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x43d5b0 spin about z
	virtual void v260(void);												// SLOT 260 0x43d630 (additive when windowed off), facing the viewport
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x43d6b0 + collected
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x42e570 + collected (hidden / shown)

	// FIELD 0x668 0x10
	float			m_spin;				// 0x668  spin speed, degrees / s (200)
	unsigned char	m_collected;		// 0x66c  taken (saved)
	float			f_670;				// 0x670  (not used)
	float			f_674;				// 0x674  (not used)
};
