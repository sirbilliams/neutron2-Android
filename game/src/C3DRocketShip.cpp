/*  C3DRocketShip - see C3DRocketShip.h. */
#include "C3DRocketShip.h"
#include "C3DPermanentSmoke.h"
#include "C3DFireStrato.h"
#include "C3DPlayer.h"										// the input tests (PlayerKey*)
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "CGameType.h"
#include "C2DInGameMenu.h"
#include "CanvasWrapper.h"
#include "SaveGame.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaTrigo.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360

namespace neg {
CGameObject *New_C3DAIGranny(void);							// the factory's (gen/factory.cpp; weak until C3DAIGranny lands)
}

// --------------------------------------------------------------------------- hooks (weak)

__attribute__((weak)) unsigned char neg::JimmyHurtFlag(CGameObject *jimmy)
{
	(void)jimmy;
	NEG_TODO(0x44bd90, "C3DJimmy +0x4cb8 (hit flag)");
	return 0;
}

__attribute__((weak)) void neg::JimmySetHurtFlag(CGameObject *jimmy, unsigned char v)
{
	(void)jimmy; (void)v;
	NEG_TODO(0x44bd90, "C3DJimmy +0x4cb8 = v (hit flag)");
}

__attribute__((weak)) void neg::JimmyHurt(CGameObject *jimmy, float energy, int points, float time, unsigned char flag)
{
	(void)jimmy; (void)energy; (void)points; (void)time; (void)flag;
	NEG_TODO(0x44bd90, "C3DJimmy ESLOT 134 (hurt)");
}

__attribute__((weak)) void neg::JimmyPain(CGameObject *jimmy)
{
	(void)jimmy;
	NEG_TODO(0x44bfe0, "C3DJimmy ESLOT 130 (pain sounds)");
}

__attribute__((weak)) void neg::JimmyRocketAim(CGameObject *jimmy, float dt, OMedia3DVector pos)
{
	(void)jimmy; (void)dt; (void)pos;
	NEG_TODO(0x446100, "C3DJimmy ESLOT 146 (rocket aim)");
}

__attribute__((weak)) void neg::JimmyRocketShoot(CGameObject *jimmy, float x, float y, float z, float side, float speed,
												 CGameObject *target, int a7, float a8)
{
	(void)jimmy; (void)x; (void)y; (void)z; (void)side; (void)speed; (void)target; (void)a7; (void)a8;
	NEG_TODO(0x44b090, "C3DJimmy ESLOT 148 (rocket shot)");
}

__attribute__((weak)) int neg::JimmyAmmo(CGameObject *jimmy)
{
	(void)jimmy;
	NEG_TODO(0x46bf02, "C3DJimmy +0x4d94 (shots left)");
	return 0;
}

__attribute__((weak)) void neg::JimmySetAmmo(CGameObject *jimmy, int n)
{
	(void)jimmy; (void)n;
	NEG_TODO(0x46c10e, "C3DJimmy +0x4d94 = n (shots left)");
}

__attribute__((weak)) void neg::JimmyGadgetShot(CGameObject *jimmy)
{
	(void)jimmy;
	NEG_TODO(0x46bf95, "C3DJimmy current gadget +0x63c/+0x640/+0x644");
}

__attribute__((weak)) int neg::AIOmtObjState(CGameObject *o)
{
	(void)o;
	NEG_TODO(0x46ac0d, "C3DAIOmtObj +0x1bbc");
	return 0;
}

__attribute__((weak)) float neg::AIAttackTime(CGameObject *o)
{
	(void)o;
	NEG_TODO(0x46adb7, "C3DDino / C3DPtero +0x1d58");
	return 0;
}

__attribute__((weak)) void neg::MineDetonate(CGameObject *o)
{
	(void)o;
	NEG_TODO(0x4521e0, "C3DMine +0x6ac = 2");
}

__attribute__((weak)) CGameObject *neg::GoddardMarker(CGameObject *goddard)
{
	(void)goddard;
	NEG_TODO(0x46b7f5, "C3DGoddard +0x1b80 (marker)");
	return NULL;
}

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// The key lists end with 0; 0x1f5 is the left mouse button, 0x1f4 the right one, anything else a
// key of g_keys. A key that counts also clears the mouse movement (0x4a39e0). (As C3DPlayer's.)
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

static bool PlayerKeyBrake(void) { return KeyListDown(g_key_brake); }	// 0x4656f0 (no joystick button)

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

// The game's random number generator: 0..1000.
static int Random1001(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return ((int)((*seed >> 16) * 1001) >> 16) & 0xffff;
}

