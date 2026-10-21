/*  C3DFish - see C3DFish.h. Reconstructed from Neutron2.exe 0x431df0..0x432b20. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFish.h"
#include "C3DGoddard.h"
#include "objects/C3DAITrigger.h"
#include "Game.h"
#include "Todo.h"
#include "CGameType.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMedia3DShape.h"
#pragma GCC diagnostic pop
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

// The animated object behind a CGameObject (the original subtracts 0xc0 without looking).
static C3DAnimated *Anim(CGameObject *o)	{ return o ? dynamic_cast<C3DAnimated *>(o) : NULL; }

// --------------------------------------------------------------------------- not reconstructed yet

__attribute__((weak)) void neg::JimmyHurt(CGameObject *jimmy, float damage, int points, float time, unsigned char a)
{
	(void)jimmy; (void)damage; (void)points; (void)time; (void)a;
	NEG_TODO(0x44bd90, "C3DJimmy element slot 134 (hurt)");
}

// C3DMiniSub (element +0x9f4: the C3DJimmy driving it): Jimmy +0x4cb8 = 0, then his element
// slot 134 (-5, -5, 5, 1).
static void MiniSubHurtDriver(CGameObject *sub)
{
	(void)sub;
	NEG_TODO(0x4328a1, "C3DMiniSub +0x9f4 (driver): +0x4cb8 = 0, element slot 134");
}

// C3DMiniSub: +0xa30 = 5.0, +0x9fc = 1, +0xa3c = 0, +0xa00 = 0 (it crashes).
static void MiniSubCrash(CGameObject *sub)
{
	(void)sub;
	NEG_TODO(0x4328f6, "C3DMiniSub +0xa30 / +0x9fc / +0xa3c / +0xa00 (crash)");
}

// C3DGoddard element +0x1b60 = 1 (he rides along).
static void GoddardSetRiding(CGameObject *goddard)
{
	C3DGoddard *g = dynamic_cast<C3DGoddard *>(goddard);
	if (g) g->f_1b60 = 1;									// 0x432709 (port: the original writes +0x1b60 of whatever the object is)
}

// --------------------------------------------------------------------------- construction

C3DFish::C3DFish()											// 0x431df0
{
	SetName((char *)"C3DFISH");
	Log("C3DFish()");
	f_649 = 0;												// ev69 loads the animations later
	C3DFish::InitObject();									// called directly (as shipped: the
															// loader's InitObject runs it again)
	C3DObject::SetObjectID('3FIS');
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 500.0f;											// Speed
	f_96c = 3;												// patrolling
	f_c08 = 3;
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 0;
	f_b9c.x = 0.5f;											// turn rates
	f_b9c.y = 0.8f;
	f_b9c.z = 0.5f;
	v42(1);
	v40(1);
	v34(0);
	C3DObject::InitPhysics();
	C3DAI::ev106(f_ac4, (char *)"WALK");					// swimming is all it does
	C3DAI::ev106(f_a9c, (char *)"WALK");
	C3DAI::ev106(f_b14, (char *)"WALK");
	C3DAI::ev106(f_a74, (char *)"WALK");
	C3DAI::ev106(f_b3c, (char *)"WALK");
	C3DAI::ev106(f_aec, (char *)"WALK");
	strcpy(f_1b50, "dolphinswim.ase");
	strcpy(f_1bb4, "dolphinstop.ase");
	strcpy(f_1c18, "none");
	strcpy(f_1c7c, "none");
	strcpy(f_1ce0, "dolphin01.png");
	f_bd4 = 1;												// hovering
	f_1d44 = 0;
	f_1d48 = 0;
	f_1d4c = NULL;
	f_1d50 = NULL;
	f_1d54 = NULL;
	f_1d58 = 0;
}

C3DFish::~C3DFish()											// 0x4320f0
{
}

namespace neg {
CGameObject *New_C3DFish(void) { return new C3DFish; }					// '3FIS' (0x1d64 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DFish::InitObject(void)								// 0x432240
{
	Log("InitObject()");
	C3DAI::InitObject();
	RegisterProperty("ASEWalk", f_1b50, PROP_ALPHA, 0);
	RegisterProperty("ASEAnim2", f_1c18, PROP_ALPHA, 0);
	RegisterProperty("ASEAnim3", f_1c7c, PROP_ALPHA, 0);
	RegisterProperty("ASEStop", f_1bb4, PROP_ALPHA, 0);
	RegisterProperty("PNGFile", f_1ce0, PROP_ALPHA, 0);
	RegisterProperty("Speed", &f_968, PROP_FLOAT, 0);
	RegisterProperty("DoubleSided", &f_1d44, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("SpecialCase", &f_1d48, PROP_LONG, 0);
}

void C3DFish::AddTouching(CGameObject *other)				// 0x432820
{
	C3DAI::AddTouching(other);
	if (f_96c == 0 && other == g_camera)					// attacking and touched the player: a bite
	{
		if (other->NameContains((char *)"C3DJIMMY")) JimmyHurt(other, -10.0f, -10, 5.0f, 1);	// element +0x218
		if (other->NameContains((char *)"C3DMINISUB"))
		{
			MiniSubHurtDriver(other);
			PlayGameSound(-1, std::string("rocketcrash"), false);	// 0x48f420
			MiniSubCrash(other);
		}
		ev107(5);											// and away
		f_c10 = 5800.0f;
		f_c3c = 20.0f;
	}
	if (f_1d48 == 1 && other->NameContains((char *)"C3DWATERBALLOON"))
	{
		ev107(1);											// the dragonfly drops
		f_bd4 = 0;
	}
	if (f_1d48 == 2 && other->NameContains((char *)"C3DJIMMY")) JimmyHurt(other, -10.0f, -10, 5.0f, 1);
	if (f_1d48 == 5 && other->NameContains((char *)"C3DAITRIGGER") && strcasecmp(other->m_tag, "BARRELROLL") == 0)
	{
		C3DAITrigger *t = dynamic_cast<C3DAITrigger *>(other);	// (element - 0xc8)
		if (!t) return;										// port: a name with "C3DAITRIGGER" that is not one
		if (t->m_ai_new_rot_y == -1.0f) f_1d58 = 0.75f;		// AINewRotY: the roll time
		else f_1d58 = t->m_ai_new_rot_y;
	}
}

unsigned char C3DFish::v231(CGameObject *other)				// 0x432a60
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3JIM' || id == '3AIT' || id == '3DOG' || id == '3LBW';
}

void C3DFish::Process(float dt)								// 0x4324e0
{
	C3DAI::Process(dt);
	if (f_1d48 == 5)										// carrying the rocket, Jimmy and Goddard
	{
		v136(0);
		HideAgain();										// element +0xdc
		f_008 = 1;
		float p[4], r[4];
		if (f_1d4c)
		{
			v195(p); f_1d4c->v196(p[0], p[1], p[2], p[3]);
			v201(r); f_1d4c->v202(r[0], r[1], r[2], r[3]);
			C3DObject *o = dynamic_cast<C3DObject *>(f_1d4c);
			if (o) o->ShowAgain();							// element +0xd8
		}
		if (f_1d50)
		{
			v195(p); f_1d50->v196(p[0], p[1], p[2], p[3]);
			v201(r); f_1d50->v202(r[0], r[1], r[2], r[3]);
		}
		if (f_1d54)
		{
			v195(p); f_1d54->v196(p[0], p[1], p[2], p[3]);
			v201(r); f_1d54->v202(r[0], r[1], r[2], r[3]);
			C3DAnimated *g = Anim(f_1d54);
			if (g && !g->IsAnim((char *)"ROCK")) g->ev58((char *)"ROCK", 1);
			GoddardSetRiding(f_1d54);
			f_1d54->v132(0);
			C3DAI *ai = dynamic_cast<C3DAI *>(f_1d54);
			if (ai) ai->ev121();							// element +0x1e4
			const OMedia3DVector s = v224(0, 10.0f, -25.0f);	// his seat
			f_1d54->v196(s.x, s.y, s.z, s.w);
			f_1d54->v132(0);
		}
	}
	if (f_1d58 > 0)											// barrel roll
	{
		f_1d58 -= dt;
		if (f_1d58 <= 0) f_1d58 = 0;
		AddRotation(0, 0, dt * -360.0f);					// 0x4ccee8
	}
}

void C3DFish::v260(void)									// 0x432350
{
	C3DAI::v260();
	if (f_1d48 == 2) v94(2);
	InitAnim();												// element +0x110
	if (f_1d44 && f_590) f_590->set_two_sided(true);		// DoubleSided (shape vtable +0x74; port: f_590 checked)
	if (f_664 == 1) ev73();									// SecondPass
	if (f_1d48 == 1)										// the dragonfly
	{
		PlayGameSound3DNoDoppler(this, -1, std::string("dragonfly"), true);	// 0x48f0f0
		f_b9c.x = 3.0f;
		f_b9c.y = 4.0f;
		f_b9c.z = 3.0f;
		v42(0);
		v40(0);
		v67(100.0f);
	}
	if (f_658 == 0)											// HasCollision
	{
		v136(0);
		v42(0);
		v40(0);
	}
	if (f_1d48 == 5)
	{
		f_1d4c = FindObjectByTag("C3DROCKETSHIP");
		f_1d50 = FindObjectByTag("JIM1");
		C3DAnimated *jim = Anim(f_1d50);					// port: checked (the original calls through NULL)
		if (jim)
		{
			jim->ev58((char *)"DRIVE", 1);
			f_1d50->v132(0);
		}
		f_1d54 = FindObjectByTag("C3DGODDARD");
	}
}

// --------------------------------------------------------------------------- element family

void C3DFish::ev69(void)									// 0x432140
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"WALK", f_1b50);						// element +0xe0
	if (strcasecmp(f_1c18, "none") != 0) CreateAnim((char *)"ANIM2", f_1c18);
	if (strcasecmp(f_1c7c, "none") != 0) CreateAnim((char *)"ANIM3", f_1c7c);
	CreateAnim((char *)"STOP", f_1bb4);
	CreateTexture(f_1ce0, 0);								// +0xfc
	ev58((char *)"WALK", 1);
	ApplyTexture(f_590, 0);									// +0x100
	v67(500.0f);											// radius
	v64(0.1f);
}
