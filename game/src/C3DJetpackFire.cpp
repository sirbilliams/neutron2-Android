/*  C3DJetpackFire - see C3DJetpackFire.h. */
#include "C3DJetpackFire.h"
#include "Game.h"
#include <strings.h>

using namespace neg;

C3DJetpackFire::C3DJetpackFire()							// 0x443500
{
	SetName((char *)"C3DJETPACKFIRE");
	C3DJetpackFire::InitObject();							// 0x443690 (called directly)
	v64(0.3f);
	C3DObject::SetObjectID('3JFI');							// 0x49ab00
	v107(0);
	v36(0);
	v52(0);
	f_924 = NULL;
	f_928 = 0;
	f_92c = 0;
}

C3DJetpackFire::~C3DJetpackFire()							// 0x443640
{
}

namespace neg {
CGameObject *New_C3DJetpackFire(void)	{ return new C3DJetpackFire; }	// factory '3JFI' (0x938 bytes)
}

void C3DJetpackFire::InitObject(void)						// 0x443690
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"ANIMATE", (char *)"jetpacksmallflame.ase");	// +0xe0
	CreateAnim((char *)"OPEN", (char *)"jetpackexpand.ase");
	// port: the original's file name is "Jetpackshrink.ase " (trailing space), which Windows
	// drops when it opens the file.
	CreateAnim((char *)"CLOSE", (char *)"Jetpackshrink.ase");
	CreateAnim((char *)"STOP", (char *)"jetpack01.ase");
	CreateTexture((char *)"jetpack.png", 0);				// +0xfc
	ev58((char *)"STOP", 1);								// +0xe8
	ApplyTexture(f_590, 0);									// +0x100
	v67(20.0f);												// +0x10c
	// (then a call to the empty 0x463890)
}

void C3DJetpackFire::ev67(void)								// 0x443760
{
	if (!strcasecmp(ev66(), "OPEN"))						// +0x108, _stricmp
	{
		ev58((char *)"ANIMATE", 1);
		return;
	}
	if (!strcasecmp(ev66(), "CLOSE")) HideAgain();			// element +0xdc
}
