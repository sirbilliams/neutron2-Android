/*  C3DAIOmtObj - see C3DAIOmtObj.h. Reconstructed from Neutron2.exe 0x4171a0..0x418000. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DAIOmtObj.h"
#include "C3DAITrigger.h"
#include "../C3DGoddard.h"
#include "../C3DRocketShip.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DShape.h"
#include "OMedia3DVector.h"
#include "OMediaViewPort.h"
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

// 0x485730: the neutron energy (g_health) += d, kept in 0..100; "low neutron energy" once when
// it falls below 19 (g_545fd4 remembers it until it is above 20 again). (As in C3DNeutron.cpp.)
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

// The element of a CGameObject (object - 0xc0 in the original, not checked there).
static C3DAnimated *Element(CGameObject *o)
{
	return dynamic_cast<C3DAnimated *>(o);
}

// --------------------------------------------------------------------------- construction

C3DAIOmtObj::C3DAIOmtObj()									// 0x4171a0
{
	SetName((char *)"C3DAIOMTOBJ");							// 0x4ab1b0
	Log("C3DAIOMTOBJ()");
	C3DAI::InitObject();									// 0x4110e0 (called directly)
	RegisterProperty("OmtDatabase", f_1b50, PROP_ALPHA, 0);
	RegisterProperty("OmtIndex", &f_1bb4, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("TerrainColl", &f_1bb8, PROP_LONG, 0);
	RegisterProperty("SpecialCase", &f_1bbc, PROP_LONG, 0);
	v94(2);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DObject::SetObjectID('3AIO');							// 0x49ab00
	v67(5.0f);
	v152(5.0f);
	f_6d0 = 2;												// neither morph (0) nor Granny (1)
	v52(1);
	strcpy(f_1b50, "objects.omt");
	f_1bb4 = 4;
	f_648 = 0;
	f_1bb8 = -1;
	f_1bbc = 0;
	f_c48 = 0;
	f_1bc0 = 0;
	f_1bc4 = 0;
	f_1bc8 = 0;
	f_1bcc = 0;
	f_1bd0 = -1;
	// (then a call to the empty 0x463890)
}

C3DAIOmtObj::~C3DAIOmtObj()									// 0x4173d0
{
}

namespace neg {
CGameObject *New_C3DAIOmtObj(void) { return new C3DAIOmtObj; }			// '3AIO' (0x1bdc bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DAIOmtObj::AddTouching(CGameObject *other)			// 0x417960
{
	C3DAI::AddTouching(other);								// 0x415150
	if (f_1bbc == 1 && g_camera && g_camera == other)		// the hazard hurts Jimmy
	{
		PlayGameSound(-1, std::string("pain ahhouch"), false);
		JimmyHurt(g_camera, -10.0f, -10, 5.0f, 1);			// camera element +0x218
		LoadTask("autosave.tsk", false);					// 0x498af0
		float p[4];
		v195(p);
		g_viewport->place(p[0], p[1], p[2]);				// port: the original stores viewport +0x44/+0x48/+0x4c
		AddEnergy(-5.0f);									// 0x485730
		g_camera->v261();
	}
	if (f_1bbc == 2 && other->NameContains((char *)"C3DAITRIGGER") &&
		strcasecmp(other->m_tag, "BARRELROLL") == 0)
	{
		C3DAITrigger *t = dynamic_cast<C3DAITrigger *>(other);	// (other - 0xc8)
		if (t)												// port: the original does not check
		{
			if (t->m_ai_new_rot_y == -1.0f) f_1bcc = 0.75f;	// "AINewRotY": the roll time
			else f_1bcc = t->m_ai_new_rot_y;
		}
	}
	if (f_1bbc == 3 && g_camera == other)					// land on top of the train
	{
		float p[4], q[4];
		other->v195(p);
		v195(q);
		if (q[1] + 850.0f > p[1] && q[1] + 700.0f < p[1])	// 0x4ccf1c / 0x4ccf18
		{
			other->v195(p);
			v195(q);
			other->v197(p[0], q[1] + 850.0f, p[2]);
		}
	}
	if (f_1bbc == 5 && g_camera == other)					// trampoline
	{
		float p[4], q[4];
		other->v195(p);
		v195(q);
		if (q[1] + 540.0f > p[1] && q[1] + 400.0f < p[1])	// 0x4ccf14 / 0x4ccf10
		{
			PlayGameSound(-1, std::string("boing"), false);
			other->v162(0, 500.0f, 0);
			float v[4];
			v174(v);
			other->v176(v[0], 0, v[2]);
		}
	}
}

void C3DAIOmtObj::Process(float dt)							// 0x417420
{
	C3DAI::Process(dt);										// 0x411240
	if (!f_589) return;
	if (f_8a8) return;										// culled
	if (f_1bbc == 1)
	{
		get_shape()->offset_textures((float)(-dt * 2.19), (float)(-dt * 0.19), false);	// 2002 element +0x8c; 0x4ccf00 / 0x4ccf08 (double)
		AddRotation(0, dt * -60.0f, 0);
		if (g_camera)										// push the camera away
		{
			float p[4], c[4];
			v195(p);
			g_camera->v195(c);
			OMedia3DVector d(p[0] - c[0], p[1] - c[1], p[2] - c[2], 0);
			const float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
			const float inv = 1.0f / d.quick_magnitude();
			d.x *= inv;
			d.y *= inv;
			d.z *= inv;
			if (dist < 2000.0f)								// 0x4cb1ec
			{
				const float f = 1.0f - dist * 0.0005f;		// 0x4ccef4
				float force;
				switch (g_difficulty)
				{
					case 0:
						force = 170.0f;
						f_968 = f_1b3c * 0.7f;				// 0x4cceec
						v152(70.0f);
						break;
					case 1:
						force = 300.0f;
						v152(130.0f);
						break;
					case 2:
						force = 400.0f;
						f_968 = f_1b3c * 1.3f;				// 0x4ccef0
						v152(250.0f);
						break;
					default:
						force = 600.0f;
						break;
				}
				g_camera->v178(d.x * force * f, d.y * force * f, d.z * force * f);
			}
		}
	}
	if (f_1bbc != 2) return;
	float p[4], a[4];
	if (f_1bc0)												// the rocket rides along
	{
		v195(p);
		f_1bc0->v196(p[0], p[1], p[2], p[3]);
		v201(a);
		f_1bc0->v202(a[0], a[1], a[2], a[3]);
		C3DAnimated *e = Element(f_1bc0);
		if (e) e->ShowAgain();								// element +0xd8
	}
	if (f_1bc4)												// Jimmy
	{
		v195(p);
		f_1bc4->v196(p[0], p[1], p[2], p[3]);
		v201(a);
		f_1bc4->v202(a[0], a[1], a[2], a[3]);
	}
	if (f_1bc8)												// Goddard, sitting in it
	{
		v195(p);
		f_1bc8->v196(p[0], p[1], p[2], p[3]);
		v201(a);
		f_1bc8->v202(a[0], a[1], a[2], a[3]);
		C3DGoddard *g = dynamic_cast<C3DGoddard *>(f_1bc8);	// (f_1bc8 - 0xc0)
		if (g)												// port: the original does not check
		{
			if (!g->IsAnim((char *)"ROCK")) g->ev58((char *)"ROCK", 1);	// 0x41c6d0 / element +0xe8
			g->f_1b60 = 1;
			f_1bc8->v132(0);
			g->ev121();										// element +0x1e4
		}
		OMedia3DVector w = v224(0, 10.0f, -25.0f);
		f_1bc8->v196(w.x, w.y, w.z, w.w);
		f_1bc8->v132(0);
	}
	// (f_b8c: a "LAMS" "%s" log of its tag to the empty 0x463890)
	if (f_1bcc > 0)											// barrel roll
	{
		f_1bcc -= dt;
		if (f_1bcc <= 0) f_1bcc = 0;
		AddRotation(0, 0, dt * -360.0f);					// 0x4ccee8
	}
}

void C3DAIOmtObj::RotateToDest(float dt)					// 0x417f80
{
	C3DAI::RotateToDest(dt);								// 0x42d280
}

void C3DAIOmtObj::v260(void)								// 0x417ca0
{
	C3DAnimated::v260();									// 0x41cec0 (sic: not C3DAI's)
	if (f_1bbc == 1) v94(1);
	m_db = GetDatabase(f_1b50);								// 0x4a4180
	set_shape(Load3DShape(m_db, f_1bb4));					// element +0xac
	MakeShapeUnLit(get_shape());							// element +0xb0
	v67(f_034);												// the radius
	if (f_1bb8 == 0)										// no terrain collision: hover
	{
		f_bd4 = 1;
		v42(0);
		v40(0);
	}
	if (f_1bb8 == 1)
	{
		v42(1);
		v40(1);
	}
	if (f_1bbc == 1)
	{
		v64(0.05f);
		f_bd4 = 1;
		v42(0);
		v40(0);
		v94(1);
		v50(0);
		ev71(0.33f);										// element +0x11c
	}
	if (f_1bbc == 2)
	{
		f_008 = 1;
		v136(1);
		HideAgain();										// element +0xdc
		f_bd4 = 1;
		f_c18 = 1;
		if (f_1bc0) f_1bc0 = FindObjectByTag("C3DROCKETSHIP");
		if (f_1bc4)
		{
			f_1bc4 = FindObjectByTag("JIM1");
			C3DAnimated *e = Element(f_1bc4);
			if (e) e->ev58((char *)"DRIVE", 1);				// port: the original does not check
			if (f_1bc4) f_1bc4->v132(0);
		}
		if (f_1bc8) f_1bc8 = FindObjectByTag("C3DGODDARD");
		f_b9c.x = 2.7f;										// turn rates
		f_b9c.y = 2.3f;
		f_b9c.z = 2.7f;
		f_c64 = 1;
	}
	if (f_1bbc == 3 && f_1bd0 == -1)
		f_1bd0 = PlayGameSound3DNoDoppler(this, -1, std::string("traintracks"), true);	// 0x48f0f0
	f_968 = f_1b3c;											// the speed: "InitialSpeed"
}

unsigned char C3DAIOmtObj::v231(CGameObject *other)			// 0x417f40 (C3DAIGranny's code)
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3GOD' || id == '3JIM' || id == '3WBL';
}
