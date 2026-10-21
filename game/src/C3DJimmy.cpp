/*  C3DJimmy - see C3DJimmy.h. This file: construction, destruction, the factory entry and the
    hooks C3DPlayer.h declares weak. The slots are in jimmy/C3DJimmy_a.cpp and _b.cpp.       */
#include "C3DJimmy.h"
#include "C3DPermanentSprite.h"
#include "C3DRadar.h"
#include "C3DShadow.h"
#include "C3DJetpackFire.h"
#include "C3DTargetCursor.h"
#include "C2DInGameMenu.h"
#include "CTaskList.h"
#include "C3DFish.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMedia3DShape.h"
#include <string.h>

using namespace neg;

// Factory entries of the helper classes (gen/factory.cpp: weak TODO stubs until they exist).
namespace neg {
CGameObject *New_C3DRocketShip(void);		// 0x46a290
CGameObject *New_C3DWaterBalloon(void);		// 0x47c780
CGameObject *New_C3DGoddard(void);			// 0x4383d0
CGameObject *New_C3DASEObj(void);			// 0x4207e0
}

// Helper classes without a factory ID: not reconstructed yet.
static CGameObject *NewPointCursor(void)	{ NEG_TODO(0x466a70, "new C3DPointCursor (0x92c bytes)"); return NULL; }
static CGameObject *NewGraplingHook(void)	{ NEG_TODO(0x43cdb0, "new C3DGraplingHook (0x92c bytes)"); return NULL; }
static CGameObject *NewMagicParticle(void)	{ NEG_TODO(0x451700, "new C3DMagicParticle (0x620 bytes)"); return NULL; }

template <class T> static T *As(CGameObject *o) { return o ? dynamic_cast<T *>(o) : NULL; }

// Menu items (menu helpers, copied as in SaveGame.cpp): 0x404dd0 set, 0x404ef0 clear the type
// of item `item` of menu page `page`.
static void SetItemState(short page, short item, short state)	// 0x404dd0
{
	// The pickups come in pairs (10/21 and 31/38): giving one also gives (and counts) the other.
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

static void ClearItemState(short page, short item)				// 0x404ef0
{
	C2DInGameMenu::MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::Page(page)) != NULL)
		p->items[item].type = 0;
}

