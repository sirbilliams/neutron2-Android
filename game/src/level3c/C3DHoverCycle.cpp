/*  C3DHoverCycle - see C3DHoverCycle.h. Reconstructed from Neutron2.exe 0x43f750..0x4419f0.
    Constants: 0x4cb1e0 = 1500 (ray down; coasting drag), 0x4dce54 = 15 (lift on the ground),
    0x4d6c30 = 0.8 (mouse to turn; the 80% speed band), 0x4dce3c / 0x4dce40 = +-45,
    0x4cb1f4 = 180 (TurnAround deg / s), 0x4cb20c = 550 (crash spin), 0x4d2c98 * 0x4e2b70 =
    21 / 1.9 (speed to frequency), 0x4dce64 = 4 (thrust), 0x4dce48 = 0.1, 0x4caa60 = 1.5.   */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DHoverCycle.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"			// PlayerKey*
#include "../C3DShadow.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Mouse.h"
#include "../Physics.h"
#include "../Sound.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaElement.h"
#include "OMediaViewPort.h"
#include "OMedia3DVector.h"
#include "OMediaTrigo.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// vtable +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

static OMedia3DVector Velocity(CGameObject *o)
{
	float v[4];
	o->v174(v);												// vtable +0x2b8
	return OMedia3DVector(v[0], v[1], v[2], v[3]);
}

static OMedia3DVector Vec(const OMedia3DPoint &p)
{
	return OMedia3DVector(p.x, p.y, p.z, p.w);
}

// CGameObject slot 224 (0x4ad800): a local point in world coordinates.
static OMedia3DVector Local(CGameObject *o, float x, float y, float z)
{
	return o->v224(x, y, z);								// vtable +0x380
}

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0. (The
// generated declaration of v226 has the wrong signature; as in C3DPlayer.cpp.)
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

// v * (1 / |v|) (quick_magnitude), w kept.
static OMedia3DVector Unit(OMedia3DVector v)
{
	const float m = v.quick_magnitude();
	if (m == 0) return OMedia3DVector(0, 0, 0, v.w);		// port: the original divides by 0 (a NaN velocity)
	const float k = 1.0f / m;								// 0x4caa6c
	v.x = k * v.x;
	v.y = k * v.y;
	v.z = k * v.z;
	return v;
}

// The velocity set to `speed` along its own direction on x and z, y kept (inline 4 times).
static void KeepHeading(CGameObject *o, float speed)
{
	const OMedia3DVector n = Unit(Velocity(o));
	const float vy = Velocity(o).y;
	o->v175(n.x * speed, vy, n.z * speed, n.w);				// +0x2bc
}

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
		return;
	}
	if (g_health > 20.0f) g_545fd4 = 0;
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

