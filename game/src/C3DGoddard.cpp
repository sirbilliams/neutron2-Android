/*  C3DGoddard - see C3DGoddard.h.
    Constants: 0x4cb228 = 16384 / 360, 0x4cb1f8 = 360 / 16384, 0x4df740 = 1 / 45, 0x4df73c = 52,
    0x4df74c = 14.4 (mouse to steering), 0x4cb21c = 200 (arrow distance from the viewport).   */
#include "C3DGoddard.h"
#include "objects/C3DBone.h"
#include "Mouse.h"
#include "C3DShadow.h"
#include "C3DCursor.h"
#include "C3DGrannyObj.h"
#include "C3DPlayer.h"				// PlayerKey*, JimmyActivateMenuItem
#include "C3DRocketShip.h"			// neg::GoddardMarker
#include "CGameType.h"
#include "Bridge.h"					// GameIsRunning
#include "Physics.h"
#include "Game.h"
#include "Sound.h"
#include "Todo.h"
#include "gen/globals.h"
#include "OMediaElement.h"
#include "OMediaViewPort.h"
#include "OMedia3DVector.h"
#include "OMediaTrigo.h"
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360
static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// The six places around the target (static OMedia3DVectors 0x545fc0, 0x545f90, 0x545fb0,
// 0x545f80, 0x545fa0, 0x545f70; w = 0): 0 behind left, 1 left, 2 behind right, 3 right,
// 4 in front, 5 behind.
static const float kSides[6][3] =
{
	{  380.0f, -125.0f,  430.0f },							// 0x545fc0
	{  430.0f, -125.0f,  -30.0f },							// 0x545f90
	{ -380.0f, -125.0f,  430.0f },							// 0x545fb0
	{ -430.0f, -125.0f,  -30.0f },							// 0x545f80
	{    0.0f, -125.0f, -430.0f },							// 0x545fa0
	{    0.0f, -125.0f,  430.0f },							// 0x545f70
};

// --------------------------------------------------------------------------- helpers

