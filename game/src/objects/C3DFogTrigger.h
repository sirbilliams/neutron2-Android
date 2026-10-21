/*  C3DFogTrigger ('3FOG') - when the camera touches it (or a ToggleObject toggles it) it sets
    the level's fog: "Red/Green/Blue", "StartFog".."EndFog", far plane "Clipdist", or with
    "EnableFog" 0 turns the fog off; "ChangeLighting" also greys the terrain to "LightingVal";
    "FogSound" starts a looping ambient clip (one for all fog triggers, 0x5367a0, stopped by the
    next trigger and by the destructor). A sprite ("icons.omt" canvas 12, size 100).
    Reconstructed from Neutron2.exe: constructor 0x435fd0, destructor 0x4361e0 (stops the fog
    sound, then ~C3DSpriteType; deleting thunks 0x436520 / 0x4361b0 / 0x436530). Vtables
    0x4dd30c (element, 56 slots: C3DSprite's 55 + TriggerFog) / 0x4dd2f8 (listener) / 0x4dce7c
    (game object) / 0x4dce6c (streamer: C3DSprite's read_class / write_class copies 0x4275e0 /
    0x4275f0). 0x6f8 bytes: C3DSpriteType's 0x668, own members 0x668..0x6f0, vtordisp 0x6f0,
    the streamer at 0x6f4.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DFogTrigger : public C3DSpriteType
{
	public:
	C3DFogTrigger();														// 0x435fd0
	virtual ~C3DFogTrigger();												// 0x4361e0

	// ---- element-family slots it adds
	virtual void TriggerFog(void);											// ESLOT 55 0x436390 (+0xdc) set the fog

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x436280
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x4364c0 the camera touched it: fire
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x45ce70 collides with the camera and '3JIM' only
	virtual void v268(int a2);												// SLOT 268 0x43dd60 ToggleObject: fire

	// FIELD 0x668 0x88
	float		m_clip_dist;			// 0x668  "Clipdist" (= 12000: the far plane)
	float		m_start_fog;			// 0x66c  "StartFog" (= 1000)
	float		m_end_fog;				// 0x670  "EndFog" (= 10000)
	float		m_red;					// 0x674  "Red" (= 1)
	float		m_green;				// 0x678  "Green" (= 1)
	float		m_blue;					// 0x67c  "Blue" (= 1)
	int			m_enable_fog;			// 0x680  "EnableFog" (= 1; 0: turn the fog off)
	char		m_fog_sound[100];		// 0x684  "FogSound" (= "none")
	int			m_change_lighting;		// 0x6e8  "ChangeLighting" (= 0)
	float		m_lighting_val;			// 0x6ec  "LightingVal" (= 1)
};