// 0x4a39f0 (CAweReal): the mouse steering, (dx, dy, dt, 8, 6, 50, 10) here. Per axis, the pointer's
// distance from the viewport's centre (centre - pointer): inside the dead zone (+-lim), with the
// held value inside it too, the axis is 0 and the held value cleared; outside, the distance / k
// (k <= 0: 0.0001) is kept when it is further out than the held value (g_56aebe x, g_56aec0 y;
// the last one computed in g_56aeba / g_56aebc), and the held value is the result. Every 0.2 s
// (g_56aea8) the pointer goes back to the centre. Nothing happens while g_56aed0 is set.
static void MouseSteer(short *dx, short *dy, float dt, float kx, float ky, short limx, short limy)
{
	if (g_56aed0) return;
	short x = 0, y = 0;
	GetMousePos(&x, &y);									// 0x4b1d10
	const short rx = (short)(MouseCentreX() - x);			// 0x4b1da0
	const short ry = (short)(MouseCentreY() - y);			// 0x4b1dc0
	short held = g_56aebe;
	if (rx > limx || rx < -limx || held > limx || held < -limx)
	{
		if (kx <= 0.0f) kx = 0.0001f;						// 0x4caa44, 0x38d1b717
		if (rx > limx)
		{
			g_56aeba = (short)(int)((float)rx / kx);		// 0x4ba850 (ftol)
			if (g_56aeba > held) { held = g_56aeba; g_56aebe = held; }
		}
		if (rx < -limx)
		{
			g_56aeba = (short)(int)((float)rx / kx);
			if (g_56aeba < held) { held = g_56aeba; g_56aebe = held; }
		}
		*dx = held;
	}
	else
	{
		g_56aebe = 0;
		*dx = 0;
	}
	held = g_56aec0;
	if (ry > limy || ry < -limy || held > limy || held < -limy)
	{
		if (ky <= 0.0f) ky = 0.0001f;
		if (ry > limy)
		{
			g_56aebc = (short)(int)((float)ry / ky);
			if (g_56aebc > held) { held = g_56aebc; g_56aec0 = held; }
		}
		if (ry < -limy)
		{
			g_56aebc = (short)(int)((float)ry / ky);
			if (g_56aebc < held) { held = g_56aebc; g_56aec0 = held; }
		}
		*dy = held;
	}
	else
	{
		g_56aec0 = 0;
		*dy = 0;
	}
	if (g_56aea8 > 0.0)										// 0x4d17c0 (a double)
	{
		g_56aea8 = g_56aea8 - dt;
		return;
	}
	CenterMouse();											// 0x4b1d50
	g_56aea8 = 0.2f;
}

// The rider: C3DJimmy element slot 134 (+0x218) hurts him.
static C3DJimmy *JimmyOf(CGameObject *o)	{ return o ? dynamic_cast<C3DJimmy *>(o) : NULL; }

// --------------------------------------------------------------------------- construction

C3DHoverCycle::C3DHoverCycle()								// 0x43f750
{
	f_1b58.w = 1.0f;										// (OMedia3DPoint constructors)
	f_1b68.w = 1.0f;
	SetName((char *)"C3DHOVERCYCLE");
	Log("C3DHoverCycle()");
	C3DHoverCycle::InitObject();							// 0x43fab0 (inlined)
	C3DObject::SetObjectID('3HOV');							// 0x49ab00
	C3DObject::InitPhysics();								// 0x49acb0
	v107(1);
	v52(1);
	v64(0.75f);
	f_1b88 = 0;
	f_1b50 = 0;
	f_1b54 = 0;
	f_1ba0 = 1600.0f;
	f_005 = 1;
	f_00a = 1;
	v44(0);
	v50(0);
	v77(0, 0, 0);
	f_1b90 = -35.0f;
	f_1b94 = 35.0f;
	f_1b98 = 1700.0f;
	f_1b9c = 500.0f;
	f_1b58.x = 0;
	f_1b58.y = 10.0f;
	f_1b58.z = -300.0f;
	f_1b68.x = 0;
	f_1b68.y = 10.0f;
	f_1b68.z = 200.0f;
	v62(0.01f);
	v172(3.15f, 4.0f, 5.0f);
	f_1ba8 = NULL;
	f_1b78 = 0;
	f_1bac = NULL;
	f_1ba4 = NULL;
	f_1b7c = 0;
	f_1bb4 = -1;
	f_1bb8 = 11025;
	f_1b80 = 0;
	f_1b84 = 0;
	f_1bbc = 0;
	f_1bc0 = 0;
	f_1bcc = 0;
	f_1bc4 = NULL;
	f_1bc8 = 0;
	f_968 = 0;												// speed
	f_1b3c = 5.0f;											// InitialSpeed
	v176(0, 0, 0);
	// (then a call to the empty 0x463890)
}

C3DHoverCycle::~C3DHoverCycle() {}							// 0x43fa60