static OMedia3DVector Position(CGameObject *o)				// vtable +0x30c
{
	float p[4];
	o->v195(p);
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0 (as in
// C3DAI.cpp; the generated declaration of v226 has the wrong signature).
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

// The game's random number generator (as C3DPlayer's): 0..1000.
static int Random1001(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return ((int)((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'L1_A').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x48f730 (CLocalGameObject family): the nearest object with ObjectID id1 or id2 that is
// active (v123) and f_140, by quick_magnitude of (o - it) (as in C3DJimmy_a.cpp).
static CGameObject *FindNearest(CGameObject *o, int id1, int id2)
{
	CGameObject *best = NULL;
	float best_d = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *it = *i;
		if (!it || (it->m_object_id != id1 && it->m_object_id != id2)) continue;
		if (!it->v123() || !it->f_140) continue;
		OMedia3DVector a = Position(o), b = Position(it);
		OMedia3DVector d(a.x - b.x, a.y - b.y, a.z - b.z, 0);
		const float m = d.quick_magnitude();
		if (!best || m < best_d)
		{
			best_d = m;
			best = it;
		}
	}
	return best;
}

// 0x465b60: a key of the "deactivate item" list (no joystick button); as the key tests of
// C3DPlayer.cpp: 0x1f5 left mouse, 0x1f4 right mouse, else a key of g_keys (which clears the
// mouse movement 0x4a39e0).
static bool KeyDeactivateItem(void)
{
	for (const int *list = g_key_deactivate_item; *list; list++)
	{
		const int k = *list;
		if (k == 0x1f5) { if (g_mouse_down) return true; }
		else if (k == 0x1f4) { if (g_mouse_rdown) return true; }
		else if ((unsigned)k < 256 && g_keys[k])			// port: index checked (the original reads g_keys[k] for any k)
		{
			g_56aebe = 0;
			g_56aec0 = 0;
			return true;
		}
	}
	return false;
}

using neg::GetMouseDelta;									// 0x4a3990 (Mouse.h)

// A C3DBone that Goddard picked up: hidden, its +0x924 cleared.
static void BoneTaken(CGameObject *bone)
{
	C3DBone *b = bone ? dynamic_cast<C3DBone *>(bone) : NULL;
	b->HideAgain();											// element +0xdc
	b->f_924 = 0;
}

// The arrow of a C3DCursor (its CGameObject part) as an element.
static C3DObject *Arrow(CGameObject *o)
{
	return o ? dynamic_cast<C3DObject *>(o) : NULL;
}

// A head prop / shadow prop of Goddard: model `file` on our bone `bone`.
static C3DGrannyObj *NewProp(C3DGoddard *g, const char *file, const char *bone)
{
	C3DGrannyObj *p = new C3DGrannyObj;						// 0xdc0 bytes
	if (!p) return p;
	strcpy(p->f_924, file);
	strcpy(p->f_c44, g->m_tag);								// AttachTo us
	strcpy(p->f_ca8, bone);
	p->v107(0);												// +0x1ac
	p->v36(0);												// +0x90
	p->f_d70 = NULL;
	p->v52(0);												// +0xd0
	p->v46(0);												// +0xb8
	p->f_d7c = 0.5f;										// attach in 0.5 s
	p->f_db0 = 0;
	strcpy(p->f_be0, "none");
	return p;
}

// --------------------------------------------------------------------------- construction

C3DGoddard::C3DGoddard()									// 0x4383d0
{
	SetName((char *)"C3DGODDARD");
	Log("C3DGoddard()");
	f_6d0 = 1;												// Granny
	f_649 = 0;
	C3DGoddard::InitObject();								// 0x438cd0 (called directly)
	C3DObject::SetObjectID('3GOD');							// 0x49ab00
	f_960 = 0;
	f_968 = 1.0f;
	f_b9c.x = 0;											// turn rates
	f_b9c.y = 3.2f;
	f_b9c.z = 0.3f;
	f_96c = 1;
	f_c08 = 1;												// "AIState"
	C3DGoddard::ev144(0);									// 0x438b90
	f_1b5e = 0;
	v136(1);
	f_974 = '3JIM';
	v50(0);
	f_1b50 = new C3DShadow;									// 0x60c bytes
	v107(0);
	v36(0);
	f_1b54 = -1;
	f_1b58 = -1;
	f_009 = 1;
	f_bd4 = 1;												// hovering
	f_1b60 = 0;
	f_1b64 = 0;
	f_1b68 = 0;
	f_1b6c = 0;
	f_1b70 = 700.0f;
	f_1b74 = 30.0f;
	f_968 = 0;
	f_1bb4 = 0;
	f_1b78 = 1;
	f_1b94 = 1500.0f;
	f_1b7c = -1;
	ev106(f_a9c, (char *)"none");							// WAG
	ev106(f_ac4, (char *)"RUN");							// RUN
	ev106(f_aec, (char *)"FLY");							// FLY
	ev106(f_b14, (char *)"none");							// SIT
	ev106(f_a74, (char *)"none");
	ev106(f_b3c, (char *)"RUN");							// WALK
	C3DCursor *c = new C3DCursor;							// 0x92c bytes
	f_1b80 = c ? static_cast<CGameObject *>(c) : NULL;
	Arrow(f_1b80)->HideAgain();								// element +0xdc
	c = new C3DCursor;
	f_1b84 = c ? static_cast<CGameObject *>(c) : NULL;
	f_934 = -1;
	Arrow(f_1b84)->HideAgain();
	f_1b88 = NULL;
	f_1b8c = NULL;
	f_1b90 = 0;
	v42(1);
	f_c60 = 150.0f;
	f_1ba0 = 0;
	f_1ba4 = 0;
	f_1ba5 = 0;
	f_1ba8 = NULL;
	f_8a4 = 1;												// never culled
	f_1bac = 0;
	f_1bb0 = 0;
	f_4c4 = 1;												// (element +0x584)
	f_1bb8 = -1;
	f_1bbc = NULL;
	f_1bbc = NewProp(this, "headspinbase.grn", "head");
	if (f_1bbc) f_1bbc->f_da0 = 0.1f;						// "SlideSpeedX"
	f_1bc0 = NULL;
	f_1bc0 = NewProp(this, "goddshadow.grn", "shadowbone");
	// (then a call to the empty 0x463890)
}

C3DGoddard::~C3DGoddard()									// 0x438b40
{
}

namespace neg {
CGameObject *New_C3DGoddard(void)	{ return new C3DGoddard; }				// factory '3GOD' (0x1bcc bytes)

// C3DRocketShip.h: Goddard's arrow towards the red neutrons (+0x1b80).
CGameObject *GoddardMarker(CGameObject *goddard)
{
	C3DGoddard *g = goddard ? dynamic_cast<C3DGoddard *>(goddard) : NULL;
	return g ? g->f_1b80 : NULL;							// (port: the original reads +0x1b80 of whatever the object is)
}
}

// --------------------------------------------------------------------------- element slots

void C3DGoddard::ev60(void)									// 0x43b570
{
	ev58((char *)"SIT", 1);									// +0xe8
}

void C3DGoddard::ev67(void)									// 0x43b510
{
	if (strcasecmp(ev66(), "FIX") && strcasecmp(ev66(), "LOSTWING")) return;	// +0x108, _stricmp
	ev58((char *)"ROCK", 1);
}

void C3DGoddard::ev107(int state)							// 0x439b50
{
	C3DAI::ev107(state);									// 0x412240
	if (state == 3)
	{
		f_968 = 400.0f;
		ev90(1);											// +0x168
		return;
	}
	if (state == 7) ev90(0);
}

void C3DGoddard::ev116(void)								// 0x439c30
{
	float r[4];
	f_964->v201(r);											// +0x324
	if (!f_998) v218(0, r[1], 0);							// CGameObject +0x368
}

void C3DGoddard::ev118(void)								// 0x43a140
{
	if (f_1b54 != -1) f_1b54 = -1;							// (not stopped)
}

void C3DGoddard::ev124(void)								// 0x43a160
{
	if (f_1b58 != -1)
	{
		StopGameSound(f_1b58, false);						// 0x48f0d0
		f_1b58 = -1;
	}
}

void C3DGoddard::ev144(short side)							// 0x438b90
{
	if (side >= 0 && side <= 5)
	{
		f_978.x = kSides[side][0];
		f_978.y = kSides[side][1];
		f_978.z = kSides[side][2];
	}
	f_1b5c = side;
}

unsigned char C3DGoddard::ev145(short side)					// 0x43a470
{
	OMedia3DVector me = Position(this);						// CGameObject +0x30c
	me.y = me.y + 20.0f;
	if (side < 0 || side > 5) return 0;
	float t[4];
	{
		const OMedia3DVector v = f_964->v224(kSides[side][0], 20.0f, kSides[side][2]);	// +0x380
		t[0] = v.x; t[1] = v.y; t[2] = v.z; t[3] = v.w;
	}
	if (PhysicsLineBlocked(me, OMedia3DVector(t[0], t[1], t[2], t[3]), false)) return 0;	// 0x4b7170
	ev144(side);											// +0x240
	return 1;
}

void C3DGoddard::ev146(float dt)							// 0x438f50
{
	float side_x = 0, side_z = 0;
	float v[4];
	v174(v);												// CGameObject +0x2b8: velocity
	const float vy = v[1];
	if ((float)sqrt((double)v[0] * v[0] + (double)v[2] * v[2] + (double)vy * vy) != 0)
	{
		f_1b6c = f_1b68;
		AddRotation(0, f_1b68 * dt, 0);						// +0x330
		// leaning out of the turn: (a, 0, 0) turned by our heading (the inlined rotation of
		// angles (0, y, 0): sin 0 / cos 0 from the table)
		const float a = (1.0f - v145()) * (f_1b6c * (1.0f / 45.0f)) * f_968 * dt;	// +0x244; 0x4df740
		float r[4];
		v201(r);											// +0x324
		const int ay = (int)(r[1] * kDegToAngle) & omc_MaxAngleMask;	// _ftol
		OMediaCosSinTable &tab = OMediaCosSinTable::global;
		const float s0 = tab.sin(0), c0 = tab.cos(0), sy = tab.sin(ay), cy = tab.cos(ay);
		const float t = -(s0 * a);
		const float u = c0 * a;
		const float w = -(s0 * t);
		side_x = u * cy - w * sy;
		side_z = w * cy + u * sy;
	}
	const OMedia3DVector f = ObjectDir(this, 0, 0, f_968);	// +0x388
	v176(side_x + f.x, vy, side_z + f.z);					// +0x2c0: velocity (y kept)
	if (f_1b64 > 0)
	{
		f_1b64 = f_1b64 - dt * 52.0f;						// 0x4df73c
		if (f_1b64 <= 0) f_1b64 = 0;
	}
	if (f_1b64 < 0)
	{
		f_1b64 = dt * 52.0f + f_1b64;
		if (!(f_1b64 < 0)) f_1b64 = 0;
	}
}

void C3DGoddard::ev147(unsigned char on)					// 0x439a40
{
	if (on)
	{
		float r[4];
		v201(r);											// +0x324
		v203(0, r[1], 0);									// +0x32c
		v152(60.0f);										// +0x260
		if (f_1b7c == -1) f_1b7c = PlayGameSound(-1, std::string("sonar"), true);	// 0x48f420
		return;
	}
	v152(f_034);											// +0x260 (CGameObject +0x34)
	Arrow(f_1b80)->HideAgain();								// element +0xdc
	Arrow(f_1b84)->HideAgain();
	f_968 = 450.0f;
	if (f_1b7c != -1)
	{
		StopGameSound(f_1b7c, true);						// 0x48f0d0
		f_1b7c = -1;
	}
}

void C3DGoddard::ev148(void)								// 0x43a1b0
{
	// try the two sides next to ours, else (while hovering) stay
	static const short next[6][2] = { { 5, 1 }, { 0, 4 }, { 3, 5 }, { 2, 4 }, { 1, 3 }, { 0, 2 } };
	const short s = f_1b5c;
	if (s < 0 || s > 5) return;
	f_960 = 0;
	f_1b5e = s;
	short a = next[s][0], b = next[s][1];
	if (s == 4)
	{
		if (!((float)Random1001() * 0.001f <= 0.5))			// 0x4cb218, 0x4cb210 (double)
		{
			a = 3;
			b = 1;
		}
	}
	if (ev145(a)) return;									// +0x244
	if (ev145(b)) return;
	if (f_bd4) ev144(s);									// +0x240
}

void C3DGoddard::ev149(void)								// 0x439c80
{
	const OMedia3DVector p = Position(this);
	const OMedia3DVector q = Position(f_964);
	const float dx = p.x - q.x, dy = p.y - q.y, dz = p.z - q.z;
	const float d = (float)sqrt((double)dx * dx + (double)dy * dy + (double)dz * dz);	// fsqrt
	if (d > 350.0f && f_1b60)								// 0x4d783c
	{
		f_1b60 = 0;
		f_bd4 = 1;
	}
	if (d > 440.0f) f_968 = 600.0f;							// 0x4df744
	else f_968 = 450.0f;
	if (d > 1500.0f && !f_1ba5)								// 0x4cb1e0: left behind - jump there
	{
		f_bb0 = 0;
		const OMedia3DVector t = f_964->v224(0, 0, -800.0f);	// +0x380
		v196(t.x, t.y, t.z, t.w);							// +0x310
	}
	if (f_1b5c != 2 && f_1b5c != 0 && f_960 > 2.0f) ev148();	// 0x4caa70; +0x250
	if (v133())												// +0x214 on the floor
	{
		float v[4];
		v174(v);											// +0x2b8
		if ((float)sqrt((double)v[0] * v[0] + (double)v[1] * v[1] + (double)v[2] * v[2]) > 20.0f)	// 0x4caa40
		{
			if (!IsAnim((char *)"RUN")) ev58((char *)"RUN", 1);
		}
		else if (!IsAnim((char *)"WAG")) ev58((char *)"WAG", 1);
	}
	else if (!IsAnim((char *)"FLY")) ev58((char *)"FLY", 1);
	if (!f_1b90) return;
	f_1b90 = 0;
	if (!g_camera)
	{
		ErrorLog("ERROR: no pco for C3DGoddard");
		return;
	}
	if (!g_camera->NameContains((char *)"C3DJIMMY")) return;	// +0x18
	const OMedia3DVector c = Position(g_camera);
	const OMedia3DVector t = g_camera->v224(0, 0, -250.0f);	// +0x380: behind Jimmy
	if (!PhysicsLineBlocked(c, t, false))					// 0x4b7170
		v196(t.x, t.y, t.z, t.w);							// +0x310
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DGoddard::InitObject(void)							// 0x438cd0
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// element +0x110
	f_649 = 1;
	InitModel(std::string("goddbase.grn"));					// +0x144
	CreateAnim((char *)"RUN", (char *)"goddwalk.grn");		// +0xe0 (granny: AddAnimation)
	CreateAnim((char *)"SIT", (char *)"goddsit.grn");
	CreateAnim((char *)"WAG", (char *)"goddstop.grn");
	CreateAnim((char *)"FLY", (char *)"goddfly.grn");
	CreateAnim((char *)"ROCK", (char *)"goddrocket.grn");
	CreateAnim((char *)"LOSTWING", (char *)"goddcatchwingrocket.grn");
	CreateAnim((char *)"FIX", (char *)"goddfixrocket.grn");
	CreateAnim((char *)"AFTERBURN", (char *)"goddafterburn.grn");
	CreateAnim((char *)"STOP", (char *)"goddstop.grn");
	CreateAnim((char *)"CHASE", (char *)"goddchase.grn");
	CreateAnim((char *)"JUMP", (char *)"goddjump.grn");
	CreateAnim((char *)"SLIDE", (char *)"goddstopshort.grn");
	CreateAnim((char *)"PLAY", (char *)"goddplay.grn");
	CreateAnim((char *)"SNIFF", (char *)"goddsniff.grn");
	CreateAnim((char *)"WAVE", (char *)"goddwave.grn");
	CreateAnim((char *)"POINT", (char *)"goddpoint.grn");
	CreateAnim((char *)"DISH", (char *)"godddish.grn");
	ev58((char *)"STOP", 1);								// +0xe8
}

void C3DGoddard::AddTouching(CGameObject *other)			// 0x43a830
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other->NameContains((char *)"C3DJIMMY"))			// +0x18
	{
		if (f_1ba5)											// brought the bone: back to our place
		{
			f_978.x = f_be4.x;
			f_978.y = f_be4.y;
			f_978.z = f_be4.z;
			f_1ba5 = 0;
			return;
		}
		if (f_96c == 4)										// following: jump over him
		{
			f_bb0 = 1;
			f_1ba0 = 0;
			f_bb4.y = Position(f_964).y + 500.0f;			// 0x4cb200
		}
		return;
	}
	if (!other->NameContains((char *)"C3DBONE")) return;
	// (debug log "MEBTEMP touched a 3dbone")
	ev107(4);												// element +0x1ac
	CGameObject *jim = FindObjectByTag("JIM1");				// 0x4aefb0
	BoneTaken(other);
	f_1ba5 = 1;
	if (!jim) return;
	f_964 = jim;											// bring it to Jimmy
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = -100.0f;
	f_bf8 = 0;
	ev124();												// element +0x1f0
}

void C3DGoddard::v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly)	// 0x43a9a0
{
	(void)nx; (void)ny; (void)nz; (void)nw; (void)poly;
	f_bb0 = 0;
}

void C3DGoddard::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x43a190
{
	(void)a; (void)b; (void)c; (void)d; (void)poly;
	if (f_96c == 4) ev148();								// element +0x250
}

void C3DGoddard::Process(float dt)							// 0x439170
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (f_bb0)												// the jump lasts 5 s at most
	{
		f_1ba0 = dt + f_1ba0;
		if (f_1ba0 > 5.0f)									// 0x4caa90
		{
			f_1ba0 = 0;
			f_bb0 = 0;
		}
	}
	if (!GameIsRunning()) return;							// 0x4b0ca0
	if (!v131())											// +0x20c
	{
		C3DAnimated::Process(dt);							// 0x41c120
		f_1b50->HideAgain();								// sprite +0xd8
		f_1bc0->HideAgain();								// element +0xdc
		return;
	}
	const OMedia3DVector head = v224(0, 100.0f, -150.0f);	// +0x380
	if (f_1ba8)
	{
		f_1ba8->place(head.x, head.y, head.z);				// 2002 +0x44..+0x4c
		float r[4];
		v201(r);											// +0x324
		const int ay = (int)(r[1] * kDegToAngle);			// _ftol
		const int ax = (int)(r[0] * kDegToAngle);
		f_1ba8->set_angle((omt_Angle)ax, (omt_Angle)ay, 0);	// 2002 +0x50..+0x54
	}
	if (v133())												// +0x214 on the floor: the Granny shadow
	{
		if (f_1bc0) f_1bc0->ShowAgain();					// element +0xd8
		if (f_1b50) f_1b50->HideAgain();					// sprite +0xd8
	}
	else
	{
		if (f_1bc0) f_1bc0->HideAgain();					// element +0xdc
		if (f_1b50) f_1b50->ShowAgain();					// sprite +0xd4
	}
	if (!g_windowed)
	{
		const OMedia3DVector q = Position(this);
		const OMedia3DVector a(q.x, q.y + 100.0f, q.z, q.w);	// 0x4d2cb0
		const OMedia3DVector b(q.x, a.y - 2000.0f, q.z, q.w);	// 0x4cb1ec
		if (PhysicsClosestHit(a, b, false))					// 0x4b7640
		{
			const float hit_y = g_phys_ray.end.y;			// 0x6311f8..
			if (f_1b50)
			{
				const OMedia3DVector s = Position(this);
				f_1b50->v197(s.x, hit_y + 30.0f, s.z);			// +0x314; 0x4ca4b4
				f_1b50->ShowAgain();						// sprite +0xd4
			}
		}
	}
	if (g_camera == static_cast<CGameObject *>(this)) return;	// steered (v243)
	if (g_56adf3)											// (never set in the shipped game)
	{
		if (!f_1bbc)
		{
			f_1bbc = NewProp(this, "headspinbase.grn", "head");
			if (f_1bbc) f_1bbc->f_da0 = 0.1f;
		}
		ev107(1);											// element +0x1ac: wait
	}
	float v[4];
	v174(v);												// +0x2b8
	if (v[1] > 0) ev130(dt, 115.0f);						// element +0x208
	C3DAI::Process(dt);										// 0x411240
	switch (f_96c)
	{
		case 1:												// waiting: sit and do tricks for Jimmy
		{
			f_1b60 = 1;
			v261();											// +0x414
			float acc[4];
			v155(acc);										// +0x26c
			const float az = acc[2];
			v155(acc);
			v157(acc[0], g_gravity.y, az);					// +0x274; 0x63119c
			f_1bb0 = f_1bb0 - dt;
			if (f_1bb0 > 0) break;
			CGameObject *j = g_jimmy;						// (g_545fd0 - 0xc0)
			const OMedia3DVector jp = Position(j);
			const OMedia3DVector mp = Position(this);
			OMedia3DVector d(jp.x - mp.x, jp.y - mp.y, jp.z - mp.z, 0);
			omt_Angle ax, ay;
			d.angles(ax, ay);								// OMedia3DVector::angles
			float r[4];
			v201(r);
			v203(0, (float)ay * kAngleToDeg, r[2]);			// +0x32c: face Jimmy
			v201(r);
			v218(0, (float)ay * kAngleToDeg, r[2]);			// +0x368
			switch (f_1bac)
			{
				case 0: ev58((char *)"PLAY", 1); f_1bb0 = 4.0f; break;
				case 1:
				case 3: ev58((char *)"SIT", 1); f_1bb0 = 3.0f; break;
				case 2: ev58((char *)"CHASE", 1); f_1bb0 = 5.0f; break;
				case 4:
					ev58((char *)"JUMP", 1);
					f_1bb0 = 4.0f;
					PlayGameSound3DNoDoppler(this, -1, std::string("goddard bark"), false);	// 0x48f0f0
					break;
				case 5: ev58((char *)"WAG", 1); f_1bb0 = 2.0f; break;
				case 6: ev58((char *)"SIT", 1); f_1bb0 = 5.0f; break;
				default: break;
			}
			if (++f_1bac > 4) f_1bac = 0;
			break;
		}
		case 2:
		case 3:
		case 13:
			if (v133())										// +0x214
			{
				if (!IsAnim((char *)"RUN")) ev58((char *)"RUN", 1);
			}
			else if (!IsAnim((char *)"FLY")) ev58((char *)"FLY", 1);
			break;
		case 4:
			ev149();										// element +0x254
			break;
		case 7:
			// (debug log "HI")
			break;
		default:
			break;
	}
}

