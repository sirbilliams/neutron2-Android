/*  C3DCarl - see C3DCarl.h. Reconstructed from Neutron2.exe 0x425840..0x426a80. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCarl.h"
#include "../C3DPlayer.h"
#include "../C3DTriggerType.h"
#include "../C3DWaterBalloon.h"
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DVector.h"
#include "OMediaTrigo.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// The game's random number generator: 0..1000.
static int Random1001(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return ((int)((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// C2DInGameMenu slot 289 (0x4044c0), called with an InterfaceElt as `this` (it does not use
// it): place a sprite at (x, y) in 640x480 space, centred alignment, its top at y.
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

// CGameObject slot 226 (0x4ad990): the local direction (x, y, z) turned by the object's angles
// v201 (as v224, without the position), w 0. port: a static copy - the generated slot is still
// a TODO with the wrong signature, and no class overrides it.
static OMedia3DVector LocalDirection(CGameObject *o, float x, float y, float z)
{
	float rot[4];
	o->v201(rot);											// vtable +0x324
	const float kDegToAngle = 45.511112213134766f;			// 0x4cb228: 16384 / 360
	omt_Angle az = (omt_Angle)((int)(rot[2] * kDegToAngle) & omc_MaxAngleMask);
	omt_Angle ay = (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask);
	omt_Angle ax = (omt_Angle)((int)(rot[0] * kDegToAngle) & omc_MaxAngleMask);
	OMediaCosSinTable &t = OMediaCosSinTable::global;
	const float sx = t.sin(ax), cx = t.cos(ax), sy = t.sin(ay), cy = t.cos(ay), sz = t.sin(az), cz = t.cos(az);
	const float a = cz * y - sz * x;						// about z
	const float b = sz * y + cz * x;
	const float c = cx * z - a * sx;						// about x
	return OMedia3DVector(b * cy - c * sy, sx * z + a * cx, b * sy + c * cy, 0);	// about y
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

// A step: "rock explosion2" from us, and Jimmy's camera shakes for a second, less the farther
// he is (1 - distance / 5000, not below 0).
static void Step(C3DCarl *carl)
{
	PlayGameSound3DNoDoppler(carl, -1, std::string("rock explosion2"), false);	// 0x48f0f0
	C3DPlayer *jim = dynamic_cast<C3DPlayer *>(carl->f_1d78);	// (element - 0xc0)
	if (!jim) return;										// port: the original writes through NULL
	jim->f_bf8 = 1.0f;										// shake time
	const OMedia3DVector a = Position(carl->f_1d78), b = Position(carl);
	const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
	jim->f_c00 = 1.0f - sqrtf(dx * dx + dy * dy + dz * dz) * 0.0002f;	// 0x4d342c
	if (jim->f_c00 < 0.0f) jim->f_c00 = 0;
}

// --------------------------------------------------------------------------- construction

C3DCarl::C3DCarl()											// 0x425840
{
	SetName((char *)"C3DCARL");
	Log("C3DCarl()");
	f_649 = 0;												// ev69 loads the animations later
	C3DCarl::InitObject();									// 0x43e920 inlined: InitObject() called directly
	C3DObject::SetObjectID('3CAR');							// 0x49ab00
	f_96c = 1;												// waiting
	f_c08 = 1;												// AIState
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 200.0f;											// speed
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 400.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_b3c, (char *)"WALK");
	C3DAI::ev106(f_a9c, (char *)"STOP");					// waiting and sitting are stopping
	C3DAI::ev106(f_b14, (char *)"STOP");
	f_1d7c.db = g_screens_db;								// the hunger bar's frame (set up like Create)
	f_1d7c.canvas_id = 411;
	f_1d7c.x = 0;
	f_1d7c.y = 0;
	f_1d7c.src_blend = omblendfc_Src_Alpha;
	f_1d7c.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1d7c.home_x = 0;
	f_1d7c.home_y = 0;
	f_1d7c.f_34 = 1;
	f_1e80.db = g_screens_db;								// and the bar
	f_1e80.canvas_id = 359;
	f_1e80.x = 0;
	f_1e80.y = 0;
	f_1e80.src_blend = omblendfc_Src_Alpha;
	f_1e80.dest_blend = omblendfc_Inv_Src_Alpha;
	f_1e80.home_x = 0;
	f_1e80.home_y = 0;
	f_1e80.f_34 = 1;
	f_6d0 = 0;												// OMT morphs
	v94(2);													// CGameObject +0x178
	f_1d74 = 0;
	f_1d75 = 0;
	v64(0.0001f);											// CGameObject +0x100
	f_1d60.y = 15.0f;										// the shadow 15 above our origin
	f_1d78 = NULL;
	f_1f84 = 0;
	f_1f88 = 0;
	g_545f34 = 0;											// CARLSHRINK not called yet
	// (then a call to the empty 0x463890)
}

C3DCarl::~C3DCarl()											// 0x425af0
{
	f_1d7c.Hide();											// 0x4826f0
	f_1e80.Hide();
}

namespace neg {
CGameObject *New_C3DCarl(void) { return new C3DCarl; }					// '3CAR' (0x1f98 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DCarl::InitObject(void)		{ FriendInitObjectAIAnim(); }		// 0x43e920
void C3DCarl::v266(void)			{ FriendTaskStateRewrite(); }		// 0x4368e0

void C3DCarl::UnInitObject(void)							// 0x425bb0
{
	Log("UnInitObject()");
	C3DAnimated::UnInitObject();							// 0x41ab10
	// (then a call to the empty 0x463890)
}

void C3DCarl::AddTouching(CGameObject *other)				// 0x425e90
{
	CGameObject::AddTouching(other);						// 0x4ab970 (sic: not C3DAI's)
	if (g_gametype->m_object_id != 'LV5C') return;			// +0x490: only the giant Carl of the museum
	if (!other->NameContains((char *)"C3DWATERBALLOON")) return;	// +0x18
	if (IsAnim((char *)"SHRINK")) return;					// 0x41c6d0
	float v[4];
	other->v174(v);											// +0x2b8: the balloon's velocity
	OMedia3DVector vel(v[0], v[1], v[2], v[3]);
	const OMedia3DVector f0 = LocalDirection(this, 0, 0, 1.0f);	// +0x388 (slot 226): the way we face
	const float speed = sqrtf(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
	OMedia3DVector fwd = f0;
	float inv = 1.0f / fwd.quick_magnitude();
	fwd.x *= inv; fwd.y *= inv; fwd.z *= inv;
	inv = 1.0f / vel.quick_magnitude();
	vel.x *= inv; vel.y *= inv; vel.z *= inv;
	const float dot = vel.z * fwd.z + vel.y * fwd.y + vel.x * fwd.x;
	// (debug log "dot is %f")
	float limit;											// how square in the face it must be
	if (g_difficulty == 0) limit = 0.5f;
	else if (g_difficulty == 1) limit = 0.0f;
	else if (g_difficulty == 2) limit = -0.7f;
	else limit = dot;
	if (dot < limit)
	{
		// in his mouth: he eats it
		other->v136(0);										// +0x220
		other->v132(0);										// +0x210
		if (!(f_1f84 < 100.0f) && !g_545f34)				// fed enough
		{
			ev58((char *)"SHRINK", 0);						// element +0xe8
			ev107(1);										// +0x1ac
			f_1d7c.Hide();									// 0x4826f0
			f_1e80.Hide();
			return;
		}
		if (g_difficulty == 0) f_1f84 += 3.0f;
		else if (g_difficulty == 1) f_1f84 += 2.0f;
		else if (g_difficulty == 2) f_1f84 += 1.0f;
		ev58((char *)"EAT", 1);
		PlayGameSound(-1, std::string("bigcarlsound2"), false);	// 0x48f420
		ev107(1);
		C3DSprite *balloon = dynamic_cast<C3DSprite *>(other);	// (other - 0xc8)
		if (balloon) balloon->HideAgain();					// sprite +0xd8
		other->v132(0);
		other->v136(0);
		f_1f88 = 8.0f;										// the bar shows while he eats
		if (f_1f84 > 0.0f) return;
		f_1d7c.Hide();
		f_1e80.Hide();
		ev107(1);
		return;
	}
	// it bounces off: reflected about itself (sic: r = v - 2 (v.v) v, i.e. back the way it
	// came), a little random spread in x and z, at a quarter of its speed
	PlayGameSound(-1, std::string("boing"), false);
	const float s = -(vel.z * vel.z + vel.y * vel.y + vel.x * vel.x);
	OMedia3DVector r(s * vel.x + s * vel.x + vel.x, vel.y * s + vel.y * s + vel.y, vel.z * s + vel.z * s + vel.z, 0);
	inv = 1.0f / r.quick_magnitude();
	r.x *= inv; r.y *= inv; r.z *= inv;
	const int r1 = Random1001(), r2 = Random1001();
	const float rx = ((float)r1 * 0.001f - 0.5f) + r.x;
	const float rz = ((float)r2 * 0.001f - 0.5f) + r.z;
	const float k = speed * 0.25f;							// 0x4d340c
	other->v176(k * rx, k * r.y, k * rz);					// +0x2c0: its new velocity
	other->v136(0);
	C3DWaterBalloon *wb = dynamic_cast<C3DWaterBalloon *>(other);	// (other - 0xc8)
	if (wb) wb->f_634 = NULL;								// it stops homing (port: the original writes it unchecked)
}

void C3DCarl::Process(float dt)								// 0x426380
{
	C3DFriends::Process(dt);								// 0x436eb0
	if (strcasecmp(ev66(), "WALK") == 0)					// element +0x108
	{
		if (f_598 > 1.7f && !f_1d75)						// 0x4cb1f0: the second step of the walk
		{
			// (debug log "BOOM2")
			Step(this);
			f_1d75 = 1;
		}
		if (f_598 > 0.35f && !f_1d74)						// 0x4d3428: the first step
		{
			// (debug log "BOOM1")
			Step(this);
			f_1d74 = 1;
		}
	}
	if (!(f_1f84 < 100.0f)) return;
	if (f_1f88 > 0.0f)
	{
		f_1f88 -= dt;
		if (f_1f88 <= 0.0f)									// done eating: walk on
		{
			f_1f88 = 0;
			ev107(3);										// element +0x1ac
			ev58((char *)"WALK", 1);						// +0xe8
		}
		OMedia3DVector p = v224(0, 1800.0f, 400.0f);		// +0x380: over his head
		float pos[4] = { p.x, p.y, p.z, p.w };
		float sx, sy;
		if (v281(g_viewport, pos, &sx, &sy) && (g_game_mode == 5 || g_game_mode == 6))	// +0x464
		{
			f_1d7c.Show();									// 0x482520
			PositionSprite(f_1d7c.canvas_elt, sx + 320.0f - 26.0f, 240.0f - sy - 12.0f);	// 0x4044c0
			f_1e80.Show();
			PositionSprite(f_1e80.canvas_elt, sx + 320.0f - 24.0f, 242.0f - sy - 12.0f);
			f_1e80.Scale(f_1f84 * 0.5f, 11.0f);				// 0x482390
			return;
		}
	}
	f_1d7c.Hide();											// 0x4826f0
	f_1e80.Hide();
}

void C3DCarl::v260(void)									// 0x425da0
{
	C3DAI::v260();											// 0x4143a0
	f_1d78 = FindObjectByTag("JIM1");
	f_1d74 = 0;
	f_1d75 = 0;
	if (g_gametype->m_object_id != 'LV5F') return;			// +0x490: small Carl
	ev106(f_ac4, (char *)"RUN");							// element +0x1a8
	ev106(f_a9c, (char *)"SMALLSTOP");
	ev106(f_b14, (char *)"SMALLSTOP");
	ev106(f_b14, (char *)"SMALLSTOP");						// (twice, as shipped)
	ev106(f_a74, (char *)"SMALLSTOP");
}

void C3DCarl::v267(int level)								// 0x425e60
{
	C3DAnimated::v267(level);								// 0x41c880
	ev70(1);												// element +0x118
}

void C3DCarl::WriteState(OMediaStreamOperators &s)			// 0x4269e0
{
	C3DAI::WriteState(s);									// 0x415760
	s << f_1f84;
}

void C3DCarl::ReadState(OMediaStreamOperators &s)			// 0x4269b0
{
	C3DAI::ReadState(s);									// 0x4155e0
	s >> f_1f84;
}

// --------------------------------------------------------------------------- element family

void C3DCarl::ev58(char *name, unsigned char loop)			// 0x4268f0
{
	// GROW and KONG fall through to the final call too, so they start twice (as shipped).
	if (strcasecmp(name, "GROW") == 0)
	{
		f_1d60.y = -2000.0f;								// the shadow out of sight
		C3DAnimated::ev58(name, loop);						// 0x41bea0
	}
	if (strcasecmp(name, "KONG") == 0)
	{
		f_1d60.y = -2000.0f;
		C3DAnimated::ev58(name, loop);
		PlayGameSound(-1, std::string("bigcarlsound3"), false);	// 0x48f420
	}
	if (strcasecmp(name, "WALK") == 0)
	{
		f_1d60.y = -2000.0f;
		C3DAnimated::ev58((char *)"WALK", 0);				// (sic: never looped; ev67 restarts it)
		return;
	}
	C3DAnimated::ev58(name, loop);
}

void C3DCarl::ev67(void)									// 0x4267e0
{
	// (debug log, empty)
	if (strcasecmp(ev66(), "WALK") == 0)					// a walk cycle ended: the next one
	{
		f_1d74 = 0;
		f_1d75 = 0;
		f_598 = 0;
		C3DAnimated::ev58((char *)"WALK", 0);				// 0x41bea0
	}
	if (strcasecmp(ev66(), "SHRINK") != 0 || g_545f34) return;
	CGameObject *t = FindObjectByTag("CARLSHRINK");
	if (!t)
	{
		ErrorLog("ERROR: could not find CARLSHRINK trigger");	// 0x4b3b10
		return;
	}
	if (!t->NameContains((char *)"C3DTRIGGERTYPE")) return;	// +0x18
	JimmyActivateMenuItem(g_jimmy, this, this, 0, 0.0f);	// Jimmy element +0x228 (C3DPlayer.h hook)
	AddScore(400);											// 0x482b50
	C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(t);	// (t - 0xc8)
	if (tt) tt->CallNextTrigger("CARLSHRINK", 0);			// sprite +0xdc
	g_545f34 = 1;
}

void C3DCarl::ev69(void)									// 0x425be0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"WALK", (char *)"giantcarlwalk.ase");	// +0xe0
	CreateAnim((char *)"KONG", (char *)"giantcarlkong.ase");
	CreateAnim((char *)"SHRINK", (char *)"giantcarlshrink.ase");
	CreateAnim((char *)"SMALLTALK", (char *)"carltalk.ase");
	CreateAnim((char *)"SMALLSTOP", (char *)"carlstop.ase");
	CreateAnim((char *)"SMALLWALK", (char *)"carlwalk.ase");
	CreateAnim((char *)"GROW", (char *)"carlgrow.ase");
	CreateAnim((char *)"DRINK", (char *)"carldrink.ase");
	CreateAnim((char *)"EAT", (char *)"giantcarleat.ase");
	CreateAnim((char *)"RUN", (char *)"carlrun.ase");
	CreateAnim((char *)"TALK", (char *)"giantcarltalk.ase");
	if (g_gametype->m_object_id == 'LV5F')					// +0x490
		CreateAnim((char *)"STOP", (char *)"carlstopsmall.ase");
	else
	{
		CreateAnim((char *)"STOP", (char *)"giantcarlstop.ase");
		m_show_on_radar = 1;								// (element +0x580)
	}
	CreateTexture((char *)"carl.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	if (g_gametype->m_object_id == 'LV5C') ev58((char *)"WALK", 1);	// +0xe8
	else ev58((char *)"SMALLSTOP", 1);
	v67(100.0f);											// CGameObject +0x10c: radius
	v64(0.75f);												// CGameObject +0x100
}
