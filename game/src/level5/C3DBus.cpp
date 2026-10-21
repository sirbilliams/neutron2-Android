/*  C3DBus - see C3DBus.h. Reconstructed from Neutron2.exe 0x4235f0..0x4238e0 and the shared
    InitObject 0x43e920. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DBus.h"
#pragma GCC diagnostic pop

using namespace neg;

C3DBus::C3DBus()											// 0x4235f0
{
	SetName((char *)"C3DBUS");
	Log("C3DBUS()");
	f_649 = 0;												// animations not loaded
	Log("InitObject()");									// 0x43e920 inlined
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// +0x110
	C3DObject::SetObjectID('3SBU');							// 0x49ab00
	f_968 = 400.0f;											// speed
	CGameObject::v64(0.05f);								// 0x4ac0c0 (direct calls)
	CGameObject::v66(1000.0f);
	CGameObject::v94(2);
	CGameObject::v34(0);
	f_1b6c = -1;
	CGameObject::v42(0);
	C3DObject::InitPhysics();								// 0x49acb0
}

C3DBus::~C3DBus()											// 0x423790
{
}

namespace neg {
CGameObject *New_C3DBus(void) { return new C3DBus; }		// '3SBU' (0x1b78 bytes)
}

void C3DBus::InitObject(void)								// 0x43e920
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// +0x110
}

void C3DBus::ev69(void)										// 0x4237e0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"HIDEFAULT", (char *)"bus.ase");		// +0xe0
	CreateTexture((char *)"bus.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"STOP", 1);								// +0xe8 looped
}

void C3DBus::v277(void)										// 0x423850
{
	ev79(1);												// +0x13c
}

void C3DBus::v278(void)										// 0x423860
{
	ev79(0);
}