void C3DGoddard::v243(float dt)								// 0x43a9b0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	ev146(dt);												// element +0x248
	short mx = 0, my = 0;
	GetMouseDelta(&mx, &my);								// 0x4a3990
	float v[4];
	v174(v);												// +0x2b8
	if (v[1] > 0) ev130(dt, 115.0f);						// element +0x208
	f_1b98 = (float)mx * dt * 14.4f + f_1b98;				// 0x4df74c
	f_1b9c = (float)my * dt * 14.4f + f_1b9c;
	int anim;												// the animation wanted
	if (PlayerKeyLeft() || f_1b98 > 3.0f)					// 0x4659f0; 0x4cb1e8
	{
		f_1b64 = f_1b64 + 7.0f;								// 0x4cb1fc
		if (f_1b64 > 60.0f) f_1b64 = 60.0f;					// 0x4d8b84
		goto turning;
	}
	if (PlayerKeyRight() || f_1b98 < -3.0f)					// 0x465a60; 0x4d0ca4
	{
		f_1b64 = f_1b64 - 7.0f;
		if (f_1b64 < -60.0f) f_1b64 = -60.0f;				// 0x4ccef8
		goto turning;
	}
	anim = (int)my;											// sic: the original leaves its animation
															// variable holding the mouse y here
	goto speed;
