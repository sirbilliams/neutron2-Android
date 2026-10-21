/*  C3DPlayerObject - see C3DPlayerObject.h. */
#include "C3DPlayerObject.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "Physics.h"
#include "CGameType.h"
#include "CEditor.h"
#include "C2DInGameMenu.h"
#include "InterfaceElement.h"
#include "SaveGame.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaViewPort.h"
#include "CanvasWrapper.h"
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4b1d50: put the mouse cursor in the middle of the viewport (window vtable +0x38 -> +0x24).
static void CentreMouse(void)
{
	NEG_TODO(0x4b1d50, "centre the mouse cursor in the viewport");
}

// 0x410510 (a C2DInGameMenu method, `this` unused): forget the keys and mouse buttons held.
static void ClearInput(void)
{
	memset(g_keys, 0, 0xff);								// 0x3f dwords + a short + a byte
	g_mouse_down = 0;
	g_mouse_rdown = 0;
	g_545dc5 = 0;
	CentreMouse();
}

// 0x40a0c0 (a C2DInGameMenu method, copied from C2DInGameMenu_b.cpp): open the help text for
// `secs` (only shown while playing; otherwise kept for later in 0x545ef4).
static void OpenHelp(C2DInGameMenu *m, float secs)
{
	if (m->f_594 != 0) return;
	if (m->f_598 == 0) m->f_598 = secs;
	if (secs == 0) return;
	if (g_game_mode == 5)
	{
		g_help_icons[g_help_index]->hide(true);
		g_elt_544908[g_help_index].Show();
		g_help_menu_elt->Show();
		m->f_594 = -130;
		m->f_59c = 1;
		g_545ef4 = 0;
	}
	else g_545ef4 = (int)secs;
}

// --------------------------------------------------------------------------- construction

C3DPlayerObject::C3DPlayerObject()							// 0x464d50
{
	f_930 = 1.0f;
	g_5461cc = 0;
	f_934 = -1;
	g_5461ce = 0;
}

C3DPlayerObject::~C3DPlayerObject()							// 0x464e80
{
}

// --------------------------------------------------------------------------- CGameObject family

void C3DPlayerObject::v260(void)							// 0x464ed0
{
	C3DAnimated::v260();
	g_5461cc = 0;
}

void C3DPlayerObject::WriteState(OMediaStreamOperators &s) { CLocalGameObject::WriteState(s); }	// 0x4651b0
void C3DPlayerObject::ReadState(OMediaStreamOperators &s) { CLocalGameObject::ReadState(s); }		// 0x4651a0

// --------------------------------------------------------------------------- the camera helpers

float C3DPlayerObject::ev95(OMedia3DVector unused, OMedia3DVector *cam)	// 0x465010
{
	(void)unused;
	float p[4];
	v195(p);												// our position (w as v195 gives it)
	OMedia3DVector a(p[0], p[1], p[2]);
	a.w = p[3];
	if (!PhysicsLineBlocked(a, *cam, false)) return 1.0f;	// 0x4b7170
	*cam = g_phys_ray.end;									// 0x6311f8: the hit point
	const float dx = (p[0] - cam->x) * 0.75f;				// 0x4e2ba8
	const float dy = (p[1] - cam->y) * 0.75f;
	const float dz = (p[2] - cam->z) * 0.75f;
	cam->x = p[0] - dx;
	cam->y = p[1] - dy;
	cam->z = p[2] - dz;
	return 1.5f;
}

float C3DPlayerObject::ev96(OMedia3DVector target, OMedia3DVector *out, float max)	// 0x464ef0
{
	if (max <= 0.0f) return 1.0f;
	OMedia3DVector d(g_viewport->getx() - target.x, g_viewport->gety() - target.y, g_viewport->getz() - target.z);
	d.w = 0;
	const double dist = sqrt((double)d.y * d.y + (double)d.z * d.z + (double)d.x * d.x);
	if (dist <= max) return 1.0f;
	const float inv = 1.0f / d.quick_magnitude();
	*out = target;
	d.x *= inv;
	d.y *= inv;
	const double dz = (double)inv * d.z;
	const double m = (double)max - 3.0f;					// 0x4cb1e8
	out->x = (float)(m * d.x + out->x);
	out->y = (float)(m * d.y + out->y);
	out->z = (float)(m * dz + out->z);
	return 2.0f;
}

