/*  C3DGrannyObj - see C3DGrannyObj.h. */
#include "C3DGrannyObj.h"
#include "C3DAI.h"
#include "C3DPlayer.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "CGameType.h"
#include "C2DInGameMenu.h"
#include "C3DSprite.h"
#include "InterfaceElement.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
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

// 0x4af000: the object whose save index (f_438) is <index>.
static CGameObject *FindObjectByIndex(int index)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && (int)(*i)->f_438 == index) return *i;
	return NULL;
}

// Menu items (menu helpers, copied as in SaveGame.cpp): 0x404dd0 set, 0x404ef0 clear the type
// of item `item` of menu page `page` (page 2 = pickups).
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


// --------------------------------------------------------------------------- construction

C3DGrannyObj::C3DGrannyObj()								// 0x43b5f0
{
	SetName((char *)"C3DGRANNYOBJ");
	Log("C3DGrannyObj()");
	f_6d0 = 1;												// Granny
	strcpy(f_924, "newgunbase.grn");
	strcpy(f_988, "none");
	strcpy(f_9ec, "none");
	strcpy(f_a50, "none");
	strcpy(f_ab4, "none");
	strcpy(f_b18, "none");
	strcpy(f_b7c, "none");
	strcpy(f_be0, "GET");
	strcpy(f_c44, "JIM1");
	strcpy(f_d0c, "none");
	strcpy(f_ca8, "Bip01 R Hand");
	v67(60.0f);												// radius
	f_649 = 0;												// ev69 loads the animations later
	C3DGrannyObj::InitObject();								// called directly (as shipped: the
															// loader's InitObject runs it again)
	C3DObject::SetObjectID('3GRO');
	if (!f_4aa) C3DObject::InitPhysics();
	f_d70 = NULL;
	v50(0);
	v40(1);
	v42(1);
	f_d74 = 0;
	f_d78 = -1;												// PIC_NUMBER
	f_d80 = 0.1f;											// DelayAttach
	f_d7c = -1.0f;
	f_d88 = 0;
	f_d94 = 1;
	f_d98 = 1;												// GlowAndScale
	f_da0 = 0;												// SlideSpeedX
	v36(1);
	v107(1);
	f_d8c = g_gametype->m_object_id;						// +0x490
	f_660 = 0;												// CanMove
	f_d90 = NULL;
	f_d9c = 50;												// Points
	f_da4 = 2.0f;											// DefaultScale
	f_da8 = -1.0f;											// DetachTime
	f_dac = -1.0f;
	f_db0 = 1;												// AnimateWhenTouched
	f_8a5 = 1;
	f_db4 = 0;												// ShowArrow
	f_d84 = 0;												// port: ResetToStop is not set by the original
	v34(1);
}

C3DGrannyObj::~C3DGrannyObj()								// 0x43b9d0
{
}

