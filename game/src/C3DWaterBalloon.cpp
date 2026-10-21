/*  C3DWaterBalloon - see C3DWaterBalloon.h.
    Constants: 0x4d92e0 = 250 (spin, degrees / s), 0x4d7f8c = 0.2 (fade per second while
    growing), 0x4d9870 = 16500 (homing pull), 0x4f1f68 / 0x50495c / 0x4ccf18 = 1700 / 1200 / 700
    (homing range at skill 0 / 1 / 2, 0x53b5b0).                                               */
#include "C3DWaterBalloon.h"
#include "CGameType.h"
#include "Game.h"
#include "Sound.h"
#include "gen/globals.h"
#include <math.h>
#include <string.h>
#include <string>

using namespace neg;

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'L1_A').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// --------------------------------------------------------------------------- construction

C3DWaterBalloon::C3DWaterBalloon()							// 0x47c780
{
	SetName((char *)"C3DWATERBALLOON");
	Log("C3DWaterBalloon()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3WBL');							// 0x49b550
	strcpy(m_sprite_db, "retainedsprites.omt");
	m_sprite_index = 20;
	m_sprite_size = 35;
	f_620 = 1.0f;
	f_624 = 0;
	f_628 = 0;
	v46(1);
	C3DSprite::InitSpriteProperties(25.0f);					// 0x49ce10
	v60(3.2f);
	v67(15.0f);
	v107(0);
	v36(0);
	v52(0);
	v62(0.46f);
	f_62c = 0;
	f_63c = 0;
	f_640 = 0;
	f_644 = 0;
	f_660.x = 0;
	f_660.y = 0;
	f_660.z = 0;
	f_64c = new C3DPermanentSprite;							// 0x608 bytes
	if (f_64c)
	{
		strcpy(f_64c->m_sprite_db, "retainedsprites.omt");
		f_64c->m_sprite_index = 11;
		f_64c->m_sprite_size = 100;
		f_64c->InitSpriteProperties(100.0f);				// element +0xd0
		f_64c->v136(0);										// +0x220
		f_64c->v46(0);										// +0xb8
		f_64c->v107(0);										// +0x1ac
		f_64c->v36(0);										// +0x90
	}
	f_650 = 0;
	f_630 = 0;
	v111(0);
	f_634 = NULL;
	f_638 = 0;
	f_658 = 0;
	f_654 = 0;
	f_65c = 0;
	// (then a call to the empty 0x463890)
}

C3DWaterBalloon::~C3DWaterBalloon()							// 0x47cb10
{
}

namespace neg {
CGameObject *New_C3DWaterBalloon(void)	{ return new C3DWaterBalloon; }	// factory '3WBL' (0x678 bytes)
}

// --------------------------------------------------------------------------- element slot

void C3DWaterBalloon::Pop(void)								// 0x47d240
{
	if (!f_64c) return;
	const int l = LevelID();
	if (l != 'VR01' && l != 'LV5C' && l != 'LV4B')
	{
		PlayGameSound(-1, std::string("SPLASH"), false);	// 0x48f420
		PlayGameSound(-1, std::string("balloon pop"), false);
	}
	else PlayGameSound(-1, std::string("vrzap"), false);
	v132(0);												// +0x210
	v136(0);												// +0x220
	HideAgain();											// element +0xd8
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DWaterBalloon::AddTouching(CGameObject *other)		// 0x47d130
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (other->NameContains((char *)"C3DBOT"))				// +0x18
	{
		Pop();												// element +0xdc
		v132(0);
	}
	if (other->NameContains((char *)"C3DTESLA"))
	{
		Pop();
		v132(0);
	}
}

void C3DWaterBalloon::v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly)	// 0x47cb50
{
	(void)nx; (void)ny; (void)nz; (void)nw; (void)poly;
	float p[4];
	v195(p);												// +0x30c
	if (f_64c)
	{
		f_64c->ShowAgain();									// element +0xd4
		f_64c->v196(p[0], p[1], p[2], p[3]);				// +0x310
	}
	f_650 = 1.0f;
	Pop();													// element +0xdc
}

