/*  C3DMovableRock - see C3DMovableRock.h. Reconstructed from Neutron2.exe 0x454cf0..0x4551c0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DMovableRock.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMedia3DShape.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DMovableRock::C3DMovableRock()								// 0x454cf0
{
	SetName((char *)"C3DMOVABLEROCK");						// 0x4ab1b0
	Log("C3DMOVABLEROCK()");
	C3DAnimated::InitObject();								// 0x41b0a0 (called directly)
	C3DObject::InitPhysics();								// 0x49acb0
	C3DObject::SetObjectID('3MOR');							// 0x49ab00
	v52(1);
	v46(1);
	v42(1);
	v40(1);
	v44(0);
	m_db = GetDatabase("objects.omt");						// 0x4a4180
	f_648 = 0;												// no animations
	C3DObject::set_shape(Load3DShape(m_db, 27));			// 0x4b2c80, 0x49ab10 (called directly)
	MakeShapeUnLit(get_shape());							// 0x4b2520
	f_6d0 = 2;												// neither morph (0) nor Granny (1)
	v67(350.0f);											// radius
	v77(1.0f, 1.0f, 1.0f);
	v107(1);
	// (then a call to the empty 0x463890)
}

C3DMovableRock::~C3DMovableRock() {}							// 0x454eb0

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DMovableRock::AddTouching(CGameObject *other)		// 0x454f00
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (other->NameContains((char *)"C3DMOVABLEROCK"))		// +0x18
		v132(0);											// +0x210
}

void C3DMovableRock::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x454f60
{
	CGameObject::v220(a, b, c, d, poly);					// 0x48ef40 (the vector copied by 0x411d70)
}

unsigned char C3DMovableRock::v231(CGameObject *other)		// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DMovableRock::Process(float dt)						// 0x454f30
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	v133();													// +0x214 (the result is not used)
}

void C3DMovableRock::WriteState(OMediaStreamOperators &s)	// 0x455070
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	float p[4], a[4];
	v195(p);												// +0x30c (called once per component)
	float x = p[0], y = p[1], z = p[2];
	s << x;
	s << y;
	s << z;
	v201(a);												// +0x324
	x = a[0], y = a[1], z = a[2];
	s << x;
	s << y;
	s << z;
	bool b = v135() != 0;									// +0x21c
	s << b;
	bool b2 = v131() != 0;									// +0x20c
	s << b2;
}

void C3DMovableRock::ReadState(OMediaStreamOperators &s)		// 0x454f90
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	float x, y, z;
	s >> x;
	s >> y;
	s >> z;
	v197(x, y, z);											// +0x314 position
	s >> x;
	s >> y;
	s >> z;
	v203(x, y, z);											// +0x32c angles
	v218(x, y, z);											// +0x368
	bool b;
	s >> b;
	v136(b);												// +0x220
	s >> b;
	v46(b);													// +0xb8
}

namespace neg {
CGameObject *New_C3DMovableRock(void)	{ return new C3DMovableRock; }		// factory '3MOR' (0x92c bytes)
}
