/*  C3DTransportEffect ('3TSP') - the glow of a transporter: an additive spinning, pulsing
    billboard (sprites.omt canvas 194, or with "EffectType" 0 the blue one, 233) with a C3DCorona
    of its own (canvas 194 / 229) on the same spot. Toggled (ToggleObject, CLocalGameObject slot
    268) it fades over "TimeToFade" seconds: when hidden ("InitiallyVisible" 0) toggling it with
    1 / -1 flashes it in and out; when shown toggling it with 0 fades it out. Game mode 5 hides
    it at once.
    Reconstructed from Neutron2.exe: constructor 0x477450 (factory '3TSP', 0x644 bytes),
    destructor 0x4776a0 (only resets the vtables, then ~C3DSprite; deleting thunks 0x477670 /
    0x477d40 / 0x477d50). Vtables 0x500a44 (element, 55 slots, none overridden) / 0x500a30
    (listener) / 0x5005b4 (game object: 7, 240, 260, 268, 270 overridden) / 0x5005a4 (streamer).
    Base C3DSprite (not C3DSpriteType); own members 0x600..0x63c, vtordisp 0x63c, the streamer
    at 0x640.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSprite.h"

class C3DCorona;

class C3DTransportEffect : public C3DSprite
{
	public:
	C3DTransportEffect();													// 0x477450
	virtual ~C3DTransportEffect();											// 0x4776a0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4776e0
	virtual void Process(float dt);											// SLOT 240 0x4779c0 fade, spin and pulse
	virtual void v260(void);												// SLOT 260 0x4777f0 load, colour, the corona
	virtual void v268(int a2);												// SLOT 268 0x477c60 toggled: start a fade
	virtual void v270(int a2, int a3);										// SLOT 270 0x477cd0 game mode 5: hidden

	// FIELD 0x600 0x3c
	float		m_red;					// 0x600  "Red" (= 1.0)
	float		m_green;				// 0x604  "Green" (= 0.1)
	float		m_blue;					// 0x608  "Blue" (= 0.1)
	float		m_time;					// 0x60c  pulse time
	float		m_spin_rate;			// 0x610  "SpinRate" (= 80)
	float		m_grow_speed;			// 0x614  "GrowSpeed" (= 2)
	float		m_grow_max;				// 0x618  "GrowMax" (= 40)
	float		m_opacity;				// 0x61c  "Opacity" (= 1.0)
	int			m_auto_mod;				// 0x620  spin and pulse (= 1; handed to the corona)
	float		m_fade_left;			// 0x624  fade time left
	float		m_time_to_fade;			// 0x628  "TimeToFade" (= 4)
	int			m_initially_visible;	// 0x62c  "InitiallyVisible" (= 1)
	int			m_shown;				// 0x630  shown (= InitiallyVisible on load)
	C3DCorona	*m_corona;				// 0x634  its corona
	int			m_effect_type;			// 0x638  "EffectType" (= 1; 0: blue)
};
