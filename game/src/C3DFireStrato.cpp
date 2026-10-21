/*  C3DFireStrato - see C3DFireStrato.h. */
#include "C3DFireStrato.h"

C3DFireStrato::C3DFireStrato()								// 0x431b60
{
	SetName((char *)"C3DFIRESTRATO");
	C3DAnimated::InitObject();								// 0x41b0a0 (called directly)
	C3DAnimated::InitAnim();								// 0x41c790
	C3DAnimated::CreateAnim((char *)"AFTERBURN", (char *)"firestrato2.ase");	// 0x41b360
	C3DAnimated::CreateAnim((char *)"DEFAULT", (char *)"firestrato.ase");
	C3DAnimated::CreateTexture((char *)"fire.png", 0);		// 0x41baa0
	C3DAnimated::ApplyTexture(f_590, 0);					// 0x41be50
	C3DAnimated::ev58((char *)"DEFAULT", 1);				// 0x41bea0
	C3DAnimated::ev65(0, 6, 1);								// 0x41bdc0 material 0: Src_Alpha, One
	C3DObject::SetObjectID('3FST');							// 0x49ab00
	v107(0);
	v36(0);
	v64(0);
	v66(1000.0f);
	v46(0);
	v52(0);
	f_664 = 1;												// SecondPass
}

C3DFireStrato::~C3DFireStrato()								// 0x431d10
{
}

void C3DFireStrato::ResetObject(void)						// 0x431d60
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	HideAgain();											// element +0xdc
}
