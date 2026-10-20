/*  C3DSpriteType - see C3DSpriteType.h. */
#include "C3DSpriteType.h"
#include "gen/globals.h"
#include "OMediaLayer.h"
#include <string.h>

C3DSpriteType::C3DSpriteType()								// 0x473420
{
	set_canvas_flags(omcanef_FreeWorldSize);				// +0xc4 = 1
	f_600 = 0;
	strcpy(f_604, g_empty_string);							// 0x545f14
}

C3DSpriteType::~C3DSpriteType() {}							// 0x473530

void C3DSpriteType::AddTouching(CGameObject *other)						// 0x4736f0
{
	CGameObject::AddTouching(other);
}

void C3DSpriteType::Process(float dt)						// 0x473700
{
	C3DSprite::Process(dt);
}

void C3DSpriteType::v242(float t)							// 0x473570
{
	CLocalGameObject::v242(t);								// (CGameObject's, empty)
	ShowAgain();											// element +0xd4
}

void C3DSpriteType::v260(void)								// 0x473650
{
	LoadSprite(false);										// (inlined, as C3DSprite's)
	set_canvas_flags(omcanef_FreeWorldSize);				// +0xc4 = 1, on every path
}

void C3DSpriteType::v282(OMediaMatrix_4x4 &m, float *w, float *h)	// 0x473630
{
	get_layer()->prepare_projection_matrix(0, m, *w, *h);	// element +0x64 its_layer, vtable +0xc
}
