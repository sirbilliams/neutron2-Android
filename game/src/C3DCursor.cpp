/*  C3DCursor - see C3DCursor.h. */
#include "C3DCursor.h"
#include "Game.h"
#include "OMedia3DShape.h"

using namespace neg;

C3DCursor::C3DCursor()										// 0x429c70
{
	SetName((char *)"C3DCURSOR");
	Log("C3DCURSOR()");
	C3DCursor::InitObject();								// 0x429e20 (called directly)
	v64(0.3f);
	C3DObject::SetObjectID('3CUR');							// 0x49ab00
	v107(0);
	v36(0);
	v52(0);
	f_88c = 1;												// "FaceViewport"
	C3DObject::InitPhysics();								// 0x49acb0
	// (then a call to the empty 0x463890)
}

C3DCursor::~C3DCursor()										// 0x429dd0
{
}

void C3DCursor::InitObject(void)							// 0x429e20
{
	Log("C3DGraplingHook::InitObject()");					// (sic: copied from C3DGraplingHook)
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"ARROW", (char *)"3darrow.ase");		// +0xe0
	CreateTexture((char *)"3darrow.png", 0);				// +0xfc
	ev65(0, 6, 1);											// +0x104: blend Src_Alpha, One
	ApplyTexture(f_590, 0);									// +0x100
	f_590->scale(5.0f, 5.0f, 5.0f);							// shape vtable +0x34
	v67(40.0f);												// +0x10c
	ev58((char *)"ARROW", 1);								// +0xe8
	// (then a call to the empty 0x463890)
}
