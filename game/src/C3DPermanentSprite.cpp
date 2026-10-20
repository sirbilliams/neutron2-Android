/*  C3DPermanentSprite - see C3DPermanentSprite.h. */
#include "C3DPermanentSprite.h"
#include <string.h>

C3DPermanentSprite::C3DPermanentSprite()					// 0x45d5e0
{
	strcpy(m_sprite_db, "permanenticons.omt");
	set_canvas_flags((omt_CanvasElementFlags)(get_canvas_flags() | 2));	// +0xc4 |= 2 (an OMT 2002 flag 2.5 does not have)
}

C3DPermanentSprite::~C3DPermanentSprite() {}				// 0x45d6e0

void C3DPermanentSprite::v258(void)							// 0x45d720
{
	CLocalGameObject::v258();
	LoadSprite(true);										// (inlined) from the permanent databases
}

void C3DPermanentSprite::v260(void)							// 0x45d7b0
{
	LoadSprite(true);										// (inlined) from the permanent databases
}