static void Play(const char *name, bool loop = false)
{
	PlayGameSound(-1, std::string(name), loop);				// 0x48f420
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// vtable +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

static OMedia3DVector Rotation(CGameObject *o)
{
	float r[4];
	o->v201(r);												// vtable +0x324
	return OMedia3DVector(r[0], r[1], r[2], r[3]);
}

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0.
static OMedia3DVector ObjectDir(CGameObject *o, float x, float y, float z)
{
	float rot[4];
	o->v201(rot);
	const omt_Angle ax = (omt_Angle)((int)(rot[0] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle ay = (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle az = (omt_Angle)((int)(rot[2] * kDegToAngle) & omc_MaxAngleMask);
	OMedia3DVector v(x, y, z, 0.0f);
	v.rotate(ax, ay, az);
	return v;
}

// The element of a game object of the C3DObject family (the original goes back by 0xc0).
static C3DObject *Element(CGameObject *o)
{
	return o ? dynamic_cast<C3DObject *>(o) : NULL;
}

static C3DAnimated *Animated(CGameObject *o)
{
	return o ? dynamic_cast<C3DAnimated *>(o) : NULL;
}

// The 4-float position / rotation setters (slots 196 / 202) with a vector.
static void SetPos4(CGameObject *o, const OMedia3DVector &v) { o->v196(v.x, v.y, v.z, v.w); }	// vtable +0x310
static void SetRot4(CGameObject *o, const OMedia3DVector &v) { o->v202(v.x, v.y, v.z, v.w); }	// +0x328

// --------------------------------------------------------------------------- construction

C3DRocketShip::C3DRocketShip()								// 0x46a290
{
	f_125c = 0;
	SetName((char *)"C3DROCKETSHIP");
	Log("C3DRocketShip()");
	C3DRocketShip::InitObject();							// 0x46aaf0 (called directly)
	v64(0.6f);
	v38(1);
	C3DObject::SetObjectID('3ROC');							// 0x49ab00
	f_9f0 = 0;
	f_9f4 = NULL;
	v44(0);
	v50(0);
	for (int i = 0; i < 20; i++)							// the smoke trail
	{
		C3DPermanentSmoke *s = new C3DPermanentSmoke;		// 0x45d220 (0x624 bytes)
		f_9f8[i] = s;
		// (debug log of the allocation, "C3DPermanentSmoke": compiled out)
		s->v36(0);											// its CGameObject, vtable +0x90
		if (g_windowed) s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);	// +0xb4 / +0xb8 = 6 / 7
		else s->set_blend(omblendfc_Dst_Color, omblendfc_Zero);					// 2 / 0
		s->HideAgain();										// element +0xd8
		s->f_618 = 0;
	}
	v36(0);

	// the parachute: a Granny AI object playing the chute animations
	f_1270 = New_C3DAIGranny();								// new C3DAIGranny (0x415e00, 0x2144 bytes)
	if (f_1270)
	{
		// port: the original writes these C3DAIGranny members directly (+0x1a90 / 0x1da8 / 0x1e0c /
		// 0x1e70 of the element); here through the properties its InitObject registers for them,
		// as C3DAIGranny is in its own file.
		static const char *const kAnims[4][2] =
		{
			{ "BASEAnimation", "chutebase.grn" }, { "ANIM1Animation", "chutedrop.grn" },
			{ "ANIM2Animation", "chutemove.grn" }, { "ANIM3Animation", "chutestop.grn" },
		};
		for (int i = 0; i < 4; i++)
		{
			CProperty *p = f_1270->FindProperty(kAnims[i][0]);
			if (p && p->data) strcpy((char *)p->data, kAnims[i][1]);
		}
		f_1270->v36(0);										// +0x90
		f_1270->v107(0);									// +0x1ac
		f_1270->v132(0);									// +0x210
		f_1270->v136(0);									// +0x220
		if (C3DObject *e = Element(f_1270)) e->HideAgain();	// element +0xdc
		if (C3DAnimated *a = Animated(f_1270)) a->f_65c = 0;	// "InitiallyVisible"
	}

	f_11f0 = 0;
	strcpy(m_task_name, "LIBBY");							// +0x4fc
	f_11f4 = NULL;
	f_11f8 = 0;
	f_1234 = 0;
	f_11fc = 0;
	f_1230 = new C3DFireStrato;								// 0x431b60 (0x92c bytes)
	// (debug log of the allocation, "C3DFireStrato": compiled out)
	v34(1);
	f_1228 = 0;
	f_122c = 0;
	f_1235 = 0;
	f_009 = 1;
	f_123c = -1;
	f_1240 = -1;
	f_1248 = 0;
	g_545f54 = 0;
	v136(0);
	f_9a0.x = 0;											// camera offset (0, 400, -300)
	f_9a0.y = 400.0f;
	f_9a0.z = -300.0f;
	f_124c = NULL;
	f_1250 = 0;
	f_1254 = 1300.0f;
	f_1258 = 550.0f;
	f_1235 = 1;
	f_1234 = 0;
	f_1274 = 0;
	f_9ec = 0;
	f_1260 = NULL;
	f_1264 = NULL;
	f_1268 = NULL;
	f_126c = NULL;
	f_1278 = 0;
	f_1275 = 0;
	f_4c4 = 1;												// +0x584
	f_1244 = 0;												// port: not set by the original
}

C3DRocketShip::~C3DRocketShip()								// 0x46a7a0
{
	Log("~C3DRocketShip()");
}

namespace neg {
CGameObject *New_C3DRocketShip(void) { return new C3DRocketShip; }		// '3ROC' (0x1284 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DRocketShip::InitObject(void)						// 0x46aaf0
{
	Log("InitObject()");
	C3DFlyingObject::InitObject();							// 0x4349d0
	InitAnim();												// element +0x110
	CreateAnim((char *)"LOSTWING", (char *)"Stratolosewing.ase");	// +0xe0
	CreateAnim((char *)"LEFT", (char *)"Stratoleft.ase");
	CreateAnim((char *)"RIGHT", (char *)"Stratoright.ase");
	CreateAnim((char *)"DEFAULT", (char *)"Strato.ase");
	CreateTexture((char *)"strato.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(50.0f);
	ev58((char *)"DEFAULT", 1);
}

void C3DRocketShip::ResetObject(void)						// 0x46c840
{
	C3DFlyingObject::ResetObject();							// 0x434b10
	if (!f_9f4) f_96c = 0;
}

void C3DRocketShip::InitPhysics(void)						// 0x43ba20
{
	if (!f_4aa) C3DObject::InitPhysics();
}

void C3DRocketShip::v260(void)								// 0x46a840
{
	C3DPlayerObject::v260();								// 0x464ed0
	InitPhysics();											// vtable +0x2c
	f_124c = NULL;
	if (g_gametype->m_object_id == 'LV5C') f_124c = FindObjectByTag("C3DCARL");	// +0x490
	f_11f8 = 0;
	f_1248 = 0;
	f_9d8 = 0;
	f_9d4 = 0;
	if (f_9f4)
	{
		// we start where the rider started
		float p[4];
		f_9f4->v88(p);										// its initial position, vtable +0x160
		unsigned int w;
		memcpy(&w, &p[3], 4);
		v89(p[0], p[1], p[2], w);							// +0x164
		float r[4];
		f_9f4->v91(r);										// its initial rotation, +0x16c
		const float rz = r[2];
		f_9f4->v91(r);
		const float ry = r[1];
		f_9f4->v91(r);
		v92(r[0], ry, rz);									// +0x170
		f_9f4->ev58((char *)"DRIVE", 1);
		f_96c = 500.0f;
		if (f_11f4) f_11f4->ev58((char *)"ROCK", 1);
	}
	f_11fc = 0;
	f_11f8 = 0;
	f_1248 = 0;
	if (f_5d4[0]) f_5d4[0]->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// +0x4c.. (r, g, b, a); port: NULL checked
	switch (g_gametype->m_object_id)						// the ceiling by level
	{
		case 'LEV1':										f_984 = 1500.0f; break;
		case 'LEV2': case 'LV2B':							f_984 = 2300.0f; break;
		case 'LEV3': case 'LV3C': case 'LV3D': case 'LV3E':	f_984 = 1700.0f; break;
		case 'LEV4':										f_984 = 2700.0f; break;
		case 'LV3A': case 'LV3B':							f_984 = 4000.0f; break;
		case 'LV4C':										f_984 = 3700.0f; break;
		case 'LEV5': case 'LV5A': case 'LV5B': case 'LV5C': case 'LV5F': case 'VR02':
															f_984 = 3900.0f; break;
	}
	if (C3DObject *e = Element(f_1270)) e->HideAgain();		// the parachute, element +0xdc (port: the original does not check)
}

void C3DRocketShip::v266(void)								// 0x46c830
{
	GetTaskState("LIBBY");									// 0x497a40 (the result is not used)
}

void C3DRocketShip::v267(int level)							// 0x46ca70
{
	C3DAnimated::v267(level);								// 0x41c880
	if (!f_9f4) return;
	f_1235 = 1;
	f_1234 = 0;
	ShowAgain();											// element +0xd8
	v234();													// +0x3a8
	v132(1);												// +0x210
}

void C3DRocketShip::v275(void)								// 0x46ca40
{
	// (debug log, compiled out)
	if (f_9f4) f_9f4->v275();								// +0x44c
}

void C3DRocketShip::v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly)	// 0x4534f0
{
	(void)nx; (void)ny; (void)nz; (void)nw; (void)poly;
	float v[4];
	v174(v);
	const float vz = v[2];
	v174(v);
	v176(v[0], -120.0f, vz);								// the ceiling stops the climb
}

void C3DRocketShip::AddTouching(CGameObject *other)			// 0x46abc0
{
	C3DFlyingObject::AddTouching(other);					// 0x435eb0
	CGameObject *jim = f_9f4;								// (CGameObject part of the rider)

	if (other->NameContains((char *)"C3DAIOMTOBJ") && f_9f4 && !f_11f8 && AIOmtObjState(other) == 3)	// the train
	{
		JimmySetHurtFlag(jim, 0);
		JimmyHurt(jim, -15.0f, -20, 5.0f, 1);				// element +0x218
		Play("rocketcrash");
		Play("trainloop");
		f_11f8 = 1;
		f_1248 = 1;
		f_1238 = 5.0f;
		f_11fc = 0;
	}
	if (other->NameContains((char *)"C3DCARL") && f_9f4)
	{
		f_9dd = 1;											// turned away
		if (f_9d8 <= 0) f_9d8 = 1.0f;
		switch (g_difficulty)
		{
			case 1:
			JimmySetHurtFlag(jim, 0);
			JimmyHurt(jim, -2.0f, -2, 3.0f, 1);
			f_11f8 = 1;
			f_1248 = 1;
			f_1238 = 3.0f;
			f_11fc = 0;
			break;
			case 2:
			JimmySetHurtFlag(jim, 0);
			JimmyHurt(jim, -5.0f, -5, 5.0f, 1);
			f_11f8 = 1;
			f_1248 = 1;
			f_1238 = 5.0f;
			f_11fc = 0;
			break;
		}
	}
	if (other->NameContains((char *)"C3DDINO") && f_9f4 && !f_11f8 && AIAttackTime(other) > 0)
	{
		JimmySetHurtFlag(jim, 0);
		Play("rocketcrash");
		switch (g_difficulty)
		{
			case 0: JimmyHurt(jim, -1.0f, -1, 1.0f, 1); break;
			case 1: JimmyHurt(jim, -5.0f, -5, 5.0f, 1); break;
			case 2: JimmyHurt(jim, -10.0f, -10, 5.0f, 1); break;
		}
		f_9dd = 1;
		if (f_9d8 <= 0) f_9d8 = 1.0f;
		Play("explosion1");
		f_11f8 = 1;
		f_1248 = 1;
		f_1238 = 5.0f;
		f_11fc = 0;
	}
	if (other->NameContains((char *)"C3DPTERO") && f_9f4 && !f_11f8 && AIAttackTime(other) > 0)
	{
		JimmySetHurtFlag(jim, 0);
		JimmyHurt(jim, -10.0f, -10, 5.0f, 1);
		Play("explosion1");
		f_11f8 = 1;
		f_1248 = 1;
		f_1238 = 5.0f;
		f_11fc = 0;
	}
	if (other->NameContains((char *)"C3DMINE") && f_9f4 && !f_11f8)
	{
		JimmySetHurtFlag(jim, 0);
		JimmyHurt(jim, -10.0f, -10, 5.0f, 1);
		MineDetonate(other);								// 0x4521e0 (on other - 0xc8)
		Play("explosion1");
		f_11f8 = 1;
		f_1248 = 1;
		f_1238 = 2.5f;
		f_11fc = 0;
	}
	if (other->NameContains((char *)"C3DROCK") && f_9f4 && !f_11f8)
	{
		JimmySetHurtFlag(jim, 0);
		JimmyHurt(jim, -10.0f, -10, 5.0f, 1);
		// quirk: the original calls the mine's 0x4521e0 on (other - 0xc8) here too. A C3DRock is a
		// C3DAnimated (CGameObject at +0xc0), so it writes 2 into its element +0x6a4: four bytes
		// in the middle of the PickupLink name (f_669).
		if (C3DAnimated *a = Animated(other))
		{
			const int two = 2;
			memcpy(a->f_669 + (0x6a4 - 0x669), &two, 4);
		}
		Play("explosion1");
		f_11f8 = 1;											// (no smoke, no new hit length)
		f_11fc = 0;
	}
	if (other->NameContains((char *)"C3DMISSILE") && f_9f4 && !f_11f8)
	{
		JimmySetHurtFlag(jim, 0);
		JimmyHurt(jim, -10.0f, -10, 5.0f, 1);
		Play("explosion2");
		if (C3DObject *e = Element(other)) e->HideAgain();	// element +0xdc
		f_11f8 = 1;
		f_1248 = 1;
		f_1238 = 2.0f;
		f_11fc = 0;
	}
}

void C3DRocketShip::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x46c1b0
{
	(void)a; (void)b; (void)c; (void)d;
	if (!f_9f4 || !poly || !poly->get_material()) return;
	CGameObject *jim = f_9f4;
	const int id = (int)poly->get_material()->get_chunk_ID();	// material +0xc -> +8 (0 without one)
	if (id >= 130 && id <= 139)
	{
		if (f_11f8) return;
		JimmySetHurtFlag(jim, 0);
		JimmyHurt(jim, -10.0f, -10, 5.0f, 1);
		Play("rocket hit");
		f_11f8 = 1;
		f_1248 = 0;
		f_11fc = 0;
		f_1238 = 5.0f;
		return;
	}
	if (id >= 185 && id <= 189)
	{
		if (JimmyHurtFlag(jim)) return;
		JimmyHurt(jim, -5.0f, -5, 5.0f, 1);
		Play("rocket hit");
		f_1238 = 5.0f;
		f_11f8 = 1;
		f_1248 = 0;
		f_11fc = 0;
		JimmyPain(jim);										// element +0x208
		return;
	}
	if (id == 253)											// turn back
	{
		ev109();											// element +0x1b4
		return;
	}
	if (id == 170 || id == -1) return;

	// anything else: a crash (not on the easy level)
	if (f_11f8) return;
	if (g_difficulty == 0) return;
	JimmySetHurtFlag(jim, 0);
	Play("rocket hit");
	f_1238 = 4.0f;
	f_11f8 = 1;
	f_1248 = 1;
	if ((double)Random1001() * 0.00100000005f < 0.43)		// 0x4f58d0: a wing comes off
	{
		ev58((char *)"LOSTWING", 0);
		if (f_11f4) f_11f4->ev58((char *)"LOSTWING", 0);	// port: the original does not check f_11f4 (here or below)
	}
	else if (f_11f4) f_11f4->ev58((char *)"FIX", 0);
	f_11fc = 0;
	Play("rocketcrash");
	const float r = (float)((double)Random1001() * 0.00100000005f);
	switch (g_difficulty)
	{
		case 0:												// (never: returned above)
		if (r < 0.2)
		{
			JimmyHurt(jim, -3.0f, -3, 3.0f, 1);
			f_1238 = 4.0f;
		}
		else
		{
			JimmyHurt(jim, -1.0f, -1, 1.0f, 1);
			f_1238 = 2.0f;
		}
		break;
		case 1:
		if (r < 0.3)
		{
			JimmyHurt(jim, -5.0f, -5, 5.0f, 1);
			f_1238 = 5.0f;
		}
		else
		{
			JimmyHurt(jim, -1.0f, -1, 1.0f, 1);
			f_1238 = 2.0f;
		}
		break;
		case 2:
		JimmyHurt(jim, -5.0f, -5, 5.0f, 1);
		f_1238 = 6.0f;
		break;
	}
	// the wing debris (f_1260..f_126c are never set, so none of this happens)
	if (r < 0.25f)
	{
		if (f_1260) { SetPos4(f_1260, v224(-150.0f, 0, 0)); f_1260->v132(1); }
		if (f_1268) { SetPos4(f_1268, v224(150.0f, 0, 200.0f)); f_1268->v132(1); }
	}
	else if (r < 0.5f)
	{
		if (f_1264) { SetPos4(f_1264, v224(150.0f, 0, 100.0f)); f_1264->v132(1); }
		if (f_1268) { SetPos4(f_1268, v224(-100.0f, 0, 0)); f_1268->v132(1); }
	}
	else if (r < 0.75f)
	{
		if (f_1264) { SetPos4(f_1264, v224(150.0f, 0, 0)); f_1264->v132(1); }
		if (f_1268) { SetPos4(f_1268, v224(-100.0f, 0, 250.0f)); f_1268->v132(1); }
	}
	else
	{
		if (f_126c) { SetPos4(f_126c, v224(150.0f, 0, 50.0f)); f_126c->v132(1); }
		if (f_1260) { SetPos4(f_1260, v224(-100.0f, 0, -20.0f)); f_1260->v132(1); }
	}
}

// --------------------------------------------------------------------------- every frame

void C3DRocketShip::Process(float dt)						// 0x46b140
{
	C3DFlyingObject::Process(dt);							// 0x434c30
	if (f_9f4)
	{
		// the flame and the riders go with us
		SetPos4(f_1230, Position(this));
		SetRot4(f_1230, Rotation(this));
		const omt_WAngle ax = get_anglex(), ay = get_angley(), az = get_anglez();	// 2002 +0x50..+0x54
		CGameObject *jim = f_9f4;
		float p[4];
		v195(p);
		const float pz = p[2];
		v195(p);
		const float py = p[1];
		v195(p);
		jim->v197(p[0], py, pz);							// +0x314
		v195(p);
		const float qz = p[2];
		v195(p);
		const float qy = p[1];
		v195(p);
		f_9f4->place(p[0], qy, qz);							// its element +0x44
		float r[4];
		v201(r);
		const float rz = r[2];
		v201(r);
		const float ry = r[1];
		v201(r);
		jim->v203(r[0], ry, rz);							// +0x32c
		f_9f4->set_angle(ax, ay, az);						// +0x50
		if (f_11f4)
		{
			CGameObject *god = f_11f4;
			const OMedia3DVector g = v224(0, 10.0f, -25.0f);
			SetPos4(god, g);
			f_11f4->place(g.x, g.y, g.z);
			v201(r);
			const float gz = r[2];
			v201(r);
			const float gy = r[1];
			v201(r);
			god->v203(r[0], gy, gz);
		}
	}
	if (g_gametype->IsPaused()) return;						// +0x114

	// the smoke trail: one puff per frame, round the ring of 20
	C3DPermanentSmoke *s = f_9f8[f_11f0];
	if (s)
	{
		if (f_96c > 20.0f)
		{
			if (f_1248)
			{
				SetPos4(s, v224(0, -10.0f, -390.0f));
				s->ShowAgain();								// element +0xd4
				const float k = 1.0f - f_11fc / f_1238;
				s->set_diffuse(OMediaFARGBColor(k, 1.0f, k, k));	// +0x90.. (r, g, b, a) = 1, k, k, k
			}
			else s->HideAgain();							// +0xd8
		}
		else s->HideAgain();
	}
	f_11f0++;
	if (f_11f0 >= 20) f_11f0 = 0;

	if (!f_9f4)
	{
		f_9cc = 0;
		return;
	}
	if (f_9cc > 10.0f && !f_1235) f_1234 = 1;				// too long in the air: the engine dies
	C3DFireStrato *fire = static_cast<C3DFireStrato *>(f_1230);
	if (!f_99d)
	{
		if (fire)
		{
			if (f_1234)
			{
				f_968 = 10.0f;
				if ((double)Random1001() * 0.00100000005f > 0.3) fire->HideAgain();	// flickers
				else fire->ShowAgain();
			}
			else if (f_96c > 0) fire->ShowAgain();			// element +0xd8
			else fire->HideAgain();							// +0xdc
			SetPos4(f_1230, Position(this));
			SetRot4(f_1230, Rotation(this));
		}
	}
	else if (fire) fire->HideAgain();						// no flame while diving
	if (f_11f8)												// hit: flash red, pushed down
	{
		if (f_1248) v159(0, (1.0f - f_11fc / f_1238) * -900.0f, 0);	// +0x27c, 0x4f58cc
		f_11fc += dt;
		float k;
		if (f_11fc > f_1238)
		{
			k = 1.0f;
			f_11f8 = 0;
			f_1248 = 0;
		}
		else k = (float)((sin((double)f_11fc * 20.0f) + 1.0) * 0.5);
		if (f_5d4[0]) f_5d4[0]->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, k, k));	// +0x4c.. (r, g, b, a) = 1, k, k, 1; port: NULL checked
	}
	if (f_1234) v159(0, -750.0f, 0);
}

void C3DRocketShip::v243(float dt)							// 0x46b960
{
	if (!GameIsRunning()) return;							// 0x4b0ca0
	C3DFlyingObject::v243(dt);								// 0x435230
	C3DAnimated *fire = Animated(f_1230);
	C3DAnimated *chute = Animated(f_1270);

	// afterburner: burns energy
	if (PlayerKeySpeed())
	{
		AddEnergy(dt * -1.0f);								// 0x485730
		if (!f_1275)
		{
			f_1275 = 1;
			Play("afterburner");
			const double r = (double)Random1001() * 0.00100000005f;
			if (r < 0.3) Play("woohoo");
			else if (r < 0.6) Play("yeah");
			fire->ev58((char *)"AFTERBURN", 1);
			if (f_11f4) f_11f4->ev58((char *)"AFTERBURN", 1);
			if (f_9f4) f_9f4->ev58((char *)"AFTERBURN", 1);
		}
	}
	else
	{
		if (f_1275)
		{
			fire->ev58((char *)"DEFAULT", 1);
			if (f_9f4) f_9f4->ev58((char *)"DRIVE", 1);
			if (f_11f4) f_11f4->ev58((char *)"ROCK", 1);
		}
		f_1275 = 0;
	}

	// the parachute (port: the chute calls are NULL checked; the original's object always exists)
	if (PlayerKeyBrake())									// 0x4656f0
	{
		if (!f_1274)
		{
			if (!f_9ec)
			{
				if (chute)
				{
					f_1270->v132(0);						// +0x210
					f_1270->v136(0);						// +0x220
					SetPos4(f_1270, Position(this));
					SetRot4(f_1270, Rotation(this));
				}
				Play("chuteopen");
				if (chute)
				{
					chute->ShowAgain();						// element +0xd8
					chute->ev58((char *)"ANIM2", 0);
				}
				f_1274 = 1;
				f_9ec = 1;									// slow speeds
			}
			else
			{
				f_9ec = 0;
				f_1274 = 1;
				if (chute) chute->ev58((char *)"ANIM1", 0);
			}
		}
	}
	else f_1274 = 0;
	if (f_9ec)
	{
		if (chute)
		{
			SetPos4(f_1270, Position(this));
			SetRot4(f_1270, Rotation(this));
		}
		if (f_9d8 > 0 || PlayerKeySpeed())					// turning round or the afterburner drop it
		{
			f_1274 = 1;
			f_9ec = 0;
			if (chute) chute->ev58((char *)"ANIM1", 0);
		}
	}

	if (f_9f4)
	{
		CGameObject *jim = f_9f4;
		if (g_menu)
		{
			JimmyRocketAim(jim, dt, Position(this));		// element +0x248
			if (JimmyAmmo(jim) > 0) g_menu->ShowGadgetBar();	// +0x4c4
			SetGadgetBarSize((float)JimmyAmmo(jim));		// 0x404d60
		}
		if (g_power_timers[3] != 0) ev111(dt);				// 0x54a430: Goddard points the way to Carl
		else if (f_11f4)
		{
			if (CGameObject *m = GoddardMarker(f_11f4))
				if (C3DObject *e = Element(m)) e->HideAgain();	// element +0xdc
		}
	}

	// shooting
	f_122c += dt;
	if (!PlayerKeyAction() || !f_9f4) return;				// 0x465750
	CGameObject *jim = f_9f4;
	const float rate = (g_power_timers[5] > 0) ? 0.2f : 0.5f;	// 0x54a438: rapid fire
	if (!(rate < f_122c)) return;
	if (JimmyAmmo(jim) <= 0 && g_gametype->m_object_id != 'LV5F')
	{
		if (!IsChannelPlaying(f_1240)) f_1240 = PlayGameSound(-1, std::string("needfood"), false);	// 0x4b89d0
		return;
	}
	f_122c = 0;
	Play("blip4");
	JimmyGadgetShot(jim);
	if (g_power_timers[8] > 0)								// 0x54a444: five shots
	{
		JimmyRocketShoot(jim, 0, 80.0f, 250.0f, 2.0f, 3500.0f, f_124c, 0, 1.0f);	// element +0x250
		JimmyRocketShoot(jim, 0, 80.0f, 250.0f, 0, 3500.0f, f_124c, 0, 1.0f);
		JimmyRocketShoot(jim, 0, 80.0f, 250.0f, -2.0f, 3500.0f, f_124c, 0, 1.0f);
		JimmyRocketShoot(jim, 0, 80.0f, 250.0f, -4.0f, 3500.0f, f_124c, 0, 1.0f);
		JimmyRocketShoot(jim, 0, 80.0f, 250.0f, 4.0f, 3500.0f, f_124c, 0, 1.0f);
	}
	else JimmyRocketShoot(jim, 0, 80.0f, 250.0f, 0, 3500.0f, f_124c, 0, 0.1f);
	JimmySetAmmo(jim, JimmyAmmo(jim) - 1);
	if (g_menu) SetGadgetBarSize((float)JimmyAmmo(jim));
}

// --------------------------------------------------------------------------- element-family slots

void C3DRocketShip::ev108(int state)						// 0x46c860
{
	switch (state)
	{
		case 0:												// engine off
		if (f_123c != -1)
		{
			StopGameSound(f_123c, false);					// 0x48f0d0
			f_123c = -1;
		}
		break;

		case 1:
		case 2:
		{
			const char *sound = (state == 2) ? "rocket blast" : "rocket fly";
			if (f_1244 == state)
			{
				if (!f_1248) break;							// running: nothing to do
				if (f_123c == -1)							// damaged: restart after a cut-out
				{
					f_123c = PlayGameSound(-1, std::string(sound), true);
					f_1244 = state;
					return;
				}
				if ((double)Random1001() * 0.00100000005f < 0.25f)	// 0x4d340c: the engine cuts out
				{
					StopGameSound(f_123c, false);
					f_123c = -1;
				}
				break;
			}
			if (f_123c != -1) StopGameSound(f_123c, false);
			f_123c = PlayGameSound(-1, std::string(sound), true);
			f_1244 = state;
			return;
		}
	}
	f_1244 = state;
}

void C3DRocketShip::ev110(void)								// 0x46c140
{
	v224(0, 20.0f, 1000.0f);								// (unused)
	ObjectDir(this, 0, 0, 1.0f);							// (unused) vtable +0x388
	if (++f_1228 >= 10) f_1228 = 0;
}

void C3DRocketShip::ev111(float dt)							// 0x46b7e0
{
	(void)dt;
	if (!f_11f4) return;
	CGameObject *mk = GoddardMarker(f_11f4);				// +0x1b80
	C3DObject *m = Element(mk);
	if (f_124c)
	{
		const OMedia3DVector p = f_124c->v224(f_1250, f_1254, f_1258);
		const float cx = g_viewport->getx(), cy = g_viewport->gety(), cz = g_viewport->getz();	// 2002 +0x44
		OMedia3DVector d(p.x - cx, p.y - cy, p.z - cz, 0.0f);
		const double inv = 1.0 / d.quick_magnitude();
		d.x = (float)(inv * d.x);
		d.y = (float)(inv * d.y);
		const double dz = inv * d.z;
		d.x *= 300.0f;
		d.y *= 300.0f;
		d.z = (float)(dz * 300.0f);
		const float qx = d.x + cx;
		const float qy = d.y + cy;
		const float qz = d.z + cz;
		if (m)
		{
			mk->v196(qx, qy, qz, 1.0f);						// +0x310
			m->ShowAgain();									// element +0xd8
		}
	}
	else if (m) m->HideAgain();								// +0xdc
}

// --------------------------------------------------------------------------- savegames

void C3DRocketShip::WriteState(OMediaStreamOperators &s)	// 0x46cc40
{
	C3DPlayerObject::WriteState(s);							// 0x4651b0
	float v[4], a, b, c;
	v195(v); a = v[0];
	v195(v); b = v[1];
	v195(v); c = v[2];
	s << a << b << c;
	v201(v); a = v[0];
	v201(v); b = v[1];
	v201(v); c = v[2];
	s << a << b << c;
}

void C3DRocketShip::ReadState(OMediaStreamOperators &s)		// 0x46cac0
{
	C3DPlayerObject::ReadState(s);							// 0x4651a0
	float a, b, c;
	s >> a >> b >> c;
	v197(a, b, c);											// position
	s >> a >> b >> c;
	v203(a, b, c);											// rotation
	v218(a, b, c);
	float r[4];
	v201(r);
	f_274 = r[0];
	f_278 = r[1];
	f_27c = r[2];
	f_280 = r[3];
	if (g_camera != (CGameObject *)this)
	{
		f_9f4 = NULL;
		f_11f4 = NULL;
		return;
	}
	f_9f4 = Animated(FindObjectByTag("JIM1"));				// (the original goes back by 0xc0)
	f_11f4 = Animated(FindObjectByTag("C3DGODDARD"));
	if (f_9f4) f_9f4->ev58((char *)"DRIVE", 1);
	if (f_11f4) f_11f4->ev58((char *)"ROCK", 1);
}
