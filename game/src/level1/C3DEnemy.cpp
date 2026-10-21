/*  C3DEnemy - see C3DEnemy.h. Reconstructed from Neutron2.exe 0x42e610..0x430640. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DEnemy.h"
#include "C3DSpriteObject.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"
#include "../C3DTriggerType.h"
#include "../C3DWaterBalloon.h"
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DVector.h"
#include "OMediaStreamOperators.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop
#include <math.h>
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

// 0x48fcb0 (C3DSpriteObject's element slot 55, called directly): count *t down; 1 when it
// reaches 0 this time.
static bool Countdown(float *t, float dt)
{
	if (!(*t > 0)) return false;
	*t -= dt;
	if (*t > 0) return false;
	*t = 0;
	return true;
}

// C2DInGameMenu slot 289 (0x4044c0), called with an InterfaceElt as `this` (it does not use
// it): place a sprite at (x, y) in 640x480 space, centred alignment, its top at y.
// (As in C3DCarl.cpp.)
static void PositionSprite(OMediaCanvasElementWrapper *e, float x, float y)
{
	const float sx = (float)(short)g_screen_w * 0.0015625f;	// 1/640
	const float sy = (float)(short)g_screen_h * 0.0020833334f;	// 1/480
	if (!e) return;
	e->set_canvas_flags(omcanef_FreeWorldSize);
	e->set_auto_align(omaac_Center, omaac_Center);
	const float px = sx * x - (float)((short)g_screen_w / 2);
	const float py = ((float)((short)g_screen_h / 2) - e->get_height()) - sy * y;
	e->place(px, py, 0);
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// +0x30c
	return OMedia3DVector(p[0], p[1], p[2]);
}

static void SetupBarElements(InterfaceElt &frame, InterfaceElt &bar)
{
	frame.db = g_screens_db;								// (set up like InterfaceElement::Create)
	frame.canvas_id = 411;
	frame.x = 0;
	frame.y = 0;
	frame.src_blend = omblendfc_Src_Alpha;
	frame.dest_blend = omblendfc_Inv_Src_Alpha;
	frame.home_x = 0;
	frame.home_y = 0;
	frame.f_34 = 1;
	bar.db = g_screens_db;
	bar.src_blend = omblendfc_Src_Alpha;
	bar.dest_blend = omblendfc_Inv_Src_Alpha;
	bar.canvas_id = 359;
	bar.x = 0;
	bar.y = 0;
	bar.home_x = 0;
	bar.home_y = 0;
	bar.f_34 = 1;
}

// The glow sprite of types 4..6 (inlined in ev69): a C3DSpriteObject, sprites.omt canvas 26,
// size 270, opacity 0.4.
static C3DSpriteObject *NewGlow(void)
{
	C3DSpriteObject *s = new C3DSpriteObject;				// 0x4b3aa0(0x718), 0x472870
	s->m_sprite_index = 26;
	s->m_sprite_size = 270;
	s->f_678 = 0.4f;										// Opacity
	s->v107(0);												// CGameObject +0x1ac
	return s;
}

// --------------------------------------------------------------------------- construction

C3DEnemy::C3DEnemy()										// 0x42e610
{
	SetName((char *)"C3DENEMY");
	Log("C3DEnemy()");
	f_6d0 = 1;												// granny
	f_649 = 0;												// ev69 loads the animations later
	C3DEnemy::InitObject();									// (inlined)
	C3DObject::SetObjectID('3ENE');							// 0x49ab00
	f_9a8 = 1000.0f;										// VisibleRange
	f_968 = 160.0f;											// speed
	f_96c = 6;
	f_c08 = 6;												// AIState
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_aec, (char *)"WALK");					// flying too
	C3DAI::ev106(f_a9c, (char *)"STOP");
	C3DAI::ev106(f_b14, (char *)"STOP");
	v50(0);
	v40(1);
	v42(1);
	f_1b50 = 0;												// EnemyType
	f_1b54 = 0;
	f_1b58 = 0;
	f_1b5c = NULL;
	f_1b60 = 100.0f;										// energy
	strcpy(f_1d74, "none");									// ActivateWhenShot
	f_1dd8 = 3;												// NumToKill
	f_1ddc = 0;
	f_1b64 = 0;
	// (then a call to the empty 0x463890)
}

C3DEnemy::~C3DEnemy() {}									// 0x42e8a0

namespace neg {
CGameObject *New_C3DEnemy(void) { return new C3DEnemy; }	// '3ENE' (0x1de8 bytes)
}

void C3DEnemy::InitObject(void)								// 0x42e960
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	RegisterProperty("EnemyType", &f_1b50, PROP_LONG, 0);
	RegisterProperty("ActivateWhenShot", f_1d74, PROP_ALPHA, 0);
	RegisterProperty("NumToKill", &f_1dd8, PROP_LONG, 0);
}

// --------------------------------------------------------------------------- element slots

void C3DEnemy::ev69(void)									// 0x42f8b0
{
	if (f_649) return;
	f_649 = 1;
	switch (f_1b50)
	{
		case 0:
		case 1:
			InitModel(std::string("nummeybase.grn"));		// +0x144
			CreateAnim((char *)"TALK", (char *)"nummeytalk.grn");	// +0xe0
			CreateAnim((char *)"WALK", (char *)"nummeywalk.grn");
			CreateAnim((char *)"STOP", (char *)"nummeystop.grn");
			CreateAnim((char *)"SMALLSTOP", (char *)"nummeystop.grn");
			CreateAnim((char *)"LAUGH", (char *)"nummeylaugh.grn");
			CreateAnim((char *)"TYPE", (char *)"nummeypushbuttons.grn");
			v67(100.0f);									// CGameObject +0x10c: radius
			ev58((char *)"STOP", 1);						// +0xe8
			v64(1.265f);									// CGameObject +0x100
			break;
		case 2:
			InitModel(std::string("ghostbase.grn"));
			CreateAnim((char *)"TALK", (char *)"ghoststop.grn");
			CreateAnim((char *)"WALK", (char *)"ghostwalk.grn");
			CreateAnim((char *)"FALL", (char *)"ghostfall.grn");
			CreateAnim((char *)"STOP", (char *)"ghoststop.grn");
			v67(60.0f);
			ev58((char *)"STOP", 1);
			v64(1.265f);
			return;											// (the ghosts are not put on the radar)
		case 3:
			InitModel(std::string("mummybase.grn"));
			CreateAnim((char *)"STOP", (char *)"mummyrest.grn");
			CreateAnim((char *)"HIT", (char *)"mummyhit.grn");
			CreateAnim((char *)"DIE", (char *)"mummyfall.grn");
			CreateAnim((char *)"WALK", (char *)"mummywalk.grn");
			CreateAnim((char *)"RUN", (char *)"mummyrun.grn");
			CreateAnim((char *)"SWING", (char *)"mummyswing.grn");
			CreateAnim((char *)"DEAD", (char *)"mummydead.grn");
			ev106(f_b3c, (char *)"WALK");					// +0x1a8
			f_1b44 = 10000.0f;
			v67(90.0f);
			ev58((char *)"STOP", 1);
			v64(1.4f);
			break;
		case 4:
			SetupBarElements(f_1b68, f_1c6c);
			f_b9c.y = 3.0f;									// turn rate
			InitModel(std::string("nummeyscooterbase.grn"));
			CreateAnim((char *)"STOP", (char *)"nummeyscooter.grn");
			CreateAnim((char *)"TALK", (char *)"nummeyscootertalk.grn");
			CreateAnim((char *)"WALK", (char *)"nummeyscooter.grn");
			CreateAnim((char *)"RUN", (char *)"nummeyscooter.grn");
			v67(80.0f);
			if (!f_1b5c) f_1b5c = NewGlow();
			ev58((char *)"STOP", 1);
			v64(1.265f);
			break;
		case 5:
			SetupBarElements(f_1b68, f_1c6c);
			f_b9c.x = 1.0f;									// turn rates
			f_b9c.y = 1.0f;
			f_b9c.z = 1.0f;
			f_c64 = 1;										// bank into turns
			InitModel(std::string("nummeyflyerbase.grn"));
			CreateAnim((char *)"STOP", (char *)"nummeyflytalk.grn");
			CreateAnim((char *)"TALK", (char *)"nummeyflytalk.grn");
			CreateAnim((char *)"WALK", (char *)"nummeyfly.grn");
			CreateAnim((char *)"FLY", (char *)"nummeyfly.grn");
			v67(220.0f);
			if (!f_1b5c) f_1b5c = NewGlow();
			ev58((char *)"STOP", 1);
			v64(1.265f);
			break;
		case 6:
			SetupBarElements(f_1b68, f_1c6c);
			f_b9c.y = 1.0f;
			InitModel(std::string("ghostnummeybase.grn"));
			CreateAnim((char *)"TALK", (char *)"ghoststop.grn");
			CreateAnim((char *)"WALK", (char *)"ghostnummeyrun.grn");
			CreateAnim((char *)"FALL", (char *)"ghostnummeyfall.grn");
			CreateAnim((char *)"TRIP", (char *)"ghostnummeytrip.grn");
			CreateAnim((char *)"STOP", (char *)"ghoststop.grn");
			v67(60.0f);
			if (!f_1b5c) f_1b5c = NewGlow();
			ev58((char *)"STOP", 1);
			v64(1.265f);
			break;
		default:
			return;
	}
	m_show_on_radar = 1;									// (element +0x580)
}

void C3DEnemy::ev67(void)									// 0x430490
{
	if (strcasecmp(ev66(), "FALL") == 0)					// +0x108
	{
		if (f_1b50 != 2) ev107(f_970);						// +0x1ac: back to what it was doing
		else ev107(3);
		return;
	}
	if (strcasecmp(ev66(), "DIE") == 0)
	{
		ev58((char *)"DEAD", 0);
		ev107(1);
	}
}

void C3DEnemy::ev144(float amount)							// 0x42f6e0
{
	f_1b60 -= amount;
	if (f_1b60 < 0)
	{
		f_1b68.Hide();										// 0x4826f0
		f_1c6c.Hide();
		f_1b60 = 0;
		ev107(1);
		CGameObject *t = FindObjectByTag("OUTOFENERGY");
		if (!t) ErrorLog("ERROR: Could not find OUTOFENERGY TRIGGER");
		else if (t->NameContains((char *)"C3DAITRIGGER"))	// +0x18
		{
			JimmyActivateMenuItem(g_jimmy, NULL, g_jimmy, 0, 0.0f);	// Jimmy element +0x228
			float rot[4];
			v201(rot);										// +0x324
			v203(0, rot[1], 0);								// +0x32c: upright
			v201(rot);
			g_jimmy->v203(0, rot[1], 0);					// Jimmy faces the way it faces
			C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(t);	// (t - 0xc8)
			if (tt) tt->CallNextTrigger("OUTOFENERGY", 0);	// sprite +0xdc
		}
	}
	ev145();												// +0x244
}

void C3DEnemy::ev145(void)									// 0x42ea10
{
	f_968 = f_1b3c;											// InitialSpeed
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DEnemy::AddTouching(CGameObject *other)				// 0x42f250
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other->NameContains((char *)"C3DWATERBALLOON"))		// +0x18
	{
		if (f_1b50 != 3) return;
		C3DWaterBalloon *b = dynamic_cast<C3DWaterBalloon *>(other);	// (other - 0xc8)
		if (b) b->Pop();									// sprite +0xdc
		if (f_96c == 1) return;
		if (f_96c != 15)									// hit: run away
		{
			ev58((char *)"HIT", 0);							// +0xe8
			f_964 = FindObjectByTag("JIM1");
			ev138(1);										// +0x228
			f_1b30 = 3.0f;
			ev106(f_ac4, (char *)"RUN");					// +0x1a8
			f_968 = f_1b3c + f_1b3c;
			f_1b58 = 20.0f;
			return;
		}
		ev107(1);											// hit while running away: dies
		ev58((char *)"DIE", 0);
		AddScore(20);										// 0x482b50
		f_1b54 = 6000.0f;
		g_545f50++;											// mummies killed
		if (g_545f50 < f_1dd8) return;						// NumToKill
		CGameObject *o = FindObjectByTag(f_1d74);			// ActivateWhenShot
		if (!o) return;
		static_cast<CLocalGameObject *>(o)->v268(-1);		// +0x430: NextState(-1)
		g_545f50 = 0;
		return;
	}
	if (other != g_camera) return;
	switch (f_1b50)
	{
		case 2:
		{
			if (f_1b64 > 0) return;
			f_1b64 = 10.0f;
			static const char *const lines[5][2] = {
				{ "LIBBY", "libbyghost" }, { "CARL", "carlghost" }, { "CINDY", "cindyghost" },
				{ "NICK", "jimmynick28" }, { "SHEEN", "sheenhi" } };
			for (int i = 0; i < 5; i++)
				if (strcasecmp(m_tag, lines[i][0]) == 0)
					PlayGameSound3DNoDoppler(this, -1, std::string(lines[i][1]), false);	// 0x48f0f0
			return;
		}
		case 3:
		{
			if (f_96c == 1 || g_54a428 > 0) return;			// power 1 protects Jimmy
			ev106(f_b3c, (char *)"SWING");
			if (!other->NameContains((char *)"C3DJIMMY")) return;
			C3DJimmy *j = dynamic_cast<C3DJimmy *>(other);	// (other - 0xc0)
			if (j) j->ev134(-10.0f, -10, 5.0f, 1);			// element +0x218
			return;
		}
		case 4:
		case 5:
		case 6:
		{
			if (f_1ddc > 0) return;
			f_1ddc = 5.0f;
			f_1d70 = 20.0f;
			ev144(30.0f);									// +0x240
			AddScore(100);
			g_power_timers[0] = 0;
			PlayGameSound(-1, std::string("vrhit"), false);	// 0x48f420
			PlayGameSound(-1, std::string("pickup2"), false);
			C3DJimmy *j = Jimmy();
			j->f_4d58 = 0;									// his sparkles
			if (f_1b50 != 5) j->ev90(1);					// element +0x168: drop his props
			return;
		}
	}
}

void C3DEnemy::RemoveTouching(CGameObject *other)			// 0x42f860
{
	C3DAI::RemoveTouching(other);							// 0x415110
	if (other == g_camera && f_1b50 == 3) ev106(f_b3c, (char *)"WALK");
}

unsigned char C3DEnemy::v231(CGameObject *other)			// 0x430520
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	if (id == '3GOD' || id == '3DOR' || id == '3ENE' || id == '3JIM') return 1;
	if (id != '3WBL') return 0;
	if (f_1b50 == 3 && f_1b54 > 0) return 0;				// dead mummies let balloons through
	return 1;
}

void C3DEnemy::Process(float dt)							// 0x42ea30
{
	C3DAI::Process(dt);										// 0x411240
	if (g_gametype->IsPaused()) return;						// +0x114
	switch (f_1b50)
	{
		case 2:
			if (f_1b64 > 0)
			{
				f_1b64 -= dt;
				if (f_1b64 <= 0) f_1b64 = 0;
			}
			if (g_54a440 > 0 && f_96c != 1 && g_game_mode != 3 && !IsAnim((char *)"FALL"))	// power 7
			{
				ev107(1);
				ev58((char *)"FALL", 0);
			}
			return;
		case 3:
			if (Countdown(&f_1b54, dt) || (f_1b54 == 0 && Countdown(&f_1b58, dt)))	// 0x48fcb0
			{
				ev107(6);									// up again / done running away
				ev106(f_ac4, (char *)"WALK");
				f_968 = f_1b3c;
				ev58((char *)"STOP", 1);
			}
			return;
		case 4:
		case 5:
		case 6:
			break;
		default:
			return;
	}
	// the energy bar over its head
	bool hide = true;
	if (f_1d70 > 0)
	{
		f_1d70 -= dt;
		OMedia3DVector p = v224(0, 75.0f, 0);				// +0x380
		float pos[4] = { p.x, p.y, p.z, p.w };
		float sx, sy;
		if (v281(g_viewport, pos, &sx, &sy))				// +0x464
		{
			hide = false;
			if (f_1b60 > 0)
			{
				f_1b68.Show();								// 0x482520
				PositionSprite(f_1b68.canvas_elt, sx + 320.0f - 26.0f, 240.0f - sy - 12.0f);	// 0x4044c0
				f_1c6c.Show();
				PositionSprite(f_1c6c.canvas_elt, sx + 320.0f - 24.0f, 242.0f - sy - 12.0f);
				f_1c6c.Scale(f_1b60 * 0.5f, 11.0f);			// 0x482390
			}
		}
	}
	if (hide)
	{
		f_1b68.Hide();										// 0x4826f0
		f_1c6c.Hide();
	}
	if (f_1ddc > 0)
	{
		f_1ddc -= dt;
		if (f_1ddc < 0) f_1ddc = 0;
	}
	if (g_54a440 > 0 && f_96c != 1 && !IsAnim((char *)"FALL") && g_game_mode != 3)
	{
		ev107(1);
		ev58((char *)"FALL", 0);
	}
	if (f_96c == 15)										// running away: slows down
	{
		if (f_1b50 == 4)
		{
			if (g_difficulty == 0) f_968 -= dt * 12.0f;
			else if (g_difficulty == 1) f_968 -= dt * 9.5f;
			else if (g_difficulty == 2) f_968 -= dt * 5.5f;
			if (f_968 < 400.0f) f_968 = 400.0f;
		}
		else if (f_1b50 == 5)
		{
			f_bd4 = 1;										// hovering
			if (g_difficulty == 0) f_968 -= dt * 7.2f;
			else if (g_difficulty == 1) f_968 -= dt * 5.8f;
			else if (g_difficulty == 2) f_968 -= dt * 4.5f;
			if (f_968 < 200.0f) f_968 = 200.0f;
		}
		else if (f_1b50 == 6)
		{
			f_bd4 = 0;
			if (g_difficulty == 0) f_968 -= dt * 7.0f;
			else if (g_difficulty == 1) f_968 -= dt * 6.0f;
			else if (g_difficulty == 2) f_968 -= dt * 5.0f;
			if (f_968 < 250.0f) f_968 = 250.0f;
		}
	}
	// the glow sprite: towards the camera, fading in within 600 of the target
	if (!f_1b5c || !f_964) return;
	if (!(f_1b60 > 0))
	{
		f_1b5c->HideAgain();								// element +0xd8
		return;
	}
	const OMedia3DVector me = Position(this);
	OMedia3DVector v(me.x - g_viewport->getx(), me.y + 100.0f - g_viewport->gety(), me.z - g_viewport->getz());	// 0x4d2cb0
	const float s = 1.0f / v.quick_magnitude();
	v.x *= s;
	v.y *= s;
	v.z *= s;
	const OMedia3DVector t = Position(f_964);
	const float dx = me.x - t.x, dy = me.y - t.y, dz = me.z - t.z;
	const float d = sqrtf(dx * dx + dy * dy + dz * dz);
	if (!(d < 600.0f))
	{
		f_1b5c->HideAgain();
		return;
	}
	f_1b5c->set_diffuse(OMediaFARGBColor(1.0f - d * 0.0016666667f, 1.0f, 1.0f, 1.0f));	// +0x90..+0x9c (0x4d92c8: 1/600)
	f_1b5c->v197(me.x - v.x * 80.0f, me.y + 50.0f - v.y * 80.0f, me.z - v.z * 80.0f);	// +0x314
	f_1b5c->ShowAgain();									// element +0xd4
}

void C3DEnemy::v258(void)									// 0x42ea20
{
	f_96c = f_c08;											// AIState
	v260();													// +0x410
}

void C3DEnemy::v260(void)									// 0x42e9d0
{
	C3DAI::v260();											// 0x4143a0
	InitAnim();												// element +0x110
	ev70(1);												// element +0x118
	g_545f50 = 0;											// mummies killed
	ev145();												// element +0x244
}

void C3DEnemy::v270(int a, int msg)							// 0x430590
{
	C3DAnimated::v270(a, msg);								// 0x41e960
	if (msg != 4) return;
	f_1b68.Hide();
	f_1c6c.Hide();
}

void C3DEnemy::WriteState(OMediaStreamOperators &s)			// 0x4303a0
{
	C3DAI::WriteState(s);									// 0x415760
	int killed = g_545f50;
	s << killed;
	s << GetAnim();											// element +0xf4: the current animation
	s << f_1b58;
	s << f_1b30;
	s << f_1b54;
}

void C3DEnemy::ReadState(OMediaStreamOperators &s)			// 0x430410
{
	C3DAI::ReadState(s);									// 0x4155e0
	int killed;
	s >> killed;
	g_545f50 = killed;
	char anim[128];
	s >> anim;
	ev58(anim, 1);											// element +0xe8
	s >> f_1b58;
	s >> f_1b30;
	s >> f_1b54;
}
