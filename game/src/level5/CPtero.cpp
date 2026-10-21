/*  CPtero - see CPtero.h. Reconstructed from Neutron2.exe 0x4671b0..0x468130. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CPtero.h"
#include "../C3DASEObj.h"
#include "../C3DJimmy.h"
#include "../C3DRadar.h"
#include "../C3DTriggerType.h"
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMediaStreamOperators.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop
#include <list>
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

// 0x4044c0 (C2DInGameMenu::PositionSprite, `this` unused): place a sprite at (x, y) in 640x480
// space, centred, its top at y (as in C3DHerman.cpp).
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

CPtero::CPtero()											// 0x4671b0
{
	// (f_1b50, f_1c54: InterfaceElt constructors 0x480a10)
	SetName((char *)"C3DPTERO");
	Log("C3DPtero()");
	f_1d68 = 0;
	f_649 = 0;												// ev69 loads the animations later
	Log("InitObject()");									// 0x467a80 inlined
	C3DAI::InitObject();									// 0x4110e0
	C3DObject::SetObjectID('3PTE');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 500.0f;											// speed
	f_96c = 3;												// state: patrol only
	f_c08 = 3;												// AIState
	f_b9c.y = 0.8f;											// turn rates
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 0;
	f_b9c.x = 0.5f;
	f_b9c.z = 0.5f;
	CGameObject::v42(1);									// 0x4abeb0 (direct calls)
	CGameObject::v40(1);
	CGameObject::v34(0);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// 0x411210: every animation is WALK
	C3DAI::ev106(f_a9c, (char *)"WALK");
	C3DAI::ev106(f_b14, (char *)"WALK");
	C3DAI::ev106(f_a74, (char *)"WALK");
	C3DAI::ev106(f_b3c, (char *)"WALK");
	C3DAI::ev106(f_aec, (char *)"WALK");
	f_1b50.db = g_screens_db;								// 0x545e50
	f_1b50.canvas_id = 411;
	f_1b50.x = 0;
	f_1b50.y = 0;
	f_1b50.src_blend = omblendfc_Src_Alpha;
	f_1b50.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1b50.home_x = 0;
	f_1b50.home_y = 0;
	f_1b50.f_34 = 1;
	f_1c54.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1c54.db = g_screens_db;
	f_1c54.canvas_id = 462;
	f_1c54.x = 0;
	f_1c54.y = 0;
	f_1c54.src_blend = omblendfc_Src_Alpha;
	f_1c54.home_x = 0;
	f_1c54.home_y = 0;
	f_1c54.f_34 = 1;
	CGameObject::v94(1);
	f_1d58 = 1.0f;											// health
	f_1d5c = 0;
	f_bd4 = 1;												// hovering
	CGameObject::v52(1);
	f_1d6c = 0;
	f_1d64 = -1;
	m_show_on_radar = 1;									// CLocalGameObject +0x4c0
	// (then a call to the empty 0x463890)
}

CPtero::~CPtero()											// 0x4674a0
{
	if (f_1d64 != -1)
	{
		StopGameSound(f_1d64, false);						// 0x48f0d0
		f_1d64 = -1;
	}
	f_1b50.Hide();											// 0x4826f0
	f_1c54.Hide();
	// (then the empty InterfaceElt destructors and ~C3DAI)
}

namespace neg {
CGameObject *New_CPtero(void) { return new CPtero; }		// '3PTE' (0x1d78 bytes)
}

void CPtero::InitObject(void)								// 0x467a80
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
}

void CPtero::ev69(void)										// 0x4677b0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"WALK", (char *)"PTerofly.ase");		// +0xe0
	CreateAnim((char *)"FALL", (char *)"PTerofall.ase");
	CreateAnim((char *)"TALK", (char *)"PTerotalk.ase");
	CreateAnim((char *)"STOP", (char *)"PTerostop.ase");
	CreateTexture((char *)"Pterodactyl.png", 0);			// +0xfc
	ev58((char *)"WALK", 1);								// +0xe8
	ApplyTexture(f_590, 0);									// +0x100
	v67(500.0f);											// radius
	v64(0.1f);
	C3DASEObj *s = new C3DASEObj;							// 0x4207e0 (0xbb0 bytes; even when windowed)
	f_1d68 = s;
	strcpy(s->f_924, "pteroshock.ase");						// ASEWalk
	strcpy(s->f_988, "dinoshockstop.ase");					// ASEStop
	strcpy(s->f_ab4, "dinoshock.png");						// PNGFile
	s->f_b7c = 1;											// DoubleSided
	s->f_b80 = 2.0f;										// SlideXSpeed
	s->f_b84 = 2.0f;										// SlideYSpeed
	s->f_8a0 = 0;											// Lit
	s->HideAgain();											// element +0xdc
	s->v107(0);												// +0x1ac
	s->v52(0);												// +0xd0
	f_1d6c = 0;
}

void CPtero::v260(void)										// 0x467ab0
{
	C3DAI::v260();											// 0x4143a0
	InitAnim();												// element +0x110
	if (f_1d64 == -1)
		f_1d64 = PlayGameSound3DNoDoppler(this, -1, std::string("pteroflap"), true);	// 0x48f0f0
	if (f_1d68) f_1d68->HideAgain();						// element +0xdc
}

void CPtero::AddTouching(CGameObject *other)				// 0x467580
{
	C3DAI::AddTouching(other);								// 0x415150
	if (!other->NameContains((char *)"C3DWATERBALLOON")) return;	// +0x18
	if (!(f_1d58 > 0)) return;
	if (g_difficulty == 0) f_1d58 -= 0.24f;					// 0x53b5b0; 0x4f3944
	else if (g_difficulty == 1) f_1d58 -= 0.15f;			// 0x4d7f7c
	else if (g_difficulty == 2) f_1d58 -= 0.09f;			// 0x4f3948
	f_1d6c = 0.3f;											// shock flash
	PlayGameSound(-1, std::string("vrzap"), false);			// 0x410850, 0x48f420
	C3DSprite *ball = dynamic_cast<C3DSprite *>(other);		// (other - 0xc8)
	if (ball) ball->HideAgain();							// element +0xd8 (port: checked)
	PlayGameSound(-1, std::string("SPLASH"), false);
	PlayGameSound(-1, std::string("balloon pop"), false);
	other->v132(0);											// +0x210
	other->v136(0);											// +0x220
	f_1d5c = 8.0f;											// show the bar
	if (f_1d58 > 0) return;
	// Out of health.
	if (f_1d68) f_1d68->HideAgain();						// (port: checked)
	AddScore(100);											// g_gametype 0x482b50
	f_1b50.Hide();											// 0x4826f0
	f_1c54.Hide();
	ev107(1);												// element +0x1ac: wait
	f_bd4 = 0;												// falls
	ev58((char *)"FALL", 0);
	f_1d60 = 8.0f;
	if (f_1d64 != -1)
	{
		StopGameSound(f_1d64, false);						// 0x48f0d0
		f_1d64 = -1;
	}
	PlayGameSound(-1, std::string("pterodown"), false);
}

void CPtero::Process(float dt)								// 0x467b30
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAI::Process(dt);										// 0x411240
	if (f_1d60 > 0)
	{
		const float t = f_1d60 - dt;
		f_1d60 = t;
		if (t < 6.0f && t > 1.0f)							// 0x4d7f90, 0x4caa6c
		{
			float pct = (t - 1.0f) * 0.2f;					// 0x4d7f8c (debug "Percentage=%f")
			if (pct < 0) pct = 0;
			ev71(pct);										// element +0x11c: fade out
		}
		else if (t < 1.0f)
		{
			HideAgain();									// element +0xdc: gone
			v136(0);										// +0x220
			v132(0);										// +0x210
			f_1d60 = 0;
			g_545f48++;
			if (g_545f48 >= 9)
			{
				CGameObject *t2 = FindObjectByTag("KILLDINOS");
				if (t2 && t2->NameContains((char *)"C3DTRIGGERTYPE"))
				{
					C3DJimmy *j = Jimmy();					// g_jimmy - 0xc0 (not checked)
					if (j && j->f_d40) j->f_d40->Hide();	// 0x468aa0 (port: checked)
					C3DTriggerType *trig = dynamic_cast<C3DTriggerType *>(t2);	// (t2 - 0xc8)
					if (trig) trig->CallNextTrigger("KILLDINOS", 0);	// element +0xdc
				}
			}
		}
	}
	if (!(f_1d58 > 0)) return;
	if (f_1d6c > 0 && f_1d68)								// (the original does not check the shock)
	{
		f_1d68->ShowAgain();								// element +0xd8
		f_1d68->ev72(0.1f);									// +0x120 additive
		float p[4], a[4];
		v195(p);											// +0x30c
		f_1d68->v196(p[0], p[1], p[2], p[3]);
		v201(a);											// +0x324
		f_1d68->v202(a[0], a[1], a[2], a[3]);
		f_1d6c = f_1d6c - dt;
		if (f_1d6c <= 0)
		{
			f_1d6c = 0;
			f_1d68->HideAgain();
		}
	}
	if (f_1d5c > 0)
	{
		f_1d5c = f_1d5c - dt;
		OMedia3DVector head = v224(0, 275.0f, 200.0f);		// +0x380
		float sx, sy;
		if (v281(g_viewport, head.xyzw(), &sx, &sy))		// +0x464: on screen
		{
			f_1b50.Show();									// 0x482520
			PositionSprite(f_1b50.canvas_elt, (sx + 320.0f) - 26.0f, (240.0f - sy) - 12.0f);	// 0x4044c0
			f_1c54.Show();
			PositionSprite(f_1c54.canvas_elt, (sx + 320.0f) - 24.0f, (242.0f - sy) - 12.0f);
			f_1c54.Scale(f_1d58 * 50.0f, 11.0f);			// 0x482390; 0x4ca4b0
			OMediaCanvasElementWrapper *bar = f_1c54.canvas_elt;
			if (f_1d58 > 0.34f)								// 0x4d7f88
				bar->set_diffuse(OMediaFARGBColor(1.0f, 0, 1.0f, 0));	// 2002 +0x90..: green
			else
				bar->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 0, 0));	// red
			return;
		}
	}
	f_1b50.Hide();											// 0x4826f0
	f_1c54.Hide();
}

void CPtero::WriteState(OMediaStreamOperators &s)			// 0x468090 (shared with C3DHerman)
{
	C3DAI::WriteState(s);									// 0x415760
	s << f_1d58;
}

void CPtero::ReadState(OMediaStreamOperators &s)			// 0x467fc0
{
	C3DAI::ReadState(s);									// 0x4155e0
	s >> f_1d58;
	if (f_1d58 > 0)
	{
		ShowAgain();										// element +0xd8
		v136(1);
		v132(1);
		ev71(0);											// +0x11c opaque
		return;
	}
	HideAgain();											// element +0xdc
	v136(0);
	v132(0);
	f_1b50.Hide();
	f_1c54.Hide();
	if (f_1d68) f_1d68->HideAgain();
}
