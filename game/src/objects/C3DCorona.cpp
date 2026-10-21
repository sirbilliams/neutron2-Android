/*  C3DCorona - see C3DCorona.h. Reconstructed from Neutron2.exe 0x428150..0x4285b0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCorona.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>

using namespace neg;

C3DCorona::C3DCorona()										// 0x428150
{
	SetName((char *)"C3DCORONA");
	Log("C3DCorona()");
	C3DCorona::InitObject();								// (called directly, as in the original)
	SetObjectID('3COR');									// element slot 48
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 36;
	m_sprite_size = 100;
	v42(0);
	v40(0);
	v52(0);
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
	// (an empty debug log)
}

C3DCorona::~C3DCorona() {}									// 0x428320

namespace neg {
CGameObject *New_C3DCorona(void) { return new C3DCorona; }	// '3COR' (0x694 bytes)
}

void C3DCorona::InitObject(void)							// 0x428360
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("Red", &m_red, PROP_FLOAT, 0);
	RegisterProperty("Green", &m_green, PROP_FLOAT, 0);
	RegisterProperty("Blue", &m_blue, PROP_FLOAT, 0);
	RegisterProperty("SpinRate", &m_spin_rate, PROP_FLOAT, 0);
	RegisterProperty("GrowSpeed", &m_grow_speed, PROP_FLOAT, 0);
	RegisterProperty("GrowMax", &m_grow_max, PROP_FLOAT, 0);
	RegisterProperty("Opacity", &m_opacity, PROP_FLOAT, 0);
	RegisterProperty("AutoMod", &m_auto_mod, PROP_LONG, 0);
}

void C3DCorona::Process(float dt)							// 0x4284d0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	if (!m_auto_mod) return;
	AddRotation(0, 0, dt * m_spin_rate);					// +0x330
	m_time = dt + m_time;
	// (x87: the size is computed in extended precision, then stored to both)
	const float s = (float)(sin((double)(m_time * m_grow_speed)) * m_grow_max + (double)m_sprite_size);
	set_size(s, s);											// 2002 +0xac / +0xb0
}

void C3DCorona::v260(void)									// 0x428440
{
	C3DSprite::v260();										// 0x49cfd0 (not C3DSpriteType's)
	if (g_windowed)											// 0x56afab
	{
		HideAgain();										// element +0xd8
		return;
	}
	set_blend(omblendfc_Src_Alpha, omblendfc_One);			// +0xb4 = 6, +0xb8 = 1
	set_diffuse(OMediaFARGBColor(m_opacity, m_red, m_green, m_blue));	// +0x90.. (r, g, b, a)
	set_flags(omelf_SecondPass | omelf_FaceViewport);		// +0x7c = 0x208
}
