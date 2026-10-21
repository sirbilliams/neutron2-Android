/*  C3DCorona ('3COR') - a glowing billboard (sprites.omt canvas 36 by default) drawn additively
    in "Red/Green/Blue" at "Opacity", that with "AutoMod" spins at "SpinRate" and pulses its size
    by sin(t * "GrowSpeed") * "GrowMax" around SpriteSize. Placed on its own, or made by
    C3DTransportEffect and C3DPickupItem as their glow. Hidden in windowed mode.
    Reconstructed from Neutron2.exe: constructor 0x428150 (factory '3COR', 0x694 bytes),
    destructor 0x428320 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x4282f0 /
    0x428570 / 0x428580). Vtables 0x4d57e0 (element, 55 slots, none overridden) / 0x4d57cc
    (listener) / 0x4d5350 (game object: 7, 240, 260 overridden; 16 and 242 C3DSpriteType's) /
    streamer. Own members 0x668..0x68c, vtordisp 0x68c, the streamer at 0x690.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DCorona : public C3DSpriteType
{
	public:
	C3DCorona();															// 0x428150
	virtual ~C3DCorona();													// 0x428320

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x428360
	virtual void Process(float dt);											// SLOT 240 0x4284d0 spin and pulse
	virtual void v260(void);												// SLOT 260 0x428440 load, colour, additive (hidden in a window)

	// FIELD 0x668 0x24
	float		m_red;					// 0x668  "Red" (= 1.0)
	float		m_green;				// 0x66c  "Green" (= 0.1)
	float		m_blue;					// 0x670  "Blue" (= 0.1)
	float		m_time;					// 0x674  pulse time
	float		m_spin_rate;			// 0x678  "SpinRate" (= 80, degrees per second)
	float		m_grow_speed;			// 0x67c  "GrowSpeed" (= 2)
	float		m_grow_max;				// 0x680  "GrowMax" (= 40)
	float		m_opacity;				// 0x684  "Opacity" (= 1.0)
	int			m_auto_mod;				// 0x688  "AutoMod" (= 1: spin and pulse)
};
