/*  C3DEnergyProjectile - see C3DEnergyProjectile.h. Reconstructed from Neutron2.exe
    0x430640..0x431180. Constants: 0x4d92e0 = 250 (spin), 0x4d9870 = 16500 (homing pull),
    0x4d986c / 0x4d9874 / 0x4d9878 = 1800 / 1400 / 800 (homing range at skill 0 / 1 / 2).    */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DEnergyProjectile.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DEnergyProjectile::C3DEnergyProjectile()					// 0x430640
{
	SetName((char *)"C3DENERGYPROJECTILE");
	Log("C3DEnergyProjectile()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3EPR');							// 0x49b550
	strcpy(m_sprite_db, "retainedsprites.omt");
	m_sprite_index = 20;
	m_sprite_size = 65;
	v46(1);
	C3DSprite::InitSpriteProperties(25.0f);					// 0x49ce10
	v60(3.2f);
	v67(25.0f);
	v107(0);
	v36(1);
	v52(0);
	v62(0.46f);
	f_600 = new C3DPermanentSprite;							// 0x608 bytes
	if (f_600)
	{
		strcpy(f_600->m_sprite_db, "retainedsprites.omt");
		f_600->m_sprite_index = 31;
		f_600->m_sprite_size = 100;
		f_600->InitSpriteProperties(100.0f);				// element +0xd0
		f_600->v46(0);										// +0xb8
		f_600->v107(0);										// +0x1ac
		f_600->v36(1);										// +0x90
	}
	v111(0);
	v176(0, 0, 0);											// +0x2c0
	v157(0, 0, 0);											// +0x274
	v136(0);												// +0x220
	v132(0);												// +0x210
	f_604 = NULL;
	f_610 = 0;
	f_614 = 0;
	f_618 = 0;
	f_608 = 0;
	f_60c = 0;
	f_61c = 0;												// port: not set by the original (zeroed memory)
	HideAgain();											// element +0xd8
	// (then CGameObject's empty slot 13)
}

C3DEnergyProjectile::~C3DEnergyProjectile()					// 0x430980
{
}

namespace neg {
CGameObject *New_C3DEnergyProjectile(void)	{ return new C3DEnergyProjectile; }	// factory '3EPR' (0x628 bytes)
}

// --------------------------------------------------------------------------- not virtual

void C3DEnergyProjectile::Pop(void)							// 0x430a10
{
	if (f_600) PlayGameSound(-1, std::string("vrzap"), false);	// 0x48f420
	Reset();												// (the same code inline)
}

void C3DEnergyProjectile::Reset(void)						// 0x430c10
{
	v176(0, 0, 0);											// +0x2c0
	v157(0, 0, 0);											// +0x274
	v136(0);												// +0x220
	v132(0);												// +0x210
	f_604 = NULL;
	f_610 = 0;
	f_614 = 0;
	f_618 = 0;
	f_608 = 0;
	f_60c = 0;
	HideAgain();											// element +0xd8
}

void C3DEnergyProjectile::Fire(OMedia3DVector dir, float speed)	// 0x430cd0
{
	v162(dir.x * speed, dir.y * speed, dir.z * speed);		// +0x288
	v136(1);
	v132(1);
	ShowAgain();											// element +0xd4
}

void C3DEnergyProjectile::Home(CGameObject *target, float dt)	// 0x430d40
{
	float q[4], p[4];
	target->v195(q);										// +0x30c
	v195(p);
	OMedia3DVector d(q[0] - p[0], q[1] - p[1], q[2] - p[2], 0);
	const float dist = (float)sqrt((double)d.z * d.z + (double)d.y * d.y + (double)d.x * d.x);	// fsqrt
	const float k = 1.0f / d.quick_magnitude();
	float range = 1800.0f;									// 0x4d986c
	bool pull = false;
	if (g_difficulty == 0) pull = dist < range;
	else if (g_difficulty == 1) pull = dist < 1400.0f;		// 0x4d9874
	else if (g_difficulty == 2) { range = 800.0f; pull = dist < range; }	// 0x4d9878
	if (pull) v178(k * d.x * dt * 16500.0f, k * d.y * dt * 16500.0f, k * d.z * dt * 16500.0f);	// +0x2c8
	// then the speed back to f_610 along the new velocity
	float v[4];
	v174(v);												// +0x2b8
	OMedia3DVector u(v[0], v[1], v[2], v[3]);
	const float m = 1.0f / u.quick_magnitude();
	v175(m * u.x * f_610, m * u.y * f_610, m * u.z * f_610, v[3]);	// +0x2bc
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DEnergyProjectile::AddTouching(CGameObject *other)	// 0x430c90
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (other->NameContains((char *)"C3DBOT") || other->NameContains((char *)"C3DMOVINGTARGET"))	// +0x18
		Pop();
}

void C3DEnergyProjectile::v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly)	// 0x430ad0
{
	(void)nx; (void)ny; (void)nz; (void)nw; (void)poly;
	Pop();
}

void C3DEnergyProjectile::v222(void)						// 0x430ac0
{
	Pop();
}

void C3DEnergyProjectile::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x430ae0
{
	(void)a; (void)b; (void)c; (void)d; (void)poly;
	float v[4], p[4];
	v174(v);												// +0x2b8
	v195(p);												// +0x30c
	OMedia3DVector u(v[0], v[1], v[2], v[3]);
	const float k = 1.0f / u.quick_magnitude();
	u.x = k * u.x;
	u.y = k * u.y;
	u.z = k * u.z;
	p[0] = p[0] - u.x * 200.0f;								// 0x4cb21c: the trail stays 200 back
	p[1] = p[1] - u.y * 200.0f;
	p[2] = p[2] - u.z * 200.0f;
	if (f_600)
	{
		f_600->ShowAgain();									// element +0xd4
		f_600->v196(p[0], p[1], p[2], p[3]);				// +0x310
	}
	f_61c = 1.0f;
	Pop();
}

unsigned char C3DEnergyProjectile::v231(CGameObject *other)	// 0x4310f0
{
	const int id = other->m_object_id;
	return id == '3BUG' || id == '3GWA' || id == '3TAR';
}

void C3DEnergyProjectile::Process(float dt)					// 0x430f60
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	AddRotation(0, 0, dt * 250.0f);							// +0x330; 0x4d92e0
	if (f_604)
	{
		if (f_60c) Home(f_604, dt);
		else if (f_608 <= 0)
		{
			f_608 = 0;
			f_60c = 1;
		}
		else f_608 = f_608 - dt;
	}
	if (f_600 && f_600->m_visible && f_61c > 0)				// the trail left behind fades out (port: f_600 checked)
	{
		f_61c = f_61c - dt;
		if (f_61c <= 0)
		{
			f_600->HideAgain();								// element +0xd8
			return;
		}
		f_600->set_diffuse(OMediaFARGBColor(f_61c, 1.0f, 1.0f, 1.0f));	// 2002 +0x90.. (r, g, b = 1, a)
	}
}

void C3DEnergyProjectile::v260(void)						// 0x4309c0
{
	C3DPermanentSprite::v260();								// 0x45d7b0
	InitPhysics();											// vtable +0x2c
	HideAgain();											// element +0xd8
	set_flags(omelf_SecondPass | omelf_FaceViewport);		// +0x7c = 0x208
	if (f_600) f_600->HideAgain();							// element +0xd8
}
