/*  C3DTransportEffect - see C3DTransportEffect.h. Reconstructed from Neutron2.exe
    0x477450..0x477d80. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DTransportEffect.h"
#include "C3DCorona.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>

using namespace neg;

// 0x4ba850 (_ftol): truncation.
static int ftol(double v) { return (int)v; }

C3DTransportEffect::C3DTransportEffect()					// 0x477450
{
	SetName((char *)"C3DTRANSPORTEFFECT");
	Log("C3DTRANSPORTEFFECT()");
	C3DTransportEffect::InitObject();						// (called directly, as in the original)
	SetObjectID('3TSP');									// element slot 48
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 194;
	m_sprite_size = 100;
	v42(0);
	v40(0);
	v52(0);
	v107(1);
	v46(0);
	m_red = 1.0f;
	m_green = 0.1f;
	m_blue = 0.1f;
	m_time = 0;
	m_spin_rate = 80.0f;
	m_grow_speed = 2.0f;
	m_grow_max = 40.0f;
	m_opacity = 1.0f;
	m_auto_mod = 1;
	m_initially_visible = 1;
	f_008 = 1;												// element +0xd0
	m_shown = 1;
	m_fade_left = 0;
	m_time_to_fade = 4.0f;
	m_corona = new C3DCorona;								// 0x694 bytes (a game object of its own)
	m_corona->v107(0);										// CGameObject +0x1ac
	m_corona->m_sprite_index = 194;
	m_effect_type = 1;
	C3DSprite::InitPhysics();								// 0x49ca30
	// (an empty debug log)
}

C3DTransportEffect::~C3DTransportEffect() {}				// 0x4776a0 (the corona is left to DeleteAllObjects)

namespace neg {
CGameObject *New_C3DTransportEffect(void) { return new C3DTransportEffect; }	// '3TSP' (0x644 bytes)
}

void C3DTransportEffect::InitObject(void)					// 0x4776e0
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("Red", &m_red, PROP_FLOAT, 0);
	RegisterProperty("Green", &m_green, PROP_FLOAT, 0);
	RegisterProperty("Blue", &m_blue, PROP_FLOAT, 0);
	RegisterProperty("SpinRate", &m_spin_rate, PROP_FLOAT, 0);
	RegisterProperty("GrowSpeed", &m_grow_speed, PROP_FLOAT, 0);
	RegisterProperty("GrowMax", &m_grow_max, PROP_FLOAT, 0);
	RegisterProperty("Opacity", &m_opacity, PROP_FLOAT, 0);
	RegisterProperty("InitiallyVisible", &m_initially_visible, PROP_LONG, 0);
	RegisterProperty("TimeToFade", &m_time_to_fade, PROP_FLOAT, 0);
	RegisterProperty("EffectType", &m_effect_type, PROP_LONG, 0);
}

// 0x4779c0: while a fade runs the corona follows us at 70 % of our pulse, counter-spinning, and
// both take the fade's alpha: shown, a straight fade out over TimeToFade (then the corona is
// hidden); hidden, up and down again (half of TimeToFade each).
void C3DTransportEffect::Process(float dt)
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	if (m_fade_left > 0)
	{
		float p[4];
		v195(p);											// +0x30c
		m_corona->v196(p[0], p[1], p[2], p[3]);				// +0x310
		m_corona->ShowAgain();								// element +0xd4
		m_corona->m_spin_rate = m_spin_rate * -0.5f;		// 0x4d2ca0
		m_corona->m_grow_max = m_grow_max * 0.7f;			// 0x4cceec
		m_corona->m_grow_speed = (float)(m_grow_speed * 0.7);	// (double 0.7)
		m_corona->m_sprite_size = ftol(m_sprite_size * 0.8);	// (double 0.8)
		const float half = m_time_to_fade * 0.5f;
		float a = 0;
		if (m_shown == 0)
		{
			if (m_fade_left > half) { if (m_time_to_fade > 0) a = (m_time_to_fade - m_fade_left) / half; }
			else if (half > 0) a = m_fade_left / half;
		}
		if (m_shown == 1 && m_time_to_fade > 0) a = m_fade_left / m_time_to_fade;
		if (m_fade_left <= 0)								// (never true here)
		{
			a = 0;
			if (m_shown == 1)
			{
				m_shown = 0;
				m_corona->HideAgain();						// element +0xd8
			}
		}
		const OMediaFARGBColor c(a, m_red, m_green, m_blue);
		set_diffuse(c);										// +0x90.. (r, g, b, a)
		m_corona->set_diffuse(c);
		m_fade_left -= dt;
	}
	if (!m_auto_mod) return;
	AddRotation(0, 0, dt * m_spin_rate);					// +0x330
	m_time = dt + m_time;
	const float s = (float)(sin((double)(m_time * m_grow_speed)) * m_grow_max + (double)m_sprite_size);
	set_size(s, s);											// 2002 +0xac / +0xb0
}

void C3DTransportEffect::v260(void)							// 0x4777f0
{
	C3DSprite::v260();										// 0x49cfd0
	set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);	// +0xb4 = 6, +0xb8 = 7 / 1
	set_diffuse(OMediaFARGBColor(m_opacity, m_red, m_green, m_blue));	// +0x90..
	set_flags(omelf_SecondPass | omelf_FaceViewport);		// +0x7c = 0x208
	m_shown = m_initially_visible;
	if (m_effect_type == 0)									// the blue one
	{
		m_corona->m_red = 0.1f;
		m_corona->m_green = 0.1f;
		m_corona->m_blue = 1.0f;
		m_corona->m_sprite_index = 229;
		m_corona->v258();									// +0x408: reload its sprite
		m_sprite_index = 233;
		m_red = 0.1f;
		m_green = 0.1f;
		m_blue = 1.0f;
		v258();
	}
	m_corona->m_grow_max = m_grow_max * 0.8f;				// 0x4caa58
	m_corona->m_auto_mod = m_auto_mod;
	float p[4];
	v195(p);
	m_corona->v196(p[0], p[1], p[2], p[3]);
	if (m_initially_visible == 0)
	{
		HideAgain();										// element +0xd8
		m_corona->HideAgain();
		m_corona->m_opacity = 0;
	}
}

void C3DTransportEffect::v268(int a2)						// 0x477c60
{
	CLocalGameObject::v268(a2);								// (empty)
	if (a2 == -1 || a2 == 1)
	{
		if (m_shown != 0) return;
		m_fade_left = m_time_to_fade;
	}
	else
	{
		if (a2 != 0) return;
		if (m_shown != 1) return;
		m_fade_left = m_time_to_fade;
	}
	ShowAgain();											// element +0xd4
	m_corona->ShowAgain();
}

void C3DTransportEffect::v270(int a2, int a3)				// 0x477cd0
{
	CLocalGameObject::v270(a2, a3);
	if (a3 != 5) return;
	m_fade_left = 0;
	HideAgain();
	m_corona->HideAgain();
}
