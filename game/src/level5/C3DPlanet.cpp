/*  C3DPlanet - see C3DPlanet.h. Reconstructed from Neutron2.exe 0x45fa90..0x45ff00. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DPlanet.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include <list>
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

C3DPlanet::C3DPlanet()										// 0x45fa90
{
	SetName((char *)"C3DPLANET");
	Log("C3DPlanet()");
	C3DSprite::InitObject();								// 0x49c950 (it has no InitObject of its own)
	SetObjectID('3PLA');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 200;
	m_sprite_size = 150;
	CGameObject::v42(0);									// 0x4abeb0 (direct calls)
	CGameObject::v40(1);
	CGameObject::v52(1);
	CGameObject::v67(100.0f);								// radius
	CGameObject::v46(1);
	m_sun = 0;
	CGameObject::v111(0);
	CGameObject::v62(0.0001f);								// (0x38d1b717)
	m_orbit = 0;
	C3DSprite::InitPhysics();								// 0x49ca30
	// (then a call to the empty 0x463890)
}

C3DPlanet::~C3DPlanet()										// 0x45fc40
{
}

namespace neg {
CGameObject *New_C3DPlanet(void) { return new C3DPlanet; }	// '3PLA' (0x678 bytes)
}

void C3DPlanet::v260(void)									// 0x45fc80
{
	C3DSpriteType::v260();									// 0x473650
	m_sun = FindObjectByTag("SUN");							// 0x4aefb0
	float s[4], p[4];
	m_sun->v195(s);											// +0x30c (sic: no check for a missing sun)
	v195(p);
	OMedia3DVector d(s[0] - p[0], s[1] - p[1], s[2] - p[2], 0);
	m_orbit = sqrtf(d.z * d.z + d.y * d.y + d.x * d.x);
	const float inv = 1.0f / d.quick_magnitude();			// 0x4caa6c
	d.x *= inv;
	d.y *= inv;
	d.z *= inv;
	v176(d.z * 0.2f, 0, d.x * 0.2f);						// +0x2c0 velocity (0x4e562c)
}

void C3DPlanet::Process(float dt)							// 0x45fd70
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	if (!m_sun) return;
	float s[4], p[4];
	m_sun->v195(s);											// +0x30c
	v195(p);
	const float dx = s[0] - p[0], dy = s[1] - p[1], dz = s[2] - p[2];
	v178(dx * dt * 0.1f, dy * dt * 0.1f, dz * dt * 0.1f);	// +0x2c8 velocity += (0x4caa68)
	if (sqrtf(dz * dz + dy * dy + dx * dx) > m_orbit)
	{
		const float k = dt * 0.1f;
		v178(k, 0, k);
		return;
	}
	const float k = dt * -1.5f;								// 0x4f1870
	v178(k, 0, k);
}