turning:
	anim = 0;
	if (f_968 <= 50.0f)										// 0x4ca4b0: turning gets him moving
	{
		if (f_968 < 40.0f) f_968 = 40.0f;					// 0x4d65c8
		f_968 = f_1b74 + f_968;
	}
speed:
	if (PlayerKeyGo() || f_1b9c > 1.0f)						// 0x465910; 0x4caa6c
	{
		anim = 0;
		if (f_968 < f_1b70)
		{
			if (f_968 < 20.0f) f_968 = 20.0f;				// 0x4caa40
			f_968 = f_1b74 + f_968;
			if (f_968 > f_1b70) f_968 = f_1b70;
		}
	}
	else if (PlayerKeyBack() || f_1b9c < -1.0f)				// 0x465980; 0x4cb1dc
	{
		anim = 0;
		if (f_968 > -200.0f) f_968 = f_968 - f_1b74;		// 0x4db928
	}
	else if (f_968 <= 0) anim = 1;
	else
	{
		f_968 = f_968 - f_1b74 * 0.25f;						// 0x4d340c
		if (f_968 < 10.0f)									// 0x4cb1cc
		{
			anim = 1;
			f_968 = 0;
		}
	}
	if (f_968 > 10.0f) f_968 = f_968 - f_1b74 * (1.0f / 3.0f);	// 0x4dc054
	else if (f_968 < -10.0f) f_968 = f_1b74 * (1.0f / 3.0f) + f_968;	// 0x4dce44
	else
	{
		f_968 = 0;
		anim = 1;
	}
	f_1b98 = 0;
	f_1b9c = 0;
	f_1b68 = f_1b64;

	float a[4];
	if (!PlayerKeyJump() && !g_joy_button1)					// 0x4658a0, 0x56b05d
	{
		v155(a);											// +0x26c
		const float az = a[2];
		v155(a);
		v157(a[0], 0, az);									// +0x274
		v174(v);
		if (v[1] < -200.0f)									// 0x4db928: falling: no faster
		{
			v174(v);
			const float vz = v[2];
			v174(v);
			v176(v[0], -200.0f, vz);						// +0x2c0
		}
	}
	else if (Position(this).y < f_1b94)						// flying up
	{
		v155(a);
		const float az = a[2];
		v155(a);
		v157(a[0], 50.0f - g_gravity.y, az);				// 0x4ca4b0, 0x63119c
		anim = 3;
		v174(v);
		if (v[1] < -100.0f)									// 0x4df748
		{
			v174(v);
			const float vz = v[2];
			v174(v);
			v176(v[0], -100.0f, vz);
		}
	}
	else													// at the ceiling
	{
		v155(a);
		const float az = a[2];
		v155(a);
		v157(a[0], 0, az);
		v174(v);
		if (v[1] < -200.0f)
		{
			v174(v);
			const float vz = v[2];
			v174(v);
			v176(v[0], -200.0f, vz);
		}
	}
	if (!v133()) anim = 3;									// +0x214 in the air: FLY
	if (f_1b78 != anim)
	{
		if (anim == 0) { f_1b78 = 0; ev58((char *)"RUN", 1); }
		else if (anim == 1) { f_1b78 = 1; ev58((char *)"WAG", 1); }
		else if (anim == 3) { f_1b78 = 3; ev58((char *)"FLY", 1); }
	}

	// the arrows: 200 units in front of the viewport towards the nearest red neutron / pickup
	float rot[4];
	v201(rot);												// +0x324 (x, y and w are used)
	f_1b8c = FindNearest(this, '3RED', -1);					// 0x48f730
	if (f_1b8c)
	{
		const float vx = g_viewport->getx(), vy = g_viewport->gety(), vz = g_viewport->getz();	// 2002 +0x44..
		const OMedia3DVector rp = Position(f_1b8c);
		OMedia3DVector d(rp.x - vx, rp.y - vy, rp.z - vz, 0);
		const float k = 1.0f / d.quick_magnitude();
		d.x = k * d.x * 200.0f;								// 0x4cb21c
		d.y = k * d.y * 200.0f;
		d.z = k * d.z * 200.0f;
		C3DObject *arrow = Arrow(f_1b80);
		float ar[4];
		f_1b80->v201(ar);
		f_1b80->v196(d.x + vx, d.y + vy, d.z + vz, 1.0f);	// +0x310
		f_1b80->v202(rot[0], rot[1], ar[2], rot[3]);		// +0x328
		f_1b80->AddRotation(0, 0, dt * 100.0f);				// +0x330; 0x4d2cb0
		arrow->ShowAgain();									// element +0xd8
	}
	else Arrow(f_1b80)->HideAgain();						// element +0xdc
	f_1b88 = FindNearest(this, '3PIC', -1);
	if (f_1b88)
	{
		const float vx = g_viewport->getx(), vy = g_viewport->gety(), vz = g_viewport->getz();
		const OMedia3DVector rp = Position(f_1b88);
		OMedia3DVector d(rp.x - vx, rp.y - vy, rp.z - vz, 0);
		const float k = 1.0f / d.quick_magnitude();
		d.x = k * d.x * 200.0f;
		d.y = k * d.y * 200.0f;
		d.z = k * d.z * 200.0f;
		C3DObject *arrow = Arrow(f_1b84);
		float ar[4];
		f_1b84->v201(ar);
		f_1b84->v196(d.x + vx, d.y + vy, d.z + vz, 1.0f);
		f_1b84->v202(rot[0], rot[1], ar[2], rot[3]);
		f_1b84->AddRotation(0, 0, dt * -100.0f);			// 0x4df748
		arrow->ShowAgain();
	}
	else Arrow(f_1b84)->HideAgain();
	if (KeyDeactivateItem() && g_jimmy)						// 0x465b60
		JimmyActivateMenuItem(g_jimmy, this, NULL, 0, 0);	// Jimmy element +0x228
	// (then the camera point behind us is worked out and thrown away: only the ray test's
	// result, g_phys_ray, is left)
	const OMedia3DVector p = Position(this);
	const OMedia3DVector t = v224(0, 110.0f, -210.0f);		// +0x380
	PhysicsLineBlocked(p, t, false);						// 0x4b7170
}

