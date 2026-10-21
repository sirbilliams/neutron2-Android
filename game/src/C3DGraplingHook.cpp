/*  C3DGraplingHook - see C3DGraplingHook.h. */
#include "C3DGraplingHook.h"
#include "Game.h"
#include "OMedia3DShape.h"

using namespace neg;

C3DGraplingHook::C3DGraplingHook()							// 0x43cdb0
{
	SetName((char *)"C3DGRAPLINGHOOK");
	Log("C3DGraplingHook()");
	C3DGraplingHook::InitObject();							// 0x43cf50 (called directly)
	v64(0.3f);
	C3DObject::SetObjectID('3GRA');							// 0x49ab00
	v107(0);
	v36(0);
	v52(0);
	C3DObject::HideAgain();									// 0x49b120
	C3DObject::InitPhysics();								// 0x49acb0
	// (then a call to the empty 0x463890)
}

C3DGraplingHook::~C3DGraplingHook()							// 0x43cf00
{
}

void C3DGraplingHook::InitObject(void)						// 0x43cf50
{
	Log("C3DGraplingHook::InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"ROPE", (char *)"rope01.ase");		// +0xe0
	CreateTexture((char *)"laser.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(10.0f);												// +0x10c
	ev58((char *)"ROPE", 1);								// +0xe8
	float r[4];
	v201(r);												// +0x324 (three times: z, y, x)
	v203(r[0], r[1], r[2] + 180.0f);						// +0x32c; 0x4cb1f4
	// (then a call to the empty 0x463890)
}

void C3DGraplingHook::Process(float dt)						// 0x43d030
{
	C3DAnimated::Process(dt);								// 0x41c120
	if (f_589) get_shape()->offset_textures(0, dt * 5.0f, false);	// 2002 +0x8c, shape +0xa4; 0x4caa90
}

void C3DGraplingHook::v260(void)							// 0x43d070
{
	C3DAnimated::v260();									// 0x41cec0
	ev71(0.4f);												// element +0x11c
}