namespace neg {
CGameObject *New_C3DGrannyObj(void) { return new C3DGrannyObj; }		// '3GRO' (0xdc0 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DGrannyObj::InitObject(void)							// 0x43ba70
{
	Log("InitObject()");
	C3DAnimated::InitObject();
	RegisterProperty("BASEAnimation", f_924, PROP_ALPHA, 0);
	RegisterProperty("STOPAnimation", f_988, PROP_ALPHA, 0);
	RegisterProperty("ANIM1Animation", f_9ec, PROP_ALPHA, 0);
	RegisterProperty("ANIM2Animation", f_a50, PROP_ALPHA, 0);
	RegisterProperty("ANIM3Animation", f_ab4, PROP_ALPHA, 0);
	RegisterProperty("ANIM4Animation", f_b18, PROP_ALPHA, 0);
	RegisterProperty("ANIMMyTouch", f_b7c, PROP_ALPHA, 0);
	RegisterProperty("ANIMYouTouch", f_be0, PROP_ALPHA, 0);
	RegisterProperty("AttachTo", f_c44, PROP_ALPHA, 0);
	RegisterProperty("GrannyYouBone", f_ca8, PROP_ALPHA, 0);
	RegisterProperty("DelayAttach", &f_d80, PROP_FLOAT, 0);
	RegisterProperty("PIC_NUMBER", &f_d78, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("ResetToStop", &f_d84, PROP_FLOAT, 0);
	RegisterProperty("GlowAndScale", &f_d98, PROP_LONG, 0);
	RegisterProperty("Points", &f_d9c, PROP_LONG, 0);
	RegisterProperty("SlideSpeedX", &f_da0, PROP_FLOAT, 0);
	RegisterProperty("DefaultScale", &f_da4, PROP_FLOAT, 0);
	RegisterProperty("DetachTime", &f_da8, PROP_FLOAT, 0);
	RegisterProperty("AnimateWhenTouched", &f_db0, PROP_LONG, 0);
	RegisterProperty("SoundName", f_d0c, PROP_ALPHA, 0);
	RegisterProperty("ShowArrow", &f_db4, PROP_LONG, 0);
}

void C3DGrannyObj::InitPhysics(void)						// 0x43ba20
{
	if (!f_4aa) C3DObject::InitPhysics();
}

void C3DGrannyObj::AddTouching(CGameObject *other)			// 0x43c430
{
	CGameObject::AddTouching(other);
	if (f_db0 == 0)											// a pickup: taken
	{
		v136(0);
		v132(0);
		HideAgain();
		AddScore(f_d9c);									// 0x482b50
		f_d94 = 0;
		if (strcasecmp(f_d0c, "none") != 0 && !IsChannelPlaying(g_voice_channel))
			g_voice_channel = PlayGameSound(-1, std::string(f_d0c), false);
		if (f_d78 != -1)
		{
			SetItemState(2, (short)f_d78, 1);
			AnnouncePickup(f_d78, f_d74);			// 0x408f80
		}
		return;
	}
	if (f_d70 != NULL || other != FindObjectByTag(f_c44)) return;
	AddScore(f_d9c);
	f_d94 = 0;
	float p[4], q[4], r[4];
	v195(p);
	other->v195(q);
	OMedia3DVector d(p[0] - q[0], p[1] - q[1], p[2] - q[2]);
	d.w = 0;
	omt_Angle ax, ay;
	d.angles(ax, ay);										// it turns towards us
	v201(r);
	const float rz = r[2];
	v201(r);
	other->v203(r[0], (float)ay * (360.0f / 16384.0f), rz);	// 0x4cb1f8
	ev96(other);
}

unsigned char C3DGrannyObj::v231(CGameObject *other)		// 0x43d610 (shared with C3DCheckPoint)
{
	return g_camera == other;
}

void C3DGrannyObj::Process(float dt)						// 0x43c040
{
	if (g_gametype->IsPaused()) return;
	C3DAnimated::Process(dt);
	if (f_da0 > 0.0f && shape)								// texture scroll
		shape->offset_textures(dt * -0.4f, 0, false);		// shape vtable +0xa4 (0x4dfdd0)
	if (f_d84 > 0.0f && !IsAnim((char *)"STOP"))
	{
		f_d84 -= dt;
		if (f_d84 <= 0.0f)
		{
			f_d84 = 0;
			ev58((char *)"STOP", 0);
		}
	}
	if (f_d70 == NULL)
	{
		if (f_d7c > 0.0f)
		{
			f_d7c -= dt;
			if (f_d7c > 0.0f) return;
			f_d7c = -1.0f;
			CGameObject *o = FindObjectByTag(f_c44);
			if (o) ev97(o);
			else ErrorLog("ERROR: Granny Object %s has attach tag of of %s", m_tag, f_c44);
			return;
		}
		if (f_d98 != 1) return;								// GlowAndScale
		double s = ((double)g_545ddc + 1.0f) * 0.5f;		// 0x40b270: a pulse in -1..1
		f_6f0 = (float)(f_da4 + s * 0.3);					// scale: DefaultScale .. +0.3
		if (s < 0.5f) s = 0.5f;
		OMedia3DMaterial *m = (*shape->get_polygons())[0].get_material();	// first polygon only
		if (m) m->set_diffuse(OMediaFARGBColor(1.0f, (float)s, (float)s, (float)s));
		return;
	}
	if (!f_d88) return;
	if (f_dac > 0.0f)
	{
		f_dac -= dt;
		if (f_dac <= 0.0f) ev98(f_d70, 0);
	}
	if (f_d70)
	{
		float v[4];
		f_d70->v195(v);	v196(v[0], v[1], v[2], v[3]);
		f_d70->v201(v);	v202(v[0], v[1], v[2], v[3]);
	}
}

void C3DGrannyObj::v241(unsigned int a)						// 0x43bda0
{
	(void)a;
	if (!f_d70 || !f_d88 || !f_d70->NameContains((char *)"C3DANIMATED")) return;
	C3DAnimated *o = dynamic_cast<C3DAnimated *>(f_d70);
	if (o->f_589) ShowAgain();
	else HideAgain();
	float v[4];
	f_d70->v195(v);	v196(v[0], v[1], v[2], v[3]);
	f_d70->v201(v);	v202(v[0], v[1], v[2], v[3]);
	ev80();
}

void C3DGrannyObj::v260(void)								// 0x43bcd0
{
	C3DAnimated::v260();
	InitPhysics();
	if (f_d70 == NULL)
	{
		InitAnim();											// element +0x110 (logs an error for granny objects)
		ev69();
		f_6f0 = f_da4;										// DefaultScale
	}
	if (v127()) ShowAgain();
	v132(0);
	if (f_d90)
	{
		f_d90->v107(0);
		f_d90->v36(v127());
	}
	f_da8 = f_dac;											// (as shipped: the timer is copied into DetachTime)
}

void C3DGrannyObj::v264(void)								// 0x43c310
{
	// (debug log, empty)
	if (f_d70)
	{
		const bool same_level = (f_d8c == g_gametype->m_object_id);
		f_d7c = -1.0f;
		f_d88 = 0;
		ev58((char *)"STOP", 1);
		if (f_d70) dynamic_cast<C3DAnimated *>(f_d70)->ev88((char *)"none", this);
		f_d70 = NULL;
		if (!same_level) HideAgain();
	}
	// (debug log, empty)
}

void C3DGrannyObj::WriteState(OMediaStreamOperators &s)		// 0x43cbd0
{
	CLocalGameObject::WriteState(s);
	float v[4], a, b, c;
	v195(v); a = v[0];
	v195(v); b = v[1];
	v195(v); c = v[2];
	s << a << b << c;
	v201(v); a = v[0];
	v201(v); b = v[1];
	v201(v); c = v[2];
	s << a << b << c;
	bool attached = f_d88 != 0, f140 = f_140 != 0;
	s << attached;
	s << f_c44;
	s << f_ca8;
	s << f140;
	int index = -1;
	if (f_d70) index = (int)f_d70->f_438;
	s << index;
	bool visible = f_d94 != 0;
	s << visible;
	s << f_6f0;
	s << f_d98;
	s << f_dac;
}

void C3DGrannyObj::ReadState(OMediaStreamOperators &s)		// 0x43c9f0
{
	CLocalGameObject::ReadState(s);
	if (f_d70) dynamic_cast<C3DAnimated *>(f_d70)->ev88((char *)"none", this);
	float a, b, c;
	s >> a >> b >> c;
	v197(a, b, c);											// position
	s >> a >> b >> c;
	v203(a, b, c);											// rotation
	v218(a, b, c);
	bool attached, f140;
	s >> attached;
	f_d88 = attached;
	s >> f_c44;
	s >> f_ca8;
	s >> f140;
	f_140 = f140;
	int index;
	s >> index;
	if (index == -1) f_d70 = NULL;
	else
	{
		f_d70 = FindObjectByIndex(index);
		if (!f_d70) ErrorLog("ERROR: AttachObj is null in LoadInfo for %s", m_tag);
	}
	if (f_d70 && f_d88)
	{
		CGameObject *o = f_d70;
		f_d70 = NULL;
		ev97(o);
	}
	bool visible;
	s >> visible;
	f_d94 = visible;
	if (f_d94) ShowAgain();
	else HideAgain();
	s >> f_6f0;
	s >> f_d98;
	s >> f_dac;
}

// --------------------------------------------------------------------------- element family

void C3DGrannyObj::ev69(void)								// 0x43c750
{
	if (f_649) return;
	f_649 = 1;
	if (strcasecmp(f_924, "none") != 0) InitModel(std::string(f_924));	// element +0x144
	if (strcasecmp(f_988, "none") != 0) CreateAnim((char *)"STOP", f_988);	// +0xe0
	if (strcasecmp(f_9ec, "none") != 0) CreateAnim((char *)"ANIM1", f_9ec);
	if (strcasecmp(f_a50, "none") != 0) CreateAnim((char *)"ANIM2", f_a50);
	if (strcasecmp(f_ab4, "none") != 0) CreateAnim((char *)"ANIM3", f_ab4);
	if (strcasecmp(f_b18, "none") != 0) CreateAnim((char *)"ANIM4", f_b18);
	if (strcasecmp(f_b7c, "none") != 0) CreateAnim((char *)"ANIMMyTouch", f_b7c);
	ev58((char *)"STOP", 1);
}

void C3DGrannyObj::ev94(void)								// 0x43c950
{
	if (strcasecmp(m_tag, "GETWATERGUN") != 0) return;
	CGameObject *o = FindObjectByTag("JIM1");
	if (!o) return;
	C3DAnimated *jim = dynamic_cast<C3DAnimated *>(o);
	o->v261();												// CGameObject +0x414
	jim->ev58((char *)"STOP", 1);
	JimmyActivateMenuItem(jim, this, g_jimmy, 6, 12.0f);	// C3DJimmy element +0x228 (C3DPlayer.h hook)
}

void C3DGrannyObj::ev95(unsigned char attached)				// 0x43ba30
{
	f_d88 = attached;
	if (attached)
	{
		ShowAgain();
		f_6f0 = 1.0f;
	}
	else
		HideAgain();
}

void C3DGrannyObj::ev96(CGameObject *other)					// 0x43c620
{
	ev94();
	if (strcasecmp(f_b7c, "none") != 0) ev58((char *)"ANIMMyTouch", 0);
	if (strcasecmp(f_be0, "none") != 0 && other->NameContains((char *)"C3DANIMATED"))
	{
		if (other->NameContains((char *)"C3DAI"))
		{
			C3DAI *ai = dynamic_cast<C3DAI *>(other);		// port: a name with "C3DAI" that is not one is left alone
			if (ai) ai->f_96c = 7;							// waiting (set directly, not through ev107)
		}
		if (f_d90) dynamic_cast<C3DSprite *>(f_d90)->HideAgain();	// sprite element (-0xc8) vtable +0xd8
		dynamic_cast<C3DAnimated *>(other)->ev58(f_be0, 0);
	}
	v136(0);
	v132(0);
	f_d7c = f_d80;											// DelayAttach
}

void C3DGrannyObj::ev97(CGameObject *target)				// 0x43be90
{
	if (f_d70 == NULL)
	{
		if (f_d98 == 1)										// GlowAndScale: back to full colour
		{
			OMedia3DMaterial *m = (*shape->get_polygons())[0].get_material();
			if (m) m->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
		f_d70 = target;
		f_d8c = g_gametype->m_object_id;
		ev95(1);											// +0x17c
		C3DAnimated *t = f_d70 ? dynamic_cast<C3DAnimated *>(f_d70) : NULL;
		t->ev88(f_ca8, this);								// our sequence on its GrannyYouBone
	}
	if (f_d78 != -1)
	{
		SetItemState(2, (short)f_d78, 1);
		AnnouncePickup(f_d78, f_d74);			// 0x408f80
	}
}

void C3DGrannyObj::ev98(CGameObject *target, unsigned char a)	// 0x43bf90
{
	if (!target) return;
	if (f_d78 != -1) ClearItemState(2, (short)f_d78);
	f_d7c = -1.0f;
	f_d88 = 0;
	C3DAnimated *t = dynamic_cast<C3DAnimated *>(target);
	t->ev88((char *)"none", this);							// +0x160
	t->ev89(this);											// +0x164
	f_d70 = NULL;
	v136(a);
	v132(0);
}
