/*  C3DFlyingObject - see C3DFlyingObject.h. */
#include "C3DFlyingObject.h"
#include "Mouse.h"
#include "C3DPlayer.h"										// the input tests (PlayerKey*)
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "CGameType.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaTrigo.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360

// --------------------------------------------------------------------------- helpers

using neg::GetMouseDelta;									// 0x4a3990 (Mouse.h)

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// vtable +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0. (The
// generated declaration of v226 has the wrong signature, so it is done here, as in C3DPlayer.)
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

// --------------------------------------------------------------------------- construction

C3DFlyingObject::C3DFlyingObject()							// 0x4346b0
{
	// (f_9a0 and f_9b0 are OMedia3DPoints: w = 1.)
	SetName((char *)"C3DFLYINGOBJECT");
	Log("C3DFlyingObject()");
	v62(0.8f);
	f_960 = 400.0f;											// AccelRate
	f_964 = 1.0f;											// DecelRate
	f_968 = 1400.0f;										// MaxSpeed
	f_984 = 2500.0f;										// MaxHeight
	f_96c = 0;
	f_9a0.y = 200.0f;
	f_9b0.z = 200.0f;
	f_970 = 35.0f;
	f_974 = 0;
	f_978 = 0;
	f_97c = 0;
	f_980 = 0;
	f_9a0.x = 0;
	f_9a0.z = -500.0f;
	f_9b0.x = 0;
	f_9b0.y = 50.0f;
	f_9c4 = 20.0f;											// AccelLean
	f_9c8 = -20.0f;											// DecelLean
	f_988 = 650.0f;											// UpRate
	f_98c = -650.0f;										// DownRate
	f_990 = 650.0f;											// MaxVertVelocity
	f_994 = 0;												// NewGravity
	v77(0, 0, 0);
	f_998 = 1;
	f_99c = 0;
	f_99e = 0;
	f_99f = 0;
	f_005 = 1;
	f_00a = 1;
	v66(100.0f);
	v52(1);
	f_9c0 = 0;
	C3DFlyingObject::InitObject();							// 0x4349d0 (called directly)
	v71(0, 0, 0);
	v157(0, 0, 0);
	f_9cc = 0;
	f_9d0 = 0;
	f_9d4 = 0;
	f_9d8 = 0;
	f_9dc = 0;
	f_9dd = 0;
	f_9de = 0;
	f_9df = 1;
	f_9e0 = 0;
	f_9ec = 0;
	// port: not set by the original (left as the allocation had them)
	f_99d = 0;
	f_9e4 = 0;
	f_9e8 = 0;
}

C3DFlyingObject::~C3DFlyingObject()							// 0x434930
{
	Log("~C3DFlyingObject()");
}

// --------------------------------------------------------------------------- CGameObject family

void C3DFlyingObject::InitObject(void)						// 0x4349d0
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("MaxHeight", &f_984, PROP_FLOAT, 0);
	RegisterProperty("MaxSpeed", &f_968, PROP_FLOAT, 0);
	RegisterProperty("AccelRate", &f_960, PROP_FLOAT, 0);
	RegisterProperty("DecelRate", &f_964, PROP_FLOAT, 0);
	RegisterProperty("UpRate", &f_988, PROP_FLOAT, 0);
	RegisterProperty("DownRate", &f_98c, PROP_FLOAT, 0);
	RegisterProperty("MaxVertVelocity", &f_990, PROP_FLOAT, 0);
	RegisterProperty("NewGravity", &f_994, PROP_FLOAT, 0);
	RegisterProperty("AccelLean", &f_9c4, PROP_FLOAT, 0);
	RegisterProperty("DecelLean", &f_9c8, PROP_FLOAT, 0);
	SetObjectID('3FLY');									// element +0xc4
}

void C3DFlyingObject::ResetObject(void)						// 0x434b10
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	f_978 = 0;
	f_97c = 0;
	f_9c0 = 0;
	f_99c = 0;
	f_99d = 0;
	v52(1);													// vtable +0xd0
	v71(0, 0, 0);											// +0x11c
	v157(0, 0, 0);											// +0x274
	v34(1);													// +0x88
	v176(0, 0, 0);											// +0x2c0
	float r[4];
	v201(r);												// +0x324
	f_980 = r[1];
}

void C3DFlyingObject::v258(void)							// 0x435ea0
{
	CLocalGameObject::v258();
}