// The game's random number generator (as C3DPlayer's): 0..1000.
static int Random1001(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return ((int)((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// A sparkle colour set for a new C3DMagicParticle: (r * 0.001) < 0.33 / < 0.66 / else.
static int ParticleKind(int a, int b, int c)
{
	const double f = (double)Random1001() * (double)0.001f;	// 0x4cb218, compared as doubles 0x4e4e68 / 0x4e4e60
	if (f < 0.33) return a;
	if (f < 0.66) return b;
	return c;
}

static void SetParticleKind(C3DPermanentSprite *p, int kind)	// C3DMagicParticle +0x594
{
	(void)kind;
	if (p) NEG_TODO(0x4440ac, "C3DMagicParticle +0x594 (sparkle kind)");
}

// The setup the constructor gives the rocket ship and the jet pack: hidden, and (CGameObject
// slots 52, 132, 120, 107, 36) v52(0), v132(0), v120(1), v107(0), v36(0).
static void ParkVehicle(C3DPlayerObject *v)
{
	if (!v) return;
	v->HideAgain();											// element +0xdc
	v->v52(0);												// +0xd0
	v->v132(0);												// +0x210
	v->v120(1);												// +0x1e0
	v->v107(0);												// +0x1ac
	v->v36(0);												// +0x90
}

// --------------------------------------------------------------------------- construction

C3DJimmy::C3DJimmy()										// 0x443830
{
	// (f_d70[], f_2cb0[], f_4bf0, f_4cbc, f_4d80, f_4dbc[] are OMedia3DPoints: w = 1;
	// f_4ca8, f_4d6c OMedia3DVectors: w = 0.)
	SetName((char *)"C3DJIMMY");
	Log("C3DJimmy()");
	strcpy(m_tag, "JIM1");
	f_6d0 = 1;												// Granny animation
	C3DJimmy::InitObject();									// 0x444780, called directly
	SetObjectID('3JIM');									// element slot 49
	f_cac = 0;
	f_4c10 = -1;

	f_4c90 = new CTaskList;									// 0x4c8 bytes
	f_cbc = NULL;
	f_cc0 = NULL;
	g_menu = new C2DInGameMenu;								// 0x5b4 bytes
	if (g_menu)
	{
		SetItemState(0, 9, 1);								// 0x404dd0
		SetItemState(0, 10, 1);
		SetItemState(1, 0, 1);
		ClearItemState(1, 2);								// 0x404ef0
		ClearItemState(1, 3);
		ClearItemState(1, 4);
		ClearItemState(1, 5);
		ClearItemState(1, 6);
		ClearItemState(1, 7);
		ClearItemState(1, 8);
	}

	f_cc8 = As<C3DPlayerObject>(New_C3DRocketShip());		// 0x1284 bytes
	ParkVehicle(f_cc8);
	f_ccc = new C3DJetPack;									// 0xa04 bytes
	ParkVehicle(f_ccc);

	f_4cfe = 0;
	f_cc4 = new C3DShadow;									// 0x60c bytes
	f_4c8c = new C3DTargetCursor;							// 0x60c bytes
	if (f_4c8c) f_4c8c->HideAgain();						// element +0xd8
	f_d4c = As<C3DAnimated>(NewPointCursor());				// 0x92c bytes
	if (f_d4c) f_d4c->HideAgain();							// element +0xdc
	for (int i = 0; i < 20; i++)
	{
		f_cd0[i] = As<C3DPermanentSprite>(New_C3DWaterBalloon());	// 0x678 bytes
		if (f_cd0[i])
		{
			f_cd0[i]->HideAgain();							// element +0xd8
			f_cd0[i]->v52(0);								// +0xd0
			f_cd0[i]->v132(0);								// +0x210
		}
	}
	f_d2c = NULL;
	f_d28 = NULL;
	f_d30 = NULL;
	f_d38 = NULL;
	f_d34 = NULL;
	f_d3c = 0;
	f_4d9c = 5000.0f;
	f_d50 = 0;
	f_d5c = NULL;
	f_4ee8 = 0;
	f_d58 = 0;
	v36(0);
	v107(0);
	f_d64 = 0;
	f_d68 = -1;
	f_d6a = 4;
	f_d60 = 50.0f;
	g_sfx_db_loaded = false;
	f_4c14 = -1;
	f_4c18 = 0;
	f_4c1c = -1;
	f_4c10 = -1;
	f_4c20 = -1;
	f_4c24 = -1;
	strcpy(f_4c28, "none");
	f_d54 = 0;
	f_009 = 1;
	f_4db8 = 0;
	f_4e78 = 0;
	f_4dbc[0].x = 0;
	f_4dbc[f_4e78].y = 0;
	f_4dbc[f_4e78].z = 0;
	f_4e60 = 0;
	f_4e5c = 0;
	f_4e64 = 0;
	f_4e68 = 0;
	f_4e6c = 0;
	f_4ccc = 0;
	f_4cd0 = 0;
	f_4c08 = -1;
	f_4c0c = -1;
	f_4d98 = -1;
	f_4cfd = 0;
	f_4d94 = 0;
	f_4ca0 = 0;
	f_4dac = 0;
	f_4db0 = -900.0f;
	f_4db4 = 0;
	f_4ee4 = 0;
	f_4cfc = 0;
	f_c9c = 0;
	f_4ef0 = NULL;

	f_d20 = As<C3DAnimated>(NewGraplingHook());				// 0x92c bytes
	if (f_d20) f_d20->f_590->scale(1.0f, 1.0f, 7.0f);		// its base shape, vtable +0x34

	for (int i = 0; i < 20; i++)
	{
		f_4d00[i] = As<C3DPermanentSprite>(NewMagicParticle());	// 0x620 bytes
		SetParticleKind(f_4d00[i], ParticleKind(0xe, 0xc, 0x11));	// port: the roll is made even without the particle
	}
	f_4d50[0] = As<C3DPermanentSprite>(NewMagicParticle());
	SetParticleKind(f_4d50[0], ParticleKind(0x1b, 0x1c, 0x1d));

	f_4d7c = 0;
	f_4d80.x = 0;
	f_4d80.y = 0;
	f_4d80.z = 0;
	f_4c94 = 0;
	f_4c98 = 0;
	f_4c9c = 0;
	f_4ca4 = 0;
	f_4ca5 = 0;
	f_4ca6 = 0;
	f_4ca7 = 0;
	f_4cb8 = 0;
	f_4cd4 = 0;
	f_4cd8 = 0;
	f_ae8 = 10.0f;
	g_537638 = 0xbf800000;									// -1.0f
	g_537634 = -1.0f;
	g_53763c = -1.0f;
	f_4cdc = 5.0f;
	f_4ce0 = 0;
	f_4cfe = 1;
	f_4ce8 = 0;
	f_4d90 = 0;
	f_4d54 = 0;
	f_4d60 = 0;
	strcpy(f_4e80, g_empty_string);
	g_save_slot = 6;
	f_d6c = 0;
	f_d6d = 0;
	f_42c = 1;
	f_4ce4 = 0;
	f_4cec = 1;
	f_4cf0 = 0;
	f_4cf4 = 0;
	g_545fd4 = 0;
	f_4cf8 = 1;
	f_4c04 = 0;
	f_4d58 = 0;
	f_4d5c = 0;
	f_4d68 = 0;
	f_4d64 = 0;

	f_d5c = As<C3DPlayerObject>(New_C3DGoddard());			// 0x1bcc bytes
	f_d40 = new C3DRadar;									// 0x640 bytes
	f_d48 = 0;
	f_d44 = As<C3DAnimated>(New_C3DASEObj());				// 0xbb0 bytes
	if (f_d44)
	{
		// C3DASEObj +0x988 / +0x924 "energybubble.ase", +0x9ec "vrtankbubble.ase",
		// +0xa50 "stratobubble.ase", +0xab4 "bubble.png", +0xb80 0.5, +0xb84 0.1
		NEG_TODO(0x4443ee, "C3DASEObj bubble files (+0x924..+0xab4) and +0xb80 / +0xb84");
		f_d44->f_89c = 0.2f;								// "Translucent"
		f_d44->HideAgain();									// element +0xdc
		f_d44->v52(0);										// +0xd0
		f_d44->v46(0);										// +0xb8
		f_d44->v107(0);										// +0x1ac
		f_d44->v36(0);										// +0x90
	}
	f_4da4 = 0;
	f_4da0 = 0;
	f_4da8 = 0.35f;
	g_jimmy = static_cast<CGameObject *>(this);
	f_8a4 = 1;												// never culled
	f_4e7c = 0;
	if (g_menu) g_menu->LoadOptions("dat\\options.opt");	// +0x514
}

C3DJimmy::~C3DJimmy()										// 0x4446a0
{
	Log("~C3DJimmy()");
	// (debug log "MEBTEMP deleting jimmy!!!")
	UnInitObject();											// vtable +0x20
	// The helpers (menu, task list, vehicles ...) are not deleted: they are game objects of
	// their own (DeleteAllObjects).
}

namespace neg {
CGameObject *New_C3DJimmy(void)	{ return new C3DJimmy; }				// factory '3JIM' (0x4efc bytes)
}

C3DJimmy *neg::Jimmy(void)
{
	return g_jimmy ? dynamic_cast<C3DJimmy *>(g_jimmy) : NULL;
}

// --------------------------------------------------------------------------- hooks (C3DPlayer.h)

// 0x443370 (a C3DJetPack method, `this` = the jet pack): the flame of g_jimmy's jet pack and
// its own (+0x9f0 -> +0x928) to 0, stop its sound (+0x9f4).
void neg::JimmyJetpackOff(CGameObject *jimmy)
{
	dynamic_cast<C3DJimmy *>(jimmy)->f_ccc->Off();
}

// 0x4433f0 (C3DJetPack, `a` unused): loop "jetpack" on its channel +0x9f4 unless it plays.
void neg::JimmyJetpackSound(CGameObject *jimmy, int a)
{
	(void)a;
	dynamic_cast<C3DJimmy *>(jimmy)->f_ccc->Sound();
}

// C3DPlayer 0x4613a0 inline: jet pack (+0xccc) -> fire (+0x9f0, if any) -> size (+0x928).
void neg::JimmyJetpackFlame(CGameObject *jimmy, float size)
{
	C3DJetPack *p = dynamic_cast<C3DJimmy *>(jimmy)->f_ccc;
	if (p->f_9f0) p->f_9f0->f_928 = size;
}

// Element slot 138 (+0x228) of g_jimmy.
void neg::JimmyActivateMenuItem(CGameObject *jimmy, CGameObject *o, CGameObject *who, int item, float value)
{
	C3DJimmy *j = jimmy ? dynamic_cast<C3DJimmy *>(jimmy) : NULL;
	if (j) j->ev138(o, who, item, value);
}

// C3DFish.h: Jimmy's element slot 134 (+0x218), hurt.
void neg::JimmyHurt(CGameObject *jimmy, float damage, int points, float time, unsigned char a)
{
	C3DJimmy *j = jimmy ? dynamic_cast<C3DJimmy *>(jimmy) : NULL;
	if (j) j->ev134(damage, points, time, a);
}