namespace neg {
CGameObject *New_C3DHoverCycle(void) { return new C3DHoverCycle; }	// '3HOV' (0x1bd8 bytes)
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DHoverCycle::InitObject(void)						// 0x43fab0
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// element +0x110
	CreateAnim((char *)"STOP", (char *)"godflycycle.ase");	// +0xe0
	CreateTexture((char *)"goddard02.png", 0);				// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(100.0f);											// CGameObject +0x10c: radius
	ev58((char *)"STOP", 1);								// +0xe8
	// (then a call to the empty 0x463890)
}

void C3DHoverCycle::v260(void)								// 0x441110
{
	C3DAI::v260();											// 0x4143a0
	f_1bc4 = FindObjectByTag("C3DTSUNAMI");
}

// Hovering; the rider sits 7 in front of our origin, turned with us.
void C3DHoverCycle::Process(float dt)						// 0x43fb40
{
	C3DAnimated::Process(dt);								// 0x41c120 (not C3DAI's)
	ev146(dt);												// element +0x248
	if (!f_1ba4) return;
	CGameObject *rider = static_cast<CGameObject *>(f_1ba4);	// (+0xc0)
	const OMedia3DVector q = Local(this, 0, 0, 7.0f);
	rider->v196(q.x, q.y, q.z, q.w);						// +0x310
	const OMedia3DVector p = Position(this);
	f_1ba4->place(p.x, p.y, p.z);							// element +0x44..+0x4c written directly
	float r[4];
	v201(r);												// +0x324
	rider->v202(r[0], r[1], r[2], r[3]);					// +0x328
}

void C3DHoverCycle::AddTouching(CGameObject *other)			// 0x43fc30
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other == g_jimmy)									// (g_jimmy - 0xc0 + 0xc0)
	{
		C3DJimmy *j = JimmyOf(g_jimmy);
		if (j) j->ev138(this, this, 12, 13.0f);				// element +0x228: ActivateMenuItem: get on
		return;
	}
	if (other->NameContains((char *)"C3DSHRUBBERY"))
	{
		f_968 = f_968 * 0.8f;								// 0x4caa58
		const OMedia3DVector v = Velocity(this);
		v176(v.x * 0.8f, v.y, v.z * 0.8f);					// +0x2c0
		PlayGameSound(-1, std::string("shrubs"), false);	// 0x48f420
		return;
	}
	if (other->NameContains((char *)"C3DTSUNAMI")) f_1bbc = 1;
}

// A wall: at full speed (difficulty 1 / 2, nearly head-on: velocity . normal < -0.96 / -0.88)
// it crashes; over 700 it slides along, slower.
void C3DHoverCycle::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x440da0
{
	CGameObject::v220(a, b, c, d, poly);					// 0x48ef40 (not C3DAI's)
	if (!(f_968 < f_1ba0))
	{
		if (g_difficulty == 0) return;						// 0x53b5b0
		if (g_difficulty == 1 || g_difficulty == 2)
		{
			const float lim = g_difficulty == 1 ? -0.96f : -0.88f;	// 0x4e2b78, 0x4e2b7c
			const OMedia3DVector n = Unit(Velocity(this));
			if (n.z * c + n.y * b + n.x * a < lim)			// nearly head-on
			{
				f_1b84 = 1.65f;								// spin
				f_00a = 0;
				v176(a * 10.0f, b * 10.0f, c * 10.0f);		// bounce off; 0x4cb1cc
				PlayGameSound(-1, std::string("goddard bark"), false);
				C3DJimmy *j = JimmyOf(g_jimmy);
				if (j) j->ev134(-5.0f, -5, 5.0f, 1);		// port: the original calls through a NULL g_jimmy
				f_968 = 400.0f;
			}
		}
	}
	if (!(f_968 > 700.0f)) return;							// 0x4ccf18
	if (f_1bac && !JimmyOf(f_1bac)->f_4cb8)					// (never set)
	{
		JimmyOf(f_1bac)->f_4cb8 = 0;						// (sic)
		JimmyOf(f_1bac)->ev134(-1.0f, -1, 1.0f, 1);
		PlayGameSound(-1, std::string("rocket hit"), false);
	}
	f_968 = f_968 * 0.8f;
	const OMedia3DVector v = Velocity(this);
	v176(a * 200.0f + Velocity(this).x * 0.8f, v.y, c * 200.0f + v.z * 0.8f);	// 0x4cb21c
}

