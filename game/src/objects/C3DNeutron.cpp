/*  C3DNeutron - see C3DNeutron.h. Reconstructed from Neutron2.exe 0x4597d0..0x459f60. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DNeutron.h"
#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../InterfaceElement.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaCanvas.h"
#include "OMediaStreamOperators.h"
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x485730: the neutron energy (g_health) += d, kept in 0..100; "low neutron energy" once when
// it falls below 19 (g_545fd4 remembers it until it is above 20 again). (As in C3DRocketShip.cpp.)
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

// 0x404dd0: set the type of item `item` of menu page `page` (as in CLoadLevel.cpp). The pickups
// come in pairs (10/21 and 31/38): giving one also gives (and counts) the other.
static void SetItemState(short page, short item, short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p) p->items[item].type = state;
	if (page != C2DInGameMenu::MENU_PICKUPS) return;
	C2DInGameMenu::MenuPage *pk = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
	if (item == 10)
	{
		if (!pk || !pk->items[21].type) { SetItemState(2, 21, 1); g_pickup_counts[2 * 21]++; }
	}
	else if (item == 21)
	{
		if (!pk || !pk->items[10].type) { SetItemState(2, 10, 1); g_pickup_counts[2 * 10]++; }
	}
	else if (item == 38)
	{
		if (!pk || !pk->items[31].type) { SetItemState(2, 31, 1); g_pickup_counts[2 * 31]++; }
	}
	else if (item == 31)
	{
		if (!pk || !pk->items[38].type) { SetItemState(2, 38, 1); g_pickup_counts[2 * 38]++; }
	}
}

// 0x490890 + element: the element behind a game object (the original looks it up by 4cc).
static OMediaElement *ElementOf(CGameObject *o)	{ return o ? dynamic_cast<OMediaElement *>(o) : NULL; }

// --------------------------------------------------------------------------- construction

C3DNeutron::C3DNeutron()									// 0x4597d0
{
	SetName((char *)"C3DNEUTRON");
	Log("C3DSprite()");										// (sic)
	C3DSprite::InitObject();								// 0x49c950 (it has no InitObject of its own)
	SetObjectID('3NEU');									// element slot 48
	OMediaDataBase *db = GetDatabase("sprites.omt");		// 0x4a4180
	for (int i = 0; i < 13; i++)
	{
		m_frames[i] = LoadCanvasID(db, i);					// 0x4b2860
		if (m_frames[i]) m_frames[i]->set_filtering(omtfc_Linear, omtfc_Linear);	// +0x60, +0x64 = 1, +0x24 vtable +0x10
	}
	set_diffuse(OMediaFARGBColor(0.5f, 1.0f, 1.0f, 1.0f));	// +0x90.. (r, g, b, a = 1, 1, 1, 0.5)
	m_frame = 0;
	m_collected = 0;
	m_time = 0;
	InitSpriteProperties(200.0f);							// element slot 52
	v67(250.0f);
	m_gone = 0;
	v42(0);
	v40(1);
	v46(0);
	v52(1);
	v34(1);
	C3DSprite::InitPhysics();								// 0x49ca30
	// (an empty debug log)
}

C3DNeutron::~C3DNeutron() {}								// 0x459b20

namespace neg {
CGameObject *New_C3DNeutron(void) { return new C3DNeutron; }	// '3NEU' (0x6c0 bytes)
}

// --------------------------------------------------------------------------- slots

void C3DNeutron::AddTouching(CGameObject *other)			// 0x459d40
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (m_collected || g_camera != other) return;
	AddEnergy(5.0f);
	AddScore(10);											// 0x482b50
	CGameObject *jim = FindObjectByTag("JIM1");
	C3DJimmy *j = jim ? dynamic_cast<C3DJimmy *>(jim) : NULL;	// (JIM1 - 0xc0)
	if (j) j->ev156(1.0f, 0.3f, 0.3f, 1.0f);				// element +0x270: a red flash
	if (g_health < 100.0f) SetItemState(2, 45, 1);			// 0x4d2cb0
	m_channel = PlayGameSound(-1, std::string("neutron"), false);	// 0x48f420
	m_collected = 1;
}

unsigned char C3DNeutron::v231(CGameObject *other)			// 0x43d610
{
	return g_camera == other;
}

// 0x459b60: every 0.05 s (scaled) the next frame: spinning 0..7; collected, 8..12, then at 13
// hidden, no collisions, gone. Gone, it comes back after 240 s.
void C3DNeutron::Process(float dt)
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	m_time = dt + m_time;
	if (m_gone)
	{
		if (!(m_time > 240.0f)) return;						// 0x4caa80
		OMediaElement *e = ElementOf(this);					// 0x490890
		if (e) e->force_show();								// (2002 +0x70 hide count: 1, then hide(false))
		m_collected = 0;
		m_time = 0;
		m_gone = 0;
		v136(1);											// +0x220
		return;
	}
	if (!(g_enemy_time_scale * m_time > 0.05f)) return;		// 0x540594, 0x4d2cbc
	m_time = 0;
	if (m_collected)
	{
		m_frame++;
		if (m_frame > 12)
		{
			if (m_frame >= 14) return;
			OMediaElement *e = ElementOf(this);
			if (e) e->hide(true);							// +0x58
			m_gone = 1;
			v136(0);
			return;
		}
	}
	else
	{
		m_frame++;
		if (m_frame > 7) m_frame = 0;
	}
	set_canvas(m_frames[m_frame]);							// element +0xac
}

void C3DNeutron::v258(void)									// 0x459d10
{
	C3DSprite::v258();										// 0x4735a0
	set_canvas(m_frames[0]);
}

void C3DNeutron::v260(void)									// 0x44f430
{
	set_canvas(m_frames[0]);
}

void C3DNeutron::WriteState(OMediaStreamOperators &s)		// 0x459ec0
{
	CLocalGameObject::WriteState(s);
	bool collected = m_collected != 0, gone = m_gone != 0;	// (bool operators, one byte each)
	s << collected;
	s << gone;
}

void C3DNeutron::ReadState(OMediaStreamOperators &s)		// 0x459e10
{
	CLocalGameObject::ReadState(s);
	bool collected, gone;
	s >> collected;
	s >> gone;
	m_collected = collected;
	m_gone = gone;
	if (!m_collected)
	{
		if (m_gone) ErrorLog("Error: INVALID NEUTRON DETECTED");
		m_gone = 0;
		set_canvas(m_frames[0]);
		ShowAgain();										// element +0xd4
		v136(1);
		return;
	}
	HideAgain();											// element +0xd8
	m_collected = 1;
	m_gone = 1;
	v136(0);
}
