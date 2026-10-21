/*  C3DMagicParticle - see C3DMagicParticle.h. */
#include "C3DMagicParticle.h"
#include "CGameType.h"
#include "Game.h"
#include <string.h>

using namespace neg;

C3DMagicParticle::C3DMagicParticle()						// 0x451700
{
	SetName((char *)"C3DMAGICPARTICLE");
	Log("C3DMagicParticle()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3MAG');							// 0x49b550
	strcpy(m_sprite_db, "RetainedSprites.omt");
	m_sprite_index = 14;
	m_sprite_size = 8;
	v42(0);
	v40(0);
	v52(0);
	v107(0);
	v36(0);
	v46(0);
	v232();													// 0x4adea0
	f_600 = 0.5f;
	f_604 = 0.4f;
	f_608 = 1.0f;
	f_60c = 500.0f;
	f_610 = 1.0f;
	f_614 = 1.0f;
	// (then a call to the empty 0x463890)
}

C3DMagicParticle::~C3DMagicParticle()						// 0x4518c0
{
}

void C3DMagicParticle::Burst(OMedia3DVector pos)			// 0x451900
{
	v196(pos.x, pos.y, pos.z, pos.w);						// CGameObject +0x310
	f_614 = f_610;
	ShowAgain();											// element +0xd4
}

void C3DMagicParticle::Process(float dt)					// 0x451950
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	if (!(f_614 > 0)) return;
	f_614 = f_614 - dt;
	if (f_614 <= 0)
	{
		HideAgain();										// element +0xd8
		return;
	}
	set_diffuse(OMediaFARGBColor(f_614 / f_610, f_600, f_604, f_608));	// +0x90.. (r, g, b, a)
	AddRotation(0, 0, dt * f_60c);							// +0x330
}

void C3DMagicParticle::v260(void)							// 0x451a40
{
	C3DPermanentSprite::v260();								// 0x45d7b0
	InitPhysics();											// vtable +0x2c
	set_blend(omblendfc_Src_Alpha, omblendfc_One);			// +0xb4 = 6, +0xb8 = 1
	set_diffuse(OMediaFARGBColor(0.0f, f_600, f_604, f_608));
	set_flags(omelf_SecondPass | omelf_FaceViewport);		// +0x7c = 0x208
}