void C3DHoverCycle::v277(void)								// 0x441900
{
	C3DAnimated::v277();									// 0x41ce80
	if (f_1bb4 != -1) StopGameSound(f_1bb4, false);			// (keeps the channel: v278 restarts it)
}

void C3DHoverCycle::v278(void)								// 0x441920
{
	C3DAnimated::v278();									// 0x41cea0
	if (f_1bb4 != -1) f_1bb4 = PlayGameSound3D(this, -1, std::string("harley1"), true);	// 0x48f200
}

void C3DHoverCycle::WriteState(OMediaStreamOperators &s)	// 0x441870
{
	C3DAI::WriteState(s);									// 0x415760
	OMedia3DVector v = Velocity(this);
	s << v.x;
	s << v.y;
	s << v.z;
	s << f_1b84;
	bool b = f_00a != 0;
	s << b;
	s << f_968;
}

void C3DHoverCycle::ReadState(OMediaStreamOperators &s)		// 0x441790
{
	C3DAI::ReadState(s);									// 0x4155e0
	float r[4];
	v201(r);
	v218(r[0], r[1], r[2]);									// +0x368
	float x, y, z;
	s >> x;
	s >> y;
	s >> z;
	v175(x, y, z, 0);										// +0x2bc: the velocity
	s >> f_1b84;
	bool b;
	s >> b;
	f_00a = b;
	s >> f_968;
	f_1bb4 = -1;
}

// --------------------------------------------------------------------------- ridden

