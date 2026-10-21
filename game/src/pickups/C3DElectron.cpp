/*  C3DElectron - see C3DElectron.h. Reconstructed from Neutron2.exe 0x42df20..0x42e610, the
    shared 0x43d610 / 0x43d6b0, and the free helpers 0x485800 / 0x40ac50. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DElectron.h"
#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../C3DMagicParticle.h"
#include "../CGameType.h"
#include "../CanvasWrapper.h"
#include "../InterfaceElement.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include "OMediaStreamOperators.h"
#include <string.h>
#include <strings.h>
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

// 0x404dd0: set the type of item `item` of menu page `page` (as in C3DNeutron.cpp; only the
// call with page 2, item 46 is used here).
static void SetItemState(short page, short item, short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p) p->items[item].type = state;
}


// 0x40ac50: show the electron indicator for 3 s (C2DInGameMenu::UpdateIndicator): sprite set a
// at `a`, set b at `b`, all nine opaque white again.
static void ShowIndicator(int a, int b)
{
	g_545e6c = b;
	g_545e74 = 3.0f;
	g_545e70 = a;
	OMediaCanvasElementWrapper *const all[9] = { g_ind_a0, g_ind_a1, g_ind_a2, g_ind_b0, g_ind_b1, g_ind_b2,
												 g_ind_b3, g_ind_b4, g_ind_b5 };	// (a0 is set twice)
	for (int i = 0; i < 9; i++)
		if (all[i]) all[i]->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// +0x90.. (port: checked)
}

// 0x485800: n electrons more. Every third one is a step of the indicator (g_54a454); at six
// steps an extra life, bonus points (100 / 200 / 300 by difficulty), 25 energy, the red
// neutron's announcement, menu item and sound. (Inlined score and energy code, as the original.)
static void AddElectrons(int n)
{
	g_54a450 += n;
	if (g_54a450 > 2)
	{
		if (g_54a450 > 3) g_54a450 -= 3;					// (exactly 3 stays 3 until the next one)
		else
		{
			g_54a454++;
			if (g_54a454 > 5)
			{
				g_54a454 -= 6;
				g_lives++;
				int bonus = 200;
				if (g_difficulty == 0) bonus = 100;
				else if (g_difficulty == 2) bonus = 300;
				g_score_popup += bonus;						// 0x54a45c
				g_score += bonus;							// 0x54a458
				g_score_popup_time = 1.2f;					// 0x54a460
				if (g_score < 0) g_score = 0;
				g_health += 25.0f;							// 0x4d056c
				if (g_health > 100.0f)
				{
					g_health = 100.0f;
					g_545fd4 = 0;
				}
				else if (g_health < 0) g_health = 0;
				else if (g_health < 19.0f && g_health != 0)
				{
					if (!g_545fd4)
					{
						PlayGameSound(-1, std::string("low neutron energy"), false);
						g_545fd4 = 1;
					}
				}
				else if (g_health > 20.0f) g_545fd4 = 0;
				AnnouncePickup(46, 0);						// 0x408f80
				SetItemState(2, 46, 1);						// 0x404dd0
				PlayGameSound(-1, std::string("red neutron"), false);
				return;
			}
		}
	}
	ShowIndicator(g_54a450, g_54a454);						// 0x40ac50
}

// --------------------------------------------------------------------------- construction

C3DElectron::C3DElectron()									// 0x42df20
{
	SetName((char *)"C3DELECTRON");
	Log("C3DElectron()");
	C3DSprite::InitObject();								// 0x49c950 (it has no InitObject of its own)
	SetObjectID('3ELE');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 219;
	m_sprite_size = 80;
	v42(0);
	v40(0);
	v52(1);
	v107(1);
	v46(0);
	v34(1);
	m_spin = 900.0f;
	m_collected = 0;
	C3DSprite::InitPhysics();								// 0x49ca30
	m_rate_x = (float)Random1000() * 0.001f + 0.5f;			// 0x4cb218, 0x4caa5c
	m_rate_y = (float)Random1000() * 0.001f + 0.5f;
	// (then a call to the empty 0x463890)
}

C3DElectron::~C3DElectron() {}								// 0x42e150

namespace neg {
CGameObject *New_C3DElectron(void) { return new C3DElectron; }			// '3ELE' (0x680 bytes)
}

// --------------------------------------------------------------------------- slots

void C3DElectron::AddTouching(CGameObject *other)			// 0x42e190
{
	if (m_collected) return;
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (!g_camera || other != g_camera) return;
	m_collected = 1;
	C3DJimmy *j = Jimmy();									// (g_jimmy - 0xc0)
	if (j)
	{
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
		AddEnergy(1.0f);									// 0x485730
		AddScore(2);										// 0x482b50
		AddElectrons(1);									// 0x485800
		PlayGameSound(-1, std::string("blip4"), false);
	}
	v136(0);
	HideAgain();											// element +0xd8
}

unsigned char C3DElectron::v231(CGameObject *other)			// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DElectron::Process(float dt)							// 0x42e460
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	if (m_collected) return;
	const float f = g_enemy_time_scale * m_spin * dt;		// 0x540594
	AddRotation((float)(m_rate_x * f * 0.05), (float)(m_rate_y * f * 0.15), f);	// 0x4d8b88 / 0x4d8b90 (double)
}

void C3DElectron::v260(void)								// 0x42e4e0
{
	C3DSpriteType::v260();									// 0x473650
	set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);	// +0xb4 = 6, +0xb8 = 7 / 1
	set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// +0x90..
	set_flags(omelf_SecondPass | omelf_FaceViewport);		// +0x7c = 0x208
	v67(200.0f);											// radius
}

void C3DElectron::WriteState(OMediaStreamOperators &s)		// 0x43d6b0 (shared code)
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	bool collected = m_collected != 0;
	s << collected;
}

void C3DElectron::ReadState(OMediaStreamOperators &s)		// 0x42e570 (shared code)
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
