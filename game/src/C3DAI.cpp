/*  C3DAI - see C3DAI.h. Reconstructed from Neutron2.exe 0x410b30..0x4159b0, 0x42d280. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DAI.h"
#include "C3DGrannyObj.h"
#include "objects/C3DPatrolPoint.h"
#include "Game.h"
#include "Bridge.h"
#include "Physics.h"
#include "CGameType.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaTrigo.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360
static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x4af000: the object whose save index (f_438) is <index>.
static CGameObject *FindObjectByIndex(int index)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && (int)(*i)->f_438 == index) return *i;
	return NULL;
}

// 0x48fcf0: count *t down by dt; true (and *t = 0) once it is over, or if it was not running.
static bool CountDown(float *t, float dt)
{
	if (*t > 0)
	{
		*t -= dt;
		if (*t > 0) return false;
		*t = 0;
	}
	return true;
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// vtable +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

// CGameObject slot 226 (0x4ad990), also inlined by ev115 and AddTouching: (x, y, z) turned by
// the object's rotation, w = 0. (The generated declaration of v226 has the wrong signature.)
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

// The length of a difference vector as the original computes it (x87: one rounding at the end).
static float Length(float x, float y, float z)
{
	return (float)sqrt((double)x * x + (double)y * y + (double)z * z);
}

// The patrol point behind a CGameObject (the original subtracts 0xc8 once NameContains or the
// '3PAT' ObjectID said so; NULL for anything else).
static C3DPatrolPoint *PatrolPoint(CGameObject *o)
{
	return o ? dynamic_cast<C3DPatrolPoint *>(o) : NULL;
}

// The granny prop behind a CGameObject that NameContains("C3DGRANNYOBJ").
static C3DGrannyObj *GrannyObj(CGameObject *o)
{
	return o ? dynamic_cast<C3DGrannyObj *>(o) : NULL;
}

// A patrol point of the type-1 network: ObjectID '3PAT', PatrolType 1 (element +0x8c4).
static C3DPatrolPoint *NetworkPoint(CGameObject *o)
{
	if (!o || o->m_object_id != '3PAT') return NULL;
	C3DPatrolPoint *p = static_cast<C3DPatrolPoint *>(o);
	return p->m_patrol_type == 1 ? p : NULL;
}

// Entry <i> of the visible-point list of a patrol point (+0x8c8) or of an AI (+0x1a8c), as
// ev134 reads it; *known = false for anything else (the original then reads nothing).
static CGameObject *VisiblePoint(CGameObject *o, int i, bool *known)
{
	*known = true;
	if (o->NameContains((char *)"C3DPATROLPOINT")) return static_cast<C3DPatrolPoint *>(o)->m_visible_points[i].point;
	if (o->NameContains((char *)"C3DAI"))
	{
		C3DAI *ai = dynamic_cast<C3DAI *>(o);
		if (ai) return ai->f_1a8c[i].point;
		return NULL;										// port: "C3DAI..." that is not one (a C3DAITRIGGER); the original reads its memory
	}
	*known = false;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DAI::C3DAI()												// 0x410b30
{
	SetName((char *)"C3DAI");
	Log("C3DAI()");
	v64(0.3f);
	C3DObject::SetObjectID('3DAI');							// element slot 49, called directly
	v67(150.0f);											// radius
	v38(1);
	f_960 = 0;
	f_968 = 200.0f;
	f_1b3c = 200.0f;										// InitialSpeed
	f_964 = NULL;
	f_974 = '3JIM';
	f_005 = 1;
	f_00a = 1;
	f_96c = 1;
	f_c08 = 1;												// AIState
	f_970 = -1;
	f_978.x = f_978.y = f_978.z = 0;
	f_988.x = f_988.y = f_988.z = 0;
	f_998 = 0;
	f_99c = 0;
	f_9a0 = 0;
	f_9a4 = 0;
	f_9a8 = 2500.0f;										// VisibleRange
	f_b98 = 90.0f;											// FOV
	v77(0, 1.0f, 1.0f);
	f_b9c.x = 0;
	f_b9c.z = 0;
	v38(1);
	strcpy(f_9ac, "none");
	f_bac = 0;
	f_bb0 = 0;
	v44(0);
	strcpy(f_a9c, "WAG");
	strcpy(f_ac4, "RUN");
	strcpy(f_aec, "FLY");
	strcpy(f_b14, "SIT");
	strcpy(f_a74, "none");
	strcpy(f_b3c, "WALK");
	strcpy(f_b64, "WALK");
	strcpy(f_a10, "JIM1");									// TargetName
	f_b8c = NULL;
	f_b9c.x = 1.0f;											// turn rates
	f_b9c.y = 3.0f;
	f_b9c.z = 2.0f;
	f_bb4.x = f_bb4.y = f_bb4.z = 0;
	f_bc4.x = f_bc4.y = f_bc4.z = 0;
	f_bd4 = 0;
	f_bd5 = 0;
	f_bd8 = 0;
	f_bdc = 0;
	f_be0 = 1;
	f_bf4 = NULL;
	f_be4.x = f_be4.y = f_be4.z = 0;
	f_bf8 = 0;
	f_bfc = 0;
	f_c00 = 0;
	f_c10 = 1500.0f;
	f_c2c = 1500.0f;										// WanderRange
	f_658 = 1;												// HasCollision
	f_c48 = 1;
	f_c04 = 0;
	f_c18 = 0;
	f_b94 = NULL;
	f_c14 = 110.0f;
	f_c1c = 0;
	f_c20 = 0;
	f_c24 = 0;
	f_c28 = 0;
	f_c40 = 0;
	f_c34 = 0;
	f_c38 = 0;
	f_c3c = 0;
	f_c44 = 0;
	f_c5c = 0;
	f_c4c.x = f_c4c.y = f_c4c.z = 0;
	f_c5d = 0;
	f_c60 = 150.0f;
	f_c64 = 0;
	f_1b2c = 0;
	f_1b30 = 0;
	f_1b40 = 0;												// CanHover
	f_1b44 = 3500.0f;
	for (int i = 0; i < 300; i++)
	{
		f_112c[i] = NULL;
		f_15dc[i] = NULL;
	}
	f_1b38 = NULL;
	f_1b34 = 0;
	f_1b48 = 0;												// GroupID
	f_1b4c = 0;
	f_c0c = -1;												// FallbackAIState
	f_c78 = 0;												// port: not set by the original until FindBestPath runs
	f_c30 = 0;												// port: likewise (only written by RotateToDest)
	for (int i = 0; i < 20; i++) { f_1a8c[i].point = NULL; f_1a8c[i].dist = 0; }	// port: likewise (ev136)
}

C3DAI::~C3DAI()												// 0x411060
{
}

namespace neg {
CGameObject *New_C3DAI(void) { return new C3DAI; }							// '3DAI' (0x1b58 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DAI::InitObject(void)								// 0x4110e0
{
	Log("InitObject()");
	C3DAnimated::InitObject();
	RegisterProperty("PatrolPoint", f_9ac, PROP_ALPHA, 0);
	RegisterProperty("VisibleRange", &f_9a8, PROP_FLOAT, 0);
	RegisterProperty("FOV", &f_b98, PROP_FLOAT, 0);
	RegisterProperty("TargetName", f_a10, PROP_ALPHA, 0);
	RegisterProperty("AIState", &f_c08, PROP_LONG, 0);
	RegisterProperty("FallbackAIState", &f_c0c, PROP_LONG, 0);
	RegisterProperty("WanderRange", &f_c2c, PROP_FLOAT, 0);
	RegisterProperty("InitialSpeed", &f_1b3c, PROP_FLOAT, 0);
	RegisterProperty("CanHover", &f_1b40, PROP_LONG, 0);
	RegisterProperty("GroupID", &f_1b48, PROP_LONG, 0);
}

void C3DAI::ResetObject(void)								// 0x4110b0
{
	CLocalGameObject::ResetObject();
	f_00a = 1;
	f_96c = f_c08;
	f_b8c = NULL;
	f_b94 = NULL;
}

void C3DAI::AddTouching(CGameObject *other)					// 0x415150
{
	CGameObject::AddTouching(other);
	other->NameContains((char *)"C3DBASEBALL");				// (result unused)
	if (f_c5d && (other == g_camera || other->NameContains((char *)"C3DGODDARD")))
	{
		f_c5c = 1;											// held where we are
		float p[4];
		v195(p);
		f_c4c.x = p[0]; f_c4c.y = p[1]; f_c4c.z = p[2]; f_c4c.w = p[3];
		ev119();											// element +0x1dc
	}
	if (other == FindObjectByTag(f_a10)) f_1b4c = 1;
	if (!f_964 || other != f_964) return;
	if (f_96c == 4)
	{
		f_998 = 0;
		f_bb0 = 1;
		const OMedia3DVector v = ObjectDir(f_964, f_978.x, f_978.y, -f_978.z);	// (z negated: the other side)
		const OMedia3DVector t = Position(f_964);
		f_bb4.x = v.x + t.x;
		f_bb4.y = v.y + t.y;
		f_bb4.z = v.z + t.z;
		const OMedia3DVector p = Position(this);
		f_988.x = f_bb4.x - p.x;
		f_988.y = f_bb4.y - p.y;
		f_988.z = f_bb4.z - p.z;
		f_9a0 = Length(f_988.x, f_988.y, f_988.z);
	}
	if (f_96c == 6) ev107(0);
}

void C3DAI::RemoveTouching(CGameObject *other)				// 0x415110
{
	CGameObject::RemoveTouching(other);
	if (f_c5d && (other == g_camera || other->NameContains((char *)"C3DGODDARD"))) f_c5c = 0;
}

void C3DAI::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x4144d0
{
	CGameObject::v220(a, b, c, d, poly);
	if (f_96c == 10 || f_96c == 11) ev129(500.0f, OMedia3DVector(a, b, c, d));	// away from the wall
}

void C3DAI::Process(float dt)								// 0x411240
{
	if (g_gametype->IsPaused())								// +0x114
	{
		v176(0, 0, 0);
		if (f_c5d && f_c5c) v196(f_c4c.x, f_c4c.y, f_c4c.z, f_c4c.w);
		return;
	}
	C3DAnimated::Process(dt);
	f_960 += dt;
	f_bac += dt;
	float acc[4];
	v155(acc);												// (read twice by the original: z, then x)
	if (f_bd4)												// hovering: gravity cancelled
		v157(acc[0], -(g_enemy_time_scale * g_gravity.y), acc[2]);
	else if (f_96c == 3)
		v157(acc[0], g_gravity.y + g_gravity.y, acc[2]);
	else
		v157(acc[0], g_gravity.y * 10.0f, acc[2]);

	if (g_camera && g_camera->NameContains((char *)"C3DJIMMY") && static_cast<CLocalGameObject *>(g_camera)->f_4ab)
	{
		v175(0, 0, 0, 0);									// Jimmy is frozen (cut scene): so are we
		return;
	}
	if (f_964) ev115();
	if (f_bf8 > 0)
	{
		f_bf8 -= dt;
		if (f_bf8 <= 0)										// ev123's time is over: back to what we did
		{
			ev107(f_be0);
			f_964 = f_bf4;
			f_978.x = f_be4.x;
			f_978.y = f_be4.y;
			f_978.z = f_be4.z;
			f_bf8 = 0;
			ev124();
		}
	}
	if (f_c00 > 0)
	{
		f_c00 -= dt;
		if (f_c00 <= 0) f_c00 = 0;
	}

	switch (f_96c)
	{
		case 0:												// attack
			if (f_964)
			{
				ev120();
				if (static_cast<CLocalGameObject *>(f_964)->f_4a9)
				{
					ev111(f_964);
					ev108();
				}
				else if (f_c08 == 6)
				{
					ev107(6);
					ev107(11);								// (as shipped: 6 is replaced at once)
				}
				else if (f_c08 == 11) ev107(11);
				else ev107(2);
			}
			if (f_9a0 > f_9a8)
			{
				if (f_c08 == 6) ev107(6);
				else ev107(2);
			}
			break;

		case 1:
			v176(0, 0, 0);
			v172(0, 0, 0);
			break;

		case 2:
			if (CanSeeTarget()) ev107(0);
			// fall through
		case 3:
		case 13:
			ev137(dt);
			break;

		case 4:
		follow:
			if (f_964 && static_cast<CLocalGameObject *>(f_964)->f_4a9)
			{
				ev118();
		chase:
				ev111(f_964);
				ev108();
			}
			break;

		case 5:												// flee
			if (f_964)
			{
				ev122();
				ev111(f_964);
				ev108();
			}
			if (f_c3c > 0)
			{
				// (debug text of the time left)
				f_c3c -= dt;
				if (f_c3c <= 0)
				{
					f_c3c = 0;
					if (!f_c40)
					{
						if (f_c0c != -1) ev107(f_c0c);
						else ev107(f_c08);
					}
				}
			}
			if (f_9a0 > f_c10)
			{
				if (f_c1c > 0) v261();
				else if (f_c0c != -1) ev107(f_c0c);
				else ev107(f_c08);
			}
			break;

		case 6:												// scan
			v172(0, 0, 0);
			if (CanSeeTarget())
			{
				// (debug log "going to Attacking")
				ev107(0);
			}
			break;

		case 7:												// waiting at a patrol point
			v172(0, 0, 0);
			v176(0, 0, 0);
			if (f_bd8 > 0)
			{
				f_bd8 -= dt;
				if (f_bd8 <= 0)
				{
					f_bd8 = 0;
					ev58((char *)"WALK", 1);
					const int prev = f_970;
					ev107(prev);
					f_998 = 0;
					if (!f_b8c) break;						// port: the original calls through NULL
					if (!f_b8c->NameContains((char *)"C3DPATROLPOINT"))
					{
						ErrorLog("ERROR: invalid NextPatrol point for %s", f_b8c->m_tag);
						break;
					}
					C3DPatrolPoint *pp = static_cast<C3DPatrolPoint *>(f_b8c);
					if (prev == 3 && strcasecmp(pp->m_attach_obj, "none") != 0)
					{
						// patrolling: put down what the point gave us (a granny prop)
						CGameObject *o = FindObjectByTag(pp->m_attach_obj);
						if (o && o->NameContains((char *)"C3DGRANNYOBJ"))	// port: o checked (the original calls through NULL)
						{
							C3DGrannyObj *g = GrannyObj(o);
							if (g && g->f_6d0 == 1 && f_6d0 == 1) g->ev98(this, 0);	// element +0x188
						}
					}
					if (f_96c == 18)
					{
						if (!f_1b4c) break;
						f_1b4c = 0;
					}
					f_b8c = FindObjectByTag(static_cast<C3DPatrolPoint *>(f_b8c)->m_next_patrol_point);
				}
			}
			break;

		case 8:												// hidden
			v176(0, 0, 0);
			v132(0);
			HideAgain();									// element +0xdc
			break;

		case 9:
			v172(0, 0, 0);
			break;

		case 10:											// wander
		case 11:
			{
				float p[4];
				v195(p);									// (result unused)
			}
			f_c24 += dt;
			ev118();
			if (f_c24 > 11.0f) ev129(200.0f, OMedia3DVector(0, 0, 0, 0));
			if (!f_c20)
			{
				f_c20 = 1;
				ev129(f_c2c, OMedia3DVector(0, 0, 0, 0));
			}
			if (f_96c == 11 && CanSeeTarget())
			{
				ev107(0);
				break;
			}
			goto chase;

		case 12:
			if (f_964) ev131(f_964);
			break;

		case 14:
			v176(0, 0, 0);
			if (CanSeeTarget())
			{
				ev138(1);
				f_1b30 = 3.0f;
			}
			break;

		case 15:
			if (CountDown(&f_1b30, dt) && CanSeeTarget())
			{
				ev138(1);
				f_1b30 = 3.0f;
			}
			ev137(dt);
			break;

		case 16:											// search
			if (CanSeeTarget())
			{
				// (debug log "MEBTEMP I see Jimmy!")
				ev107(17);
				break;
			}
			// (debug log "TimeToLook %f")
			if (CountDown(&f_1b30, dt))
			{
				f_1b30 = 7.0f;
				CGameObject *pat = ev143(Position(f_964), 1);
				if (pat)
				{
					if (f_1b38 != pat)
					{
						f_1b38 = pat;
						ev142(0);
						ev137(dt);
						break;
					}
				}
				else
				{
					pat = ev143(Position(f_964), 0);
					if (f_1b38 != pat)
					{
						f_1b38 = pat;
						ev142(0);
					}
				}
			}
			ev137(dt);
			break;

		case 17:
			if (!CanSeeTarget())
			{
				ev107(16);
				ev118();
				f_1b30 = 0;
				break;
			}
			goto follow;

		case 18:
			ev137(dt);
			break;
	}
	if (f_c5d && f_c5c) v196(f_c4c.x, f_c4c.y, f_c4c.z, f_c4c.w);
	ev112(dt);
}

void C3DAI::v242(float t)									// 0x47e3c0 (shared with C3DWaterGun)
{
	C3DAnimated::v242(t);
}

void C3DAI::RotateToDest(float dt)							// 0x42d280
{
	if (!GameIsRunning() || !f_005 || !f_00a) return;		// 0x4b0ca0
	f_00a = 0;
	float dx = 0, dy = 0;
	double dz = 0;											// (x87: kept on the stack, never rounded)
	if (f_274 != f_224)
	{
		dx = f_274 - f_224;
		if (dx > 180.0f) { dx -= 360.0f; f_00a = 1; }
		else
		{
			if (dx < -180.0f) dx += 360.0f;
			f_00a = 1;
		}
	}
	if (f_278 != f_228)
	{
		float t = f_278 - f_228;
		dy = t;
		if (t > 180.0f) { t -= 360.0f; dy = t; }
		else if (t < -180.0f) { t += 360.0f; dy = t; }
		f_c30 = t;
		f_00a = 1;
	}
	if (f_27c != f_22c)
	{
		dz = (double)f_27c - f_22c;
		if (dz > 180.0f) dz -= 360.0f;
		else if (dz < -180.0f) dz += 360.0f;
		f_00a = 1;
	}
	AddRotation((float)((double)dx * f_b9c.x * dt), (float)((double)dy * f_b9c.y * dt), (float)(dz * f_b9c.z * dt));	// vtable +0x330
}

void C3DAI::v258(void)										// 0x414380
{
	f_96c = f_c08;
	v260();													// (a tail jump through the vtable)
}

void C3DAI::v260(void)										// 0x4143a0
{
	C3DPlayerObject::v260();
	f_964 = FindObjectByTag(f_a10);
	if (f_964)
	{
		f_974 = f_964->m_object_id;
		const OMedia3DVector t = Position(f_964);
		f_bb4.x = t.x;
		f_bb4.y = t.y;
		f_bb4.z = t.z;
	}
	f_c08 = f_96c;
	ev107(f_96c);
	float r[4];
	v91(r);													// the rotation it was placed with
	v202(r[0], r[1], r[2], r[3]);
	v91(r);
	f_274 = r[0];											// ... is also where it turns to
	f_278 = r[1];
	f_27c = r[2];
	f_280 = r[3];
	f_968 = f_1b3c;											// InitialSpeed
	if (f_1b40) f_bd4 = 1;									// CanHover
}

void C3DAI::v261(void)										// 0x415450
{
	v176(0, 0, 0);
}

void C3DAI::v265(float a, float b, float c, float d)		// 0x414540
{
	CLocalGameObject::v265(a, b, c, d);
	// (debug log "Hitwall called by %s")
	if (f_bd4)												// hovering: rise over it
	{
		f_bb0 = 1;
		const OMedia3DVector p = Position(this);
		f_bb4.x = p.x;
		f_bb4.y = Position(this).y + 500.0f;
		f_bb4.z = Position(this).z;
	}
	if (f_96c == 10 || f_96c == 11) ev129(500.0f, OMedia3DVector(a, b, c, d));
}

void C3DAI::WriteState(OMediaStreamOperators &s)			// 0x415760
{
	C3DPlayerObject::WriteState(s);
	float v[4], a, b, c;
	v195(v); a = v[0];
	v195(v); b = v[1];
	v195(v); c = v[2];
	s << a << b << c;
	v201(v); a = v[0];
	v201(v); b = v[1];
	v201(v); c = v[2];
	s << a << b << c;
	s << f_96c;
	s << f_970;
	s << f_968;
	s << f_bd8;
	s << f_bb4.x;
	s << f_bb4.y;
	s << f_bb4.z;
	s << f_988.x;
	s << f_988.y;
	s << f_988.z;
	int index = -1;
	if (f_b8c && f_b8c->NameContains((char *)"C3DPATROLPOINT"))
	{
		index = (int)f_b8c->f_438;
		// (debug log "NextPP=%s  index=%d")
	}
	// else (debug log "MEBTEMP saving none for %s")
	s << index;
}

void C3DAI::ReadState(OMediaStreamOperators &s)				// 0x4155e0
{
	C3DPlayerObject::ReadState(s);
	float a, b, c;
	s >> a >> b >> c;
	v197(a, b, c);											// position
	s >> a >> b >> c;
	v203(a, b, c);											// rotation
	v218(a, b, c);
	s >> f_96c;
	s >> f_970;
	s >> f_968;
	s >> f_bd8;
	s >> f_bb4.x;
	s >> f_bb4.y;
	s >> f_bb4.z;
	s >> f_988.x;
	s >> f_988.y;
	s >> f_988.z;
	int index;
	s >> index;
	if (index == -1)
	{
		f_b8c = NULL;
		return;
	}
	// (debug log "NextIndex is %d")
	f_b8c = FindObjectByIndex(index);
	if (!f_b8c) ErrorLog("ERROR: Invalid NextPatrolPoint for %s in ::LoadInfo", m_tag);
}

// --------------------------------------------------------------------------- seeing

unsigned char C3DAI::CanSeeTarget(void)						// 0x414a60
{
	CGameObject *t = f_964;
	if (!t) return 0;
	if (!static_cast<CLocalGameObject *>(t)->f_4a9 && !(f_c00 > 0)) return 0;
	OMedia3DVector d;
	{
		const OMedia3DVector a = Position(t), b = Position(this);
		d.x = a.x - b.x;
		d.y = a.y - b.y;
		d.z = a.z - b.z;
		d.w = 0;
	}
	if (!(Length(d.z, d.y, d.x) < f_9a8)) return 0;		// VisibleRange
	if (f_b98 <= 358.0f)									// FOV: not all round
	{
		const float inv = 1.0f / d.quick_magnitude();
		const OMedia3DVector f = ObjectDir(this, 0, 0, 1.0f);	// vtable +0x388 (v226): our heading
		const float n0 = inv * d.x, n1 = inv * d.y, n2 = inv * d.z;
		const double dot = (double)f.z * n2 + (double)f.y * n1 + (double)f.x * n0;
		double lim;
		if (f_b98 > 180.0f) lim = -(((double)f_b98 - 180.0f) * 0.0055555557f);	// 0x4cb220: 1 / 180
		else lim = 1.0f - (double)f_b98 * 0.0055555557f;
		// The original then also tests an uninitialised local byte ([esp+0xf]) and only gives up
		// when it is 0. port: taken as 0.
		if (dot <= lim) return 0;
	}
	if (f_c34) return 1;
	OMedia3DVector a = Position(this), b = Position(t);
	a.y += 200.0f;											// 0x4cb21c: eye height
	b.y += 200.0f;
	return !PhysicsLineBlocked(a, b, false);				// 0x4b7170
}

unsigned char C3DAI::ev132(OMedia3DVector a, OMedia3DVector b)	// 0x412670
{
	return !PhysicsLineBlocked(a, b, false);
}

// --------------------------------------------------------------------------- states

void C3DAI::ev106(char *dst, char *src)						// 0x411210
{
	strcpy(dst, src);
}

void C3DAI::ev107(int state)								// 0x412240
{
	if (f_c40)
	{
		// (debug log "State %d not set for %s")
		return;
	}
	f_970 = f_96c;
	f_96c = state;
	float r[4];
	v76(r);													// the initial f_194 (friction) back
	v171(r[0], r[1], r[2], r[3]);
	if (f_96c == f_970) return;
	switch (f_96c)
	{
		case 0:
			// (debug log "Setting to ATTACKing state for %s")
			if (f_c38 == 0 && !f_c40)
			{
				ev127();
				ev120();
			}
			break;
		case 1:
			v172(0, 0, 0);
			// (debug log "Setting to waiting state for %s")
			ev119();
			break;
		case 2:
			// (debug log "1" for the C3DGODDARD tag)
		case 4:
		case 16:
		case 17:
			ev118();
			break;
		case 3:
		case 10:
		case 11:
			ev118();
			break;
		case 6:
			// (debug log "Setting to Scanning state for %s")
			if (!f_c40)
			{
				ev119();
				// (debug log "Setting seq to AnimWait for %s %s", twice)
				v261();
			}
			break;
		case 9:
			v172(0, 0, 0);
			break;
	}
}

void C3DAI::ev108(void)										// 0x412470
{
	if (f_9a0 < f_c14)
	{
		f_998 = 1;
		ev113();
		if (f_bb0)
		{
			f_bb0 = 0;
			v42(1);
			v40(1);
		}
		return;
	}
	if (f_998)
	{
		if (f_96c == 4)
		{
			if ((double)f_c14 * 1.5 < f_9a0)				// 0x4cb1d0
			{
				f_998 = 0;
				ev119();
			}
			return;
		}
		if (f_9a0 <= f_c14) return;
	}
	f_998 = 0;
}

void C3DAI::ev109(void)										// 0x413790
{
	if (f_b8c == NULL)
	{
		const int s = f_96c;
		if (s != 12)
		{
			if (s == 13 || s == 15 || s == 16)				// walking a route
			{
				f_b8c = f_112c[f_1b2c];
				if (f_b8c)
				{
					// (debug log "MEBTEMP going to next PP %s")
					f_1b2c++;
				}
				else if (s == 15 || s == 14)
				{
					ev107(14);
					f_b8c = NULL;
				}
				else if (s == 16)
				{
					// (debug log "1 MEBTEMP Got to end")
					f_1b38 = ev143(Position(f_964), 1);
					ev142(1);
				}
				else
				{
					ev107(1);
					f_b8c = NULL;
				}
			}
			else f_b8c = FindObjectByTag(f_9ac);
			if (f_c18 && f_b8c && f_b8c->NameContains((char *)"C3DPATROLPOINT")) f_c14 = 440.0f;
		}
	}
	if (f_b8c)
	{
		ev110();
		return;
	}
	const int s = f_96c;
	if (s == 5 || (s > 13 && s <= 17)) return;
	ev107(1);
	float v[4];
	v174(v);
	v176(0, v[1], 0);										// stop, but keep falling
}

// The direction and distance to the destination point (the tail of ev110 and ev115).
static void DestDir(C3DAI *ai)
{
	const OMedia3DVector p = Position(ai);
	ai->f_988.x = ai->f_bb4.x - p.x;
	ai->f_988.y = ai->f_bb4.y - p.y;
	ai->f_988.z = ai->f_bb4.z - p.z;
	ai->f_9a0 = Length(ai->f_988.x, ai->f_988.y, ai->f_988.z);
}

void C3DAI::ev110(void)										// 0x413930
{
	if (!f_bb0)
	{
		const OMedia3DVector p = Position(f_b8c);
		f_bb4.x = p.x;
		f_bb4.y = p.y;
		f_bb4.z = p.z;
	}
	DestDir(this);
}

void C3DAI::ev111(CGameObject *dest)						// 0x413c50
{
	if (!dest) return;
	OMedia3DVector n = f_988;
	const float inv = 1.0f / n.quick_magnitude();
	n.x *= inv;
	n.y *= inv;
	const double nz = (double)inv * n.z;
	double m = 1.0f;
	if (f_96c == 4)											// following: hurry when far behind
	{
		m = f_9a0 > 1000.0f ? 1.7f : 1.0f;
		if (f_9a0 > 2000.0f) m = 3.0f;
	}
	m *= f_968;
	if (f_96c == 5)
	{
		n.x = (float)-(m * n.x);
		n.z = (float)-(m * nz);								// (as shipped: y is not scaled when fleeing)
	}
	else
	{
		n.x = (float)(m * n.x);
		n.y = (float)(m * n.y);
		n.z = (float)(m * nz);
	}
	if (f_998)
	{
		v176(0, 0, 0);
		ev119();
		return;
	}
	if (f_c18)												// along our heading instead
	{
		const OMedia3DVector f = ObjectDir(this, 0, 0, 1.0f);	// vtable +0x388
		v176(f.x * f_968, f.y * f_968, f.z * f_968);
		return;
	}
	v176(n.x, n.y, n.z);
}

// The yaw/pitch towards f_988 in degrees (+180 when fleeing) and the roll, as ev112 and ev128 do.
static void FacingAngles(C3DAI *ai, float *rx, float *ry)
{
	omt_Angle a1, a2;
	ai->f_988.angles(a1, a2);								// 0x4ca124
	*rx = (float)a1 * kAngleToDeg;
	if (ai->f_96c == 5) *ry = (float)a2 * kAngleToDeg + 180.0f;
	else *ry = (float)a2 * kAngleToDeg;
}

void C3DAI::ev112(float dt)									// 0x413f50
{
	if (f_bdc)
	{
		if (f_968 > 1.0f) f_bdc = 0;
		return;
	}
	const int s = f_96c;
	if (s == 9 || s == 1 || s == 6 || s == 7) return;
	{
		const OMedia3DVector p = Position(this);			// (the same code for state 12 and the rest)
		f_988.x = f_bb4.x - p.x;
		f_988.y = f_bb4.y - Position(this).y;
		f_988.z = f_bb4.z - Position(this).z;
	}
	float rx, ry;
	FacingAngles(this, &rx, &ry);
	if (!v133())											// in the air
	{
		v218(rx, ry, f_c64 ? f_228 - f_278 : 0.0f);
		ev117();
		return;
	}
	if (!f_998)
	{
		v218(0, ry, 0);
		return;
	}
	if (f_964 && s != 3 && s != 2 && s != 6) ev116();
	f_99c += dt;
	if (s != 3 && s != 2 && s != 6) ev114();
	if (f_c38 != 0) return;
	if (f_99c > 7.0f) ev119();								// 0x4cb1fc
	else ev121();
}

void C3DAI::ev113(void)										// 0x414630
{
	if (f_96c == 10 || f_96c == 11) ev129(1400.0f, OMedia3DVector(0, 0, 0, 0));
}

void C3DAI::ev115(void)										// 0x414d00
{
	const OMedia3DVector v = ObjectDir(f_964, f_978.x, f_978.y, f_978.z);
	if (f_96c != 10 && f_96c != 11 && !f_bb0)
	{
		f_bb4.x = v.x + Position(f_964).x;
		f_bb4.y = v.y + Position(f_964).y;
		f_bb4.z = v.z + Position(f_964).z;
	}
	DestDir(this);
}

void C3DAI::ev116(void)										// 0x414300
{
	float r[4];
	f_964->v201(r);
	float y;
	if (r[1] <= 180.0f) y = r[1] + 180.0f;
	else y = r[1] - 180.0f;
	v218(0, y, 0);
}

void C3DAI::ev117(void) { if (!IsAnim(f_aec) && f_c38 == 0) ev58(f_aec, 1); }	// 0x4141e0
void C3DAI::ev118(void) { if (!IsAnim(f_ac4) && f_c38 == 0) ev58(f_ac4, 1); }	// 0x414210
void C3DAI::ev119(void) { if (!IsAnim(f_b14) && f_c38 == 0) ev58(f_b14, 1); }	// 0x414240
void C3DAI::ev120(void) { if (!IsAnim(f_b3c) && f_c38 == 0) ev58(f_b3c, 1); }	// 0x4142a0
void C3DAI::ev121(void) { if (!IsAnim(f_a9c) && f_c38 == 0) ev58(f_a9c, 1); }	// 0x414270
void C3DAI::ev122(void) { if (!IsAnim(f_b64) && f_c38 == 0) ev58(f_b64, 1); }	// 0x4142d0

void C3DAI::ev123(CGameObject *o, float time)				// 0x414f30
{
	if (f_bf8 > 0) return;
	f_be0 = f_96c;
	f_bf4 = f_964;
	f_be4 = f_978;
	ev107(0);
	f_978.x = 0;
	f_964 = o;
	f_978.y = 0;
	f_978.z = 0;
	f_bf8 = time;
}

void C3DAI::ev125(float dt)									// 0x414fd0
{
	f_bfc += dt;
	if (!(f_bfc > 5.0f)) return;							// 0x4caa90
	f_bfc = 0;
	if (f_9a4 - 10.0f <= f_9a0 && f_9a4 + 10.0f > f_9a0)	// hardly moved in 5 s
	{
		CGameObject *o;
		if (f_b8c && f_b8c->NameContains((char *)"C3DPATROLPOINT"))
		{
			f_b8c = FindObjectByTag(static_cast<C3DPatrolPoint *>(f_b8c)->m_next_patrol_point);	// skip it
			f_9a4 = f_9a0;
			return;
		}
		o = FindObjectByTag(f_9ac);
		if (o)
		{
			float p[4];
			o->v195(p);
			v196(p[0], p[1], p[2], p[3]);					// back to the start of the patrol
		}
	}
	f_9a4 = f_9a0;
}

void C3DAI::ev127(void)										// 0x415440
{
	f_998 = 0;
	f_99c = 0;
}

void C3DAI::ev128(float unused)								// 0x413e20
{
	(void)unused;
	{
		const OMedia3DVector p = Position(this);			// (only f_988: no distance here)
		f_988.x = f_bb4.x - p.x;
		f_988.y = f_bb4.y - Position(this).y;
		f_988.z = f_bb4.z - Position(this).z;
	}
	float rx, ry;
	FacingAngles(this, &rx, &ry);
	if (!v133())
	{
		v218(rx, ry, f_c64 ? f_228 - f_278 : 0.0f);
		return;
	}
	if (f_998) return;
	v218(rx, ry, 0);
}

void C3DAI::ev129(float range, OMedia3DVector away)			// 0x414660
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	const unsigned int a = *seed >> 16;
	*seed = *seed * 0x19660d + 0x7fff;
	const unsigned int b = *seed >> 16;
	OMedia3DVector r(0, 0, 0, 0);							// a random direction on the ground
	r.x = (float)((double)(((int)(a * 1001) >> 16) & 0xffff) * 0.001f - 0.5);	// 0x4cb218, 0x4cb210
	r.z = (float)((double)(((int)(b * 1001) >> 16) & 0xffff) * 0.001f - 0.5);
	{
		const float inv = 1.0f / r.quick_magnitude();
		r.x *= inv;
		r.y *= inv;
		r.z *= inv;
	}
	if (away.x != 0)										// away from a wall: 550 along it
	{
		f_bb4.x = away.x * 550.0f + Position(this).x;		// 0x4cb20c
		f_bb4.y = Position(this).y;
		f_bb4.z = away.z * 550.0f + Position(this).z;
	}
	else
	{
		bool done = false;
		if (f_c1c > 0 && CanSeeTarget())
		{
			OMedia3DVector d;
			const OMedia3DVector p = Position(this), t = Position(f_964);
			d.x = p.x - t.x;
			d.y = Position(this).y - Position(f_964).y;
			d.z = Position(this).z - Position(f_964).z;
			d.w = 0;
			if (Length(d.x, d.y, d.z) < f_9a8)				// away from the target
			{
				const float inv = 1.0f / d.quick_magnitude();
				r.x = inv * d.x;
				r.z = inv * d.z;
				done = true;
			}
		}
		if (!done)
		{
			if (f_b8c == NULL) f_b8c = FindObjectByTag(f_9ac);
			if (f_b8c)										// too far from the patrol point: towards it
			{
				const OMedia3DVector p = Position(this), q = Position(f_b8c);
				const float dx = p.x - q.x;
				const float dy = Position(this).y - Position(f_b8c).y;
				const float dz = Position(this).z - Position(f_b8c).z;
				if (Length(dx, dy, dz) > f_c2c)				// WanderRange
				{
					away.x = -dx;
					away.z = -dz;
				}
			}
		}
		f_bb4.x = (float)((double)r.x * range + Position(this).x + (double)away.x * 550.0f);
		f_bb4.y = Position(this).y;
		f_bb4.z = (float)((double)r.z * range + Position(this).z + (double)away.z * 550.0f);
	}
	if (f_96c == 10 || f_96c == 11) f_c24 = 0;
}

void C3DAI::ev130(float unused, float height)				// 0x415470
{
	(void)unused;
	OMedia3DVector a = Position(this);
	OMedia3DVector b = a;
	b.y += height;
	if (!PhysicsLineBlocked(a, b, false)) return;			// 0x4b7170
	float v[4];
	v174(v);
	if (!(v[1] > 0)) return;
	v174(v);
	float y = (float)(-v[1] * 0.6);							// 0x4cb230
	if (y > -85.0f) y = -85.0f;								// 0x4cb22c
	float w[4];
	v174(w);
	v174(v);
	v176(v[0], y, w[2]);
	f_bb0 = 0;
	if (g_camera == static_cast<CGameObject *>(this))
		PlayGameSound(-1, std::string("HitHead"), false);	// 0x48f420
}

unsigned char C3DAI::ev131(CGameObject *o)					// 0x413a30
{
	OMedia3DVector e = Position(this);
	e.y += 150.0f;											// 0x4cb1e4
	OMedia3DVector d;
	{
		const OMedia3DVector p = Position(this), q = Position(o);
		d.x = p.x - q.x;
		d.y = p.y - q.y;
		d.z = p.z - q.z;
		d.w = 0;
	}
	const float dist = (float)sqrt((double)d.z * d.z + (double)d.y * d.y + (double)d.x * d.x);
	{
		const float inv = 1.0f / d.quick_magnitude();
		d.x *= inv;
		d.y *= inv;
		d.z *= inv;
	}
	if (dist < 1500.0f)										// 0x4cb1e0
	{
		f_c68.x = e.x; f_c68.y = e.y; f_c68.z = e.z; f_c68.w = e.w;
		f_c68.x = d.x + f_c68.x;
		f_c68.y = d.y + f_c68.y;
		f_c68.z = d.z + f_c68.z;
		f_bb4 = f_c68;
		f_988.x = d.x;
		f_988.y = d.y;
		f_988.z = d.z;
		f_988.w = 0;
		OMedia3DVector n = f_988;
		const float inv = 1.0f / n.quick_magnitude();
		v176(inv * n.x * f_968, inv * n.y * f_968, (float)((double)inv * n.z * f_968));
		return 0;
	}
	v176(0, 0, 0);
	return 0;
}

// --------------------------------------------------------------------------- path finding

unsigned char C3DAI::FindBestPath(CGameObject *goal)		// 0x412e50
{
	// (debug log "MEBTEMP Calling FindBestPath")
	if (!goal) return 0;
	// (debug log "Recursive search for %s")
	f_c78 = 0;
	for (int i = 0; i < 300; i++) f_112c[i] = NULL;
	ev136(f_1b48);
	ev134(this, goal);
	// (debug log "Recursive return %f, outputting list" and each "PatrolPoint = %s")
	f_b8c = NULL;
	f_1b2c = 0;
	if (!f_112c[0]) return 0;
	if (f_112c[0]->NameContains((char *)"C3DAI"))			// the route starts at ourselves
	{
		if (!f_112c[1]) return 0;
		f_1b2c = 1;
	}
	return 1;
}

float C3DAI::ev134(CGameObject *from, CGameObject *to)		// 0x412f50
{
	// (a debug indent string of "   " per level is built here)
	if (f_c78 >= 100)
	{
		ErrorLog("ERROR: too deep in recursion loop something wrong");
		return -1.0f;
	}
	f_c7c[f_c78] = from;
	f_c78++;
	float dist;
	{
		const OMedia3DVector a = Position(to), b = Position(from);
		dist = Length(a.z - b.z, a.y - b.y, a.x - b.x);
	}
	if (!from->NameContains((char *)"C3DPATROLPOINT") && !from->NameContains((char *)"C3DAI"))
	{
		f_c7c[f_c78] = NULL;								// (as shipped: the slot after the one set)
		f_c78--;
		return -1.0f;
	}
	if (ev132(Position(from), Position(to)))				// to is in view: the route is complete
	{
		for (int i = 0; i < 300; i++) f_112c[i] = NULL;
		for (int i = 0; i < f_c78; i++) f_112c[i] = f_c7c[i];
		f_112c[f_c78] = to;
		f_c7c[f_c78] = NULL;
		f_c78--;
		return dist;
	}
	float best = 1e8f;										// 0x4cbebc20
	float cur = 0;
	int best_i = -1;
	for (int i = 0; i < 20; i++)
	{
		bool known;
		CGameObject *p = VisiblePoint(from, i, &known);
		if (!ev141(p)) continue;
		bool on_stack = false;
		for (int k = 0; k < f_c78; k++)
		{
			bool kn;
			CGameObject *q = VisiblePoint(from, i, &kn);
			if (kn && f_c7c[k] == q) { on_stack = true; break; }
		}
		if (on_stack) continue;
		if (known) cur = ev134(VisiblePoint(from, i, &known), to);
		if (cur > 0 && cur < best)
		{
			best = cur;
			best_i = i;										// (two NameContains calls, results unused)
		}
	}
	f_c7c[f_c78] = NULL;
	f_c78--;
	if (best_i == -1) return -1.0f;
	return (float)((double)best + dist);
}

CGameObject *C3DAI::ev135(OMedia3DVector pos, unsigned char nearest)	// 0x413640
{
	CGameObject *best = NULL;
	float bestd = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		CGameObject *o = *it;
		if (!o || !ev141(o)) continue;
		C3DPatrolPoint *p = NetworkPoint(o);
		if (!p || p->m_group_id != f_1b48) continue;
		const OMedia3DVector q = Position(o);
		OMedia3DVector d(pos.x - q.x, pos.y - q.y, pos.z - q.z, 0);
		const float m = d.quick_magnitude();
		if (!best)
		{
			best = o;
			bestd = m;
		}
		else if (nearest ? m < bestd : m > bestd)
		{
			bestd = m;
			best = o;
		}
	}
	return best;
}

void C3DAI::ev136(int group)								// 0x412530
{
	(void)group;											// (not used: every type-1 point counts)
	for (int i = 0; i < 20; i++) { f_1a8c[i].point = NULL; f_1a8c[i].dist = 0; }
	int n = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		CGameObject *o = *it;
		if (!NetworkPoint(o)) continue;
		if (PhysicsLineBlocked(Position(this), Position(o), false) || n >= 20) continue;
		f_1a8c[n].point = o;
		const OMedia3DVector a = Position(o), b = Position(this);
		f_1a8c[n].dist = Length(a.z - b.z, a.y - b.y, a.x - b.x);
		n++;
	}
}

void C3DAI::ev137(float dt)									// 0x411d90
{
	(void)dt;
	ev109();
	if (f_c38 == 0)
	{
		if (!v133()) ev117();								// FLY in the air, RUN on the floor
		else ev118();
	}
	if (f_b8c)
	{
		if (f_c00 > 0)										// still sees the target: go for it
		{
			ev115();
			const OMedia3DVector t = Position(f_964);
			f_bb4.x = t.x;
			f_bb4.y = Position(f_964).y;
			f_bb4.z = Position(f_964).z;
		}
		else ev111(f_b8c);
		ev108();
	}
	if (!f_998 || !f_b8c) return;
	// Arrived at f_b8c. The original reads it as a patrol point (PatrolSpeed +0x970, WaitTime
	// +0x8c0) without checking; port: only patrol points have those (another AI gives its speed).
	C3DPatrolPoint *pp = PatrolPoint(f_b8c);
	float speed = 0, wait = 0;
	if (pp)
	{
		speed = pp->m_patrol_speed;
		wait = pp->m_wait_time;
	}
	else if (C3DAI *ai = dynamic_cast<C3DAI *>(f_b8c)) speed = ai->f_968;
	if (speed > 0) f_968 = speed;
	if (wait > 0)
	{
		if (strcasecmp(pp->m_sound_name, "none") != 0)
			PlayGameSound3DNoDoppler(this, -1, std::string(pp->m_sound_name), false);	// 0x48f0f0
		ev107(7);
		if (f_c38 == 0) ev58(pp->m_wait_anim, 1);
		f_bd8 = pp->m_wait_time;
		v176(0, 0, 0);
		if (strcasecmp(pp->m_attach_obj, "none") == 0) return;
		CGameObject *o = FindObjectByTag(pp->m_attach_obj);
		if (!o || !o->NameContains((char *)"C3DGRANNYOBJ")) return;	// port: o checked (the original calls through NULL)
		C3DGrannyObj *g = GrannyObj(o);
		if (!g || g->f_6d0 != 1 || f_6d0 != 1) return;
		g->ev96(this);										// element +0x180: we pick it up
		g->f_dac = -1.0f;
		return;
	}
	if (f_96c == 18)
	{
		if (!f_1b4c) return;
		f_1b4c = 0;
	}
	if (!f_b8c->NameContains((char *)"C3DPATROLPOINT"))
	{
		ErrorLog("ERROR: invalid NextPatrol point for %s", f_b8c->m_tag);
		return;
	}
	f_b94 = f_b8c;
	char next[100];
	strcpy(next, static_cast<C3DPatrolPoint *>(f_b8c)->m_next_patrol_point);
	if (strcasecmp(next, "none") == 0)
	{
		f_b8c = NULL;
		return;
	}
	f_b8c = FindObjectByTag(next);
	if (f_b8c) return;
	if (strcasecmp(next, "WAIT") == 0) ev107(1);
	else if (strcasecmp(next, "SCAN") == 0) ev107(6);
	if (strcasecmp(next, "FREEZE") != 0) return;
	ev107(1);
	v176(0, 0, 0);
	f_968 = 0;
	// (debug log "Setting position for %s")
	float p[4];
	f_b94->v195(p);
	v196(p[0], p[1], p[2], p[3]);							// stay on the last point
}

void C3DAI::ev138(unsigned char far)						// 0x412be0
{
	if (!f_964) return;
	f_1b44 = far ? 9000.0f : 4000.0f;
	OMedia3DVector d;
	{
		const OMedia3DVector p = Position(this), t = Position(f_964);
		d.x = p.x - t.x;
		d.y = p.y - t.y;
		d.z = p.z - t.z;
		d.w = 0;
	}
	const float inv = 1.0f / d.quick_magnitude();
	d.x *= inv;
	d.y *= inv;
	d.z *= inv;
	ev139(Position(f_964), d, far);							// the points beyond us, seen from the target
	CGameObject *pat = ev135(Position(this), 0);			// the farthest of them
	f_1b38 = pat;
	if (pat)
	{
		const OMedia3DVector p = Position(this), q = Position(f_1b38);
		if (Length(p.z - q.z, p.y - q.y, p.x - q.x) < 300.0f)	// 0x4caa7c
		{
			if (far)
			{
				ev138(0);
				f_1b34 = 0;
				return;
			}
		}
		else
		{
			if (FindBestPath(pat))
			{
				ev107(15);
				f_1b34 = 0;
				return;
			}
			if (far)
			{
				ev138(0);
				f_1b34 = 0;
				return;
			}
		}
	}
	else if (far)
	{
		ev138(0);
		f_1b34 = 0;
		return;
	}
	ev107(5);												// no way out: just run
	f_c3c = 10.0f;
	f_1b34 = 0;
}

void C3DAI::ev139(OMedia3DVector pos, OMedia3DVector dir, unsigned char cone)	// 0x4126a0
{
	f_1b34 = 1;
	for (int i = 0; i < 300; i++) f_15dc[i] = NULL;
	int n = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		C3DPatrolPoint *p = NetworkPoint(*it);
		if (!p) continue;
		p->f_968 = 0;
		const OMedia3DVector q = Position(*it);
		OMedia3DVector d(q.x - pos.x, q.y - pos.y, q.z - pos.z, 0);
		if (!(Length(d.y, d.z, d.x) < f_1b44)) continue;
		if (cone)
		{
			const float inv = 1.0f / d.quick_magnitude();
			d.x *= inv;
			d.y *= inv;
			d.z *= inv;
			const float inv2 = 1.0f / dir.quick_magnitude();	// (the argument itself is normalised, again each time)
			dir.x *= inv2;
			dir.y *= inv2;
			dir.z *= inv2;
			if ((double)dir.z * d.z + (double)dir.y * d.y + (double)dir.x * d.x <= 0)
			{
				p->f_968 = 0;
				continue;
			}
		}
		if (n < 300) f_15dc[n++] = *it;						// port: bound checked
		p->f_968 = 1;
	}
}

void C3DAI::ev140(OMedia3DVector pos, OMedia3DVector dir, unsigned char cone)	// 0x412830
{
	f_1b34 = 1;
	for (int i = 0; i < 300; i++) f_15dc[i] = NULL;
	int n = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		C3DPatrolPoint *p = NetworkPoint(*it);
		if (!p) continue;
		const OMedia3DVector q = Position(*it);
		OMedia3DVector d(q.x - pos.x, q.y - pos.y, q.z - pos.z, 0);
		if (!(Length(d.z, d.y, d.x) < f_1b44)) continue;
		if (cone)
		{
			const float inv = 1.0f / d.quick_magnitude();
			d.x *= inv;
			d.y *= inv;
			d.z *= inv;
			if ((double)d.z * dir.z + (double)d.y * dir.y + (double)dir.x * d.x <= 0)
			{
				p->f_968 = 0;
				continue;
			}
		}
		if (n < 300) f_15dc[n++] = *it;						// port: bound checked
		p->f_968 = 1;
	}
}

unsigned char C3DAI::ev141(CGameObject *o)					// 0x412990
{
	if (!o) return 0;
	if (!f_1b34) return 1;
	for (int i = 0; i < 300; i++)
		if (f_15dc[i] && f_15dc[i] == o) return 1;
	return 0;
}

void C3DAI::ev142(unsigned char unused)						// 0x4129d0
{
	(void)unused;
	if (!f_964) return;
	OMedia3DVector d;
	{
		const OMedia3DVector t = Position(f_964), p = Position(this);
		d.x = t.x - p.x;
		d.y = t.y - p.y;
		d.z = t.z - p.z;
		d.w = 0;
	}
	const float inv = 1.0f / d.quick_magnitude();
	d.x *= inv;
	d.y *= inv;
	d.z *= inv;
	OMedia3DVector behind = Position(this);					// 1000 behind us, looking at the target
	behind.x = behind.x - d.x * 1000.0f;					// 0x4cb1d8
	behind.y = behind.y - d.y * 1000.0f;
	behind.z = behind.z - d.z * 1000.0f;
	ev139(behind, d, 1);
	CGameObject *pat = ev135(Position(f_964), 1);			// the candidate nearest the target
	if (pat)
	{
		if (!FindBestPath(pat))
		{
			f_1b38 = NULL;
			ev107(17);
		}
		else ev107(16);
		f_1b34 = 0;
		return;
	}
	f_1b38 = NULL;
	ev107(17);
	PlayGameSound(-1, std::string("shrink ray"), false);	// 0x48f420
	// (debug text "DESTPAT IS NULL")
	f_1b34 = 0;
}

CGameObject *C3DAI::ev143(OMedia3DVector pos, unsigned char visible)	// 0x4133d0
{
	CGameObject *best = NULL;
	float bestd = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		C3DPatrolPoint *p = NetworkPoint(*it);
		if (!p || p->m_group_id != f_1b48) continue;
		const OMedia3DVector q = Position(*it);
		OMedia3DVector d(pos.x - q.x, pos.y - q.y, pos.z - q.z, 0);
		if (!best)
		{
			if (visible && !ev132(Position(*it), pos)) continue;
			best = *it;
			bestd = d.quick_magnitude();
			continue;
		}
		const float m = d.quick_magnitude();
		if (!(m < bestd)) continue;
		if (visible && !ev132(Position(*it), pos)) continue;
		best = *it;
		bestd = m;
	}
	return best;
}