void C3DHoverCycle::v243(float dt)							// 0x440300
{
	C3DPlayerObject::v243(dt);								// 0x464ee0 (empty)
	ev145(dt);												// element +0x244
	const bool left = PlayerKeyLeft();						// 0x4659f0
	const bool right = PlayerKeyRight();					// 0x465a60
	short mx = 0, my = 0;
	MouseSteer(&mx, &my, dt, 8.0f, 6.0f, 50, 10);			// 0x4a39f0
	if (v133())												// +0x214 on the ground: lift a little
	{
		const OMedia3DVector p = Position(this);
		v196(p.x, p.y + 15.0f, p.z, p.w);					// +0x310; 0x4dce54
	}
	if (left) f_1b88 = -25.0f;
	else if (right) f_1b88 = 25.0f;
	else
	{
		f_1b88 = (float)((double)(-mx) * 0.8);				// 0x4d6c30
		if (f_1b88 > 45.0f) f_1b88 = 45.0f;					// 0x4dce3c
		else if (f_1b88 < -45.0f) f_1b88 = -45.0f;			// 0x4dce40
	}
	if (PlayerKeyTurnAround() && f_1bcc <= 0) f_1bcc = 1.0f;	// 0x4657c0
	if (f_1bcc > 0)
	{
		f_1bcc = f_1bcc - dt;
		if (f_1bcc > 0) AddRotation(0, dt * 180.0f, 0);		// +0x330; 0x4cb1f4
		else
		{
			AddRotation(0, (f_1bcc + dt) * 180.0f, 0);		// the rest of the half turn
			f_1bcc = 0;
		}
	}
	KeepHeading(this, f_968);

	// the engine: "harley1", its frequency 21 / 1.9 per unit of horizontal speed above the base
	if (f_1bb4 == -1)
	{
		f_1bb4 = PlayGameSound3D(this, -1, std::string("harley1"), true);	// 0x48f200
		f_1bb8 = g_chan_freq[f_1bb4];						// 0x6f73d4
	}
	else
	{
		const OMedia3DVector v = Velocity(this);
		int f = (int)(sqrt(v.x * v.x + v.z * v.z) * 0.5263157894736842 * 21.0 + f_1bb8);	// 0x4ba850 (_ftol)
		// (a debug text "%d" of f at 20, 150: compiled out)
		if (f <= 7000) f = 7000;
		g_chan_freq[f_1bb4] = f;
	}

	// crashed: spin until the time is over
	if (f_1b84 > 0)
	{
		f_1bc8 = 0;
		f_1b84 = f_1b84 - dt;
		if (f_1b84 > 0)
		{
			AddRotation(0, f_1b84 * dt * 550.0f, 0);		// 0x4cb20c
			return;
		}
		f_1b84 = 0;
		f_00a = 1;
	}
	const OMedia3DVector v0 = Velocity(this);
	if (PlayerKeyBack())									// 0x465980: brake
	{
		if (f_968 > 0)
		{
			f_1b90 = -26.0f;
			f_968 = f_968 - (dt * f_1b9c + dt * f_1b9c);
		}
		if (f_968 < 0)
		{
			f_968 = 0;
			f_1b90 = 0;
		}
		KeepHeading(this, f_968);
	}
	else if (!PlayerKeyGo() && !PlayerKeyAction() && f_1b88 <= 20.0f && !(f_1b88 < -20.0f))	// 0x465910, 0x465750; 0x4caa40, 0x4d65cc
	{
		// coasting: the momentum turns towards the heading
		if (f_968 > 0)
		{
			const OMedia3DVector n = Unit(Velocity(this));
			v178(-(n.x * 1500.0f * dt), 0, -(n.z * 1500.0f * dt));	// +0x2c8; 0x4cb1e0
			const OMedia3DVector d = ObjectDir(this, 0, 0, 950.0f);	// +0x388
			v178(d.x * dt, 0, d.z * dt);
			f_1b8c = 0;
			f_1b90 = 0;
			f_968 = f_968 - dt * f_1b9c;
			v175(d.x, Velocity(this).y, d.z, d.w);			// (sic: the heading at 950, not f_968)
		}
	}
	else
	{
		// accelerating
		f_1b8c = 1;
		switch (g_difficulty)
		{
			case 0: AddEnergy(dt * -0.3f); break;			// 0x4e2b60
			case 1: AddEnergy(dt * -0.6f); break;			// 0x4e2b64
			case 2: AddEnergy(dt * -0.8f); break;			// 0x4e2b68
		}
		if (f_968 > f_1ba0) f_1b90 = 0;
		else
		{
			f_968 = dt * f_1b98 + f_968;
			if (!PlayerKeyGo() && !PlayerKeyAction())		// (turning only: half speed)
			{
				const float half = f_1ba0 * 0.5f;			// 0x4caa5c
				if (half < f_968)
				{
					f_968 = half;
					f_1b90 = 0;
				}
				else f_1b90 = 27.0f;
			}
			else if (f_968 > f_1ba0)
			{
				f_968 = f_1ba0;
				f_1b90 = 0;
			}
			else f_1b90 = 27.0f;
		}
		const OMedia3DVector n = Unit(v0);
		v175(n.x * f_968, Velocity(this).y, n.z * f_968, n.w);
		const OMedia3DVector d = ObjectDir(this, 0, 0, f_968 * 4.0f);	// 0x4dce64
		v178(d.x, 0, d.z);
		KeepHeading(this, f_968);
		// the top speed: Jimmy's power (g_power_timers[0]) gives 2800, his sparkle without it 1800
		C3DJimmy *j = JimmyOf(g_jimmy);
		const float sparkle = j ? j->f_4d58 : 0;			// port: the original reads through a NULL g_jimmy
		if (sparkle == 0)
		{
			if (g_power_timers[0] < 0)
			{
				f_1ba0 = 1600.0f;
				if (f_968 > 1600.0f) f_968 = 1600.0f;		// 0x4e2b5c
				f_1bc0 = 0;
			}
		}
		else if (g_power_timers[0] > 0)						// 0x54a424
		{
			f_1ba0 = 2800.0f;
			f_968 = 2800.0f;
			f_1bc0 = 20.0f;
		}
		else
		{
			f_1ba0 = 1800.0f;
			if (f_968 > 1800.0f) f_968 = 1800.0f;			// 0x4d986c
			f_1bc0 = 0;
		}
	}
	// over 80% of the top speed without accelerating: slow down
	if (f_968 > (float)(f_1ba0 * 0.8) && f_968 <= f_1ba0 && !f_1b8c)
	{
		f_968 = f_968 - f_1b9c * 0.1f;						// 0x4dce48
		if (f_968 < 0) f_968 = 0;
	}
	float r[4];
	v201(r);												// +0x324
	v218(0, r[1] - f_1b88, f_1b88 * 1.5f);					// +0x368: turn and lean; 0x4caa60
	v152(100.0f);											// +0x260
	ev144(dt);												// element +0x240: the camera
}