void C3DPlayerObject::ev97(OMedia3DVector pos, OMedia3DVector target)	// 0x465110
{
	g_viewport->place(pos.x, pos.y, pos.z);					// +0x44
	OMedia3DVector d(target.x - pos.x, target.y - pos.y, target.z - pos.z);
	d.w = 0;
	omt_Angle ax, ay;
	d.angles(ax, ay);
	g_viewport->set_angle(ax, ay, 0);						// +0x50
}

void C3DPlayerObject::ev98(float a, float b, float mode, OMedia3DVector cur, OMedia3DVector want,
						   OMedia3DVector target, OMedia3DVector axis)	// 0x4651c0
{
	if (mode != 1.0f)
	{
		ev97(want, target);									// element +0x184
		return;
	}
	const double dx = (double)want.x - cur.x;				// (x87: dx stays extended, dy/dz are stored)
	const float dy = want.y - cur.y, dz = want.z - cur.z;
	if (!f_8a7)
	{
		g_viewport->setx((float)((double)axis.x * dx * a * b + g_viewport->getx()));
		g_viewport->sety((float)((double)axis.y * dy * a * b + g_viewport->gety()));
		g_viewport->setz((float)((double)axis.z * dz * a * b + g_viewport->getz()));
	}
	OMedia3DVector d(target.x - cur.x, target.y - cur.y, target.z - cur.z);
	d.w = 0;
	omt_Angle ax, ay;
	d.angles(ax, ay);
	const short vx = g_viewport->get_anglex(), vy = g_viewport->get_angley();
	short turn = (short)((ay - vy) & 0x3fff);				// the short way round
	g_54a490 = (float)vx;
	g_54a494 = (float)vy;
	g_54a480 = g_viewport->getx();
	g_54a484 = g_viewport->gety();
	g_54a488 = g_viewport->getz();
	if (turn > 0x2000) turn -= 0x4000;
	if (!f_8a7)
	{
		g_viewport->set_anglex((omt_WAngle)(vx + (short)(ax - vx)));
		g_viewport->set_angley((omt_WAngle)(vy + turn));
	}
}

// --------------------------------------------------------------------------- buttons

unsigned char C3DPlayerObject::ev103(void)					// 0x4655d0
{
	if (!g_mouse_rdown && !g_keys[0x2d] && !g_keys[0x10] && !g_keys[0x60] && !g_joy_button1)	// VK_INSERT, VK_SHIFT, VK_NUMPAD0
		return 0;
	if (g_menu && g_menu->IsCurrentGadget(6) && ev104() && v133()) return 0;	// menu +0x4e0
	return 1;
}

unsigned char C3DPlayerObject::ev104(void)					// 0x465650
{
	if (g_5461ce) return 0;
	if (!g_mouse_down && !g_keys[0x0d] && !g_keys[0x11] && !g_joy_button0) return 0;	// VK_RETURN, VK_CONTROL
	return 1;
}

// --------------------------------------------------------------------------- out

void C3DPlayerObject::ev105(void)							// 0x465360
{
	if (g_editor->m_load_countdown > 0) return;				// +0x7fa
	if (g_editor->m_load_pending) return;					// +0x74c
	char game[80];
	strcpy(game, g_game_file);
	g_5461cd = 1;
	ClearInput();											// 0x410510
	if (ReadTaskFile("autosave.tsk"))						// 0x497b10
	{
		if (strcasecmp(game, g_game_file) == 0)
		{
			// (debug log "test %s   FIleName %s  TaskName %s", empty)
			LoadTask("autosave.tsk", false);				// 0x498af0
			if (g_camera)
			{
				g_camera->v136(1);							// +0x220
				g_5461cc = 0;
			}
		}
		else LoadTask("autosave.tsk", false);
	}
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	const float r = (float)(((int)((*seed >> 16) * 1001) >> 16) & 0xffff) * 0.001f;	// 0x4cb218
	const char *say;
	if (r < 0.2f) say = "jimmyself25";
	else if (r < 0.4f) say = "jimmyself60";
	else if (r < 0.6f) say = "jimmyself61";
	else if (r < 0.8f) say = "jimmyself62";
	else say = "jimmyself63";
	if (!IsChannelPlaying(g_voice_channel))					// 0x4b89d0
		g_voice_channel = PlayGameSound(-1, std::string(say), false);	// 0x48f420
	OpenHelp(g_menu, 8.0f);									// 0x40a0c0
}