void C3DFlyingObject::AddTouching(CGameObject *other)		// 0x435eb0
{
	CGameObject::AddTouching(other);
	if (other->m_object_id == '3PIC')						// a pickup
	{
		if (strcasecmp(other->m_tag, "BARRELROLL") == 0) f_9d4 = 1.2f;
		if (strcasecmp(other->m_tag, "BARRELROLL2") == 0) f_9d4 = 2.4f;
	}
	else if (other->m_object_id == '3AIT')					// an AI trigger
	{
		if (strcasecmp(other->m_tag, "BARRELROLL") == 0) f_9d4 = 1.2f;
		if (strcasecmp(other->m_tag, "BARRELROLL2") == 0) f_9d4 = 2.4f;
	}
}

void C3DFlyingObject::RotateToDest(float dt)				// 0x435b60
{
	if (f_9d4 > 0) return;									// no turning towards the destination while rolling
	if (!GameIsRunning() || !f_00a) return;					// 0x4b0ca0
	f_00a = 0;
	const float want_x = f_274, want_y = f_278, want_z = f_27c;
	float r[4];
	v201(r);
	const float rx = r[0];
	v201(r);
	const float ry = r[1];
	v201(r);
	const float rz = r[2];
	float dx = 0, dy = 0, dz = 0;
	if (want_x != rx)
	{
		dx = want_x - rx;
		if (dx > 180.0f) dx -= 360.0f;
		else if (dx < -180.0f) dx += 360.0f;
		f_00a = 1;
	}
	if (want_y != ry)
	{
		dy = want_y - ry;
		if (dy > 180.0f) dy -= 360.0f;						// (no check for < -180 here, unlike x and z)
		f_00a = 1;
	}
	if (want_z != rz)
	{
		dz = want_z - rz;
		if (dz > 180.0f) dz -= 360.0f;
		else if (dz < -180.0f) dz += 360.0f;
		f_00a = 1;
	}
	AddRotation((float)((double)dx * dt * 2), (float)((double)dy * dt * 4.0f), (float)((double)dz * dt * 3.0f));	// vtable +0x330
}

// --------------------------------------------------------------------------- flight

void C3DFlyingObject::Process(float dt)						// 0x434c30
{
	float side_x = 0, side_z = 0;							// the lean part of the velocity (0 when not moving)
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	if (g_camera != (CGameObject *)this) return;
	float v[4];
	v174(v);												// velocity, vtable +0x2b8
	const float vy0 = v[1];
	f_9cc += dt;
	const float speed = (float)sqrt((double)v[0] * v[0] + (double)v[2] * v[2] + (double)vy0 * vy0);

	if (PlayerKeyTurnAround() || f_9de)
	{
		f_9de = 0;
		ev109();											// element +0x1b4
	}
	else f_9dd = 0;

	if (f_9d8 > 0)											// turning around
	{
		AddRotation(0, dt * 180.0f, 0);
		f_9d8 -= dt;
		if (f_9d8 < 0) f_9d8 = 0;
	}
	else if (speed != 0)
	{
		f_97c = f_978;
		AddRotation(0, f_978 * dt, 0);
	}

	if (f_9d4 > 0)											// barrel roll
	{
		f_9d4 -= dt;
		if (f_9d4 <= 0) f_9d4 = 0;
		if (f_9df) AddRotation(0, 0, dt * 300.0f);
		else AddRotation(0, 0, dt * -300.0f);
	}

	if (speed != 0)
	{
		// the sideways part: (lean, 0, 0) turned by the heading (OMT's rotate inlined; the x
		// angle is 0, so its sine and cosine are the tables' first entries)
		const float lean = (float)((double)f_97c * 0.0181818176f * (1.0 - (double)v145()) * f_96c);	// 0x4dce30
		float r[4];
		v201(r);
		const OMediaCosSinTable &t = OMediaCosSinTable::global;
		const omt_Angle a = (omt_Angle)((int)((double)r[1] * kDegToAngle) & omc_MaxAngleMask);	// 0x4ba850 (_ftol)
		const float s0 = t.sin(0), c0 = t.cos(0);
		const float sa = t.sin(a), ca = t.cos(a);
		const float c0l = c0 * lean;
		const float s0s0l = -(s0 * -(s0 * lean));
		side_x = c0l * ca - s0s0l * sa;
		side_z = s0s0l * ca + c0l * sa;
	}
	OMedia3DVector fwd = ObjectDir(this, 0, 0, f_96c);		// vtable +0x388
	if (f_9d8 <= 0 || m_object_id == '3SUB')				// (the mini sub keeps steering while it turns)
		v176(side_x + fwd.x, vy0, side_z + fwd.z);

	float p[4];
	v195(p);												// position
	const float px = p[0], py = p[1], pz = p[2];
	if (f_96c < 100.0f)										// slow: climb / dive twice as hard
	{
		f_9c0 = 0;
		if (f_99c) f_9c0 = f_988 + f_988;
		const float top = f_984;
		if (!(py < top))
		{
			v197(px, top, pz);								// +0x314: the ceiling
			f_9c0 = 0;
		}
		if (f_99d) f_9c0 = f_98c + f_98c;
	}
	else
	{
		const float top = f_984;
		if (!(py < top))
		{
			v197(px, top, pz);
			f_9c0 = 0;
			if (f_99d) f_9c0 = f_98c;
		}
		else if (f_99c) f_9c0 = f_988;
		else if (f_99d) f_9c0 = f_98c;
		else
		{
			// neither: bring the vertical speed back to 0
			f_9c0 = 0;
			float w[4];
			v174(w);
			if (w[1] <= 0) f_9c0 = 160.0f;
			else f_9c0 = -160.0f;
			if (w[1] < 60.0f && w[1] > -60.0f)
			{
				v175(w[0], 0, w[2], w[3]);					// +0x2bc
				f_9c0 = 0;
			}
		}
	}
	float acc[4];
	v155(acc);
	const float az = acc[2];
	v155(acc);
	v157(acc[0], f_9c0 - g_gravity.y, az);					// gravity cancelled (0x63119c)
	// (debug text "VERT V %d" / "MAX V %d" of the velocity y and f_990: compiled out)
	float w[4];
	v174(w);
	if (w[1] > f_990)
	{
		v174(w);
		const float wz = w[2];
		v174(w);
		v176(w[0], f_990, wz);
	}
	v174(w);
	if (-f_990 > w[1])
	{
		v174(w);
		const float wz = w[2];
		v174(w);
		v176(w[0], -f_990, wz);
	}
}

