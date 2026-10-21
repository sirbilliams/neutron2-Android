/*  C3DPermanentSmoke - see C3DPermanentSmoke.h. */
#include "C3DPermanentSmoke.h"
#include "Game.h"
#include "CGameType.h"
#include "gen/globals.h"
#include <string.h>

using namespace neg;

C3DPermanentSmoke::C3DPermanentSmoke()						// 0x45d220
{
	SetName((char *)"C3DPERMANENTSMOKE");
	Log("C3DPermanentSmoke()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3BAS');							// 0x49b550
	strcpy(m_sprite_db, "retainedsprites.omt");
	m_sprite_index = 7;
	m_sprite_size = 80;
	v46(1);
	C3DSprite::InitSpriteProperties(80.0f);					// 0x49ce10
	v67(15.0f);
	v107(0);
	v36(0);
	v52(0);
	f_600 = 0;
	f_610 = 0.5f;
	f_604 = 1.0f;
	f_608 = 1.0f;
	f_60c = 1.0f;
	f_614 = 120.0f;
	f_618 = 0;
}

C3DPermanentSmoke::~C3DPermanentSmoke()						// 0x45d3f0
{
}

void C3DPermanentSmoke::Process(float dt)					// 0x45d430
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	if (!f_618) return;
	f_600 += dt;
	m_sprite_size = (int)((double)dt * f_614 + m_sprite_size);	// 0x4ba850 (_ftol)
	if (m_sprite_size > 400) m_sprite_size = 80;
	const float size = (float)m_sprite_size;
	set_size(size, size);									// 2002 +0xac / +0xb0
	if (f_600 < f_610)
	{
		set_diffuse(OMediaFARGBColor(1.0f - f_600 / f_610, f_604, f_608, f_60c));	// +0x90.. (r, g, b, a)
		return;
	}
	HideAgain();											// element +0xd8
}

void C3DPermanentSmoke::v260(void)							// 0x45d540
{
	C3DPermanentSprite::v260();								// 0x45d7b0
	InitPhysics();											// vtable +0x2c
	v132(0);												// +0x210
	set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);	// +0xb4 = 6, +0xb8 = 7 / 1
}
