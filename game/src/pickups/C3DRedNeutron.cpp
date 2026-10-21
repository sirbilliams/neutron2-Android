/*  C3DRedNeutron - see C3DRedNeutron.h. Reconstructed from Neutron2.exe 0x469040..0x469990. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DRedNeutron.h"
#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../InterfaceElement.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaCanvas.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

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

// 0x404dd0: set the type of item `item` of menu page `page` (as in C3DNeutron.cpp). The pickups
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

C3DRedNeutron::C3DRedNeutron()								// 0x469040
{
	SetName((char *)"C3DREDNEUTRON");
	Log("C3DREDNEUTRON()");
	C3DRedNeutron::InitObject();							// 0x469410 (inlined)
	SetObjectID('3RED');									// 0x49b550
	OMediaDataBase *db = GetDatabase("sprites.omt");		// 0x4a4180
	for (int i = 0; i < 13; i++)
	{
		m_frames[i] = LoadCanvasID(db, i);					// 0x4b2860
		if (m_frames[i]) m_frames[i]->set_filtering(omtfc_Linear, omtfc_Linear);	// +0x60, +0x64 = 1, +0x24 vtable +0x10 (port: checked)
	}
	set_diffuse(OMediaFARGBColor(0.5f, 1.0f, 0.2f, 0.2f));	// +0x90.. (r, g, b, a = 1, 0.2, 0.2, 0.5)
	set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);	// +0xb4 = 6, +0xb8 = 7 / 1
	m_frame = 0;
	m_collected = 0;
	m_time = 0;
	InitSpriteProperties(300.0f);							// element slot 52
	m_pulse = 0;
	m_gone = 0;
	v67(250.0f);
	v42(0);
	v40(1);
	v46(0);
	v52(1);
	v34(1);
	C3DSprite::InitPhysics();								// 0x49ca30
	// (an empty debug log)
}

C3DRedNeutron::~C3DRedNeutron() {}

namespace neg {
CGameObject *New_C3DRedNeutron(void) { return new C3DRedNeutron; }		// '3RED' (0x80c bytes)
}

// --------------------------------------------------------------------------- slots

void C3DRedNeutron::InitObject(void)						// 0x469410
{
	CPickupType::InitObject();								// 0x496d60
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
}

void C3DRedNeutron::AddTouching(CGameObject *other)			// 0x4696b0
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (m_collected || other != g_camera) return;
	Collect();												// element +0xe0
	if (strcasecmp(m_next_trigger, "none") != 0)
		CallNextTrigger(m_next_trigger, 0);					// element +0xdc
}

// 0x469440: the pulse (size and blue channel), then every 0.05 s (scaled) the next frame:
// spinning 0..7; collected, 8..12, then at 13 hidden for good.
void C3DRedNeutron::Process(float dt)
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	m_time = dt + m_time;
	const float t = dt + m_pulse;
	m_pulse = t;
	const float k = (float)(m_sprite_size / 100);			// (integer division)
	const float h = (float)((sin(t * 8.0f) * 150.0 + 330.0) * k);	// 0x4f12e4, 0x4f4490, 0x4f4488
	const float w = (float)((sin((t + 0.3) * 10.0) * 70.0 + 330.0) * k);	// 0x4dfdc8, 0x4f4480, 0x4f4478
	set_size(w, h);											// +0xac, +0xb0
	double b = sin(m_pulse * 3.0f) * (double)0.7f;			// 0x4cb1e8, 0x4ce640
	if (b > 0.7f) b = 0.8f;									// 0x4cceec, 0x4caa58 (sic)
	else if (b < 0) b = 0;
	set_diffuse(OMediaFARGBColor(0.9f, 1.0f, 0.2f, (float)b));	// +0x90.. (r, g, b, a = 1, 0.2, b, 0.9)
	set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);	// +0xb4 = 6, +0xb8 = 7 / 1
	if (!(g_enemy_time_scale * m_time > 0.05f)) return;		// 0x540594, 0x4d2cbc
	m_time = 0;
	if (m_collected)
	{
		m_frame++;
		if (m_frame > 12)
		{
			if (m_frame >= 14) return;
			OMediaElement *e = ElementOf(this);				// 0x490890
			if (e) e->hide(true);							// +0x58
			m_gone = 1;
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

void C3DRedNeutron::v258(void)								// 0x469680
{
	C3DSprite::v258();										// 0x4735a0
	set_canvas(m_frames[0]);
}

void C3DRedNeutron::v260(void)								// 0x4697f0
{
	// ("PostLoad 1 %d" to the empty 0x463890)
	CPickupType::v260();									// 0x496de0
	// ("PostLoad 2 %d  %d")
	set_canvas(m_frames[0]);								// element +0xac
	if (g_object_states[m_pickup_index] > 0)				// taken already
	{
		// ("PostLoad 3 %d  %d")
		OMediaElement *e = ElementOf(this);					// 0x490890
		if (e) e->hide(true);								// +0x58
		m_gone = 1;
	}
}

void C3DRedNeutron::WriteState(OMediaStreamOperators &s)	// 0x4698f0
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	bool collected = m_collected != 0, gone = m_gone != 0;	// (bool operators, one byte each)
	s << collected;
	s << gone;
}

void C3DRedNeutron::ReadState(OMediaStreamOperators &s)		// 0x4698b0
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	bool collected, gone;
	s >> collected;
	s >> gone;
	m_collected = collected;
	m_gone = gone;
}

// --------------------------------------------------------------------------- element-family slots

void C3DRedNeutron::Collect(void)							// 0x469710
{
	g_lives++;												// 0x54a44c
	AnnouncePickup(46, 0);									// 0x408f80
	AddScore(500);											// 0x482b50
	AddEnergy(25.0f);										// 0x485730
	C3DJimmy *j = Jimmy();									// (g_jimmy - 0xc0)
	if (j) j->ev156(12.0f, 1.0f, 0.4f, 0.4f);				// element +0x270: a red flash
	SetItemState(2, 46, 1);									// 0x404dd0
	v124(0);												// +0x1f0
	m_channel = PlayGameSound(-1, std::string("red neutron"), false);	// 0x48f420
	g_object_states[m_pickup_index]++;						// 0x54a4d8
	m_collected = 1;
}