void C3DFlyingObject::ev109(void)							// 0x434ba0
{
	float v[4];
	v174(v);												// +0xc0 vtable +0x2b8
	if ((float)sqrt((double)v[0] * v[0] + (double)v[1] * v[1] + (double)v[2] * v[2]) == 0) return;
	if (f_9dd) return;
	f_9dd = 1;
	if (f_9d8 <= 0) f_9d8 = 1.0f;
}

OMedia3DVector C3DFlyingObject::ev106(float x, float y, float z)	// 0x435d20 (the same code as C3DPlayer::ev113)
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

// --------------------------------------------------------------------------- controls + camera

void C3DFlyingObject::v243(float dt)						// 0x435230
{
	if (g_gametype->IsPaused()) return;
	short mdx = 0, mdy = 0;
	GetMouseDelta(&mdx, &mdy);								// 0x4a3990
	OMedia3DVector cam(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);	// 2002 +0x44
	const float mx = (float)mdx;
	f_9d0 += (float)mdy * dt;

	// turning
	double turn;											// (x87: kept unrounded until it is added to f_978)
	if (mx == 0)
	{
		if (PlayerKeyLeft())
		{
			if (!IsAnim((char *)"LEFT")) ev58((char *)"LEFT", 0);
			turn = (double)dt * 180.0f + mx;
			f_99e = 1;
			f_9e4 += dt;
			f_9e8 = 0;
		}
		else if (PlayerKeyRight())
		{
			if (!IsAnim((char *)"RIGHT")) ev58((char *)"RIGHT", 0);
			turn = mx - (double)dt * 180.0f;
			f_99e = 1;
			f_9e8 += dt;
			f_9e4 = 0;
		}
		else
		{
			if (f_99e)
			{
				if (!IsAnim((char *)"DEFAULT")) ev58((char *)"DEFAULT", 0);
				if (f_978 < 0)								// ease back to 0
				{
					f_978 = (float)((double)dt * 180.0f + f_978);
					if (f_978 > 0) f_978 = 0;
				}
				else if (f_978 > 0)
				{
					f_978 = (float)(f_978 - (double)dt * 180.0f);
					if (f_978 < 0) f_978 = 0;
				}
			}
			else
			{
				f_9e4 = 0;
				f_9e8 = 0;
			}
			turn = mx;
		}
	}
	else													// the mouse turns
	{
		turn = (double)mx * dt;
		f_99e = 0;
		if (turn > 0)
		{
			f_9e4 = (float)((double)dt * 2.5 + f_9e4);
			f_9e8 = 0;
		}
		else
		{
			f_9e4 = 0;
			f_9e8 = (float)((double)dt * 2.5 + f_9e8);
		}
	}
	f_978 = (float)(turn + f_978);

	// speed
	if (PlayerKeySpeed())
	{
		f_968 = 2100.0f;
		if (f_96c < 2100.0f)
		{
			f_96c = (float)((double)dt * f_960 * 20.0f + f_96c);
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	else if (f_9ec)
	{
		switch (g_difficulty)
		{
			case 0: f_968 = 400.0f; break;
			case 1: f_968 = 700.0f; break;
			case 2: f_968 = 900.0f; break;
		}
		if (f_96c > f_968) f_96c = (float)(f_96c - (double)dt * 30.0f);
	}
	else
	{
		switch (g_difficulty)
		{
			case 0: f_968 = 1000.0f; break;
			case 1: f_968 = 1200.0f; break;
			case 2: f_968 = 1400.0f; break;
		}
	}

	f_99c = 0;
	f_99d = 0;
	if (PlayerKeyGo() || f_9d0 > 15.0f)						// climb (and speed up)
	{
		f_99c = 1;
		if (f_96c < f_968)
		{
			f_96c = (float)((double)dt * f_960 * 20.0f + f_96c);
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	if (!f_99c && f_968 * 0.818181813f < f_96c)				// 0x4dce50: above 9/11 of the top speed, slow down
		f_96c = (float)(f_96c - (double)dt * 300.0f);
	if (v133())												// on the floor
	{
		f_9cc = 0;
		if (f_96c > 0)
		{
			f_96c = (float)(f_96c - (double)dt * 500.0f);
			if (f_96c < 0) f_96c = 0;
		}
		else f_96c = 0;
	}
	if (PlayerKeyBack() || f_9d0 < -15.0f)					// dive (and brake)
	{
		f_99d = 1;
		if (f_96c > 0)
		{
			if (v133()) f_96c = (float)(f_96c - (double)dt * f_964);
			else f_96c = (float)(f_96c - (double)f_964 * 0.1f);	// (per frame, not per second)
			if (f_96c < 0) f_96c = 0;
		}
		else if (f_96c < 0)
		{
			f_96c = f_964 + f_96c;
			if (f_96c > 0) f_96c = 0;
		}
	}
	if (f_99c) f_99d = 0;
	if (f_9d0 > 10.0f) f_9d0 = (float)(f_9d0 - (double)dt * 20.0f);
	if (f_9d0 < -10.0f) f_9d0 = (float)((double)dt * 20.0f + f_9d0);

	if (f_9e8 > 1.5f || f_9e4 > 1.5f)						// held for a while: turn faster
	{
		if (f_978 < -80.0f) f_978 = -80.0f;
		else if (f_978 > 80.0f) f_978 = 80.0f;
	}
	else
	{
		if (f_978 < -45.0f) f_978 = -45.0f;
		else if (f_978 > 45.0f) f_978 = 45.0f;
	}

	// pitch and roll the model (destination rotation), and tell the engine
	float r[4];
	if (f_99c)
	{
		v201(r);
		v218(f_9c4, r[1], -f_978);							// +0x368
		ev108(2);											// element +0x1b0
	}
	else if (f_99d)
	{
		v201(r);
		v218(f_9c8, r[1], -f_978);
		ev108(0);
	}
	else if (!(f_96c > 0))
	{
		v201(r);
		v218(0, r[1], 0);
		ev108(0);
	}
	else
	{
		if (f_9dc)
		{
			f_9dc = 0;
			PlayGameSound(-1, std::string("barrelroll"), false);	// 0x48f420
			if (f_9d4 <= 0) f_9d4 = 0.7f;
		}
		if (!(f_9d4 > 0))
		{
			v201(r);
			v218(0, r[1], -f_978);
		}
		ev108(1);
	}

	// the chase camera
	OMedia3DVector want = v224(f_9a0.x, f_9a0.y, f_9a0.z);	// +0x380
	const float k = ev96(Position(this), &want, 1400.0f);	// element +0x180: kept within reach
	OMedia3DVector look = v224(f_9b0.x, f_9b0.y, f_9b0.z);
	const float k2 = ev95(look, &want);						// element +0x17c: in front of walls
	if (k2 > 0)
		ev98(dt, k2, k, cam, want, look, OMedia3DVector(1.2f, 1.2f, 1.2f, 0.0f));	// element +0x188
}