void C3DWaterBalloon::v222(void)							// 0x47d1a0
{
	float p[4];
	v195(p);												// +0x30c
	if (f_64c)
	{
		f_64c->ShowAgain();									// element +0xd4
		f_64c->v196(p[0], p[1], p[2], p[3]);				// +0x310
	}
	f_650 = 1.0f;
	Pop();													// element +0xdc
}

void C3DWaterBalloon::Process(float dt)						// 0x47cbf0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (f_654 < f_65c)										// still waiting to appear
	{
		f_654 = dt + f_654;
		if (f_654 < f_65c)
		{
			v136(0);										// +0x220
			v132(0);										// +0x210
			HideAgain();									// element +0xd8
			return;
		}
		v136(1);
		v132(1);
		ShowAgain();										// element +0xd4
	}
	else f_654 = dt + f_654;
	C3DSprite::Process(dt);									// 0x49cfa0

	CGameObject *target = NULL;
	if (f_64c && f_650 > 0)									// the trail left behind fades out
	{
		f_650 = f_650 - dt;
		if (f_650 <= 0) f_64c->HideAgain();					// element +0xd8
		else f_64c->set_diffuse(OMediaFARGBColor(f_650, 1.0f, 1.0f, 1.0f));	// 2002 +0x90.. (r, g, b, a)
	}
	if (f_62c > 0)
	{
		f_62c = f_62c - dt;
		if (f_62c <= 0)
		{
			f_62c = 0;
			v136(1);
		}
	}
	else
	{
		target = f_634;
		if (f_64c) f_64c->v136(0);
	}
	AddRotation(0, 0, dt * 250.0f);							// +0x330; 0x4d92e0
	if (f_658 > 0)
	{
		const float t = f_654 - f_65c;
		const float size = t * f_658 + (float)m_sprite_size;	// (fiadd)
		set_size(size, size);								// 2002 +0xac / +0xb0
		const float a = 1.0f - t * 0.2f;					// 0x4caa6c, 0x4d7f8c
		if (a > 0) set_diffuse(OMediaFARGBColor(a, 1.0f, 1.0f, 1.0f));
		else
		{
			v136(0);
			v132(0);
			HideAgain();									// element +0xd8
		}
	}
	else set_size((float)m_sprite_size, (float)m_sprite_size);
	if (!target) return;

	// homing: pull towards the target's aim point when it is close enough (by skill level)
	float p[4], q[4];
	v195(p);												// +0x30c
	target->v195(q);
	OMedia3DVector d(q[0] + f_63c - p[0], q[1] + f_640 - p[1], q[2] + f_644 - p[2], 0);
	const float dist = (float)sqrt((double)d.x * d.x + (double)d.y * d.y + (double)d.z * d.z);	// fsqrt
	const float k = 1.0f / d.quick_magnitude();
	d.x = k * d.x;
	d.y = k * d.y;
	d.z = k * d.z;
	bool pull = false;
	if (g_difficulty == 0) pull = dist < 1700.0f;				// 0x4f1f68
	else if (g_difficulty == 1) pull = dist < 1200.0f;			// 0x50495c
	else if (g_difficulty == 2) pull = dist < 700.0f;			// 0x4ccf18
	if (pull) v178(d.x * dt * 16500.0f, d.y * dt * 16500.0f, d.z * dt * 16500.0f);	// +0x2c8; 0x4d9870
	// then the speed back to f_638 along the new velocity
	float v[4];
	v174(v);												// +0x2b8
	OMedia3DVector u(v[0], v[1], v[2], v[3]);
	const float m = 1.0f / u.quick_magnitude();
	v175(m * u.x * f_638, m * u.y * f_638, m * u.z * f_638, v[3]);	// +0x2bc
}

void C3DWaterBalloon::v260(void)							// 0x47d310
{
	C3DPermanentSprite::v260();								// 0x45d7b0
	InitPhysics();											// vtable +0x2c
	v67(25.0f);												// +0x10c
	f_634 = NULL;
	HideAgain();											// element +0xd8
	if (f_64c)
	{
		f_64c->HideAgain();									// element +0xd8
		f_64c->v136(0);										// +0x220
	}
}