// --------------------------------------------------------------------------- element slots

void C3DHoverCycle::ev144(float dt)							// 0x43fe70
{
	const OMedia3DVector cam(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);	// 2002 +0x44
	OMedia3DVector want = Local(this, f_1b58.x, f_1b58.y, f_1b58.z);
	const OMedia3DVector look = Local(this, f_1b68.x, f_1b68.y, f_1b68.z);
	const float k = ev96(Position(this), &want, 1200.0f);	// element +0x180
	const float k2 = ev95(look, &want);						// element +0x17c
	const float dy = want.y - cam.y;
	Position(this);											// (result unused)
	float ay;
	if (dy < 0) ay = 1.5f;
	else if (dy <= 200.0f) ay = 1.8f;						// 0x4cb21c
	else ay = 2.5f;
	OMedia3DVector d(0, 0, 0, 0);
	if (f_1bc4)												// port: the original has no check
	{
		const OMedia3DVector t = Position(f_1bc4), p = Position(this);
		d = OMedia3DVector(t.x - p.x, t.y - p.y, t.z - p.z, 0);
	}
	else d.x = 3000.0f;
	const float speed = Velocity(this).quick_magnitude();
	if (d.quick_magnitude() < 3000.0f && speed < 500.0f)	// 0x4d2ca4, 0x4cb200
	{
		// near the tidal wave and slow: look back at us from 1000 in front
		const OMedia3DVector p = Position(this);
		OMedia3DVector q = Local(this, 0, 300.0f, 1000.0f);
		ev95(p, &q);
		ev97(q, p);											// element +0x184
		return;
	}
	if (PlayerKeyBrake())									// 0x4656f0
	{
		const OMedia3DVector p = Position(this);
		OMedia3DVector q = Local(this, 0, 300.0f, 1000.0f);
		ev95(p, &q);
		const float ex = q.x - p.x, ey = q.y - p.y, ez = q.z - p.z;
		if (!(sqrtf(ey * ey + ex * ex + ez * ez) < 200.0f))
		{
			ev97(q, p);
			return;
		}
	}
	ev98(dt, k2, k, cam, want, look, OMedia3DVector(2.7f, ay, 2.7f, 0));	// element +0x188
}

void C3DHoverCycle::ev145(float dt)							// 0x43fd60
{
	if (!f_1bc4) return;
	const OMedia3DVector t = Position(f_1bc4), p = Position(this);
	const float dx = t.x - p.x, dy = t.y - p.y, dz = t.z - p.z;
	OMedia3DVector v = Velocity(this);
	if (v.quick_magnitude() > 0)
	{
		const float k = 1.0f / v.quick_magnitude();
		if (v.x * k * dx + v.y * k * dy + v.z * k * dz > 0.5f)	// 0x4caa5c
		{
			f_1bc8 = dt + f_1bc8;
			return;
		}
	}
	f_1bc8 = 0;
}

