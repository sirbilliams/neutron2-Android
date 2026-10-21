/*  C3DTrampoline - see C3DTrampoline.h. Reconstructed from Neutron2.exe 0x476cc0..0x477450
    (and the shared 0x43d610). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DTrampoline.h"
#include "../C3DPlayer.h"					// PlayerKeyJump
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string>
#include <strings.h>

using namespace neg;

C3DTrampoline::C3DTrampoline()								// 0x476cc0
{
	SetName((char *)"C3DTRAMPOLINE");
	Log("C3DTrampoline()");
	f_649 = 0;
	C3DTrampoline::InitObject();							// 0x476ec0 (called directly)
	C3DObject::SetObjectID('3TRA');							// 0x49ab00
	v94(2);
	v66(10.0f);
	v46(0);
	v52(1);
	v34(1);
	C3DObject::InitPhysics();								// 0x49acb0
	v107(1);
	f_930 = 1.0f;
	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = 0;
	m_bounce_time = 0.5f;
	m_timer = 0;
	m_min_strength = 400.0f;
	m_max_strength = 1000.0f;
	m_vel_y = 0;
	// (then a call to the empty 0x463890)
}

C3DTrampoline::~C3DTrampoline()								// 0x476e70
{
}

namespace neg {
CGameObject *New_C3DTrampoline(void) { return new C3DTrampoline; }		// '3TRA' (0x954 bytes)
}

// --------------------------------------------------------------------------- element family

void C3DTrampoline::ev67(void)								// 0x477080
{
	if (strcasecmp(ev66(), "BOUNCE") == 0)					// +0x108; 0x4c4520 _stricmp
		ev58((char *)"DEFAULT", 1);							// +0xe8
}

// --------------------------------------------------------------------------- CGameObject family

void C3DTrampoline::InitObject(void)						// 0x476ec0
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("MinStrength", &m_min_strength, PROP_FLOAT, 0);
	RegisterProperty("MaxStrength", &m_max_strength, PROP_FLOAT, 0);
	InitAnim();												// element +0x110
	f_649 = 1;
	CreateAnim((char *)"BOUNCE", (char *)"trampoline2.ase");	// +0xe0
	CreateAnim((char *)"DEFAULT", (char *)"trampoline.ase");
	CreateTexture((char *)"trampoline.png", 0);				// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"DEFAULT", 1);								// +0xe8
}

void C3DTrampoline::AddTouching(CGameObject *other)			// 0x476f90
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (g_camera != other || m_timer > 0.0f) return;		// 0x4caa44
	PlayGameSound(-1, std::string("boing"), false);			// 0x48f420
	m_timer = m_bounce_time;
	g_camera->v195(m_pos);									// +0x30c
	float p[4];
	v195(p);
	m_pos[1] = p[1] + 40.0f;								// 0x4d65c8
	ev58((char *)"BOUNCE", 0);								// element +0xe8
	float v[4];
	other->v174(v);											// +0x2b8 velocity
	m_vel_y = v[1];
}

unsigned char C3DTrampoline::v231(CGameObject *other)		// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DTrampoline::Process(float dt)						// 0x4770c0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	if (!g_camera || m_timer <= 0.0f) return;
	m_timer -= dt;
	float v[4], v2[4], a[4], a2[4];
	if (m_timer <= 0.0f)
	{
		// the end of the bounce: up he goes
		m_timer = 0;
		g_camera->v197(m_pos[0], m_pos[1], m_pos[2]);		// +0x314 position
		float s;
		if (PlayerKeyJump())								// 0x4658a0
		{
			s = m_vel_y * -1.2f;							// 0x50059c
			if (s > m_max_strength) s = m_max_strength;
			if (s < m_min_strength) s = m_min_strength;
		}
		else
		{
			s = m_vel_y * -0.8f;							// 0x4e2b68
			if (s > m_max_strength) s = m_max_strength;
			if (s < m_min_strength) s = m_min_strength;
		}
		if (g_camera->NameContains((char *)"C3DJIMMY")) g_5461cc = 0;	// +0x18
		v174(v);											// (our own velocity, as the original)
		v174(v2);
		g_camera->v176(v2[0], s, v[2]);						// +0x2c0 velocity
		g_camera->v157(0, 0, 0);							// +0x274
		return;
	}
	v174(v);
	v174(v2);
	g_camera->v176(v2[0], 0, v[2]);
	if (m_timer > m_bounce_time * 0.5f)						// 0x4caa5c: sinking
		g_camera->v197(m_pos[0], (m_timer - m_bounce_time) * 330.0f + m_pos[1], m_pos[2]);	// 0x500598
	else													// coming back up
		g_camera->v197(m_pos[0], m_pos[1] - m_timer * 330.0f, m_pos[2]);
	v155(a);												// +0x26c
	v155(a2);
	g_camera->v157(a2[0], -g_gravity.y, a[2]);				// gravity cancelled (0x63119c)
}
