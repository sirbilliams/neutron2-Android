/*  C3DVRTank - see C3DVRTank.h. Reconstructed from Neutron2.exe 0x47aaf0..0x47c3c0.
    Constants: 0x4dce30 = 1/55 (skid), 0x4cb228 = 16384 / 360, 0x4dce54 / 0x4dce4c = +-15
    (steering), 0x4e5628 / 0x4e562c = 0.6 / 0.2 (fire interval), 0x4cb200 / 0x503d5c = +-500,
    0x4cb1dc = -1, 0x4d340c = 0.25, 0x4d6c30 = 0.8, 0x4dce48 = 0.1, 0x4caa5c = 0.5, 0x4cb1f4 =
    180, 0x4d2ca0 = -0.5 (lean), 0x4d2c98 / 0x4e2b70 = 1 / 1.9, 21 (speed to frequency),
    0x4df740 = 1/45 (shot pitch), 0x503d68 / 0x4dce64 / 0x4dce58 = 5.15 / 4 / 1.2 (turn rates). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DVRTank.h"
#include "C3DEnergyProjectile.h"
#include "../C3DGoddard.h"
#include "../C3DGrannyObj.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"			// PlayerKey*, JimmyActivateMenuItem
#include "../C3DRocketShip.h"		// JimmyHurt
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Mouse.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#include "OMedia3DVector.h"
#include "OMediaTrigo.h"
#pragma GCC diagnostic pop
#include <SDL.h> // TEMPDBG
#include <stdio.h> // TEMPDBG
#include <stdlib.h> // TEMPDBG
#include <math.h>
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

static OMedia3DVector Position(CGameObject *o)	{ float p[4]; o->v195(p); return OMedia3DVector(p[0], p[1], p[2], p[3]); }
static OMedia3DVector Angles(CGameObject *o)	{ float a[4]; o->v201(a); return OMedia3DVector(a[0], a[1], a[2], a[3]); }
static C3DJimmy *JimmyOf(CGameObject *o)		{ return o ? dynamic_cast<C3DJimmy *>(o) : NULL; }
static C3DObject *Element(CGameObject *o)		{ return o ? dynamic_cast<C3DObject *>(o) : NULL; }

// 0x485730: the neutron energy (g_health) += d, kept in 0..100; "low neutron energy" once when
// it falls below 19 (g_545fd4 remembers it until it is above 20 again). (As in C3DRocketShip.cpp.)
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

// 0x4656f0: the brake key list (no joystick button). (As in C3DRocketShip.cpp.)
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
static bool PlayerKeyBrake(void) { return KeyListDown(g_key_brake); }	// 0x4656f0

// --------------------------------------------------------------------------- construction

C3DVRTank::C3DVRTank()										// 0x47aaf0
{
	f_1b80.w = 1.0f;										// (OMedia3DPoint constructors)
	f_1b90.w = 1.0f;
	SetName((char *)"C3DVRTANK");
	Log("C3DVRTank()");
	C3DVRTank::InitObject();								// 0x47ae00 (called directly)
	C3DObject::SetObjectID('3VRT');							// 0x49ab00
	C3DObject::InitPhysics();								// 0x49acb0
	v107(1);
	v52(1);
	v64(0.25f);
	f_1b54 = 0;
	f_1b5c = 0;
	f_1b60 = 0;
	f_1b50 = 1200.0f;
	f_968 = 0;
	f_005 = 1;
	f_00a = 1;
	v44(0);
	v50(0);
	v77(0, 0, 0);
	f_1b3c = 0;												// InitialSpeed
	f_1b6c = -35.0f;
	f_1b78 = 35.0f;
	f_1b70 = 800.0f;
	f_1b7c = 500.0f;
	f_1b80.x = 0;
	f_1b80.y = 350.0f;
	f_1b80.z = -500.0f;
	f_1b90.x = 0;
	f_1b90.y = 50.0f;
	f_1b90.z = 200.0f;
	v62(0.01f);
	f_1ba0 = NULL;
	f_1ba4 = NULL;
	f_1b64 = 0;
	f_1ba8 = NULL;
	f_1b58 = 0;
	f_1b74 = 0;												// port: not set by the original (zeroed memory)
	f_1b68 = 0;
	f_1bdc = 0;
	for (int i = 0; i < 10; i++)
	{
		C3DEnergyProjectile *s = new C3DEnergyProjectile;	// 0x628 bytes
		f_1bb4[i] = s;
		if (s)
		{
			// (a call to the empty 0x463890 with the size and the name: a leftover allocation log)
			s->HideAgain();									// element +0xd8
			s->v52(0);										// +0xd0
			s->v132(0);										// +0x210
		}
	}
	// (then a call to the empty 0x463890)
}

C3DVRTank::~C3DVRTank() {}									// 0x47adb0

namespace neg {
CGameObject *New_C3DVRTank(void) { return new C3DVRTank; }				// '3VRT' (0x1be8 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DVRTank::InitObject(void)							// 0x47ae00
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// element +0x110
	CreateAnim((char *)"STRUGGLE", (char *)"tankfly.ase");	// element +0xe0
	CreateAnim((char *)"STOP", (char *)"tankstop.ase");
	CreateTexture((char *)"tank.png", 0);					// element +0xfc
	ApplyTexture(f_590, 0);									// element +0x100
	ev58((char *)"STOP", 1);								// element +0xe8
	f_1bac = -1;
	f_1bb0 = 11025;
	ev107(1);												// element +0x1ac: wait
	// (then CGameObject's empty slot 13)
}

void C3DVRTank::v260(void)									// 0x47b800
{
	C3DAI::v260();											// 0x4143a0
	f_1b3c = 0;
	f_968 = 0;
	f_964 = NULL;
	f_1ba0 = FindObjectByTag("JIM1");						// 0x4aefb0
}

void C3DVRTank::AddTouching(CGameObject *other)				// 0x47b840
{
	C3DAI::AddTouching(other);								// 0x415150
	if (g_jimmy && other == g_jimmy)						// Jimmy gets in
		JimmyActivateMenuItem(g_jimmy, this, this, 5, 13.0f);	// his element +0x228
	if (!other->NameContains((char *)"C3DFLYINGBUG") && !other->NameContains((char *)"C3DGIANTWASP")) return;
	C3DAI *bug = dynamic_cast<C3DAI *>(other);				// (element - 0xc0)
	if (!bug) return;										// port: checked
	// its knock-out time (C3DFlyingBot +0x1b50, read through the element)
	float out;
	memcpy(&out, (char *)bug + ((char *)&f_1b50 - (char *)this), 4);	// port: the same member of the bug
	if (out > 0 || !f_1ba0) return;
	PlayGameSound(-1, std::string("vrhit"), false);			// 0x48f420
	if (g_difficulty == 0) JimmyHurt(f_1ba0, -1.0f, -1, 5.0f, 1);	// JIM1 element +0x218
	else if (g_difficulty == 1) JimmyHurt(f_1ba0, -3.0f, -3, 5.0f, 1);
	else if (g_difficulty == 2) JimmyHurt(f_1ba0, -5.0f, -5, 5.0f, 1);
}

void C3DVRTank::Process(float dt)							// 0x47aec0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (g_camera != this)									// not driven: an ordinary C3DAI
	{
		C3DAI::Process(dt);									// 0x411240
		return;
	}
	C3DAnimated::Process(dt);								// 0x41c120

	// the skid: sideways with the steering and the speed (the OMT rotation of (side, 0, 0)
	// about y, inlined)
	float dx = 0, dz = 0;
	float v[4];
	v174(v);												// +0x2b8
	const float vy = v[1];
	if ((float)sqrt((double)vy * vy + (double)v[2] * v[2] + (double)v[0] * v[0]) != 0)
	{
		const float side = (float)((1.0 - (double)v145()) * ((double)f_1b54 * 0.018181818f) * f_968);	// +0x244
		const OMedia3DVector a = Angles(this);
		const int i = (int)(a.y * 45.511112f) & 0x3fff;		// 0x4ba850 (_ftol)
		const float *sn = OMediaCosSinTable::global.get_sintab();	// OMediaCosSinTable +0
		const float *cs = OMediaCosSinTable::global.get_costab();	// +0x10004
		const float c0 = cs[0] * side;
		const float s0 = -(-(sn[0] * side) * sn[0]);
		dx = c0 * cs[i] - s0 * sn[i];
		dz = c0 * sn[i] + s0 * cs[i];
	}
	const OMedia3DVector f = v226(0, 0, f_968);				// +0x388: forward at the speed
	v176(dx + f.x, vy, dz + f.z);							// +0x2c0

	// the riders
	C3DJimmy *jim = JimmyOf(g_jimmy);						// (0x453130)
	if (jim)
	{
		OMedia3DVector q = v224(0, 100.0f, -100.0f);		// +0x380
		g_jimmy->v196(q.x, q.y, q.z, q.w);					// +0x310
		OMedia3DVector r = Angles(this);
		g_jimmy->v202(r.x, r.y, r.z, r.w);					// +0x328
		jim->ev133(dt);										// element +0x214: his hurt flashes
		if (jim->f_d5c)										// and Goddard
		{
			C3DGoddard *g = jim->f_d5c;
			q = v224(135.0f, 165.0f, -90.0f);
			g->v196(q.x, q.y, q.z, q.w);
			r = Angles(this);
			g->v202(r.x, r.y, r.z, r.w);
			if (g->f_1bbc)
			{
				unsigned int bits;							// (the slot takes the frame time)
				memcpy(&bits, &dt, 4);
				g->f_1bbc->v241(bits);						// his head prop follows (+0x3c4)
			}
		}
	}
	ev144(dt);												// element +0x240: the cursor
}

void C3DVRTank::v243(float dt)								// 0x47b9d0
{
	short mdx = 0, mdy = 0;
	GetMouseDelta(&mdx, &mdy);								// 0x4a3990
	{	// TEMPDBG teleport: NEG_TP="n:x:y:z:ay,..." at the n-th driven frame
		static int tick = 0; tick++;
		const char *tp = getenv("NEG_TP");
		for (const char *q = tp; q && *q; )
		{
			int n; float x, y, z, ay; int used = 0;
			if (sscanf(q, "%d:%f:%f:%f:%f%n", &n, &x, &y, &z, &ay, &used) != 5) break;
			q += used; if (*q == ',') q++;
			if (n == tick) { v196(x, y, z, 1.0f); v203(0, ay, 0); f_274 = 0; f_278 = ay; f_27c = 0; SDL_Log("TEMPDBG tp %d", n); }
		}
		if (tick % 200 == 0) { float p[4]; v195(p); SDL_Log("TEMPDBG tank at %.0f %.0f %.0f speed %.0f hp %.1f", p[0], p[1], p[2], f_968, g_health); }
	}
	f_1b50 = (g_54a424[0] != 0) ? 2000.0f : 1200.0f;		// the power-up
	float boost = 1.0f;
	if (g_540594 < 1.0f) boost = 4.0f;

	// steering
	if (PlayerKeyLeft()) f_1b54 = -15.0f;					// 0x4659f0
	else if (PlayerKeyRight()) f_1b54 = 15.0f;				// 0x465a60
	else if (mdx >= -3 && mdx <= 3) f_1b54 = 0;
	else
	{
		float m = (float)mdx;
		if (m > 15.0f) m = 15.0f;							// 0x4dce54
		else if (m < -15.0f) m = -15.0f;					// 0x4dce4c
		f_1b54 = -m;
	}

	// firing
	f_1b64 = dt + f_1b64;
	if (PlayerKeyAction())									// 0x465750
	{
		float every = 0.6f;									// 0x4e5628
		if (g_54a438 > 0) every = 0.2f;						// 0x4e562c
		if (every < f_1b64 && g_health >= 0)
		{
			Fire(60.0f, 70.0f, 680.0f, 2.5f, 2000.0f, f_1ba8, 0, 0);	// 0x47b260
			f_1b64 = 0;
		}
	}

	// speed
	if (PlayerKeyGo())										// 0x465910
	{
		f_1b68 = 1;
		if (boost * f_1b50 > f_968)
		{
			f_968 = dt * f_1b70 + f_968;
			const float top = boost * f_1b50;
			if (f_968 > top) f_968 = top;
			f_1b6c = 10.0f;
		}
	}
	else if (PlayerKeyBack())								// 0x465980
	{
		f_1b68 = 1;
		if (boost * f_1b50 * -1.0f < f_968)
		{
			f_968 = f_968 - dt * f_1b7c;
			const float low = boost * f_1b50 * -1.0f;
			if (f_968 < low) f_968 = low;
			f_1b6c = -7.0f;
		}
	}
	else if (mdy >= -10 && mdy <= 10)
	{
		f_1b68 = 0;
		f_1b6c = 3.0f;
	}
	else
	{
		f_1b68 = 1;
		float m = (float)(mdy * 250);
		if (m > 500.0f) m = 500.0f;							// 0x4cb200
		else if (m < -500.0f) m = -500.0f;					// 0x503d5c
		f_968 = m * dt + f_968;
		if (m > 0) f_1b6c = 10.0f;
		else if (m < 0) f_1b6c = -7.0f;
	}
	if (PlayerKeyBrake())									// 0x4656f0 (per frame, not per second)
	{
		if (boost * f_1b50 > f_968)
		{
			f_968 = f_968 - f_1b7c * 0.25f;
			if (f_968 < 0) f_968 = 0;
		}
		if (f_968 < 0) f_968 = 0;
	}
	if ((float)(f_1b50 * boost * 0.8) < f_968)				// over 80%: slow down
	{
		f_968 = f_968 - f_1b7c * 0.1f;
		if (f_968 < 0) f_968 = 0;
	}
	if ((float)(boost * f_1b50 * -1.0f * 0.8) > f_968)		// reversing over 80%
	{
		f_968 = f_1b7c * 0.5f + f_968;
		if (f_968 > 0) f_968 = 0;
	}

	// half round
	if (PlayerKeyTurnAround() && f_1b58 <= 0) f_1b58 = 1.0f;	// 0x4657c0
	if (f_1b58 > 0)
	{
		f_1b58 = f_1b58 - dt;
		if (f_1b58 > 0) AddRotation(0, dt * 180.0f, 0);		// +0x330
		else
		{
			AddRotation(0, (f_1b58 + dt) * 180.0f, 0);		// the rest of the half turn
			f_1b58 = 0;
		}
	}

	// turn and lean (the wanted angles: RotateToDest eases there), the pitch
	{
		const float lean = f_1b54 * -0.5f;					// 0x4d2ca0
		const OMedia3DVector r = Angles(this);
		v218(0, r.y - f_1b54, lean);						// +0x368
	}
	f_274 = f_1b6c;											// (+0x274: the x angle wanted)

	// the chase camera
	OMedia3DVector want = v224(f_1b80.x, f_1b80.y, f_1b80.z);	// +0x380
	const OMedia3DVector look = v224(f_1b90.x, f_1b90.y, f_1b90.z);
	const float k = ev95(look, &want);						// element +0x17c: out of the walls
	const OMedia3DVector cam(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);	// 2002 +0x44
	ev98(dt, k, 1.0f, cam, want, look, OMedia3DVector(0.8f, 0.8f, 0.8f, 0));	// element +0x188

	// the engine: "tank1", its frequency 21 / 1.9 per unit of horizontal speed above the base
	if (f_1bac == -1)
	{
		f_1bac = PlayGameSound3D(this, -1, std::string("tank1"), true);	// 0x48f200
		f_1bb0 = g_chan_freq[f_1bac];						// 0x6f73d4
	}
	else
	{
		float vel[4];
		v174(vel);											// +0x2b8
		int fr = (int)(sqrt((double)vel[0] * vel[0] + (double)vel[2] * vel[2]) * 0.5263157894736842 * 21.0 + f_1bb0);	// 0x4ba850
		if (fr <= 7000) fr = 7000;
		g_chan_freq[f_1bac] = fr;
	}

	// Jimmy's bubble over JIM1
	if (f_1ba0)
	{
		C3DJimmy *j = JimmyOf(f_1ba0);
		const OMedia3DVector p = v224(0, -30.0f, 10.0f);
		if (j) j->ev146(dt, p);								// element +0x248 (port: checked)
	}
}

void C3DVRTank::RotateToDest(float dt)						// 0x47c110
{
	if (g_gametype->IsPaused() || !f_00a) return;			// +0x114
	const float wx = f_274, wy = f_278, wz = f_27c;			// the angles wanted
	float dx = 0, dy = 0, dz = 0;
	const float ax = Angles(this).x;						// +0x324 (three calls)
	const float ay = Angles(this).y;
	const float az = Angles(this).z;
	f_00a = 0;
	if (wx != ax)
	{
		dx = wx - ax;
		if (dx > 180.0f) dx -= 360.0f;
		else if (dx < -180.0f) dx += 360.0f;
		f_00a = 1;
	}
	if (wy != ay)
	{
		dy = wy - ay;
		if (dy > 180.0f) dy -= 360.0f;						// (no -180 test for y)
		f_00a = 1;
	}
	if (wz != az)
	{
		dz = wz - az;
		if (dz > 180.0f) dz -= 360.0f;
		else if (dz < -180.0f) dz += 360.0f;
		f_00a = 1;
	}
	AddRotation((float)(dx * dt * 1.2), dy * dt * 4.0f, (float)(dz * dt * 5.15));	// +0x330
}

void C3DVRTank::v277(void)									// 0x47c2d0
{
	C3DAnimated::v277();									// 0x41ce80
	if (f_1bac != -1) StopGameSound(f_1bac, false);			// 0x48f0d0 (keeps the channel: v278 restarts it)
}

void C3DVRTank::v278(void)									// 0x47c2f0
{
	C3DAnimated::v278();									// 0x41cea0
	if (f_1bac != -1) f_1bac = PlayGameSound3D(this, -1, std::string("tank1"), true);	// 0x48f200
}

// --------------------------------------------------------------------------- element family

void C3DVRTank::ev144(float dt)								// 0x47b490
{
	(void)dt;
	const OMedia3DVector aim = v224(0, 200.0f, 3000.0f);	// +0x380
	CGameObject *t = ev145(aim);							// element +0x244
	f_1ba8 = t;
	C3DJimmy *jim = JimmyOf(g_jimmy);
	f_1ba4 = (jim && jim->f_d5c) ? static_cast<CGameObject *>(jim->f_d5c) : NULL;
	C3DGoddard *g = jim ? jim->f_d5c : NULL;
	CGameObject *cursor = g ? g->f_1b80 : NULL;				// Goddard's red-neutron cursor
	C3DObject *ce = Element(cursor);
	if (!ce) return;										// port: the original uses it unchecked
	if (!t)
	{
		ce->HideAgain();									// element +0xdc
		return;
	}
	const OMedia3DVector p = Position(t);					// +0x30c
	const float vx = g_viewport->getx(), vy = g_viewport->gety(), vz = g_viewport->getz();	// 2002 +0x44
	OMedia3DVector d(p.x - vx, p.y - vy, p.z - vz, 0);
	const float k = 1.0f / d.quick_magnitude();
	cursor->v196((k * d.x) * 300.0f + vx, (k * d.y) * 300.0f + vy, (k * d.z) * 300.0f + vz, 1.0f);	// +0x310; 0x4caa7c
	ce->ShowAgain();										// element +0xd8
}

CGameObject *C3DVRTank::ev145(OMedia3DVector pos)			// 0x47b680
{
	// (the velocity is fetched and normalised first, then not used)
	CGameObject *best = NULL;
	float best_d = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *o = *i;
		if (!o || o->m_object_id != '3BUG' || !o->v123() || !o->f_140) continue;
		const OMedia3DVector q = Position(o);
		OMedia3DVector d(pos.x - q.x, pos.y - q.y, pos.z - q.z, 0);
		const float m = d.quick_magnitude();
		if (!best || m < best_d)
		{
			best_d = m;
			best = o;
		}
	}
	return best;
}

// --------------------------------------------------------------------------- not virtual

void C3DVRTank::Fire(float x, float y, float z, float pitch, float speed, CGameObject *target, float delay, float unused)	// 0x47b260
{
	C3DEnergyProjectile *s = f_1bb4[f_1bdc];
	if (!s) return;
	SDL_Log("TEMPDBG tank fire %d target %p", f_1bdc, (void*)target);
	const OMedia3DVector m = v224(x, y, z);					// +0x380: the muzzle
	s->v196(m.x, m.y, m.z, m.w);							// +0x310
	s->Reset();												// 0x430c10
	s->f_608 = delay;
	s->f_604 = target;
	s->f_610 = speed;
	s->f_614 = 0;
	s->f_618 = unused;
	OMedia3DVector d = v226(0, pitch * 0.022222223f, 1.0f);	// +0x388; 0x4df740
	const float k = 1.0f / d.quick_magnitude();
	d.x = k * d.x;
	d.y = d.y * k;
	d.z = d.z * k;
	PlayGameSound(-1, std::string("tankfire"), false);		// 0x48f420
	if (g_54a438 <= 0) AddEnergy(-0.5f);					// 0x485730
	s->Fire(d, speed);										// 0x430cd0
	f_1bdc++;
	if (f_1bdc >= 10) f_1bdc = 0;
}
