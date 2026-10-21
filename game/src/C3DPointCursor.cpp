/*  C3DPointCursor - see C3DPointCursor.h. */
#include "C3DPointCursor.h"
#include "Game.h"

using namespace neg;

C3DPointCursor::C3DPointCursor()							// 0x466a70
{
	SetName((char *)"C3DPOINTCURSOR");
	Log("C3DPOINTCURSOR()");
	C3DPointCursor::InitObject();							// 0x466c10 (called directly)
	v64(0.3f);
	C3DObject::SetObjectID('3PCU');							// 0x49ab00
	v107(0);
	v36(0);
	v52(0);
	C3DObject::InitPhysics();								// 0x49acb0
	// (then a call to the empty 0x463890)
}

C3DPointCursor::~C3DPointCursor()							// 0x466bc0
{
}

void C3DPointCursor::InitObject(void)						// 0x466c10
{
	Log("C3DGraplingHook::InitObject()");					// (sic: copied from C3DGraplingHook)
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"ARROW", (char *)"pointarrow.ase");	// +0xe0
	CreateTexture((char *)"3darrow.png", 0);				// +0xfc
	ev65(0, 6, 1);											// +0x104: blend Src_Alpha, One
	ApplyTexture(f_590, 0);									// +0x100
	v67(40.0f);												// +0x10c
	ev58((char *)"ARROW", 1);								// +0xe8
	// (then a call to the empty 0x463890)
}
