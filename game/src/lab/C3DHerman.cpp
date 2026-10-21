/*  C3DHerman - see C3DHerman.h. Reconstructed from Neutron2.exe 0x43e2a0..0x43f010, 0x468090. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DHerman.h"
#include "../C3DRocketShip.h"				// neg::JimmyHurt
#include "../C3DTriggerType.h"
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMediaStreamOperators.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x4044c0 (C2DInGameMenu::PositionSprite, `this` unused: Herman calls it with his interface
// element as `this`): place a sprite at (x, y) in 640x480 space, centred, its top at y.
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

// --------------------------------------------------------------------------- construction

C3DHerman::C3DHerman()										// 0x43e2a0
{
	// (f_1b50, f_1c54: InterfaceElt constructors 0x480a10)
	SetName((char *)"C3DHERMAN");
	Log("C3DHerman()");
	f_649 = 0;												// ev69 loads the animations later
	f_6d0 = 1;												// Granny
	Log("InitObject()");									// 0x43e920 inlined
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// element +0x110
	C3DObject::SetObjectID('3HER');							// 0x49ab00
	f_96c = 2;												// patrolling
	f_c08 = 2;												// AIState
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 160.0f;											// speed
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 20.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_a9c, (char *)"STOP");					// waiting and sitting are stopping
	C3DAI::ev106(f_b14, (char *)"STOP");
	C3DAI::ev106(f_b3c, (char *)"WALK");
	C3DAI::ev106(f_b64, (char *)"RUN");
	m_show_on_radar = 1;									// CLocalGameObject +0x4c0
	f_1b50.db = g_screens_db;								// 0x545e50
	f_1b50.canvas_id = 411;
	f_1b50.x = 0;
	f_1b50.y = 0;
	f_1b50.src_blend = omblendfc_Src_Alpha;
	f_1b50.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1b50.home_x = 0;
	f_1b50.home_y = 0;
	f_1b50.f_34 = 1;
	f_1c54.canvas_id = 462;
	f_1c54.db = g_screens_db;
	f_1c54.x = 0;
	f_1c54.y = 0;
	f_1c54.src_blend = omblendfc_Src_Alpha;
	f_1c54.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1c54.home_x = 0;
	f_1c54.home_y = 0;
	f_1c54.f_34 = 1;
	f_1d58 = 1.0f;											// health
	f_1d5c = 50.0f;
	f_1d60 = 0;
	v50(0);													// 0x4abf70
	v77(0, 1.0f, 0);										// 0x4ac290
	// (then a call to the empty 0x463890)
}

C3DHerman::~C3DHerman()										// 0x43e560
{
	f_1b50.Hide();											// 0x4826f0
	f_1c54.Hide();
	// (then the empty InterfaceElt destructors and ~C3DAI)
}

namespace neg {
CGameObject *New_C3DHerman(void) { return new C3DHerman; }				// '3HER' (0x1d6c bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DHerman::InitObject(void)							// 0x43e920
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// element +0x110
	// (then a call to the empty 0x463890)
}

void C3DHerman::v260(void)									// 0x417110
{
	C3DAI::v260();											// 0x4143a0
	ev70(1);												// element +0x118
}

void C3DHerman::AddTouching(CGameObject *other)				// 0x43e960
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other->NameContains((char *)"C3DJIMMY"))			// +0x18
	{
		v261();												// +0x414: stop
		ev107(7);											// element +0x1ac
		ev58((char *)"ATTACK1", 0);							// element +0xe8
		JimmyHurt(other, -5.0f, -5, 5.0f, 1);				// Jimmy's element +0x218
	}
	if (other->NameContains((char *)"C3DWATERBALLOON"))
	{
		ev107(1);
		v261();
		ev58((char *)"HIT", 0);
		ev144();											// element +0x240
		AddScore(20);										// 0x482b50
	}
}

void C3DHerman::Process(float dt)							// 0x43ea40
{
	C3DAI::Process(dt);										// 0x411240
	if (g_game_mode == 5)									// playing (0x53b5b4)
	{
		ev145(dt);											// element +0x244
		return;
	}
	f_1b50.Hide();											// 0x4826f0
	f_1c54.Hide();
}

void C3DHerman::v270(int old_mode, int new_mode)			// 0x43eeb0
{
	C3DAnimated::v270(old_mode, new_mode);					// 0x41e960
	// (debug log "C3DHerman::ChangeState %d  %d")
	if (new_mode == 3 && f_934 == -1 && strcasecmp("STOP", GetAnim()) == 0)	// element +0xf4
	{
		strcpy(f_938, "STOP");
		f_934 = 1;
		ev58((char *)"THINKMOVE", 1);
	}
	if (old_mode == 3 && f_934 != -1)
	{
		f_c38 = 0;											// animations enabled again
		ev58(f_938, 1);
		f_934 = -1;
	}
}

void C3DHerman::WriteState(OMediaStreamOperators &s)		// 0x468090
{
	C3DAI::WriteState(s);									// 0x415760
	s << f_1d58;
}

void C3DHerman::ReadState(OMediaStreamOperators &s)			// 0x43ee80
{
	C3DAI::ReadState(s);									// 0x4155e0
	s >> f_1d58;
}

// --------------------------------------------------------------------------- element family

void C3DHerman::ev60(void)									// 0x43ef90
{
	ev58((char *)"THINKMOVE", 1);							// element +0xe8
}

void C3DHerman::ev67(void)									// 0x43ea90
{
	// (debug log, empty)
	if (strcasecmp(ev66(), "HIT") == 0)						// +0x108
	{
		ev58((char *)"LOOK", 0);
		return;
	}
	if (strcasecmp(ev66(), "LOOK") == 0)
	{
		ev107(17);											// keep the target in view
		return;
	}
	if (strcasecmp(ev66(), "ATTACK1") == 0)
	{
		f_c38 = 0;
		ev107(5);											// flee ...
		f_c0c = 17;											// ... then FallbackAIState 17
		f_c3c = 3.0f;										// for 3 s
	}
}

void C3DHerman::ev69(void)									// 0x43e620
{
	if (f_649) return;
	f_649 = 1;
	InitModel(std::string("hermanbase.grn"));				// element +0x144
	CreateAnim((char *)"WALK", (char *)"hermanwalk.grn");	// +0xe0
	CreateAnim((char *)"FLY", (char *)"hermanwalk.grn");
	CreateAnim((char *)"TALK", (char *)"hermantalk.grn");
	CreateAnim((char *)"LAUGH", (char *)"hermanlaugh.grn");
	CreateAnim((char *)"STOP", (char *)"hermanstop.grn");
	CreateAnim((char *)"ANALYZE", (char *)"hermananalyze.grn");
	CreateAnim((char *)"ATTACK1", (char *)"hermanattack1.grn");
	CreateAnim((char *)"ATTACK2", (char *)"hermanattack2.grn");
	CreateAnim((char *)"ATTACK3", (char *)"hermanattack3.grn");
	CreateAnim((char *)"BEFORERUN", (char *)"hermanbeforerun.grn");
	CreateAnim((char *)"DIES", (char *)"hermandies.grn");
	CreateAnim((char *)"HALT", (char *)"hermanhalt.grn");
	CreateAnim((char *)"HIT", (char *)"hermanhit.grn");
	CreateAnim((char *)"LOOK", (char *)"hermanlook.grn");
	CreateAnim((char *)"MAIL", (char *)"hermanmail.grn");
	CreateAnim((char *)"HOLIDAY", (char *)"hermanneedsaholiday.grn");
	CreateAnim((char *)"RUN", (char *)"hermanrun.grn");
	CreateAnim((char *)"THINKMOVE", (char *)"hermanthinksmove.grn");
	CreateAnim((char *)"THINKSTOP", (char *)"hermanthinksstop.grn");
	CreateAnim((char *)"TURNGRAB", (char *)"hermanturngrab.grn");
	CreateAnim((char *)"TURNSPEAK", (char *)"hermanturnspeak.grn");
	CreateAnim((char *)"SPININ", (char *)"hermanspinin.grn");
	CreateAnim((char *)"SPINOUT", (char *)"hermanspinout.grn");
	ev58((char *)"STOP", 1);								// +0xe8
	v67(130.0f);											// CGameObject +0x10c: radius
	v64(0.75f);												// CGameObject +0x100
}

void C3DHerman::ev144(void)									// 0x43eb40
{
	f_1d60 = 15.0f;											// show the bar
	if (f_1d58 > 0)
	{
		if (g_difficulty == 0) f_1d58 -= 0.1f;				// 0x53b5b0; 0x4caa68
		else if (g_difficulty == 1) f_1d58 -= 0.06f;		// 0x4e167c
		else if (g_difficulty == 2) f_1d58 -= 0.03f;		// 0x4e1680
		return;
	}
	ev58((char *)"STOP", 1);								// defeated
	f_1b50.Hide();
	f_1c54.Hide();
	AddScore(200);											// 0x482b50
	CGameObject *t = FindObjectByTag("DEFEAT");
	if (!t) return;
	C3DTriggerType *trig = dynamic_cast<C3DTriggerType *>(t);	// port: the original calls the slot of whatever is tagged DEFEAT
	if (trig) trig->CallNextTrigger("DEFEAT", 0);			// sprite element +0xdc
}

void C3DHerman::ev145(float dt)								// 0x43ec10
{
	if (f_1d58 <= 0) return;
	// heal (x87: dt * a double constant + health, rounded once)
	if (g_difficulty == 0) f_1d58 = (float)((double)dt * 0.001 + (double)f_1d58);	// 0x4e1688
	else if (g_difficulty == 1) f_1d58 = (float)((double)dt * 0.0015 + (double)f_1d58);	// 0x4e1690
	else if (g_difficulty == 2) f_1d58 = (float)((double)dt * 0.002 + (double)f_1d58);	// 0x4e1698
	if (f_1d58 > 1.0f) f_1d58 = 1.0f;
	if (f_1d60 > 0)
	{
		f_1d60 -= dt;
		OMedia3DVector head = v224(0, 200.0f, 0);			// +0x380
		float sx, sy;
		if (v281(g_viewport, head.xyzw(), &sx, &sy))		// +0x464: on screen
		{
			// The bar sits over his head (screen y up: 240 - sy is 640x480 space).
			f_1b50.Show();									// 0x482520
			PositionSprite(f_1b50.canvas_elt, (sx + 320.0f) - 26.0f, (240.0f - sy) - 12.0f);	// 0x4044c0
			f_1c54.Show();
			PositionSprite(f_1c54.canvas_elt, (sx + 320.0f) - 24.0f, (242.0f - sy) - 12.0f);
			f_1c54.Scale(f_1d5c * f_1d58, 11.0f);			// 0x482390
			OMediaCanvasElementWrapper *bar = f_1c54.canvas_elt;
			if (f_1d58 <= 0.34f)							// 0x4d7f88
				bar->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 0, 0));	// 2002 +0x90..: red
			else
				bar->set_diffuse(OMediaFARGBColor(1.0f, 0, 1.0f, 0));	// green
			return;
		}
	}
	f_1b50.Hide();
	f_1c54.Hide();
}
