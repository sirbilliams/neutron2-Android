/*  C3DSwingDoor - see C3DSwingDoor.h. Reconstructed from Neutron2.exe 0x474da0..0x475360. */
#include "C3DSwingDoor.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include <string.h>
#include <string>

using namespace neg;

C3DSwingDoor::C3DSwingDoor()								// 0x474da0
{
	SetName((char *)"C3DDOORSWING");
	Log("C3DSwingDoor()");
	v52(1);
	v42(0);
	v40(0);
	v94(2);
	v46(0);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DObject::SetObjectID('3SWN');							// 0x49ab00
	strcpy(f_92d, "doorcloset.ase");
	strcpy(f_991, "doorcloset.png");
	C3DSwingDoor::InitObject();								// 0x474ff0 (called directly)
	f_9f8 = 50.0f;											// OpenSpeed
	f_9fc = 1.8f;											// TimeToOpen
	f_928 = 0;
	v42(0);
	v40(0);
	v34(0);
	f_a00 = 1;												// TouchActivated
	f_924 = 0;
	f_92c = 0;
	v52(1);
	// (then a call to the empty 0x463890)
}

C3DSwingDoor::~C3DSwingDoor()								// 0x474fa0
{
}

namespace neg {
CGameObject *New_C3DSwingDoor(void) { return new C3DSwingDoor; }		// '3SWN' (0xa0c bytes)
}

void C3DSwingDoor::InitObject(void)							// 0x474ff0
{
	Log("C3DShrinkRay::InitObject()");						// (sic)
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("ASEFile", f_92d, PROP_ALPHA, 0);
	RegisterProperty("PNGFile", f_991, PROP_ALPHA, 0);
	RegisterProperty("TimeToOpen", &f_9fc, PROP_FLOAT, 0);
	RegisterProperty("OpenSpeed", &f_9f8, PROP_FLOAT, 0);
	RegisterProperty("TouchActivated", &f_a00, PROP_LONG, 0);
	// (then a call to the empty 0x463890)
}

void C3DSwingDoor::AddTouching(CGameObject *other)			// 0x4750a0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (g_camera == other && f_a00 != 0) v268(1);			// +0x430
}

void C3DSwingDoor::v268(int state)							// 0x4750e0
{
	(void)state;
	if (f_924) return;
	f_924 = 1;
	f_928 = f_9fc;											// TimeToOpen
	PlayGameSound(-1, std::string("squeak"), false);		// 0x48f420
}

void C3DSwingDoor::Process(float dt)						// 0x475130
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (!f_924) return;
	f_928 -= dt;
	if (!f_92c)
	{
		AddRotation(0, dt * f_9f8, 0);						// +0x330
		if (f_928 <= 0)
		{
			f_924 = 0;
			f_92c = 1;
			f_928 = 0;
		}
	}
	else
	{
		AddRotation(0, -(dt * f_9f8), 0);
		if (f_928 <= 0)
		{
			f_924 = 0;
			f_92c = 0;
			f_928 = 0;
		}
	}
}

void C3DSwingDoor::v260(void)								// 0x475200
{
	C3DAnimated::v260();									// 0x41cec0
	InitAnim();												// element +0x110
	CreateAnim((char *)"RAY", f_92d);						// +0xe0
	CreateTexture(f_991, 0);								// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(40.0f);												// CGameObject +0x10c: radius
	ev58((char *)"RAY", 1);									// +0xe8
	v42(0);													// +0xa8
	v40(0);													// +0xa0
}

unsigned char C3DSwingDoor::v231(CGameObject *other)		// 0x4752a0
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3GOD' || id == '3BOT' || id == '3EPR' || id == '3JIM' || id == '3WBL';
}
