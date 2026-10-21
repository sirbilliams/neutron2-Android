/*  C3DPlayer - see C3DPlayer.h. */
#include "C3DPlayer.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "Physics.h"
#include "CGameType.h"
#include "CJimmyGame.h"
#include "CEditor.h"
#include "C2DInGameMenu.h"
#include "InterfaceElement.h"
#include "SaveGame.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaTrigo.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360
static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// --------------------------------------------------------------------------- input

// The key lists end with 0; 0x1f5 is the left mouse button, 0x1f4 the right one, anything else a
// key of g_keys. A key that counts also clears the mouse movement (0x4a39e0).
static bool KeyListDown(const int *list)
{
	for (; *list; list++)
	{
		const int k = *list;
		if (k == 0x1f5) { if (g_mouse_down) return true; }
		else if (k == 0x1f4) { if (g_mouse_rdown) return true; }
		else if ((unsigned)k < 256 && g_keys[k])			// port: index checked (the original reads g_keys[k] for any k)
		{
			g_56aebe = 0;									// 0x4a39e0
			g_56aec0 = 0;
			return true;
		}
	}
	return false;
}

bool neg::PlayerKeyAction(void)			{ return g_joy_button0 || KeyListDown(g_key_action); }			// 0x465750
bool neg::PlayerKeyTurnAround(void)		{ return g_joy_button8 || KeyListDown(g_key_turnaround); }		// 0x4657c0
bool neg::PlayerKeySpeed(void)			{ return g_joy_button1 || KeyListDown(g_key_speed); }			// 0x465830
bool neg::PlayerKeyJump(void)			{ return g_joy_button1 || KeyListDown(g_key_jump); }			// 0x4658a0
bool neg::PlayerKeyGo(void)				{ return g_joy_y > 0.5f || KeyListDown(g_key_go); }				// 0x465910
bool neg::PlayerKeyBack(void)			{ return g_joy_y < -0.5f || KeyListDown(g_key_back); }			// 0x465980
bool neg::PlayerKeyLeft(void)			{ return g_joy_x < -0.5f || KeyListDown(g_key_turn_left); }		// 0x4659f0
bool neg::PlayerKeyRight(void)			{ return g_joy_x > 0.5f || KeyListDown(g_key_turn_right); }		// 0x465a60
bool neg::PlayerKeyCutsceneBypass(void)	{ return g_joy_button0 || KeyListDown(g_key_cutscene_bypass); }	// 0x465ad0

// 0x4a3990: cursor offset from the window centre, then the cursor is put back (as CEditor's).
static void GetMouseDelta(short *dx, short *dy)
{
	if (!g_56aed0) { NEG_TODO(0x4a3990, "GetMouseDelta"); (void)dx; (void)dy; }
}

// --------------------------------------------------------------------------- C3DJimmy hooks

__attribute__((weak)) void neg::JimmyJetpackOff(CGameObject *jimmy)
{
	(void)jimmy;
	NEG_TODO(0x443370, "C3DJimmy jetpack off");
}

__attribute__((weak)) void neg::JimmyJetpackSound(CGameObject *jimmy, int a)
{
	(void)jimmy; (void)a;
	NEG_TODO(0x4433f0, "C3DJimmy jetpack sound");
}

__attribute__((weak)) void neg::JimmyJetpackFlame(CGameObject *jimmy, float size)
{
	(void)jimmy; (void)size;
	NEG_TODO(0x4613a0, "C3DJimmy jetpack flame size");
}

__attribute__((weak)) void neg::JimmyActivateMenuItem(CGameObject *jimmy, CGameObject *o, CGameObject *who, int item, float value)
{
	(void)jimmy; (void)o; (void)who; (void)item; (void)value;
	NEG_TODO(0x449df0, "C3DJimmy::ActivateMenuItem (element slot 138)");
}

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// g_jimmy as a C3DPlayer (the original goes from the CGameObject part to the object by -0xc0).
static C3DPlayer *JimmyPlayer(void)
{
	return g_jimmy ? dynamic_cast<C3DPlayer *>(g_jimmy) : NULL;
}

