/*  C3DWaterPlane - see C3DWaterPlane.h. Reconstructed from Neutron2.exe 0x47e420..0x47f7f0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DWaterPlane.h"
#include "../C3DPlayer.h"
#include "../C3DRocketShip.h"									// JimmyHurt
#include "../C2DInGameMenu.h"
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../CJimmyGame.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DShape.h"
#include "OMediaStreamOperators.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

static const float kNoLevel = -99999.0f;					// f_928: not moving (compared as bits)

// --------------------------------------------------------------------------- small helpers

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

static C3DObject *AsObject(CGameObject *o)	{ return o ? dynamic_cast<C3DObject *>(o) : NULL; }	// (o - 0xc0)

// The four waterfall objects, shown while the level rises (element +0xd8 / +0xdc).
static const char *const kFalls[4] = { "WATERSLOWLEFT", "WATERFASTLEFT", "WATERSLOWRIGHT", "WATERFASTRIGHT" };
static void ShowFalls(bool show)
{
	for (int i = 0; i < 4; ++i)
	{
		C3DObject *o = AsObject(FindObjectByTag(kFalls[i]));
		if (!o) continue;									// (Process's lowering branch does not check: port: checked)
		if (show) o->ShowAgain(); else o->HideAgain();
	}
}

// The camera as the player (element = camera - 0xc0). port: the original assumes it is one.
static C3DPlayer *CameraPlayer(void)	{ return g_camera ? dynamic_cast<C3DPlayer *>(g_camera) : NULL; }

static OMedia3DVector Position(CGameObject *o)	{ float p[4]; o->v195(p); return OMedia3DVector(p[0], p[1], p[2], p[3]); }
static OMedia3DVector Velocity(CGameObject *o)	{ float p[4]; o->v174(p); return OMedia3DVector(p[0], p[1], p[2], p[3]); }

// CJimmyGame element slots (every level game type is one).
static CJimmyGame *Game(void)	{ return dynamic_cast<CJimmyGame *>(g_gametype); }

// 0x485730: the neutron energy (g_health) += d, kept in 0..100; "low neutron energy" once when it
// falls below 19 (g_545fd4 remembers it until it is above 20 again).
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
	}
	else if (g_health > 20.0f) g_545fd4 = 0;
}

// 0x404d60: the gadget bar's size from a count (height = count * 1.04, scaled to the screen).
static void SetGadgetBarSize(float n)
{
	if (n < 0) n = 0;
	if (!g_gadget_bar) return;
	const float h = (float)(short)g_screen_h * 0.00208333344f;	// 0x4ca4b8: 1 / 480
	g_gadget_bar->set_size((float)(short)g_screen_w * 0.00156250002f * 20.0f,	// 0x4ca4bc: 1 / 640
						   (float)((double)h * n * 1.04));
}

// The swim sound: f_934 restarted as <name>, looping (unless the underwater music is on).
static void SwimSound(C3DWaterPlane *w, const char *name)
{
	if (w->f_931) return;
	if (w->f_934 != -1)
	{
		StopGameSound(w->f_934, false);						// 0x48f0d0
		w->f_934 = -1;
	}
	if (w->f_934 == -1) w->f_934 = PlayGameSound(-1, std::string(name), true);	// 0x48f420
}

// --------------------------------------------------------------------------- construction

C3DWaterPlane::C3DWaterPlane()								// 0x47e420
{
	SetName((char *)"C3DWATERPLANE");						// 0x4ab1b0
	Log("C3DWATERPLANE()");
	f_649 = 0;
	C3DWaterPlane::InitObject();							// 0x47e620 (called directly)
	C3DObject::SetObjectID('3WAT');							// 0x49ab00
	v66(0);
	v46(0);
	v52(0);
	v34(1);
	C3DObject::InitPhysics();								// 0x49acb0
	v107(1);
	f_924 = 0;
	C3DObject::ev51();										// 0x49afd0 texture filtering on
	f_92c = 50.0f;
	f_928 = kNoLevel;
	v94(2);
	f_930 = 0;
	f_931 = 0;
	f_934 = -1;
	f_938 = -1;
	f_93c = 0;
	f_940 = 40.0f;
	f_944 = NULL;
	// (then a call to the empty 0x463890)
}

C3DWaterPlane::~C3DWaterPlane() {}							// 0x47e5d0

// --------------------------------------------------------------------------- element-family slots

// 0x47f420: when not moving, move the level by dy at <speed> per s.
void C3DWaterPlane::ev94(float dy, float speed)
{
	if (f_928 != kNoLevel) return;
	float p[4];
	v195(p);
	f_92c = speed;
	f_928 = dy + p[1];
}

// 0x47f470: when not moving, move the level to y at <speed> per s, with the "waterfalls" loop;
// rising, the four waterfall objects are shown.
void C3DWaterPlane::ev95(float y, float speed)
{
	if (f_928 != kNoLevel) return;
	if (f_938 == -1) f_938 = PlayGameSound(-1, std::string("waterfalls"), true);
	f_928 = y;
	f_92c = speed;
	float p[4];
	v195(p);
	if (p[1] < f_928) ShowFalls(true);						// element +0xd8 ShowAgain
}

// 0x47f390: the viewport came out of the water.
void C3DWaterPlane::ev96(void)
{
	if (f_931 != 1) return;
	f_931 = 0;
	CJimmyGame *g = Game();									// port: checked
	if (!g) return;
	g->PlayLevelMusic("musictomb1.omt", 0);					// +0x198
	g->SetFog(0.36f, 0.45f, 0.58f, 0, 1000.0f, 5000.0f, 5700.0f, true);	// +0x194
}

// 0x47f2d0: the viewport went under the water: the swim sound stops, music and a closer fog.
void C3DWaterPlane::ev97(void)
{
	if (f_931) return;
	if (f_934 != -1)
	{
		StopGameSound(f_934, false);
		f_934 = -1;
	}
	f_931 = 1;
	CJimmyGame *g = Game();
	if (!g) return;
	g->PlayLevelMusic("musictomb1.omt", 1);
	g->SetFog(0.36f, 0.46f, 0.58f, 0, 100.0f, 3500.0f, 4500.0f, true);
}

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DWaterPlane::InitObject(void)						// 0x47e620
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	f_649 = 1;
	if (g_gametype->m_object_id == 'LV3A')					// +0x490
	{
		CreateAnim((char *)"DEFAULT", (char *)"watertest.ase");
		CreateTexture((char *)"watertest.png", 0);
	}
	else
	{
		CreateAnim((char *)"DEFAULT", (char *)"cavewater.ase");
		CreateTexture((char *)"cavewater.png", 0);
	}
	ApplyTexture(f_590, 0);
	ev58((char *)"DEFAULT", 1);
}

// 0x47f590
void C3DWaterPlane::v260(void)
{
	C3DAnimated::v260();
	f_944 = FindObjectByTag("JIM1");
	g_5461cc = 0;
	ev71(0.7f);												// element +0x11c alpha
	ev91(1);												// +0x16c second pass
	get_shape()->set_two_sided(true);						// element +0xb0, shape +0x74
	ShowFalls(false);										// element +0xdc HideAgain
}

void C3DWaterPlane::Process(float dt)						// 0x47e6e0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (g_gametype->m_object_id == 'LV3A')
	{
		ev71((float)((cos((double)f_924 * 4.0) + 1.0) * 0.5));	// (f_924 does not advance here)
		return;
	}
	if (!g_camera) return;
	C3DAnimated::Process(dt);								// 0x41c120
	f_924 += dt;

	// the level moving to f_928
	if (f_928 != kNoLevel)
	{
		OMedia3DVector p = Position(this);
		bool arrived;
		if (p.y < f_928)
		{
			v197(p.x, dt * f_92c + p.y, p.z);				// +0x314
			arrived = !(Position(this).y < f_928);
		}
		else
		{
			v197(p.x, p.y - dt * f_92c, p.z);
			arrived = Position(this).y <= f_928;
		}
		if (arrived)
		{
			p = Position(this);
			v197(p.x, f_928, p.z);
			f_928 = kNoLevel;
			ShowFalls(false);								// element +0xdc HideAgain
			if (f_938 != -1)
			{
				StopGameSound(f_938, false);
				f_938 = -1;
			}
		}
	}

	C3DPlayer *pl = CameraPlayer();
	if (!f_930)
	{
		OMedia3DVector cam = Position(g_camera);
		OMedia3DVector me = Position(this);
		if (!((double)me.y + 7.0 - 1.0 > cam.y))			// (x87)
		{
			// out of the water
			if (!g_camera->v133()) return;					// +0x214
			if (pl && pl->IsAnim((char *)"FLOAT")) pl->ev58((char *)"STOP", 1);	// 0x41c6d0, element +0xe8
			if (pl && pl->IsAnim((char *)"SWIM")) pl->ev58((char *)"RUN", 1);
			if (f_934 != -1)
			{
				StopGameSound(f_934, false);
				f_934 = -1;
			}
			return;
		}
		// in
		v261();												// +0x414 (empty)
		f_930 = 1;
		OMedia3DVector vel = Velocity(g_camera);			// +0x2b8
		if (pl && !pl->IsAnim((char *)"SWIM"))
		{
			pl->f_a68 = 4;
			pl->ev58((char *)"SWIM", 1);
		}
		if (vel.y < -400.0f) PlayGameSound(-1, std::string("SPLASH"), false);
	}
	if (!g_camera) return;

	// the viewport under / above the surface
	if (g_viewport->gety() < Position(this).y) ev97();		// viewport +0x48; element +0x184
	if (g_viewport->gety() > Position(this).y) ev96();		// element +0x180

	// the oxygen (the gadget bar)
	if (g_game_mode == 5)
	{
		f_93c += dt;
		if (f_93c > 3.0f)
		{
			g_menu->ShowGadgetBar();						// +0x4c4
			float left = (1.0f - (f_93c - 3.0f) / f_940) * 100.0f;
			if (left < 0.0f)
			{
				AddEnergy(dt * -1.0f);						// 0x485730
				left = 0;
				JimmyHurt(g_camera, 0, -5, 3.0f, 0);		// camera element +0x218
			}
			SetGadgetBarSize(left);							// 0x404d60
		}
	}
	else if (g_game_mode != 6) g_menu->HideGadgetBar();	// +0x4c8

	OMedia3DVector cam = Position(g_camera);
	OMedia3DVector me = Position(this);
	float a[4];
	if (me.y + 7.0f < cam.y)
	{
		// out of the water
		if (g_camera->NameContains((char *)"C3DJIMMY"))
		{
			v261();
			if (pl) pl->f_968 = 600.0f;						// top speed
		}
		g_menu->HideGadgetBar();
		f_93c = 0;
		f_930 = 0;
		v155(a);											// +0x26c (ours)
		g_camera->v157(a[0], 0, a[2]);						// +0x274
		return;
	}

	if (g_cur_gadget != 8) JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 8, 13.0f);	// Jimmy element +0x228
	if (pl)
	{
		pl->ev116();										// element +0x1d0
		if (pl->f_96c > 300.0f) pl->f_96c = 300.0f;
		else if (pl->f_96c < 100.0f)
		{
			if (!pl->IsAnim((char *)"FLOAT"))
			{
				pl->f_a68 = 4;
				pl->ev58((char *)"FLOAT", 1);
				SwimSound(this, "sloshing");
			}
		}
		else if (!pl->IsAnim((char *)"SWIM"))
		{
			pl->f_a68 = 4;
			pl->ev58((char *)"SWIM", 1);
			SwimSound(this, "swim");
		}
	}

	OMedia3DVector vel = Velocity(g_camera);
	if (vel.y < -450.0f)
	{
		vel.y = -200.0f;
		g_camera->v175(vel.x, vel.y, vel.z, vel.w);			// +0x2bc
	}
	else if (vel.y > 90.0f)
	{
		vel.y = 90.0f;
		g_camera->v175(vel.x, vel.y, vel.z, vel.w);
	}
	if (g_camera->v133())									// on the floor: lifted off it
	{
		OMedia3DVector p = Position(g_camera);
		g_camera->v197(p.x, p.y + 10.0f, p.z);
		vel.y = 0;
		g_camera->v175(vel.x, vel.y, vel.z, vel.w);
	}
	if (PlayerKeyJump())									// 0x4658a0
	{
		v155(a);
		g_camera->v157(a[0], -(g_gravity.y * 0.9f), a[2]);	// 0x63119c
		if (vel.y < -80.0f) g_camera->v175(vel.x, -80.0f, vel.z, vel.w);
		return;
	}
	v155(a);
	g_camera->v157(a[0], -(g_gravity.y * 1.4f), a[2]);
}

void C3DWaterPlane::WriteState(OMediaStreamOperators &s)	// 0x47f710
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	bool b = f_930 != 0;
	s << b;
	float p[4];
	v195(p);
	float x = p[0], y = p[1], z = p[2];
	s << x;
	s << y;
	s << z;
	s << f_928;
}

void C3DWaterPlane::ReadState(OMediaStreamOperators &s)		// 0x47f680
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	bool b;
	s >> b;
	f_930 = b;
	float x, y, z;
	s >> x;
	s >> y;
	s >> z;
	v196(x, y, z, 1.0f);									// +0x310 (port: w = 1; the original passes an unset local)
	s >> f_928;
}

namespace neg {
CGameObject *New_C3DWaterPlane(void)	{ return new C3DWaterPlane; }		// factory '3WAT' (0x950 bytes)
}
