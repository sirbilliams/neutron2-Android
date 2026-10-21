/*  C3DGiantWasp - see C3DGiantWasp.h. Reconstructed from Neutron2.exe 0x437ad0..0x4382b0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DGiantWasp.h"
#include "../C3DRocketShip.h"				// neg::JimmyHurt
#include "../C3DTriggerType.h"
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop
#include <SDL.h> // TEMPDBG
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
// space, centred, its top at y (as C3DHerman's).
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

C3DGiantWasp::C3DGiantWasp()								// 0x437ad0
{
	// (f_1b68, f_1c6c: InterfaceElt constructors 0x480a10)
	Log("C3DGiantWasp()");
	SetName((char *)"C3DGIANTWASP");
	C3DObject::SetObjectID('3GWA');							// 0x49ab00
	CGameObject::v67(800.0f);								// 0x4ac130: radius
	CGameObject::v152(800.0f);								// 0x4aca00
	f_1b68.db = g_screens_db;								// 0x545e50
	f_1b68.canvas_id = 411;
	f_1b68.x = 0;
	f_1b68.y = 0;
	f_1b68.src_blend = omblendfc_Src_Alpha;
	f_1b68.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1b68.home_x = 0;
	f_1b68.home_y = 0;
	f_1b68.f_34 = 1;
	f_1c6c.canvas_id = 359;
	f_1c6c.db = g_screens_db;
	f_1c6c.x = 0;
	f_1c6c.y = 0;
	f_1c6c.src_blend = omblendfc_Src_Alpha;
	f_1c6c.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1c6c.home_x = 0;
	f_1c6c.home_y = 0;
	f_1d74 = 0;
	f_1d75 = 0;
	f_1c6c.f_34 = 1;
	f_1d70 = 100.0f;										// health
}

C3DGiantWasp::~C3DGiantWasp()								// 0x437ca0
{
	f_1b68.Hide();											// 0x4826f0
	f_1c6c.Hide();
}

namespace neg {
CGameObject *New_C3DGiantWasp(void) { return new C3DGiantWasp; }		// '3GWA' (0x1d80 bytes)
}

// --------------------------------------------------------------------------- element family

void C3DGiantWasp::ev69(void)								// 0x437d60
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"MOVE", (char *)"waspmove.ase");		// element +0xe0
	CreateAnim((char *)"STOP", (char *)"waspstop.ase");
	CreateTexture((char *)"wasp.png", 0);					// element +0xfc
	CreateTexture((char *)"wirewasp.png", 1);
	ev58((char *)"MOVE", 1);								// element +0xe8
	ApplyTexture(f_590, 0);									// element +0x100
	v67(300.0f);											// +0x10c: radius
	v64(0.7f);												// +0x100
}

// --------------------------------------------------------------------------- CGameObject family

void C3DGiantWasp::AddTouching(CGameObject *other)			// 0x438100
{
	CGameObject::AddTouching(other);						// 0x4ab970 (not C3DAI's / C3DFlyingBot's)
	if (other->NameContains((char *)"C3DENERGYPROJECTILE"))
	{
		SDL_Log("TEMPDBG wasp hit health=%f", f_1d70);
		if (f_1d70 <= 0)									// no health left: dead
		{
			f_1d74 = 1;
			f_1b68.Hide();									// 0x4826f0
			f_1c6c.Hide();
			ev107(1);										// element +0x1ac
		}
		else
		{
			if (g_difficulty == 0) f_1d70 -= 10.0f;			// 0x53b5b0; 0x4cb1cc
			else if (g_difficulty == 1) f_1d70 -= 5.0f;		// 0x4caa90
			else if (g_difficulty == 2) f_1d70 -= 2.0f;		// 0x4caa70
			AddScore(20);									// 0x482b50
			PlayGameSound(-1, std::string("vrzap"), false);	// 0x48f420
			f_1d75 = 1;
		}
	}
	if (other == g_camera)
	{
		ev107(5);											// flee
		f_c10 = 5000.0f;
		f_c3c = 15.0f;
		if (g_jimmy) JimmyHurt(g_jimmy, -10.0f, -10, 5.0f, 1);	// element +0x218 (port: checked)
	}
}

void C3DGiantWasp::Process(float dt)						// 0x437e10
{
	C3DAI::Process(dt);										// 0x411240 (not C3DFlyingBot's)

	// the health bar over it
	OMedia3DVector head = v224(0, 0, 400.0f);				// +0x380
	float sx, sy;
	if (v281(g_viewport, head.xyzw(), &sx, &sy) && f_1d75)	// +0x464: on screen
	{
		f_1b68.Show();										// 0x482520
		PositionSprite(f_1b68.canvas_elt, (sx + 320.0f) - 26.0f, (240.0f - sy) - 12.0f);	// 0x4044c0
		f_1c6c.Show();
		PositionSprite(f_1c6c.canvas_elt, (sx + 320.0f) - 24.0f, (242.0f - sy) - 12.0f);
		f_1c6c.Scale(f_1d70 * 0.5f, 11.0f);					// 0x482390
	}
	else
	{
		f_1b68.Hide();
		f_1c6c.Hide();
	}

	if (g_gametype->IsPaused())								// +0x114
	{
		f_1b68.Hide();
		f_1c6c.Hide();
		return;
	}
	if (f_1d70 > 0 || !f_1d74) return;

	// dead. (f_1b50, the knock-out time, stays 0 for the wasp: only C3DFlyingBot::AddTouching
	// sets it, and the wasp's does not call it. So it is gone at once.)
	ApplyTexture(f_590, 1);									// element +0x100: the wire
	const float lo = f_1b60 - 5.0f;							// 0x4caa90
	if (f_1b50 < f_1b60 - 1.0f && f_1b50 > lo)				// 0x4caa6c: fading out
	{
		float a = (f_1b50 - lo) / ((f_1b60 - 1.0f) - lo);
		if (a < 0) a = 0;
		ev71(a);											// element +0x11c
		if (v133())											// +0x214
		{
			v136(0);
			v132(0);
		}
		return;
	}
	if (f_1b50 < lo)										// gone (every frame from now on)
	{
		HideAgain();										// element +0xdc
		f_1b68.Hide();
		f_1c6c.Hide();
		v136(0);											// +0x220
		v132(0);											// +0x210
		CGameObject *block = FindObjectByTag("BLOCKWASP");
		if (block) block->v136(0);							// port: checked
		CGameObject *t = FindObjectByTag("WASPDIE");
		if (t && t->NameContains((char *)"C3DAITRIGGER"))
		{
			C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(t);	// (element - 0xc8)
			if (tt) tt->CallNextTrigger("WASPDIE", 0);		// element +0xdc
		}
	}
}
