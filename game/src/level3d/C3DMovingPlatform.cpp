/*  C3DMovingPlatform - see C3DMovingPlatform.h. Reconstructed from Neutron2.exe
    0x455810..0x456320. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DMovingPlatform.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"					// PlayerKeyJump
#include "../C3DShadow.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string>

using namespace neg;

// The running level's 4cc: the game type's ObjectID (g_gametype +0x490).
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// The camera's C3DPlayer part (g_camera - 0xc0; the original does not check the class).
static C3DPlayer *CameraPlayer(void)
{
	return dynamic_cast<C3DPlayer *>(g_camera);
}

C3DMovingPlatform::C3DMovingPlatform()						// 0x455810
{
	f_1b68 = 1.0f;
	SetName((char *)"C3DMOVINGPLATFORM");
	Log("C3DMOVINGPLATFORM()");
	Log("InitObject()");									// 0x455b80 inlined
	C3DAI::InitObject();									// 0x4110e0
	RegisterProperty("IsBoat", &m_is_boat, PROP_LONG, 0);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DObject::SetObjectID('3MOP');							// 0x49ab00
	f_968 = 250.0f;											// speed
	f_96c = 3;												// state
	f_c08 = 3;												// AIState
	f_bd4 = 1;												// no gravity
	f_1b40 = 1;												// "CanHover"
	C3DAI::ev106(f_a9c, (char *)"none");					// no animations
	C3DAI::ev106(f_ac4, (char *)"none");
	C3DAI::ev106(f_aec, (char *)"none");
	C3DAI::ev106(f_b14, (char *)"none");
	C3DAI::ev106(f_a74, (char *)"none");
	C3DAI::ev106(f_b3c, (char *)"none");
	v67(100.0f);											// radius
	v152(200.0f);
	v94(2);
	v52(1);
	f_b9c.y = 0.3f;											// turn rate y
	v42(0);
	v40(0);
	m_db = GetDatabase("objects.omt");						// 0x4a4180 (element +0x574)
	f_648 = 0;												// no animations
	const int level = LevelID();							// g_gametype +0x490
	if (level == 'LV5B' || level == 'LV5A' || level == 'L5B1')
	{
		C3DObject::set_shape(Load3DShape(m_db, 37));		// 0x4b2c80, 0x49ab10
		m_radius = 235.0f;
		v152(235.0f);
	}
	else
	{
		C3DObject::set_shape(Load3DShape(m_db, 26));
		m_radius = 200.0f;
	}
	MakeShapeUnLit(get_shape());							// 0x4b2520
	v34(1);
	C3DObject::ev51();										// texture filtering on
	m_saved_axis[0] = 0;
	m_saved_axis[1] = 0;
	m_saved_axis[2] = 0;
	f_c48 = 0;
	m_channel = -1;
	f_c38 = 1;												// (no cut-scene animations)
	m_jump_held = 0;
	m_riding = 0;
	m_is_boat = 0;
}

C3DMovingPlatform::~C3DMovingPlatform()						// 0x455b30
{
}

namespace neg {
CGameObject *New_C3DMovingPlatform(void) { return new C3DMovingPlatform; }	// '3MOP' (0x1b7c bytes)
}

void C3DMovingPlatform::InitObject(void)					// 0x455b80
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	RegisterProperty("IsBoat", &m_is_boat, PROP_LONG, 0);
}

void C3DMovingPlatform::v260(void)							// 0x456060
{
	C3DAI::v260();											// 0x4143a0
	v42(0);													// +0xa8
	v40(0);													// +0xa0
}

unsigned char C3DMovingPlatform::v231(CGameObject *other)	// 0x456290
{
	if (g_camera == other) return 1;
	// (sic: it compares the game type's ObjectID, the level's 4cc, with three object IDs, so
	// only the camera collides)
	const int id = LevelID();
	return id == '3GOD' || id == '3JIM' || id == '3WBL';
}

void C3DMovingPlatform::AddTouching(CGameObject *other)		// 0x456090
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other != g_camera) return;
	float p[4], c[4];
	v195(p);												// +0x30c
	g_camera->v195(c);
	const float dx = p[0] - c[0], dy = p[1] - c[1], dz = p[2] - c[2];
	if (!(sqrtf(dx * dx + dy * dy + dz * dz) <= m_radius)) return;
	m_jump_held = PlayerKeyJump() ? 1 : 0;					// 0x4658a0
	if (other->NameContains((char *)"C3DJIMMY"))			// +0x18
	{
		C3DJimmy *j = dynamic_cast<C3DJimmy *>(other);		// (other - 0xc0)
		if (j)
		{
			j->f_96c = 0;									// forward speed
			j->f_4d90 = 1;
		}
	}
	g_camera->v176(0, 0, 0);								// +0x2c0 velocity
	g_camera->v195(c);
	v195(p);
	g_camera->v197(c[0], p[1], c[2]);						// +0x314: at our height
	C3DPlayer *pl = CameraPlayer();
	if (pl)													// port: checked
	{
		m_saved_axis[0] = pl->f_af0.x;
		m_saved_axis[1] = pl->f_af0.y;
		m_saved_axis[2] = pl->f_af0.z;
		pl->f_af0.x = 3.0f;
		pl->f_af0.y = 5.0f;
		pl->f_af0.z = 3.0f;
	}
	m_riding = 1;
}

void C3DMovingPlatform::Process(float dt)					// 0x455bc0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAI::Process(dt);										// 0x411240
	if (f_96c == 3)
	{
		if (m_channel == -1)
			m_channel = PlayGameSound3DNoDoppler(this, -1, std::string("elevator"), true);	// 0x48f0f0
	}
	else if (m_channel != -1)
	{
		StopGameSound(m_channel, false);					// 0x48f0d0
		m_channel = -1;
	}
	if (m_riding)
	{
		// (debug prints of the speed f_9a0 and of f_acc's tag, compiled empty)
		float v[4], cv[4];
		v174(v);											// +0x2b8 velocity
		g_camera->v174(cv);
		g_camera->v176(v[0] + cv[0], v[1], v[2] + cv[2]);	// +0x2c0: carried along
		float c[4], p[4];
		g_camera->v195(c);
		v195(p);
		g_camera->v197(c[0], p[1], c[2]);					// at our height
		float a[4], a2[4];
		v155(a);											// +0x26c
		v155(a2);
		v157(a2[0], -g_gravity.y, a[2]);					// +0x274: gravity cancelled (0x63119c)
		v195(p);
		g_camera->v195(c);
		const float dx = p[0] - c[0], dy = p[1] - c[1], dz = p[2] - c[2];
		const bool jump = PlayerKeyJump();					// 0x4658a0
		if (!(sqrtf(dy * dy + dz * dz + dx * dx) <= m_radius) || (!m_jump_held && jump))
		{
			// off
			C3DPlayer *pl = CameraPlayer();
			if (pl)											// port: checked
			{
				pl->f_af0.x = m_saved_axis[0];
				pl->f_af0.y = m_saved_axis[1];
				pl->f_af0.z = m_saved_axis[2];
			}
			m_riding = 0;
		}
		if (m_jump_held == 1 && !jump) m_jump_held = 0;
		if (g_camera->NameContains((char *)"C3DJIMMY"))
		{
			C3DJimmy *j = dynamic_cast<C3DJimmy *>(g_camera);	// (g_camera - 0xc0)
			if (j)
			{
				if (m_riding)
				{
					if (dt < 1.0f) j->f_96c = (1.0f - dt) * j->f_96c;	// his speed dies away
					j->f_4d90 = 1;
					float q[4];
					static_cast<CGameObject *>(j)->v195(q);
					if (j->f_cc4) j->f_cc4->v196(q[0], q[1] - 50.0f, q[2], q[3]);	// his shadow (0x4ca4b0); port: checked
				}
				else j->f_4d90 = 0;
			}
		}
	}
	float r[4];
	v91(r);													// +0x16c
	const float pulse = g_545ddc;							// 0x40b270
	if (m_is_boat)
	{
		v203(pulse + pulse + r[0], r[1], (float)((double)pulse * 1.5 + r[2]));	// +0x32c; 0x4cb1d0 (double)
		return;
	}
	v202(r[0], r[1], r[2], r[3]);							// +0x328
}
