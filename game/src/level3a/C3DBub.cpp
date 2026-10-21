/*  C3DBub - see C3DBub.h. Reconstructed from Neutron2.exe 0x422cf0..0x423130. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DBub.h"
#include "C3DMiniSub.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DBub::C3DBub()											// 0x422cf0
{
	SetName((char *)"C3DBUB");
	Log("C3DBub()");
	C3DBub::InitObject();									// (inlined)
	C3DSprite::SetObjectID('3BUU');							// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 26;
	m_sprite_size = 20;
	v46(1);
	C3DSprite::InitPhysics();								// 0x49ca30 (slot 11)
	v67(1.0f);
	v107(0);
	v52(0);
	f_668 = 100.0f;
	f_66c = 200.0f;
	f_670 = 0;
	// (then CGameObject's empty slot 13)
}

C3DBub::~C3DBub() {}										// 0x422ed0

namespace neg {
CGameObject *New_C3DBub(void) { return new C3DBub; }					// '3BUU' (0x67c bytes)

// C3DMiniSub's hook (weak there): a new trail bubble starts faded by v (0.2).
void MiniSubBubSet670(CGameObject *bub, float v)			// 0x453856
{
	C3DBub *b = dynamic_cast<C3DBub *>(bub);
	if (b) b->f_670 = v;									// (element +0x670)
}
}

// --------------------------------------------------------------------------- CGameObject family

void C3DBub::InitObject(void)								// 0x423070
{
	Log("InitObject()");
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("UpAccel", &f_668, PROP_FLOAT, 0);		// +0x3fc
	RegisterProperty("MaxUpRate", &f_66c, PROP_FLOAT, 0);
	// (then CGameObject's empty slot 13)
}

void C3DBub::Process(float dt)								// 0x422f10
{
	C3DSpriteType::Process(dt);								// 0x473700
	float a[4];
	v155(a);												// +0x26c
	const float az = a[2];
	v155(a);
	v157(a[0], -g_63119c, az);								// +0x274: gravity cancelled
	v159(0, dt * f_668, 0);									// +0x27c: pushed up
	float v[4];
	v174(v);												// +0x2b8
	if (v[1] < f_66c)										// rising at least MaxUpRate
	{
		v174(v);
		const float vz = v[2];
		v174(v);
		v176(v[0], f_66c, vz);								// +0x2c0
	}
	f_670 = (float)(dt * 0.4 + f_670);						// 0x4d1230
	if (f_670 > 1.0f) f_670 = 1.0f;
	if (f_670 < 0) f_670 = 0;
	set_diffuse(OMediaFARGBColor(1.0f - f_670, 1.0f, 1.0f, 1.0f));	// 2002 +0x90.. (r, g, b = 1, a)
}
