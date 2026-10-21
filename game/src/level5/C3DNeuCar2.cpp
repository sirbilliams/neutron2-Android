/*  C3DNeuCar2 - see C3DNeuCar2.h. Reconstructed from Neutron2.exe 0x4594f0..0x4596f0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DNeuCar2.h"
#pragma GCC diagnostic pop

using namespace neg;

C3DNeuCar2::C3DNeuCar2()										// 0x4594f0
{
	SetName((char *)"C3DNEUCAR2");
	Log("C3DNEUCAR2()");
	Log("InitObject()");									// 0x4596f0 inlined
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// +0x110
	CreateAnim((char *)"DEFAULT", (char *)"car.ase");		// +0xe0
	CreateTexture((char *)"cargreen.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(300.0f);											// radius
	ev58((char *)"DEFAULT", 1);								// +0xe8 looped
	C3DObject::SetObjectID('3NC2');							// 0x49ab00
	f_968 = 20.0f;											// speed
	CGameObject::v42(0);									// 0x4abeb0 (direct call)
	C3DObject::InitPhysics();								// 0x49acb0
}

C3DNeuCar2::~C3DNeuCar2()
{
}

namespace neg {
CGameObject *New_C3DNeuCar2(void) { return new C3DNeuCar2; }	// '3NC2' (0x1b74 bytes)
}

void C3DNeuCar2::InitObject(void)							// 0x4596f0
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0 (C3DAICar has none)
	InitAnim();
	CreateAnim((char *)"DEFAULT", (char *)"car.ase");
	CreateTexture((char *)"cargreen.png", 0);
	ApplyTexture(f_590, 0);
	v67(300.0f);
	ev58((char *)"DEFAULT", 1);
}