static OMedia3DVector Vec(const OMedia3DPoint &p)
{
	return OMedia3DVector(p.x, p.y, p.z, p.w);
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// vtable +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0. (The
// generated declaration of v226 has the wrong signature, so it is done here.)
static OMedia3DVector ObjectDir(CGameObject *o, float x, float y, float z)
{
	float rot[4];
	o->v201(rot);											// vtable +0x324
	const omt_Angle ax = (omt_Angle)((int)(rot[0] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle ay = (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle az = (omt_Angle)((int)(rot[2] * kDegToAngle) & omc_MaxAngleMask);
	OMedia3DVector v(x, y, z, 0.0f);
	v.rotate(ax, ay, az);
	return v;
}

// The side slip of the movement states: (k, 0, 0) turned by our heading only.
static OMedia3DVector SideSlip(CGameObject *o, float k)
{
	float rot[4];
	o->v201(rot);											// vtable +0x324
	OMedia3DVector v(k, 0.0f, 0.0f, 0.0f);
	v.rotate(0, (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask), 0);
	return v;
}

// The game's random number generator (as ev105's): 0..1000.
static int Random1001(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return ((int)((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// The jump height: 550, more with the jump power-up (g_54a448 seconds left, 950 from 3 on).
static float JumpSpeed(bool *powered)
{
	float up = 550.0f;
	*powered = false;
	if (g_54a448 > 0)
	{
		*powered = true;
		if (g_54a448 < 3.0f) up = g_54a448 * 133.33333f + 550.0f;	// 0x4f1f5c
		else up = 950.0f;
	}
	return up;
}

// The low-gravity fall of states 1 and 5: acceleration y = gravity * 0.33.
static void LowGravity(CGameObject *o)
{
	float acc[4];
	o->v155(acc);											// vtable +0x26c
	o->v157(acc[0], g_gravity.y * 1.33f - g_gravity.y, acc[2]);	// 0x4f1f58
}

// --------------------------------------------------------------------------- construction

C3DPlayer::C3DPlayer()										// 0x45ff00
{
	// (f_9b8, f_a6c, f_a7c, f_aa0, f_ad4, f_bdc, f_c8c are OMedia3DPoints: w = 1; f_ab0 and f_af0
	// OMedia3DVectors: w = 0.)
	SetName((char *)"C3DPLAYER");
	Log("C3DPlayer()");
	f_960 = 30.0f;
	f_964 = 50.0f;
	f_968 = 600.0f;
	f_96c = 0;
	f_970 = 35.0f;
	f_974 = 0;
	f_978 = 0;
	f_97c = 0;
	f_980 = 0;
	v62(0.8f);
	v64(0.7f);
	v77(0, 0, 0);
	float t[4];
	v76(t);
	v171(t[0], t[1], t[2], t[3]);
	v38(1);
	v66(100.0f);
	v44(0);
	v50(0);
	f_984 = 1;
	v218(0, 0, 0);
	f_a6c.set(0, 200.0f, -300.0f);
	f_a7c.set(0, 80.0f, 0);
	f_af0.x = 1.4f;
	f_af0.y = 1.0f;
	f_af0.z = 1.4f;
	f_a90 = -2.0f;
	f_a94 = 2.0f;
	f_988 = 0;
	f_989 = 0;
	f_98a = 0;
	f_98b = 0;
	f_a99 = 1;
	f_9b4 = 0;
	f_594 = 0;
	f_a68 = 0;
	f_a98 = 0;
	f_a9c = 0;
	f_ac4 = 0;
	f_aa0.set(0, 0, 0);
	f_ac0 = 0;
	f_acc = 0;
	f_ad0 = 0;
	f_bf8 = 0;
	f_bfc = 0;
	f_c00 = 1.0f;
	f_c10 = 120.0f;
	f_b08 = 0;
	strcpy(f_b24, "none");
	strcpy(f_9c8, "none");
	strcpy(f_a18, "none");
	strcpy(f_b89, "level1b.gam");
	f_ac6 = 0;
	f_ac8 = 0;
	f_b20 = 0;
	f_ae4 = 0;
	f_ae8 = 10.0f;
	f_aec = 0;
	f_aed = 0;
	f_b00 = 0;
	f_b04 = 0;
	f_b0c = 0;
	f_b10 = 0;
	f_b1c = 8.0f;
	f_b88 = 0;
	f_bec = 0;
	f_c04 = 0;
	f_c08 = 0;
	strcpy(f_c25, "level1b.gam");
	f_bdc.set(0, 0, 0);
	f_bd9 = 0;
	f_bf0 = 0;
	f_bf4 = 0;
	f_c0c = 0;
	f_c14 = 0;
	f_c18 = 0;
	f_c1c = 0;
	f_c24 = 0;
	f_ca0 = 0;
	f_ca4 = 0;
	f_ca8 = 0;
	// port: not set by the original (left as the allocation had them)
	f_a8c = 0;
	f_b14 = 0;
	f_b18 = 0;
	f_c20 = NULL;
	f_c9c = 0;
}

C3DPlayer::~C3DPlayer()										// 0x4603d0
{
	Log("~C3DPlayer()");
}

// --------------------------------------------------------------------------- CGameObject family

void C3DPlayer::InitObject(void)							// 0x460470
{
	Log("C3DPlayer::InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("StartPoint", f_9c8, PROP_ALPHA, 0);
	SetObjectID('3PLA');									// element +0xc4
	InitPhysics();											// vtable +0x2c
}

void C3DPlayer::ResetObject(void)							// 0x4604d0
{
	C3DPlayerObject::ResetObject();							// CLocalGameObject's (0x48ee80)
	f_96c = 0;
	f_978 = 0;
	f_97c = 0;
	float r[4];
	v201(r);
	f_980 = r[1];
}

void C3DPlayer::v258(void)									// 0x435ea0
{
	CLocalGameObject::v258();
}

void C3DPlayer::v260(void)									// 0x4637d0
{
	C3DPlayerObject::v260();
	f_c20 = FindObjectByTag("C3DHERMAN");
}

void C3DPlayer::v261(void)									// 0x4637f0
{
	f_96c = 0;
	v176(0, 0, 0);
	ev116();
}

void C3DPlayer::v262(CGameObject *other, float dist)		// 0x463830
{
	if (!other->NameContains((char *)"C3DGODDARD"))			// vtable +0x18
	{
		CGameObject::v262(other, dist);
		return;
	}
	if (g_camera != (CGameObject *)this) v176(0, 0, 0);
}

void C3DPlayer::v271(void)									// 0x461b20
{
	if (g_power_timers[0] <= 0) f_968 = 600.0f;
}

void C3DPlayer::RotateToDest(float dt)						// 0x462f90
{
	if (!GameIsRunning() || !f_005 || !f_00a) return;		// 0x4b0ca0
	f_00a = 0;
	float dx = 0, dy = 0, dz = 0;
	if (f_274 != f_224)
	{
		dx = f_274 - f_224;
		if (dx > 180.0f) dx -= 360.0f;
		else if (dx < -180.0f) dx += 360.0f;
		f_00a = 1;
	}
	if (f_278 != f_228)
	{
		dy = f_278 - f_228;
		if (dy > 180.0f) dy -= 360.0f;
		else if (dy < -180.0f) dy += 360.0f;
		f_00a = 1;
	}
	if (f_27c != f_22c)
	{
		dz = f_27c - f_22c;
		if (dz > 180.0f) dz -= 360.0f;
		else if (dz < -180.0f) dz += 360.0f;
		f_00a = 1;
	}
	AddRotation((float)(dx * dt * 4.5), (float)(dy * dt * 3.7), (float)(dz * dt * 4.5));	// vtable +0x330
}

void C3DPlayer::Process(float dt)							// 0x460510
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (g_5461ce)
	{
		v176(0, 0, 0);
		ev80();
		return;
	}
	f_ad0 += dt;
	C3DAnimated::Process(dt);
	f_c24 = v133();
	if (g_camera != (CGameObject *)this || g_5461ce) return;
	switch (f_a68)
	{
		case 0: case 1: case 4: case 5:
		if (f_c04 <= 0)
		{
			if (PlayerKeyJump())
			{
				if (f_a68 != 4) f_a68 = 1;
			}
			else if (v133())
			{
				if (g_camera == (CGameObject *)this)
				{
					if (f_96c > 10.0f)
					{
						if (f_a68 == 1) JimmyJetpackOff(g_jimmy);	// 0x443370
						if (f_c04 <= 0 && !ev77((char *)"RUN")) ev114((char *)"RUN");
					}
					else
					{
						if (f_a68 == 1) JimmyJetpackOff(g_jimmy);
						if (f_ac6 == 0)
						{
							if (PlayerKeyAction() && !PlayerKeyJump())
							{
								if (g_menu && !g_menu->IsCurrentGadget(4)) ev116();
							}
							else if (!ev77((char *)"STOP")) ev114((char *)"STOP");
						}
						else if (f_ac6 == -1)
						{
							if (f_c04 <= 0 && !PlayerKeyAction() && !PlayerKeyBack() &&
								!ev77((char *)"BACK") && !ev77((char *)"LEFT"))
								ev58((char *)"LEFT", 1);
						}
						else
						{
							if (f_c04 <= 0 && !PlayerKeyAction() && !PlayerKeyBack() &&
								!ev77((char *)"BACK") && !ev77((char *)"RIGHT"))
								ev58((char *)"RIGHT", 1);
						}
					}
				}
				f_a8c = 0;
				f_a68 = 0;
				f_978 = 0;
			}
		}
		if (f_a68 == 1) ev112(dt);
		else if (f_a68 == 5) ev106(dt);
		else
		{
			if (g_camera && f_96c < 10.0f && f_c04 <= 0 && ev77((char *)"RUN")) ev114((char *)"STOP");
			ev108(dt);
		}
		break;

		case 2:
		ev107(dt);
		break;
	}
	v154(f_96c);											// vtable +0x268
}

void C3DPlayer::v243(float dt)								// 0x4618d0
{
	if (!g_camera) return;
	if (f_b20 > 0)
	{
		// climbing: the camera rises with the climb target f_c8c, the player is put there
		short mx, my;
		GetMouseDelta(&mx, &my);							// (not used)
		f_c8c.y = dt * f_c10 + f_c8c.y;
		OMedia3DVector cam(g_viewport->getx(), (float)(f_c10 * 0.25f * dt + g_viewport->gety()), g_viewport->getz(), 1.0f);
		v196(f_c8c.x, f_c8c.y, f_c8c.z, f_c8c.w);			// vtable +0x310
		ev97(cam, Vec(f_c8c));								// element +0x184
		f_b20 -= dt;
		if (f_b20 <= 0)
		{
			v136(1);
			f_920 = 0;
			f_b20 = 0;
			g_5461ce = 0;
			v196(f_ad4.x, f_ad4.y, f_ad4.z, f_ad4.w);
			ev58((char *)"STOP", 0);
			f_988 = 0;
			f_96c = 0;
			v42(1);
			v176(0, 0, 0);
			f_8a5 = 0;
			f_8a4 = 1;
		}
		return;
	}
	switch (f_a68)
	{
		case 0: case 4: case 5: ev109(dt); break;			// element +0x1b4
		case 1: case 2: ev110(dt); break;					// +0x1b8
		case 3: ev111(dt); break;							// +0x1bc
	}
}

// Wall contact (CGameObject slot 220, the physics passes the polygon's plane). The wall's
// material chunk ID: 120..129 -> ev119, 110..119 -> ev120; in state 0, 60..69 is a fence and
// 180 a ladder to climb.
void C3DPlayer::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x463c20
{
	int id = -1;
	if (f_ad0 <= 0.5f) return;
	if (poly && poly->get_material())
	{
		id = (int)poly->get_material()->get_chunk_ID();		// material +0xc -> +8 (0 without one)
		if (id >= 0x78 && id <= 0x81) ev119(OMedia3DVector(a, b, c, d));	// element +0x1dc
		if (id >= 0x6e && id <= 0x77) ev120();				// +0x1e0
	}
	if (f_a68 != 0) return;
	if (!((id >= 0x3c && id <= 0x45) || id == 0xb4)) return;
	omt_Angle ax, ay;
	if (id == 0xb4)
	{
		// ladder: on the floor, the ladder in front at knee height
		if (!f_c24) return;
		OMedia3DVector p = Position(this);
		if (!PhysicsLineBlocked(OMedia3DVector(p.x, p.y + 50.0f, p.z, p.w), v224(0, 50.0f, 200.0f), false)) return;
		if (strcasecmp(ev66(), "LADDER") == 0) return;
		OMedia3DVector n(-a, -b, -c, d);
		n.angles(ax, ay);
		const float ry = (float)ay * kAngleToDeg;
		f_278 = ry;
		v203(0, ry, 0);
		OMedia3DVector q = v224(0, 0, 25.0f);
		v196(q.x, q.y, q.z, q.w);
		OMedia3DVector at = Position(this);
		f_c8c.set(at.x, at.y, at.z, at.w);
		f_920 = 1;
		ev114((char *)"LADDER");
		return;
	}
	// fence: nothing above it, both sides of us against it, on the floor
	OMedia3DVector p = Position(this);
	if (PhysicsLineBlocked(OMedia3DVector(p.x, p.y + 320.0f, p.z, p.w), v224(0, 320.0f, 300.0f), false)) return;
	p = Position(this);
	if (!PhysicsLineBlocked(OMedia3DVector(p.x + 10.0f, p.y + 50.0f, p.z, p.w), v224(10.0f, 50.0f, 300.0f), false)) return;
	p = Position(this);
	if (!PhysicsLineBlocked(OMedia3DVector(p.x - 10.0f, p.y + 50.0f, p.z, p.w), v224(-10.0f, 50.0f, 300.0f), false)) return;
	if (!f_c24) return;
	OMedia3DVector n(-a, -b, -c, d);
	n.angles(ax, ay);
	f_278 = (float)ay * kAngleToDeg;
	float r[4];
	v201(r);
	v203(r[0], f_278, r[2]);
	OMedia3DVector q = v224(0, 0, 25.0f);
	v196(q.x, q.y, q.z, q.w);
	OMedia3DVector at = Position(this);
	f_c8c.set(at.x, at.y, at.z, at.w);
	at = Position(this);
	f_c8c.set(at.x, at.y, at.z, at.w);
	f_920 = 1;
	ev114((char *)"FENCE");
}

// --------------------------------------------------------------------------- animations

void C3DPlayer::ev67(void)									// 0x4639b0 "AnimEnded"
{
	if (strcasecmp(ev66(), "FENCE") == 0)
	{
		ev78((char *)"STOP");
		ev86((char *)"FULLSTOP", NULL, NULL, NULL, 0, 0.0f);
		f_8a4 = 1;
		f_8a7 = 1;
		f_b20 = 0;
		v196(f_ad4.x, f_ad4.y, f_ad4.z, f_ad4.w);			// on top of the fence
		f_988 = 0;
		f_96c = 0;
		v42(1);
		v176(0, 0, 0);
	}
	else if (strcasecmp(ev66(), "LADDER") == 0)
	{
		ev78((char *)"STOP");
		ev86((char *)"FULLSTOP", NULL, NULL, NULL, 0, 0.0f);
		f_8a5 = 1;
		f_8a4 = 0;
		f_8a7 = 1;
		f_b20 = 0;
		v261();
		f_988 = 0;
		f_96c = 0;
		v196(f_ad4.x, f_ad4.y, f_ad4.z, f_ad4.w);
		v42(1);
		v136(1);
	}
	else if (strcasecmp(ev66(), "FULLSTOP") == 0)
	{
		if (g_5461ce)
		{
			f_8a5 = 0;
			f_8a4 = 1;
			f_920 = 0;
		}
		f_8a7 = 0;
		g_5461ce = 0;
		v42(1);
	}
	else if (strcasecmp(ev66(), "SPLAT") == 0) ev58((char *)"STOP", 0);
	else if (strcasecmp(ev66(), "HIT") == 0)
	{
		ev58((char *)"STOP", 0);
		g_5461ce = 0;
		f_b20 = 0;
	}
}

void C3DPlayer::ev114(char *name)							// 0x464260
{
	if (g_5461ce || f_8a7) return;
	if (strcasecmp(name, ev66()) == 0) return;
	// these play to their end
	if (strcasecmp(ev66(), "HIT") == 0 || strcasecmp(ev66(), "SPLAT") == 0 ||
		strcasecmp(ev66(), "FENCE") == 0 || strcasecmp(ev66(), "LADDER") == 0 ||
		strcasecmp(ev66(), "SWIM") == 0 || strcasecmp(ev66(), "FLOAT") == 0 ||
		strcasecmp(ev66(), "SWING") == 0) return;
	if (strcasecmp(name, "RUN") == 0)
	{
		ev115();
		ev58((char *)"RUN", 1);
		v157(0, 0, 0);
		return;
	}
	if (strcasecmp(ev66(), "EDGE") != 0 && strcasecmp(name, "EDGE") == 0)
	{
		ev116();
		ev58((char *)"EDGE", 1);
		return;
	}
	if (strcasecmp(ev66(), "JUMP") != 0 && strcasecmp(name, "JUMP") == 0)
	{
		ev58((char *)"JUMP", 0);
		return;
	}
	if (strcasecmp(name, "SWING") == 0)
	{
		ev116();
		ev58((char *)"SWING", 1);
		return;
	}
	if (f_96c < 0) return;									// going backwards: BACK stays
	if (strcasecmp(name, "STOP") == 0 && strcasecmp(ev66(), "SPLAT") != 0)
	{
		ev116();
		ev58((char *)"STOP", 1);
		v157(0, 0, 0);
		return;
	}
	if (strcasecmp(name, "FALL") == 0)
	{
		ev58((char *)"FALL", 1);
		ev116();
		return;
	}
	if (strcasecmp(name, "LEFT") == 0)
	{
		ev116();
		ev58((char *)"LEFT", 1);
		v157(0, 0, 0);
		return;
	}
	if (strcasecmp(name, "RIGHT") == 0)
	{
		ev116();
		ev58((char *)"RIGHT", 1);
		v157(0, 0, 0);
		return;
	}
	if (strcasecmp(name, "HIT") == 0)
	{
		ev58((char *)"HIT", 0);
		return;
	}
	if (strcasecmp(name, "FENCE") == 0)
	{
		OMedia3DVector top = v224(0, 260.0f, 70.0f);
		f_ad4.set(top.x, top.y, top.z, top.w);
		v176(0, 0, 0);
		v42(0);
		ev116();
		g_5461ce = 1;
		f_b20 = 1.5f;
		f_b04 = 1.55f;
		f_ad0 = 0;
		f_c10 = 150.0f;
		// (debug log "Playing animation FENCE! Damnit", empty)
		ev78((char *)"FENCE");
		ev86((char *)"FENCE", NULL, NULL, NULL, 0, 0.0f);
		f_8a7 = 1;
		PlayGameSound(-1, std::string("umph climb"), false);	// 0x48f420
		return;
	}
	if (strcasecmp(name, "LADDER") == 0)
	{
		OMedia3DVector top = v224(0, 630.0f, 75.0f);
		f_ad4.set(top.x, top.y, top.z, top.w);
		v176(0, 0, 0);
		v42(0);
		ev116();
		g_5461ce = 1;
		f_b20 = 2.2f;
		f_b04 = 2.0f;
		f_ad0 = 0;
		f_c10 = 350.0f;
		v136(0);
		ev58((char *)"LADDER", 0);
		f_8a7 = 1;
		PlayGameSound(-1, std::string("xylophone"), false);
		return;
	}
	ev58(name, 1);
}

// --------------------------------------------------------------------------- movement

void C3DPlayer::ev106(float dt)								// 0x460940
{
	OMedia3DVector off(0, 0, 0);							// (y never set)
	f_a90 = -2.0f;
	f_a94 = 2.0f;
	float vel[4];
	v174(vel);												// vtable +0x2b8
	if (sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]) != 0)
	{
		f_97c = f_978;
		AddRotation(0, f_978 * dt * 2.0f, 0);
		const float k = f_97c * 0.018181818f * (1.0f - v145()) * f_96c;	// 0x4dce30: 1/55
		off = SideSlip(this, k);
	}
	OMedia3DVector ahead = ObjectDir(this, 0, 0, f_96c);	// vtable +0x388
	v176(off.x + ahead.x, vel[1], off.z + ahead.z);
	float t[4];
	v195(t);												// (read, not used)
	v174(t);
	C3DPlayer *jim = JimmyPlayer();
	if (PlayerKeyJump())
	{
		if (v133() && f_c04 <= 0 && jim)					// port: jim tested (NULL is read through in the original)
		{
			if (jim->f_a8c == 0)
			{
				f_96c = f_968 * 0.75f;
				bool powered;
				const float up = JumpSpeed(&powered);
				jim->ev114((char *)"JUMP");
				jim->ev116();
				v162(0, up, 0);									// vtable +0x288
				jim->f_a8c = 1;
			}
			else
			{
				jim->f_a68 = 0;
				v261();
			}
		}
	}
	else if (jim && jim->f_a8c == 1) jim->f_a8c = 0;
	if (v112()) LowGravity(this);
}

void C3DPlayer::ev107(float dt)								// 0x461640
{
	OMedia3DVector off(0, 0, 0);
	f_a90 = -10.0f;
	f_a94 = 10.0f;
	float vel[4];
	v174(vel);
	if (sqrtf(vel[0] * vel[0] + vel[2] * vel[2] + vel[1] * vel[1]) != 0)
	{
		f_97c = f_978;
		AddRotation(0, f_978 * dt, 0);
		const float k = f_97c * 0.018181818f * (1.0f - v145()) * f_96c;	// 1/55
		off = SideSlip(this, k);
	}
	OMedia3DVector ahead = ObjectDir(this, 0, 0, f_96c);
	v176(off.x + ahead.x, vel[1], off.z + ahead.z);
	float t[4];
	v195(t);												// (read, not used)
	v174(t);
	if (!v133()) return;
	ev118();
	if (PlayerKeyJump())
	{
		v174(t);
		v176(t[0], 900.0f, t[2]);
		PlayGameSound(-1, std::string("bubble boing"), false);
		return;
	}
	f_96c *= 0.5f;
	v174(t);
	v176(t[0], 300.0f, t[2]);
}

void C3DPlayer::ev108(float dt)								// 0x460ce0
{
	OMedia3DVector off(0, 0, 0);
	float vel[4];
	v174(vel);
	if (g_5461ce) return;
	if (sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]) != 0 || f_b0c != 0)
	{
		f_97c = f_978;
		AddRotation(0, f_978 * dt, 0);
		const float k = (f_97c * 0.022222222f * (1.0f - v145()) * f_96c + f_b0c) * dt;	// 0x4df740: 1/45
		off = SideSlip(this, k);
	}
	OMedia3DVector ahead = ObjectDir(this, 0, 0, f_96c);
	v176(off.x + ahead.x, vel[1], off.z + ahead.z);
	if (!v133() || !v205()) return;							// on a floor polygon (vtable +0x334)
	// (the original works out the floor's tilt here - v224(0, 0, 1), the polygon's normal,
	// OMedia3DVector::angles and 0x4b1fc0 - into values it never reads; left out)
	float t[4];
	v174(t);
	v176(t[0], t[1] - 250.0f, t[2]);						// pressed onto the floor
}

void C3DPlayer::ev112(float dt)								// 0x461010
{
	OMedia3DVector off(0, 0, 0);
	f_a90 = -2.0f;
	f_a94 = 2.0f;
	float vel[4];
	v174(vel);
	if (sqrtf(vel[0] * vel[0] + vel[2] * vel[2] + vel[1] * vel[1]) != 0)
	{
		f_97c = f_978;
		AddRotation(0, f_978 * dt * 2.0f, 0);
		const float k = f_97c * 0.018181818f * (1.0f - v145()) * f_96c;	// 1/55
		off = SideSlip(this, k);
	}
	OMedia3DVector ahead = ObjectDir(this, 0, 0, f_96c);
	v176(off.x + ahead.x, vel[1], off.z + ahead.z);
	float t[4];
	v195(t);												// (read, not used)
	v174(t);
	if (!g_menu) return;
	if (!g_menu->IsCurrentGadget(3) && !g_flag_545e54)
	{
		// a plain jump
		if (PlayerKeyJump())
		{
			if (v133())
			{
				if (f_a8c == 0)
				{
					f_96c = f_968 * 0.75f;
					bool powered;
					const float up = JumpSpeed(&powered);
					if (powered) PlayGameSound(-1, std::string("boing"), false);
					ev114((char *)"JUMP");
					ev116();
					v162(0, up, 0);
					f_a8c = 1;
				}
				else
				{
					f_a68 = 0;
					v261();
				}
			}
		}
		else if (f_a8c == 1) f_a8c = 0;
		if (v112()) LowGravity(this);
		return;
	}
	// the jetpack (gadget 3 selected, or g_flag_545e54)
	if (PlayerKeyJump())
	{
		if (f_a8c == 0)
		{
			if (v133())
			{
				ev116();
				JimmyJetpackFlame(g_jimmy, 10.0f);
				ev58((char *)"FLY", 1);
				v162(0, 100.0f, 0);
				f_a8c = 1;
				f_b10 = 0;
				f_96c = 700.0f;
				f_968 = 1200.0f;
			}
			else
			{
				f_96c = 1200.0f;
				if (!ev77((char *)"FLY")) ev58((char *)"FLY", 1);
				v174(t);
				if (t[1] < 380.0f) v178(0, 80.0f, 0);			// vtable +0x2c8
			}
		}
		JimmyJetpackSound(g_jimmy, 1);
		OMedia3DVector p = Position(this);
		float acc[4];
		if (p.y < 1700.0f)
		{
			v155(acc);
			v157(acc[0], (float)(-(double)g_gravity.y - g_gravity.y * 0.03), acc[2]);	// up
		}
		else
		{
			v155(acc);
			v157(acc[0], (float)(g_gravity.y * 0.1 - g_gravity.y), acc[2]);	// ceiling: hover
			v174(t);
			v178(0, -t[1], 0);
		}
		// (debug text of the height and f_96c, empty)
		return;
	}
	JimmyJetpackOff(g_jimmy);
	float acc[4];
	v155(acc);
	v157(acc[0], 0, acc[2]);
	if (f_a8c == 1 || f_a8c == 5) f_a8c = 0;
}

OMedia3DVector C3DPlayer::ev113(float x, float y, float z)	// 0x435d20 (also C3DFlyingObject's)
{
	float r[4];
	v201(r);
	float a = r[1] + f_978;
	if (a < 0) a = (float)(360 - abs((int)a));
	else if (a > 360.0f) a -= 360.0f;
	const omt_Angle az = (omt_Angle)((int)(r[2] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle ay = (omt_Angle)((int)(a * kDegToAngle) & omc_MaxAngleMask);
	OMedia3DVector v(x, y, z, 0.0f);
	v.rotate(0, ay, az);
	OMedia3DVector p = Position(this);
	return OMedia3DVector(p.x + v.x, p.y + v.y, p.z + v.z, 1.0f);
}

unsigned char C3DPlayer::ev117(void)						// 0x4638a0
{
	if (f_a68 != 0) return 0;
	OMedia3DVector a = v224(0, 0, 70.0f);
	OMedia3DVector b(a.x, a.y - 2000.0f, a.z, a.w);
	if (!PhysicsClosestHit(a, b, false)) return 0;			// 0x4b7640
	return abs((int)(g_phys_ray.end.y - a.y)) > 70 ? 1 : 0;	// 0x6311fc
}

// --------------------------------------------------------------------------- control + camera

void C3DPlayer::ev109(float dt)								// 0x461b50
{
	short mdx = 0, mdy = 0;
	GetMouseDelta(&mdx, &mdy);
	if (f_c04 > 0) mdx = (short)-mdx;						// dazed: mouse reversed
	if (g_5461ce) return;
	if (g_power_timers[0] > 0) f_968 = 1000.0f;
	f_ae4 = (float)mdx + f_ae4;
	// camera height: Action + Go / Back / mouse y, with gadget 6 or 10
	if (PlayerKeyAction() && !PlayerKeyJump())
	{
		if (g_menu && (g_menu->IsCurrentGadget(6) || g_menu->IsCurrentGadget(10)))
		{
			if (PlayerKeyBack()) f_ae8 = f_ae8 - dt * 20.0f;
			else if (PlayerKeyGo()) f_ae8 = dt * 20.0f + f_ae8;
			else f_ae8 = (float)mdy * dt + f_ae8;
		}
	}
	else if (g_menu && !g_menu->IsCurrentGadget(6) && !g_menu->IsCurrentGadget(10)) f_ae8 = 0;

	// (the position is read here and not used)
	f_988 = 0;
	float k = 1.0f;
	if (!v133())
	{
		if (f_a68 == 4) { if (ev77((char *)"FALL")) ev58((char *)"SWIM", 1); }
		else if (!f_aed) ev114((char *)"FALL");
	}
	f_9b8.x = g_viewport->getx();
	f_9b8.y = g_viewport->gety();
	f_9b8.z = g_viewport->getz();
	// (the camera's forward direction, 0x4b1ee0 + angles, is worked out here and not used)

	// turning
	int turning = 0;
	f_ac6 = 0;
	f_aec = 0;
	if (PlayerKeyBack() && g_menu && !f_b00) f_aec = 1;
	float T;
	if (f_c9c) T = (float)(mdx * 0.5);
	else if (f_c04 > 0) T = (float)((1.05 - (double)f_96c / f_968) * mdx * 2.0);
	else T = (float)(((1.0 - (double)f_96c / f_968) + 0.1f) * mdx);
	if (T > 1.0f) { turning = 1; f_ac6 = -1; f_b14 = 0.1f; }
	if (T < -1.0f) { turning = 1; f_ac6 = 1; f_b18 = 0.1f; }
	if (T > 45.0f) { turning = 1; T = 45.0f; f_ac6 = -1; f_b14 = 0.5f; }
	else if (T < -45.0f) { turning = 1; T = -45.0f; f_ac6 = 1; f_b18 = 0.5f; }
	if (f_ac6 == 0)
	{
		// a turn animation keeps going a little
		if (f_b18 > 0) f_b18 -= dt;
		if (f_b18 > 0) f_ac6 = 1;
		else f_b18 = 0.1f;
		if (f_b14 > 0) f_b14 -= dt;
		if (f_b14 > 0) f_ac6 = -1;
		else f_b14 = 0.1f;
	}
	bool mleft = false, mright = false;
	f_b0c = 0;
	if (f_c08 > 0)
	{
		if (T > 1.0f) mleft = true;
		if (T < -1.0f) mright = true;
		T *= 0.33333334f;									// 0x4dc054
	}
	if (PlayerKeyLeft() || mleft)
	{
		f_bf4 += dt;
		f_bf0 = 0;
		f_b0c = -250.0f;
		T = T + (f_c9c ? (f_bf4 * 14.0f + 1.0f) * 2.0f : (f_bf4 * 10.0f + 1.0f) * 4.0f);
		if (T > 45.0f) T = 45.0f;
		f_ac6 = -1;
		turning = 1;
		f_98a = 1;
		if (f_96c < f_968)
		{
			f_96c = f_960 + f_96c;
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	else if (PlayerKeyRight() || mright)
	{
		f_bf0 += dt;
		f_bf4 = 0;
		f_b0c = 250.0f;
		T = T - (f_c9c ? (f_bf0 * 14.0f + 1.0f) * 2.0f : (f_bf0 * 10.0f + 1.0f) * 4.0f);
		if (T < -45.0f) T = -45.0f;
		turning = 1;
		f_ac6 = 1;
		f_98a = 1;
		if (f_96c < f_968)
		{
			f_96c = f_960 + f_96c;
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	else
	{
		f_bf0 = 0;
		f_bf4 = 0;
	}

	// going forward (Go, or the left mouse button unless f_c9c)
	const bool mouse_go = !f_c9c && g_mouse_down;
	int moved;
	if ((PlayerKeyGo() || mouse_go) && f_c04 <= 0)
	{
		if (f_96c > f_968) f_96c = f_968;
		f_988 = 1;
		moved = 1;
		if (T != 0 && !PlayerKeyLeft() && !PlayerKeyRight())
		{
			T *= 5.0f;
			if (T < -75.0f) T = -75.0f;
			else if (T > 75.0f) T = 75.0f;
		}
		if (f_96c < f_968)
		{
			f_96c = f_960 + f_96c;
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	else moved = turning;
	if (f_aec)
	{
		if (f_c04 <= 0 && !ev77((char *)"BACK")) ev58((char *)"BACK", 1);
		f_988 = 1;
		f_96c = (float)(f_96c - f_964 * 1.5);
		const float lim = f_968 * -0.5f;
		if (f_96c < lim) f_96c = lim;
		moved = 1;
	}
	if (f_ca4 <= 0 && !f_988)
	{
		// nothing pressed: brake to a stop
		if (f_96c > 0)
		{
			f_96c = f_96c - f_964;
			if (f_96c < 0) f_96c = 0;
		}
		if (f_96c < 0)
		{
			f_96c = f_964 + f_96c;
			if (!(f_96c < 0)) f_96c = 0;
		}
	}
	if (g_menu && (g_menu->IsCurrentGadget(6) || g_menu->IsCurrentGadget(10)) && f_c9c && PlayerKeyAction())
		f_96c = 0;
	if (f_96c < 0) ev114((char *)"BACK");
	bool turn = moved != 0;
	if (!f_aec && mdy >= 2)
	{
		f_96c = (float)(mdy * 16) + f_96c;					// mouse forward
		if (f_96c > f_968) f_96c = f_968;
		turn = true;
	}
	if (turn)
	{
		float r[4];
		v201(r);
		v218(0, T + r[1], 0);
	}

	// the chase camera
	k = 1.0f;
	OMedia3DVector axis(1.2f, 1.2f, 1.2f, f_af0.w);
	float lift = 0;
	if (f_c20)
	{
		OMedia3DVector h = Position(f_c20), p = Position(this);
		const float dx = h.x - p.x, dy = h.y - p.y, dz = h.z - p.z;
		if (sqrtf(dz * dz + dy * dy + dx * dx) < 900.0f) lift = 300.0f;	// Herman near: higher
	}
	OMedia3DVector want(0, 0, 0), look(0, 0, 0);			// port: set (only f_c1c 0..3 sets them)
	switch (f_c1c)
	{
		case 0: case 1:
		want = v224(f_a6c.x, f_a6c.y - f_ae8 * 3.0f + lift, f_a6c.z);
		look = v224(f_a7c.x, (float)(f_ae8 * 3.8 + f_a7c.y), f_a7c.z + 40.0f);
		if (f_c1c == 0)
		{
			float maxd = 480.0f;
			if (f_c04 > 0) maxd = 780.0f;
			k = ev96(Position(this), &want, maxd);
		}
		else axis.x = axis.y = axis.z = 1.7f;
		break;

		case 2: case 3:
		want = v224(f_a6c.x, f_a6c.y - f_ae8 * 3.0f + lift + 100.0f, f_a6c.z);
		look = v224(f_a7c.x, (float)(f_ae8 * 3.8 + f_a7c.y + 100.0), f_a7c.z + 40.0f);
		break;
	}
	float k2 = ev95(look, &want);
	OMedia3DVector p = Position(this);
	const float dx = p.x - f_9b8.x, dy = p.y - f_9b8.y, dz = p.z - f_9b8.z;
	const float dist = sqrtf(dz * dz + dy * dy + dx * dx);
	f_ca8 = 0;
	ShowAgain();
	if ((f_c1c == 0 || f_c1c == 1) && dist < 150.0f && k2 > 1.0f && k2 < 3.0f)
	{
		// the camera got pushed right up to us by a wall: look from Jimmy's head
		HideAgain();
		f_ca8 = 1;
		want = v224(f_a6c.x, (f_a7c.y - 10.0f) + f_ae8 * 1.5f, f_a7c.z - 30.0f);
		k2 = ev95(look, &want);
		axis.x = 2.8f;
		axis.y = 2.5f;
		axis.z = 2.8f;
	}
	ev98(dt, k2, k, Vec(f_9b8), want, look, axis);
}

void C3DPlayer::ev110(float dt)								// 0x463140
{
	short mdx = 0, mdy = 0;
	GetMouseDelta(&mdx, &mdy);
	float T = (float)mdx;
	float rate = 140.0f;
	if (!g_menu->IsCurrentGadget(3)) rate = 40.0f;
	if (T == 0)
	{
		if (PlayerKeyLeft())
		{
			T = rate * dt + T;
			f_98a = 1;
		}
		else if (PlayerKeyRight())
		{
			T = T - rate * dt;
			f_98a = 1;
		}
		else if (f_98a)
		{
			// keys let go: the turn eases back to 0
			if (f_978 < 0)
			{
				f_978 = rate * dt + f_978;
				if (f_978 > 0) f_978 = 0;
			}
			else if (f_978 > 0)
			{
				f_978 = f_978 - rate * dt;
				if (f_978 < 0) f_978 = 0;
			}
		}
	}
	else
	{
		T = T * dt * (g_menu->IsCurrentGadget(3) ? 7.0f : 0.6f);
		f_98a = 0;
	}
	f_978 = T + f_978;
	f_988 = 0;
	f_989 = 0;
	f_c14 = (float)((1.0f / dt) * (double)mdy);
	if (PlayerKeyGo() || f_c14 > 40.0f || (f_a68 == 2 && PlayerKeyAction()))
	{
		f_988 = 1;
		if (f_96c < f_968)
		{
			if (PlayerKeyGo() || (f_a68 == 2 && PlayerKeyAction())) f_96c = f_960 + f_96c;
			else f_96c = f_96c + f_c14;
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	if (!f_988 && f_968 * 0.90909094f < f_96c) f_96c = f_96c - f_964 * 0.5f;	// 0x4f1fa0
	if (v133())
	{
		if (f_96c > 0) f_96c = f_96c - f_964 * 0.5f;
		else f_96c = 0;
	}
	if (PlayerKeyBack() || f_c14 < -40.0f)
	{
		f_989 = 1;
		if (f_96c > 0)
		{
			f_96c = f_96c - f_964;							// (the original tests v133 here, both ways the same)
			if (f_96c < 0) f_96c = 0;
		}
		else if (f_96c < 0)
		{
			f_96c = f_964 + f_96c;
			if (f_96c > 0) f_96c = 0;
		}
	}
	float r[4];
	if (f_a68 == 2)
	{
		if (f_978 < -50.0f) f_978 = -50.0f;
		else if (f_978 > 50.0f) f_978 = 50.0f;
		if (!f_988)
		{
			const float roll = f_978 * -0.25f;
			v201(r);
			v218(f_989 ? f_a94 : 0.0f, r[1], roll);
		}
	}
	else
	{
		if (f_978 < -45.0f) f_978 = -45.0f;
		else if (f_978 > 45.0f) f_978 = 45.0f;
		if (f_988)
		{
			v201(r);
			v218(f_a90, f_978 * dt + r[1], -f_978);
		}
		else
		{
			v201(r);
			v218(f_989 ? f_a94 : 0.0f, r[1], -f_978);
		}
	}
	// camera
	OMedia3DVector cur(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);
	OMedia3DVector want = ev113(f_a6c.x, f_a6c.y, f_a6c.z);
	float k = 1.0f;
	if (f_c1c == 0) k = ev96(Position(this), &want, 480.0f);
	OMedia3DVector look = v224(f_a7c.x, f_a7c.y, f_a7c.z);
	const float k2 = ev95(look, &want);
	ShowAgain();
	ev98(dt, k2, k, cur, want, look, OMedia3DVector(1.2f, 2.0f, 1.2f, 0.0f));
}

void C3DPlayer::ev111(float dt)								// 0x462b70
{
	short mdx = 0, mdy = 0;
	GetMouseDelta(&mdx, &mdy);
	float T = (float)mdx;
	if (T == 0)
	{
		if (PlayerKeyLeft())
		{
			T = dt * 45.0f + T;
			f_98a = 1;
		}
		else if (PlayerKeyRight())
		{
			T = T - dt * 45.0f;
			f_98a = 1;
		}
		else if (f_98a)
		{
			if (f_978 < 0)
			{
				f_978 = dt * 45.0f + f_978;
				if (f_978 > 0) f_978 = 0;
			}
			else if (f_978 > 0)
			{
				f_978 = f_978 - dt * 45.0f;
				if (f_978 < 0) f_978 = 0;
			}
		}
	}
	else
	{
		T = T * dt;
		f_98a = 0;
	}
	f_978 = T + f_978;
	OMedia3DVector cur(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);
	OMedia3DVector want = ev113(f_a6c.x, f_a6c.y, f_a6c.z);
	OMedia3DVector look = v224(f_a7c.x, f_a7c.y, f_a7c.z);
	const float k2 = ev95(look, &want);
	ev98(dt, k2, 1.0f, cur, want, look, OMedia3DVector(1.2f, 1.2f, 1.2f, 0.0f));
}

void C3DPlayer::ev122(void)									// 0x460380
{
	f_a6c.x = 0;
	f_a6c.y = 200.0f;
	f_a6c.z = -300.0f;
	f_a7c.x = 0;
	f_a7c.y = 80.0f;
	f_a7c.z = 0;
	f_af0.x = 1.4f;
	f_af0.y = 1.0f;
	f_af0.z = 1.4f;
}

void C3DPlayer::ev123(float k, OMedia3DVector off, OMedia3DVector look, float dt)	// 0x462de0
{
	const float cx = g_viewport->getx(), cy = g_viewport->gety(), cz = g_viewport->getz();
	OMedia3DVector want = ev113(off.x, off.y, off.z);
	OMedia3DVector at = v224(look.x, look.y, look.z);
	const float k2 = ev95(at, &want);
	const float dx = want.x - cx, dy = want.y - cy, dz = want.z - cz;
	if (!f_aed)
	{
		g_viewport->setx(dx * k2 * k * dt + g_viewport->getx());
		g_viewport->sety(dy * k2 * k * dt + g_viewport->gety());
		g_viewport->setz(dz * k2 * k * dt + g_viewport->getz());
	}
	OMedia3DVector d(at.x - cx, at.y - cy, at.z - cz, 0.0f);
	omt_Angle ax, ay;
	d.angles(ax, ay);
	const short vx = g_viewport->get_anglex(), vy = g_viewport->get_angley();
	short turn = (short)((ay - vy) & 0x3fff);				// the short way round
	if (turn > 0x2000) turn -= 0x4000;
	g_viewport->set_anglex((omt_WAngle)(vx + (short)(ax - vx)));
	g_viewport->set_angley((omt_WAngle)(vy + turn));
}

void C3DPlayer::ev124(float dt)								// 0x4629c0
{
	if (f_bf8 <= 0) return;
	f_bf8 -= dt;
	f_bfc -= dt;
	if (f_bfc >= 0) return;
	const int r1 = Random1001();
	const int r2 = Random1001();
	OMedia3DVector v((float)r1 * 0.001f - 0.5f, (float)r2 * 0.001f - 0.5f, 0.0f, 0.0f);
	const float inv = 1.0f / v.quick_magnitude();
	f_bfc = 0.1f;
	v.x *= inv;
	v.y *= inv;
	v.z *= inv;
	float t = f_bf8;
	if (t < 0) t = 0;
	if (!f_aed)
	{
		g_viewport->setx(v.x * f_c00 * t * 20.0f + g_viewport->getx());
		g_viewport->sety(v.y * f_c00 * t * 20.0f + g_viewport->gety());
	}
	if (f_bf8 < -2.0f) f_bf8 = 0;
}

void C3DPlayer::ev125(char *start)							// 0x464840
{
	strcpy(f_9c8, start);
}

void C3DPlayer::ev126(char *game)							// 0x464870
{
	strcpy(f_c25, game);
}

// "PlayerLoadLevel": go to <game> at <start>. f_c18 1 first asks the game type whether a
// promotion is due (then the promotion level comes first and <game> is remembered in f_c25);
// f_c18 2 comes back from it. <picture> is shown on the HUD while loading.
void C3DPlayer::ev121(char *game, char *start, char *ret_game, char *ret_start, int picture)	// 0x4648a0
{
	// (debug log "Calling PlayerLoadLevel  %s  %s  %s  %s", empty)
	JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 0, 0.0f);	// Jimmy element +0x228
	g_menu->ShowHudPicture(picture, true, 0, -1);			// +0x4f4
	strcpy(g_task_file, g_545f14);							// 0x54e71c
	f_a99 = 0;
	g_editor->f_7f8 = 1;
	StopAllChannels();										// 0x4b8990
	char buf[80];
	strcpy(buf, game);
	if (f_c18 == 2)
	{
		strcpy(buf, f_c25);
		ev125(f_a18);
		strcpy(f_a18, f_b24);
		ev126((char *)"Promotion.gam");
		strcpy(f_b24, "phonebooth");
		strcpy(f_b89, f_c25);
		// port: the original passes the short g_545d9c in a register whose upper half is left over
		JimmyActivateMenuItem(g_jimmy, this, NULL, (short)g_545d9c, 0.0f);
		f_c18 = 0;
	}
	else
	{
		CJimmyGame *jg = dynamic_cast<CJimmyGame *>(g_gametype);
		if (f_c18 == 1 && jg && jg->CheckPromotion())		// +0x188 (port: only a CJimmyGame has it)
		{
			strcpy(f_a18, "none");
			ev126(buf);
			strcpy(buf, "Promotion.gam");
			ev125((char *)"phonebooth");
			strcpy(f_b24, start);
			strcpy(f_b89, "promotion.gam");
			g_gametype->SetCamera(NULL);					// +0x124
		}
		else
		{
			ev125(start);
			strcpy(f_a18, start);
			ev126(ret_game);
			strcpy(f_b24, ret_start);
			strcpy(f_b89, buf);
		}
	}
	if (!f_bd9)
	{
		OMedia3DVector p = Position(this);
		f_bdc.set(p.x, p.y, p.z, p.w);
	}
	LoadTask(buf, true);									// 0x498af0
	ev116();
}
