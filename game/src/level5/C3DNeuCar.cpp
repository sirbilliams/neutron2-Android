/*  C3DNeuCar - see C3DNeuCar.h. Reconstructed from Neutron2.exe 0x459210..0x459410. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DNeuCar.h"
#pragma GCC diagnostic pop

using namespace neg;

C3DNeuCar::C3DNeuCar()										// 0x459210
{
	SetName((char *)"C3DNEUCAR");
	Log("C3DNEUCAR()");
	Log("InitObject()");									// 0x459410 inlined
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// +0x110
	CreateAnim((char *)"DEFAULT", (char *)"car.ase");		// +0xe0
	CreateTexture((char *)"carred.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(300.0f);											// radius
	ev58((char *)"DEFAULT", 1);								// +0xe8 looped
	C3DObject::SetObjectID('3NCA');							// 0x49ab00
	f_968 = 20.0f;											// speed
	CGameObject::v42(0);									// 0x4abeb0 (direct call)
	C3DObject::InitPhysics();								// 0x49acb0
}

C3DNeuCar::~C3DNeuCar()
{
}

namespace neg {
CGameObject *New_C3DNeuCar(void) { return new C3DNeuCar; }	// '3NCA' (0x1b74 bytes)
}

void C3DNeuCar::InitObject(void)							// 0x459410
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0 (C3DAICar has none)
	InitAnim();
	CreateAnim((char *)"DEFAULT", (char *)"car.ase");
	CreateTexture((char *)"carred.png", 0);
	ApplyTexture(f_590, 0);
	v67(300.0f);
	ev58((char *)"DEFAULT", 1);
}
