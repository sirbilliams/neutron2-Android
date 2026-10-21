/*  C3DMiniSub - see C3DMiniSub.h. Reconstructed from Neutron2.exe 0x4523e0..0x454cc0, 0x46c830,
    0x46c840 and 0x43ba20 (shared). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DMiniSub.h"
#include "C3DBub.h"
#include "C3DLight.h"
#include "C3DSpotLight.h"
#include "C3DSubBubble.h"
#include "../C3DGoddard.h"
#include "../C3DGrannyObj.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"										// PlayerKey*
#include "../C3DRocketShip.h"									// the Jimmy hooks
#include "../C3DSprite.h"
#include "../CGameType.h"
#include "../CJimmyGame.h"
#include "../Bridge.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Mouse.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Todo.h"
#include "../gen/globals.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DShape.h"
#include "OMediaStreamOperators.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- small helpers

static OMedia3DVector Position(CGameObject *o)	{ float p[4]; o->v195(p); return OMedia3DVector(p[0], p[1], p[2], p[3]); }
static OMedia3DVector Angles(CGameObject *o)	{ float a[4]; o->v201(a); return OMedia3DVector(a[0], a[1], a[2], a[3]); }

static float Distance(const OMedia3DVector &a, const OMedia3DVector &b)	// (x87: dz^2 + dy^2 + dx^2)
{
	const double dx = (double)a.x - b.x, dy = (double)a.y - b.y, dz = (float)((double)a.z - b.z);
	return (float)sqrt(dz * dz + dy * dy + dx * dx);
}

// The element (object - 0xc0) of g_jimmy (0x453130).
static C3DAnimated *JimmyElement(void)	{ return g_jimmy ? dynamic_cast<C3DAnimated *>(g_jimmy) : NULL; }

// A trail bubble's sprite element (game object view - 0xc8).
static C3DSprite *BubSprite(CGameObject *b)	{ return b ? dynamic_cast<C3DSprite *>(b) : NULL; }

static CJimmyGame *Game(void)	{ return dynamic_cast<CJimmyGame *>(g_gametype); }	// port: checked

// 0x48f730 (CLocalGameObject family): the nearest object with ObjectID id1 or id2 that is
// active (v123) and f_140, by quick_magnitude of (o - it).
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

// 0x4af000: the object whose save index (f_438) is <index>.
static CGameObject *FindObjectByIndex(int index)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && (int)(*i)->f_438 == index) return *i;
	return NULL;
}

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

static void Play(const char *name)	{ PlayGameSound(-1, std::string(name), false); }	// 0x48f420

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

// 0x4a39f0 (CAweReal): the mouse steering, (dx, dy, dt, 8, 6, 40, 20) here. (As in
// C3DHoverCycle.cpp: per axis, outside the dead zone +-lim the distance of the pointer from the
// viewport's centre / k, held while it grows; every 0.2 s the pointer goes back to the centre.)
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
		if (kx <= 0.0f) kx = 0.0001f;
		if (rx > limx)
		{
			g_56aeba = (short)(int)((float)rx / kx);
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
	if (g_56aea8 > 0.0)
	{
		g_56aea8 = g_56aea8 - dt;
		return;
	}
	CenterMouse();											// 0x4b1d50
	g_56aea8 = 0.2f;
}

// --------------------------------------------------------------------------- construction

C3DMiniSub::C3DMiniSub()									// 0x4523e0
{
	f_a40[3] = 1.0f;										// 0xa4c (the point's w)
	SetName((char *)"C3DMINISUB");							// 0x4ab1b0
	Log("C3DMiniSub()");
	C3DMiniSub::InitObject();								// 0x452a30 (called directly)
	v64(0.6f);
	v38(1);
	v46(1);
	C3DObject::SetObjectID('3SUB');							// 0x49ab00
	f_9f0 = 0;
	f_9f4 = NULL;
	v44(0);
	v50(0);
	f_9f8 = NULL;
	f_9fc = 0;
	f_a00 = 0;
	f_a40[0] = 2.0f;
	f_a40[1] = 4.0f;
	f_a40[2] = 1.1f;
	f_9a0.x = 0;											// the chase camera
	f_9a0.y = 300.0f;
	f_9a0.z = -500.0f;
	f_9b0.x = 0;
	f_9b0.y = 50.0f;
	f_9b0.z = 200.0f;
	f_968 = 650.0f;											// MaxSpeed
	f_988 = 460.0f;											// UpRate
	f_98c = -460.0f;										// DownRate
	f_960 = 100.0f;											// AccelRate
	f_990 = 950.0f;											// MaxVertVelocity
	if (!f_4aa) C3DObject::InitPhysics();					// (InitPhysics inlined)
	f_a34 = -1;
	f_a38 = 0;												// port: not set by the original
	f_a3c = 0;
	g_545f54 = 0;
	f_a50 = 0;
	f_a51 = 0;
	f_a52 = 0;
	f_a54 = NULL;
	f_a2c = 0;												// port: not set by the original

	f_a58 = new C3DSubBubble;								// 0x4747f0 (0x944 bytes)
	f_a5c = 0;
	for (int i = 0; i < 10; ++i)
		f_a04[i] = new C3DBub;								// 0x422cf0 (0x67c bytes; game object at +0xc8)
	f_a64 = new C3DSpotLight;								// 0x4721f0 (0x934 bytes)
	f_a64->f_928 = 1;										// on

	f_a74 = new C3DLight;									// 0x49a280 (0x604 bytes)
	f_a74->v107(0);											// +0x1ac
	f_a74->m_specular[0] = 0;
	f_a74->m_specular[1] = 0;
	f_a74->m_specular[2] = 0;
	f_a74->m_specular[2] = 0;								// (twice)
	f_a74->m_diffuse[2] = 1.0f;
	f_a74->m_diffuse[0] = 0;
	f_a74->m_diffuse[1] = 0;
	f_a74->m_range = 5000.0f;
	f_a74->m_ambient[2] = 0.5f;
	f_a74->m_ambient[0] = 0.5f;
	f_a74->m_ambient[1] = 0.5f;
	f_a74->m_attenuation = 0.9f;
	f_a74->v258();											// +0x408 apply
	f_a74->InitLight(1.0f, 0, 0, 1.0f);						// element +0xb0: blue
	f_a60 = 0;
	f_a68 = 0;
	f_a7c = 0;
	f_a69 = 0;
	f_660 = 1;
	// (then a call to the empty 0x463890)
}

C3DMiniSub::~C3DMiniSub() {}								// 0x4528f0

// --------------------------------------------------------------------------- element-family slots

void C3DMiniSub::ev67(void)									// 0x453770
{
	// (a call to the empty 0x463890)
	if (strcasecmp(ev66(), "GRAB") == 0) ev58((char *)"DEFAULT", 1);	// element +0x108, 0x4c4520
}

// 0x453440: the engine sound state. Only stopping is left: the channel f_a34 is never started.
void C3DMiniSub::ev108(int state)
{
	switch (state)
	{
		case 0:
			if (f_a34 != -1)
			{
				StopGameSound(f_a34, false);				// 0x48f0d0
				f_a34 = -1;
			}
			break;
		case 1:
		case 2:
			if (f_a38 != state && f_a34 != -1)
				StopGameSound(f_a34, false);				// (f_a34 is not cleared here)
			break;
	}
	f_a38 = state;
}

// 0x454870: the claw camera (close and above, slow handling) on / off.
void C3DMiniSub::ev111(unsigned char claw)
{
	if (!f_a50)
	{
		if (!claw) return;
		f_a50 = 1;
		f_9a0.x = 0;
		f_9a0.y = 700.0f;
		f_9a0.z = 100.0f;
		f_988 = 30.0f;
		f_98c = -30.0f;
		f_960 = 10.0f;
		f_964 = 40.0f;
		f_990 = 15.0f;
		f_9c4 = 5.0f;
		f_9c8 = -5.0f;
		f_96c = 0;
	}
	else if (!claw)
	{
		f_a50 = 0;
		f_9a0.x = 0;
		f_9a0.y = 300.0f;
		f_9a0.z = -500.0f;
		f_964 = 1.0f;
		f_988 = 160.0f;
		f_98c = -160.0f;
		f_960 = 100.0f;
		f_990 = 350.0f;
		f_9c4 = 20.0f;
		f_9c8 = -20.0f;
	}
}

CGameObject *C3DMiniSub::ev112(void)						// 0x453730
{
	if (++f_a60 >= 10) f_a60 = 0;
	return f_a04[f_a60];
}

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DMiniSub::InitObject(void)							// 0x452a30
{
	Log("InitObject()");
	C3DFlyingObject::InitObject();							// 0x4349d0
	InitAnim();												// element +0x110
	CreateAnim((char *)"DEFAULT", (char *)"SUBMOVE.ase");
	CreateAnim((char *)"GRAB", (char *)"SUBclawpropeler.ase");
	CreateAnim((char *)"STOP", (char *)"SUBstop.ase");
	CreateTexture((char *)"sub.png", 0);
	ApplyTexture(f_590, 0);
	ev58((char *)"DEFAULT", 1);
	v67(300.0f);											// radius
}

void C3DMiniSub::ResetObject(void)							// 0x46c840
{
	C3DFlyingObject::ResetObject();							// 0x434b10
	if (!f_9f4) f_96c = 0;
}

void C3DMiniSub::InitPhysics(void)							// 0x43ba20
{
	if (!f_4aa) C3DObject::InitPhysics();
}

void C3DMiniSub::AddTouching(CGameObject *other)			// 0x452af0
{
	C3DFlyingObject::AddTouching(other);					// 0x435eb0
	if (!other->NameContains((char *)"C3DMISSILE") || f_9fc) return;
	JimmySetHurtFlag(g_jimmy, 0);							// +0x4cb8
	JimmyHurt(g_jimmy, -10.0f, -10, 5.0f, 1);				// element +0x218
	Play("rock explosion2");
	if (C3DObject *m = dynamic_cast<C3DObject *>(other)) m->HideAgain();	// element +0xdc
	f_9fc = 1;
	f_a3c = 1;
	f_a30 = 2.0f;
	f_a00 = 0;
}

void C3DMiniSub::v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly)	// 0x4534f0
{
	(void)nx; (void)ny; (void)nz; (void)nw; (void)poly;
	float v[4];
	v174(v);
	const float vz = v[2];
	v174(v);
	v176(v[0], -120.0f, vz);								// the ceiling stops the climb
}

// 0x453150: a wall or floor touched while Jimmy drives (and not in claw mode or carrying).
void C3DMiniSub::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)
{
	(void)a; (void)b; (void)c; (void)d;
	if (f_a50 || f_a54 || !f_9f4 || !poly || !poly->get_material()) return;
	{
		const int id = (int)poly->get_material()->get_chunk_ID();	// material +0xc -> +8 (no chunk (0): the walls
															// below, as in the original)
		if (id >= 130 && id <= 139)
		{
			if (f_9fc) return;
			JimmySetHurtFlag(g_jimmy, 0);
			JimmyHurt(g_jimmy, -10.0f, -10, 5.0f, 1);
			Play("rocket hit");
			f_9fc = 1;
			f_a3c = 0;
			f_a00 = 0;
			f_a30 = 5.0f;
			return;
		}
		if (id == 253)
		{
			ev109();										// element +0x1b4 turn round
			return;
		}
		if (id >= 185 && id <= 189)
		{
			if (JimmyHurtFlag(g_jimmy)) return;
			JimmyHurt(g_jimmy, -5.0f, -5, 5.0f, 1);
			f_a30 = 5.0f;
			f_9fc = 1;
			f_a3c = 0;
			f_a00 = 0;
			Play("SPLASH");
			Play("pain agh ahh");
			return;
		}
		if (id == 170 || id == -1) return;
	}
	if (f_9fc) return;
	if (g_difficulty == 1)
	{
		JimmySetHurtFlag(g_jimmy, 0);
		JimmyHurt(g_jimmy, -0.2f, -1, 5.0f, 1);
	}
	else if (g_difficulty == 2)
	{
		JimmySetHurtFlag(g_jimmy, 0);
		JimmyHurt(g_jimmy, -0.5f, -1, 5.0f, 1);
	}
	else return;
	Play("subcrash2");
	f_a30 = 5.0f;
	f_9fc = 1;
	f_a3c = 0;
	f_a00 = 0;
}

void C3DMiniSub::Process(float dt)							// 0x452bd0
{
	C3DFlyingObject::Process(dt);							// 0x434c30
	if (g_gametype->IsPaused()) return;						// +0x114
	f_a7c += dt;

	// the blue light follows, its green pulsing
	OMedia3DVector p = Position(this);
	f_a74->v196(p.x, p.y, p.z, p.w);						// +0x310
	f_a74->v247();											// +0x3dc
	const float g = (float)((cos((double)f_a7c * 15.0f) + 1.0) * 0.5 * 0.1 + (double)0.6f);	// 0x4dce54, 0x4e9be8, 0x4e9be0
	f_a74->set_ambient(OMediaFARGBColor(0, 0.3f, g, 0.35f));	// light element +0xb0

	// the glass bubble and the beam
	if (f_a58)
	{
		p = Position(this);
		f_a58->v196(p.x, p.y, p.z, p.w);
		OMedia3DVector a = Angles(this);
		f_a58->v202(a.x, a.y, a.z, a.w);					// +0x328
		OMedia3DVector l = v224(0, -20.0f, 30.0f);			// +0x380
		f_a64->v196(l.x, l.y, l.z, l.w);
		a = Angles(this);
		f_a64->v202(a.x, a.y, a.z, a.w);
	}

	if (!f_9f4)
	{
		f_9cc = 0;
		return;
	}

	// Jimmy sits in it
	C3DAnimated *jim = JimmyElement();
	OMedia3DVector s = v224(0, -80.0f, -35.0f);
	jim->v196(s.x, s.y, s.z, s.w);							// (Jimmy's game object)
	jim->place(s.x, s.y, s.z);								// element +0x44
	OMedia3DVector a = Angles(this);
	jim->v203(a.x, a.y, a.z);								// +0x32c
	jim->set_angle(get_anglex(), get_angley(), get_anglez());	// element +0x50..+0x54
	if (f_9f8)												// and Goddard
	{
		OMedia3DVector q = v224(0, -45.0f, -55.0f);
		f_9f8->v196(q.x, q.y, q.z, q.w);
		f_9f8->place(q.x, q.y, q.z);
		f_9f8->v132(0);										// +0x210
		a = Angles(this);
		f_9f8->v203(a.x, a.y, a.z);
		if (f_9f8->f_1bbc)
		{
			unsigned int bits;								// (the slot takes the frame time)
			memcpy(&bits, &dt, 4);
			f_9f8->f_1bbc->v241(bits);						// his head prop follows (+0x3c4)
		}
	}

	// the hit flash
	if (!f_9fc) return;
	if (f_a3c) v159(0, -700.0f, 0);							// +0x27c pushed down
	f_a00 += dt;
	double k;
	if (f_a00 > f_a30)
	{
		k = 1.0f;
		f_9fc = 0;
		f_a3c = 0;
	}
	else k = (sin((double)f_a00 * 20.0f) + 1.0) * 0.5;	// 0x4caa40
	C3DJimmy *j = static_cast<C3DJimmy *>(jim);				// (g_jimmy is the C3DJimmy)
	if (j->f_4da0 > 0 || j->f_4da4 > 0) return;				// not while his bubble shields him
	if (f_5d4[0]) f_5d4[0]->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, (float)k, (float)k));	// +0x4c.. (r, g, b, a) = 1, k, k, 1; port: NULL checked
}

void C3DMiniSub::v243(float dt)								// 0x4537b0
{
	if (!GameIsRunning()) return;							// 0x4b0ca0

	// a trail bubble every 0.25 s
	f_a5c += dt;
	if (f_a5c > 0.25)										// 0x4e9c18 (a double)
	{
		f_a5c = (float)(f_a5c - 0.25);
		CGameObject *b = ev112();							// element +0x1c0
		OMedia3DVector p = v224(0, 90.0f, -80.0f);
		b->v196(p.x, p.y, p.z, p.w);
		dynamic_cast<C3DBub *>(b)->f_670 = 0.2f;			// (element +0x670)
		b->v176(0, 0, 0);									// +0x2c0
	}

	// above or under the water
	OMedia3DVector cam(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);	// 2002 +0x44
	CJimmyGame *game = Game();
	if (cam.y > 4390.0f)									// 0x4e9c14
	{
		f_9a0.y = f_99d ? -120.0f : 300.0f;
		if (!f_a69)
		{
			if (game && game->m_sky) game->m_sky->ShowAgain();	// +0x5c0, element +0xd8
			if (game) game->SetFog(0, 0.33f, 0.55f, 0, 9000.0f, 11000.0f, 14000.0f, true);	// +0x194
			if (f_a58)
			{
				f_a58->ev71(1.0f);							// +0x11c opaque
				f_a58->ev91(0);								// +0x16c
			}
			for (int i = 0; i < 10; ++i)
				if (C3DSprite *s = BubSprite(ev112())) s->HideAgain();	// element +0xd8
			Play("SPLASH");
			if (game) game->PlayLevelMusic("surfsound.omt", 0);	// +0x198
			f_a69 = 1;
		}
	}
	else
	{
		if (game) game->SetFog(0.05f, 0.227f, 0.533f, 0, 500.0f, 10000.0f, 11500.0f, true);
		const float y = Position(this).y;
		if (y < 4180.0f && Position(this).y > 3900.0f)		// 0x4e9c10, 0x4e9c0c
			f_9a0.y = f_99d ? -120.0f : 100.0f;
		else if (Position(this).y > 3900.0f && f_99d)
			f_9a0.y = -120.0f;
		else
			f_9a0.y = 300.0f;
		if (f_a69 == 1)
		{
			if (game && game->m_sky) game->m_sky->HideAgain();	// element +0xdc
			Play("SPLASH");
			f_a69 = 0;
			if (f_a58)
			{
				f_a58->ev71(0.4f);
				f_a58->ev91(1);
			}
			for (int i = 0; i < 10; ++i)
				if (C3DSprite *s = BubSprite(ev112())) s->ShowAgain();	// element +0xd4
			if (game) game->PlayLevelMusic("musicunderwater.omt", 1);
		}
	}

	// near a movable rock: the claw camera
	if (!f_a54)
	{
		CGameObject *rock = FindNearest(this, '3MOR', -1);	// 0x48f730
		if (rock)
		{
			OMedia3DVector claw = v224(0, 0, 520.0f);
			if (Distance(Position(rock), claw) < 520.0f) ev111(1);	// 0x4e9c08, element +0x1bc
			else ev111(0);
		}
	}

	// Speed: faster, for neutron energy
	if (PlayerKeySpeed())									// 0x465830
	{
		AddEnergy(dt * -0.5f);								// 0x485730
		f_968 = 1100.0f;
		if (f_96c < 1100.0f)								// 0x4e9c04
		{
			f_96c = f_960 + f_96c;							// (per frame)
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	else f_968 = 650.0f;

	// L: the headlight
	if (g_keys['L'])										// 0x56adec
	{
		if (!f_a68)
		{
			f_a68 = 1;
			const unsigned char on = f_a64->f_928 ? 0 : 1;
			f_a64->f_928 = on;
			g_5388c4 = on;
		}
	}
	else f_a68 = 0;

	// Action: the claw
	if (PlayerKeyAction())									// 0x465750
	{
		if (f_a50)
		{
			if (!f_a52)
			{
				f_a52 = 1;
				if (!f_a54)
				{
					ev58((char *)"GRAB", 0);
					CGameObject *rock = FindNearest(this, '3MOR', -1);
					OMedia3DVector claw = v224(0, 0, 620.0f);
					if (rock && Distance(Position(rock), claw) < 620.0f)	// 0x4e9c00 (the distance is taken first)
					{
						rock->v136(0);						// +0x220
						v152(850.0f);						// +0x260 radius
						rock->v132(0);						// +0x210
						f_a54 = rock;
						ev111(0);
					}
				}
			}
		}
		else if (!f_a52)
		{
			if (f_a54)
			{
				OMedia3DVector c = v224(0, 0, 620.0f);
				f_a54->v196(c.x, c.y, c.z, c.w);
				OMedia3DVector an = Angles(this);
				f_a54->v202(an.x, an.y, an.z, an.w);
				if (PlayerKeyAction() && !f_a52)			// drop it
				{
					f_a54->v136(1);
					v152(f_034);							// our radius again
					f_a54->v176(0, 0, 0);
					f_a54->v132(1);
					ev58((char *)"DEFAULT", 1);
					f_a54 = NULL;
					f_a52 = 1;
				}
			}
			f_a52 = 1;
			ev58((char *)"GRAB", 0);
		}
	}
	else f_a52 = 0;
	if (f_a54)												// carried 620 in front
	{
		OMedia3DVector c = v224(0, 0, 620.0f);
		f_a54->v196(c.x, c.y, c.z, c.w);
		OMedia3DVector an = Angles(this);
		f_a54->v202(an.x, an.y, an.z, an.w);
	}

	// turning
	short mx = 0, my = 0;
	MouseSteer(&mx, &my, dt, 8.0f, 6.0f, 40, 20);			// 0x4a39f0
	f_9d0 = (float)my;
	f_99e = 0;
	float turn = 0;
	if (PlayerKeyLeft())
	{
		turn = dt * 160.0f;									// 0x4e9bfc
		f_99e = 1;
	}
	else if (PlayerKeyRight())
	{
		turn = dt * -160.0f;								// 0x4e9bf8
		f_99e = 1;
	}
	else if (f_99e)											// (never: f_99e was just cleared)
	{
		if (f_978 < 0)
		{
			f_978 = dt * 160.0f + f_978;
			if (!(f_978 <= 0)) f_978 = 0;
		}
		else if (!(f_978 <= 0))
		{
			f_978 = f_978 - dt * 180.0f;
			if (f_978 < 0) f_978 = 0;
		}
	}
	else
	{
		f_99e = 0;
		if (mx <= 10 && mx >= -10)
		{
			f_99e = 0;
			if (f_978 > 3.0f) f_978 = f_978 - dt * 30.0f;	// 0x4cb1e8, 0x4ca4b4: back to straight
			else if (f_978 < -3.0f) f_978 = dt * 30.0f + f_978;
			else f_978 = 0;
		}
		else
		{
			if (mx > 40) mx = (short)(mx - 30);
			else if (mx < -40) mx = (short)(mx + 30);
			turn = (float)((double)mx * dt * 3.0f);
		}
	}
	f_978 = turn + f_978;

	// speed
	f_99c = 0;
	f_99d = 0;
	if (PlayerKeyGo() || f_9d0 > 15.0f)						// 0x465910, 0x4dce54
	{
		f_99c = 1;
		if (!PlayerKeyBrake() && f_96c < f_968)
		{
			f_96c = (float)((double)f_960 * 0.0909090936f + f_96c);	// 0x4e9bf4 (per frame)
			if (f_96c > f_968) f_96c = f_968;
		}
	}
	if (v133() && !f_a50)									// on the floor
	{
		f_9cc = 0;
		if (f_96c > 0)
		{
			f_96c = (float)(f_96c - (double)dt * 500.0f);	// 0x4cb200
			if (f_96c < 0) f_96c = 0;
		}
		else f_96c = 0;
	}
	if (!f_99c)
	{
		if (f_96c > 0) f_96c = (float)(f_96c - (double)dt * 40.0f);	// 0x4d65c8
		else f_96c = 10.0f;									// (it creeps on)
	}
	if (PlayerKeyBack() || f_9d0 < -15.0f)					// 0x465980, 0x4dce4c
	{
		f_99d = 1;
		if (f_96c > 0)
		{
			if (v133())
			{
				f_96c = (float)(f_96c - (double)dt * f_964);
				if (f_96c < 0) f_96c = 0;
			}
			else if (!PlayerKeyBrake())
			{
				f_96c = (float)((double)f_960 * 0.111111112f + f_96c);	// 0x4e9bf0 (per frame)
				if (f_96c > f_968) f_96c = f_968;
			}
		}
		else if (f_96c < 0)
		{
			f_96c = f_964 + f_96c;
			if (f_96c > 0) f_96c = 0;
		}
	}
	if (f_99c) f_99d = 0;
	if (PlayerKeyBrake() && f_96c > 10.0f)					// 0x4cb1cc
	{
		f_96c = (float)(f_96c - ((double)dt * f_960 + (double)dt * f_960));
		if (f_96c < 10.0f) f_96c = 10.0f;
	}
	if (f_9d0 > 10.0f) f_9d0 = (float)(f_9d0 - (double)dt * 20.0f);	// 0x4caa40
	if (f_9d0 < -10.0f) f_9d0 = (float)((double)dt * 20.0f + f_9d0);	// 0x4dce44
	if (f_978 < -45.0f) f_978 = -45.0f;						// 0x4dce40
	else if (f_978 > 45.0f) f_978 = 45.0f;					// 0x4dce3c

	// pitch and roll the model, and tell the engine
	if (f_99c)
	{
		v218(f_a69 ? 0.0f : f_9c4, Angles(this).y, -f_978);	// +0x368 (no climbing pitch at the surface)
		ev108(2);											// element +0x1b0
	}
	else if (f_99d)
	{
		v218(f_9c8, Angles(this).y, -f_978);
		ev108(0);
	}
	else if (!(f_96c > 0))
	{
		v218(0, Angles(this).y, -f_978);
		ev108(0);
	}
	else
	{
		v218(0, Angles(this).y, -f_978);
		ev108(1);
	}

	// the chase camera
	OMedia3DVector want = v224(f_9a0.x, f_9a0.y, f_9a0.z);	// +0x380
	want.y = Position(this).y + f_9a0.y;					// (the height is not turned with the sub)
	OMedia3DVector look = v224(f_9b0.x, f_9b0.y, f_9b0.z);
	float k = 1.0f;
	if (f_968 > 1000.0f && Position(this).y < 4180.0f)		// 0x4cb1d8, 0x4e9c10
		k = ev96(Position(this), &want, 1300.0f);			// element +0x180: kept within reach
	const float k2 = ev95(look, &want);						// element +0x17c: in front of walls
	ev98(dt, k2, k, cam, want, look, OMedia3DVector(1.2f, 1.9f, 1.2f, 0.0f));	// element +0x188

	// Jimmy aims from it
	if (f_9f4)
	{
		OMedia3DVector aim = v224(0, -15.0f, 0);
		JimmyRocketAim(f_9f4, dt, aim);						// his element +0x248 (ESLOT 146)
	}
}

void C3DMiniSub::RotateToDest(float dt)						// 0x453580
{
	if (!GameIsRunning() || !f_005 || !f_00a) return;		// 0x4b0ca0
	f_00a = 0;
	float dx = 0, dy = 0;
	double dz = 0;
	if (f_274 != f_224)										// wanted / current angles
	{
		dx = f_274 - f_224;
		if (dx > 180.0f) { dx -= 360.0f; f_00a = 1; }		// 0x4cb1f4
		else
		{
			if (dx < -180.0f) dx += 360.0f;					// 0x4cb204
			f_00a = 1;
		}
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
		dz = (double)f_27c - f_22c;							// (x87: kept unrounded)
		if (dz > 180.0f) dz -= 360.0f;
		else if (dz < -180.0f) dz += 360.0f;
		f_00a = 1;
	}
	AddRotation((float)((double)dx * f_a40[0] * dt), (float)((double)dy * f_a40[1] * dt),
				(float)(dz * f_a40[2] * dt));				// +0x330
}

void C3DMiniSub::v260(void)									// 0x452990
{
	C3DPlayerObject::v260();								// 0x464ed0
	v132(1);												// +0x210
	f_9fc = 0;
	f_a3c = 0;
	f_9d8 = 0;
	f_9d4 = 0;
	if (f_a58)
	{
		f_a58->ev71(0.4f);									// +0x11c
		f_a58->ev91(1);										// +0x16c
	}
	f_990 = 350.0f;											// MaxVertVelocity
	f_984 = 4340.0f;										// MaxHeight
	if (f_a64) f_a64->f_928 = g_5388c4;						// the beam as it was left
}

void C3DMiniSub::v266(void)									// 0x46c830
{
	GetTaskState("LIBBY");									// 0x497a40 (the result is not used)
}

void C3DMiniSub::v267(int level)							// 0x453530
{
	C3DAnimated::v267(level);								// 0x41c880
	if (!JimmyElement()) return;
	ShowAgain();											// element +0xd8
	v234();													// +0x3a8
	v132(1);												// +0x210
}

void C3DMiniSub::v275(void)									// 0x4534c0
{
	// (a call to the empty 0x463890)
	if (C3DAnimated *j = JimmyElement()) j->v275();			// +0x44c
}

void C3DMiniSub::WriteState(OMediaStreamOperators &s)		// 0x454b10
{
	C3DPlayerObject::WriteState(s);							// 0x4651b0
	OMedia3DVector p = Position(this);
	float x = p.x, y = p.y, z = p.z;
	s << x;
	s << y;
	s << z;
	OMedia3DVector a = Angles(this);
	x = a.x, y = a.y, z = a.z;
	s << x;
	s << y;
	s << z;
	bool beam = f_a64 ? f_a64->f_928 != 0 : false;
	s << beam;
	int held = f_a54 ? (int)f_a54->f_438 : -1;
	s << held;
	float r = v151();										// +0x25c radius
	s << r;
	s << f_660;
}

void C3DMiniSub::ReadState(OMediaStreamOperators &s)		// 0x454970
{
	C3DPlayerObject::ReadState(s);							// 0x4651a0
	float x, y, z;
	s >> x;
	s >> y;
	s >> z;
	v197(x, y, z);											// +0x314
	s >> x;
	s >> y;
	s >> z;
	v203(x, y, z);											// +0x32c
	v218(x, y, z);											// +0x368
	bool beam;
	s >> beam;
	if (f_a64)
	{
		f_a64->f_928 = beam ? 1 : 0;
		g_5388c4 = beam ? 1 : 0;
	}
	int held;
	s >> held;
	f_a54 = held == -1 ? NULL : FindObjectByIndex(held);	// 0x4af000
	float r;
	s >> r;
	v152(r);												// +0x260
	s >> f_660;
	f_9fc = 0;
	f_a3c = 0;
	if (f_5d4[0]) f_5d4[0]->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// port: NULL checked
}

namespace neg {
CGameObject *New_C3DMiniSub(void)	{ return new C3DMiniSub; }		// factory '3SUB' (0xa88 bytes)
}