void C3DGoddard::v260(void)									// 0x439f90
{
	C3DAI::v260();											// 0x4143a0
	f_1b94 = 1500.0f;
	if (LevelID() == 'LV3C')
	{
		HideAgain();										// element +0xdc
		v52(0);												// +0xd0
		v132(0);
		f_1ba4 = 1;
	}
	else
	{
		if (LevelID() == 'VR06') f_1b94 = 3500.0f;
		ShowAgain();										// element +0xd8
		v52(1);
		v132(1);
		InitPhysics();										// +0x2c
		ev144(0);											// element +0x240
		f_1b90 = 1;											// next ev149: behind Jimmy
		v67(130.0f);
		f_1ba4 = 0;
	}
	if (f_1b80) Arrow(f_1b80)->HideAgain();					// element +0xdc
	if (f_1b84) Arrow(f_1b84)->HideAgain();
}

void C3DGoddard::v266(void)									// 0x43a090
{
	if (!f_1bc0) return;
	f_1bc0->ev65(0, 2, 0);									// +0x104: blend Dst_Color, Zero
	f_1bc0->ev91(1);										// +0x16c: second pass
	f_1bbc->f_d7c = 0.5f;									// (sic: the head prop)
	f_1bbc->f_d70 = NULL;
}

void C3DGoddard::v270(int old_mode, int new_mode)			// 0x43b410
{
	// (debug log "C3DGoddard::ChangeState %d  %d")
	if (new_mode == 3)										// a cut scene: look ahead, wait
	{
		float r[4];
		v201(r);
		v203(0, r[1], 0);									// +0x32c
		v201(r);
		v218(0, r[1], 0);									// +0x368
		if (f_934 == -1)
		{
			// (debug log "MEBTEMP C3DGoddard::Swithing to WAITING %d  %d")
			if (f_96c == 4)
			{
				f_934 = 4;
				ev107(1);									// element +0x1ac
			}
		}
	}
	if (old_mode == 3 && f_934 != -1)
	{
		// (debug log "MEBTEMP C3DGoddard::Swithing back to AI %d   %d  %d")
		if (f_934 == 4)
		{
			ev107(4);
			f_934 = -1;
		}
	}
}

void C3DGoddard::ReadState(OMediaStreamOperators &s)		// 0x43a0f0
{
	C3DAI::ReadState(s);									// 0x4155e0
	if (f_96c == 3)
	{
		f_968 = 400.0f;
		ev90(1);											// element +0x168
	}
}