// 200 above the ground: the lower, the harder it is pushed up (acceleration against gravity,
// a vertical speed limit); far above, it falls faster.
void C3DHoverCycle::ev146(float dt)							// 0x441130
{
	f_1b78 = 200.0f;
	f_1b80 = f_1b80 - dt;
	const float g = g_gravity.y;							// 0x63119c
	const OMedia3DVector p = Position(this);
	const OMedia3DVector down(p.x, p.y - 1500.0f, p.z, p.w);	// 0x4cb1e0
	OMedia3DVector ground;
	if (PhysicsClosestHit(p, down, false))					// 0x4b7640
		ground = OMedia3DVector(g_phys_ray.end.x, g_phys_ray.end.y, g_phys_ray.end.z, g_phys_ray.end.w);	// 0x6311f8
	else ground = down;
	if (f_1ba8)												// (never set)
	{
		const float h = (float)(f_1ba8->f_602 / 2 - 2);
		const OMedia3DVector q = Position(this);
		static_cast<CGameObject *>(f_1ba8)->v197(Position(this).x, ground.y + h, q.z);	// +0x314
	}
	if (!f_1ba4) return;
	const float hx = p.x - ground.x, hy = p.y - ground.y, hz = p.z - ground.z;
	const float height = sqrtf(hx * hx + hy * hy + hz * hz);
	if (v133() && g_camera && g_camera == this)				// +0x214
	{
		const OMedia3DVector q = Position(this);
		v197(q.x, q.y + 20.0f, q.z);						// +0x314; 0x4caa40
		PlayGameSound(-1, std::string("shrubs"), false);
	}
	float a[4], v[4];
	const float hh = f_1b78;
	if (height < hh * 0.5f)
	{
		v155(a); const float az = a[2]; v155(a);
		v157(a[0], (float)(-g * 12.1), az);					// +0x274; 0x4e2bb0
		v174(v);
		if (v[1] > 2500.0f) { v174(v); const float vz = v[2]; v174(v); v176(v[0], 2500.0f, vz); }	// 0x4e2bac
	}
	else if (height < hh * 0.75f)							// 0x4e2ba8
	{
		v155(a); const float az = a[2]; v155(a);
		v157(a[0], (float)(-g * 3.6), az);					// 0x4e2ba0
		v174(v);
		if (v[1] > 400.0f) { v174(v); const float vz = v[2]; v174(v); v176(v[0], 400.0f, vz); }	// 0x4ccf10
	}
	else if (height < hh)
	{
		v155(a); const float az = a[2]; v155(a);
		v157(a[0], g * -2.0f, az);							// 0x4e2b98
		v174(v);
		if (v[1] > 200.0f) { v174(v); const float vz = v[2]; v174(v); v176(v[0], 200.0f, vz); }
	}
	else if (height < (float)(hh * 4.5))					// 0x4e2b90
	{
		v155(a); const float az = a[2]; v155(a);
		v157(a[0], (float)(g * 0.5), az);					// 0x4cb210
	}
	else
	{
		v155(a); const float az = a[2]; v155(a);
		v157(a[0], (float)(g * 6.2), az);					// 0x4e2b88
		v174(v);
		if (v[1] < -1680.0f) { v174(v); const float vz = v[2]; v174(v); v176(v[0], -1680.0f, vz); }	// 0x4e2b84
		v174(v);
		if (v[1] > 1600.0f) { v174(v); const float vz = v[2]; v174(v); v176(v[0], 1600.0f, vz); }	// 0x4e2b5c
	}
	if (PlayerKeyJump() && f_1b80 <= 0 && height < hh * 3.0f)	// 0x4658a0; 0x4cb1e8
	{
		f_1b80 = 0.5f;
		if (g_difficulty == 2) AddEnergy(-1.0f);
		v174(v);
		float vy = v[1];
		if (vy <= 0) vy = 0;
		v174(v);
		const float vz = v[2];
		v174(v);
		v176(v[0], vy + 900.0f, vz);						// 0x4e2b80
		PlayGameSound(-1, std::string("boing"), false);
	}
}
