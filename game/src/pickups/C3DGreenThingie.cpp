/*  C3DGreenThingie - see C3DGreenThingie.h. Reconstructed from Neutron2.exe 0x43d0f0..0x43d720
    and the shared 0x42e570. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DGreenThingie.h"
#include "../C3DJimmy.h"
#include "../C3DMagicParticle.h"
#include "../CGameType.h"
#include "../InterfaceElement.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include "OMediaStreamOperators.h"
#include <string.h>
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// The game's random number: 0..1000 (the LCG at g_random_seed).
static int Random1000(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return (int)(((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// 0x485730: the neutron energy (g_health) += d, kept in 0..100; "low neutron energy" once when
// it falls below 19 (g_545fd4 remembers it until it is above 20 again). (As in C3DNeutron.cpp.)
static void AddEnergy(float d)
{
	g_health += d;
	if (g_health > 100.0f)
	{
		g_health = 100.0f;
		g_545fd4 = 0;
		return;
	}
	if (g_health < 0)
	{
		g_health = 0;
		return;
	}
	if (g_health < 19.0f && g_health != 0)					// 0x506f68
	{
		if (!g_545fd4)
		{
			PlayGameSound(-1, std::string("low neutron energy"), false);
			g_545fd4 = 1;
		}
		return;
	}
	if (g_health > 20.0f) g_545fd4 = 0;
}

// --------------------------------------------------------------------------- construction

C3DGreenThingie::C3DGreenThingie()							// 0x43d0f0
{
	SetName((char *)"C3DGREENTHINGIE");
	Log("C3DGreenThingie()");
	C3DSprite::InitObject();								// 0x49c950 (it has no InitObject of its own)
	SetObjectID('3GRT');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 295;
	m_sprite_size = 30;
	v42(0);
	v40(0);
	v52(1);
	v107(1);
	v46(0);
	v34(1);
	m_spin = 200.0f;
	m_collected = 0;
	C3DSprite::InitPhysics();								// 0x49ca30
	// (then a call to the empty 0x463890)
}

C3DGreenThingie::~C3DGreenThingie() {}

namespace neg {
CGameObject *New_C3DGreenThingie(void) { return new C3DGreenThingie; }	// '3GRT' (0x680 bytes)
}

// --------------------------------------------------------------------------- slots

void C3DGreenThingie::AddTouching(CGameObject *other)		// 0x43d2d0
{
	if (m_collected) return;
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (!g_camera || other != g_camera) return;
	m_collected = 1;
	C3DJimmy *j = Jimmy();									// (g_jimmy - 0xc0)
	if (j)
	{
		j->ev156(1.0f, 0.2f, 1.0f, 0.2f);					// element +0x270: a green flash
		for (int i = 0; i < 13; i++)						// a burst of Jimmy's sparkles
		{
			j->ev154();										// element +0x268: next sparkle
			const float rx = (float)Random1000() * 0.001f - 0.5f;
			const float ry = (float)Random1000() * 0.001f - 0.5f;
			const float rz = (float)Random1000() * 0.001f - 0.5f;
			float p[4];
			v195(p);
			OMedia3DVector at(rx * 60.0f + p[0], ry * 60.0f + p[1], rz * 60.0f + p[2], 1.0f);	// 0x4d8b84
			C3DMagicParticle *s = j->f_4d00[j->f_4d54];
			s->Burst(at);									// element +0xdc
			s->f_610 = 2.0f;
			s->f_614 = 2.0f;
			s->f_600 = rx + 0.5f;							// a random colour
			s->f_604 = ry + 0.5f;
			s->f_608 = rz + 0.5f;
		}
		AddEnergy(0.5f);									// 0x485730
		AddScore(10);										// 0x482b50
		PlayGameSound(-1, std::string("greenthingy"), false);
	}
	v136(0);
	HideAgain();											// element +0xd8
}

unsigned char C3DGreenThingie::v231(CGameObject *other)		// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DGreenThingie::Process(float dt)						// 0x43d5b0
{
	if (m_collected) return;
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	AddRotation(0, 0, g_enemy_time_scale * m_spin * dt);	// 0x540594
}

void C3DGreenThingie::v260(void)							// 0x43d630
{
	C3DSpriteType::v260();									// 0x473650
	if (!g_windowed) set_blend(omblendfc_Src_Alpha, omblendfc_One);	// +0xb4 = 6, +0xb8 = 1
	set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// +0x90..
	set_flags(omelf_SecondPass | omelf_FaceViewport);		// +0x7c = 0x208
	v67(200.0f);											// radius
}

void C3DGreenThingie::WriteState(OMediaStreamOperators &s)	// 0x43d6b0
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	bool collected = m_collected != 0;
	s << collected;
}

void C3DGreenThingie::ReadState(OMediaStreamOperators &s)	// 0x42e570 (shared code)
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	bool collected;
	s >> collected;
	m_collected = collected;
	if (!m_collected)
	{
		ShowAgain();										// element +0xd4
		v136(1);
		return;
	}
	HideAgain();											// element +0xd8
	v136(0);
}
