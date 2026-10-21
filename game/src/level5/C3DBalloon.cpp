/*  C3DBalloon - see C3DBalloon.h. Reconstructed from Neutron2.exe 0x421300..0x421850. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DBalloon.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../InterfaceElement.h"				// AddScore
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <string>

using namespace neg;

C3DBalloon::C3DBalloon()									// 0x421300
{
	SetName((char *)"C3DBALLOON");
	C3DSprite::InitObject();								// 0x49c950 (it has no InitObject of its own)
	SetObjectID('3BAL');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 298;
	m_sprite_size = 100;
	CGameObject::v42(0);									// 0x4abeb0 (direct calls)
	CGameObject::v40(0);
	CGameObject::v52(1);
	CGameObject::v46(0);
	CGameObject::v34(1);
	CGameObject::v67(100.0f);								// radius
	m_tied = 1;
	m_red = 1.0f;
	m_green = 1.0f;
	m_blue = 1.0f;
	m_popped = 0;
	RegisterProperty("Red", &m_red, PROP_FLOAT, 0);			// 0x4aeee0
	RegisterProperty("Green", &m_green, PROP_FLOAT, 0);
	RegisterProperty("Blue", &m_blue, PROP_FLOAT, 0);
	C3DSprite::InitPhysics();								// 0x49ca30
}

C3DBalloon::~C3DBalloon()									// 0x4214e0
{
}

namespace neg {
CGameObject *New_C3DBalloon(void) { return new C3DBalloon; }	// '3BAL' (0x680 bytes)
}

void C3DBalloon::ResetObject(void)							// 0x421520
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	v176(0, 0, 0);											// +0x2c0 velocity
	v157(0, 0, 0);											// +0x274 acceleration
	set_diffuse(OMediaFARGBColor(0.8f, m_red, m_green, m_blue));	// element +0x90.. (r, g, b, 0.8)
	m_tied = 1;
}

void C3DBalloon::AddTouching(CGameObject *other)			// 0x4215a0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (m_popped) return;
	if (other->NameContains((char *)"C3DJIMMY") && g_camera == other)	// +0x18
	{
		if (!m_tied) return;
		v132(1);											// +0x210
		v136(0);											// +0x220
		m_tied = 0;
		return;
	}
	if (!other->NameContains((char *)"C3DBASEBALL") && !other->NameContains((char *)"C3DROCKET"))
		return;
	HideAgain();											// element +0xd8
	int bonus = 0;
	if (g_camera)
	{
		float p[4], c[4];
		v195(p);											// +0x30c
		g_camera->v195(c);
		const float dx = p[0] - c[0], dy = p[1] - c[1], dz = p[2] - c[2];
		const float dist = sqrtf(dz * dz + dy * dy + dx * dx);
		if (dist > 1000.0f)									// 0x4cb1d8
			bonus = (int)(dist * 0.033333335f - 33.0f);		// 0x4cf244, 0x4cf240; _ftol
	}
	AddScore(bonus + (m_tied ? 10 : 200));					// g_gametype 0x482b50
	PlayGameSound(-1, std::string("balloon pop"), false);	// 0x410850, 0x48f420
	m_popped = 1;
}

void C3DBalloon::Process(float dt)							// 0x421720
{
	C3DSprite::Process(dt);									// 0x49cfa0
	if (m_popped || m_tied) return;
	float v[4];
	v174(v);												// +0x2b8 velocity
	if (v[1] < 70.0f)										// 0x4caa50
	{
		v174(v);
		v176(0, v[1], 0);									// +0x2c0
	}
	else
		v176(0, 70.0f, 0);
	float a[4], a2[4];
	v155(a);												// +0x26c acceleration
	const float ay = 10.0f - g_gravity.y;					// 0x4cb1cc, 0x63119c: gravity cancelled, + 10
	v155(a2);
	v157(a2[0], ay, a[2]);									// +0x274
}
