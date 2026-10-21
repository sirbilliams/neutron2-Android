/*  C3DChain and C3DCraneClaw - see C3DCraneParts.h. Reconstructed from Neutron2.exe
    0x426a80..0x426d50 and 0x4293c0..0x429c70. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCraneParts.h"
#include "C3DCraneBoom.h"
#include "../C3DShadow.h"
#include "../objects/C3DOmtObj.h"
#include "../InterfaceElement.h"				// AddScore
#include "../Physics.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include <string>
#include <strings.h>

using namespace neg;

// The game's random number: 0..1000 (the LCG at g_random_seed).
static int Random1000(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return (int)(((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// =========================================================================== C3DChain

C3DChain::C3DChain()										// 0x426a80
{
	SetName((char *)"C3DCHAIN");
	Log("C3DChain()");
	C3DChain::InitObject();									// 0x426c20 (called directly)
	v64(0.3f);
	C3DObject::SetObjectID('3CHA');							// 0x49ab00
	v107(0);
	v52(0);
	C3DObject::HideAgain();									// 0x49b120
	C3DObject::InitPhysics();								// 0x49acb0
	// (then a call to the empty 0x463890)
}

C3DChain::~C3DChain()										// 0x426bd0
{
}

void C3DChain::InitObject(void)								// 0x426c20
{
	Log("C3DGraplingHook::InitObject()");					// (sic)
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"CHAIN", (char *)"cranecable.ase");	// +0xe0
	CreateTexture((char *)"craneclaw.png", 0);				// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(10.0f);												// +0x10c
	ev58((char *)"CHAIN", 1);								// +0xe8
	float r[4];
	v201(r);												// +0x324 (three times: z, y, x)
	v203(r[0], r[1], r[2]);									// +0x32c (writes back what it read)
	// (then a call to the empty 0x463890)
}

// =========================================================================== C3DCraneClaw

C3DCraneClaw::C3DCraneClaw()								// 0x4293c0
{
	SetName((char *)"C3DCRANECLAW");
	Log("C3DCRANECLAW()");
	C3DCraneClaw::InitObject();								// 0x429620 (called directly)
	C3DObject::SetObjectID('3CLW');							// 0x49ab00
	v46(0);
	v52(1);
	v42(1);
	v40(0);
	v44(0);
	v107(0);
	v34(1);
	m_boom = NULL;											// port: the boom sets it right after
	m_shadow = new C3DShadow;								// 0x60c bytes
	// (debug log "C3DShadow", 0x60c)
	m_shadow->SetSize(140, 60);								// 0x46ec30
	m_shadow->v107(0);										// CGameObject +0x1ac
	m_shadow->v36(1);										// CGameObject +0x90
	v111(0);
	m_holding = 0;
	m_held = NULL;
	m_active = 0;
	C3DObject::InitPhysics();								// 0x49acb0
}

C3DCraneClaw::~C3DCraneClaw()								// 0x4295d0
{
}

void C3DCraneClaw::InitObject(void)							// 0x429620
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"OPEN", (char *)"craneclawopen.ase");	// +0xe0
	CreateAnim((char *)"CLOSE", (char *)"craneclawclosed.ase");
	CreateTexture((char *)"craneclaw.png", 0);				// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(10.0f);												// +0x10c
	ev58((char *)"OPEN", 1);								// +0xe8
}

void C3DCraneClaw::Drop(CGameObject *held)					// 0x429980
{
	C3DOmtObj *o = dynamic_cast<C3DOmtObj *>(held);			// (held - 0xc0)
	if (o) o->f_9a8 = 0;									// port: the original does not check
	m_holding = 0;
	m_held = NULL;
	ev58((char *)"OPEN", 1);								// +0xe8
	m_boom->m_up = 1;
}

void C3DCraneClaw::AddTouching(CGameObject *other)			// 0x4296d0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (other->NameContains((char *)"C3DOMTOBJ") && strcasecmp(other->m_tag, "PLATFORM") != 0 && !m_holding)
	{
		// close on a crate
		ev58((char *)"CLOSE", 1);
		m_held = other;
		m_holding = 1;
		C3DOmtObj *o = dynamic_cast<C3DOmtObj *>(other);	// (other - 0xc0)
		if (o) o->f_9a8 = 1;								// port: the original does not check
		C3DAnimated *a = dynamic_cast<C3DAnimated *>(m_held);
		if (a) a->f_660 = 1;								// "CanMove"
		m_boom->m_up = 1;
		PlayGameSound(-1, std::string("cranedrop"), false);	// 0x48f420
	}
	if (other->NameContains((char *)"C3DOMTOBJ") && strcasecmp(other->m_tag, "PLATFORM") == 0 && m_held)
	{
		// put it down on the platform
		C3DOmtObj *platform = dynamic_cast<C3DOmtObj *>(other);	// (other - 0xc0)
		if (platform) platform->ev94(m_held);				// element +0x178
		PlayGameSound(-1, std::string("cranedrop"), false);
		const float r = (float)Random1000() * 0.001f;		// 0x4cb218
		const char *voice;
		if (r < 0.2) voice = "jimmyitem1";					// 0x4d6c38 (doubles)
		else if (r < 0.4) voice = "jimmyitem2";				// 0x4d1230
		else if (r < 0.6) voice = "jimmyitem3";				// 0x4cb230
		else if (r < 0.8) voice = "jimmyitem4";				// 0x4d6c30
		else voice = "jimmyitem5";
		PlayGameSound(-1, std::string(voice), false);
		AddScore(100);										// 0x482b50
		if (platform) platform->f_9a8 = 0;					// (the platform's, as the original)
		m_holding = 0;
		m_held = NULL;
		ev58((char *)"OPEN", 1);
		m_boom->m_up = 1;
	}
}

unsigned char C3DCraneClaw::v231(CGameObject *other)		// 0x4299f0
{
	if (!m_active) return 0;
	return other->m_object_id == '3OMT';
}

void C3DCraneClaw::Process(float dt)						// 0x429a20
{
	C3DAnimated::Process(dt);								// 0x41c120
	if (m_active != 1) return;
	if (!g_windowed)
	{
		// the shadow on the ground below
		float p[4];
		v195(p);											// +0x30c
		const OMedia3DVector a(p[0], p[1] + 100.0f, p[2], p[3]);	// 0x4d2cb0
		const OMedia3DVector b(p[0], a.y - 2000.0f, p[2], p[3]);	// 0x4cb1ec
		if (PhysicsClosestHit(a, b, false))					// 0x4b7640
		{
			const float hit_y = g_phys_ray.end.y;			// 0x6311f8..
			if (m_shadow)
			{
				float q[4];
				v195(q);
				m_shadow->v197(q[0], hit_y + 30.0f, q[2]);	// +0x314 of its CGameObject part; 0x4ca4b4
				m_shadow->ShowAgain();						// element +0xd4
			}
		}
	}
	if (m_held)
	{
		float p[4], r[4];
		v195(p);
		m_held->v196(p[0], p[1], p[2], p[3]);				// +0x310
		v201(r);											// +0x324
		m_held->v202(r[0], r[1], r[2], r[3]);				// +0x328
	}
}

void C3DCraneClaw::v260(void)								// 0x4299d0
{
	C3DAnimated::v260();									// 0x41cec0
	v152(95.0f);											// +0x260
}
