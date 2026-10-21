/*  C3DTargetCursor - see C3DTargetCursor.h. */
#include "C3DTargetCursor.h"
#include "Game.h"
#include <string.h>

using namespace neg;

C3DTargetCursor::C3DTargetCursor()							// 0x475b20
{
	SetName((char *)"C3DSHADOW");							// (sic: copied from C3DShadow)
	Log("C3DShadow()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3TAR');							// 0x49b550
	strcpy(m_sprite_db, "retainedsprites.omt");
	m_sprite_index = 8;
	m_sprite_size = 150;
	v42(1);
	v40(1);
	v52(0);
	v107(0);
	v36(0);
	f_600 = 110;
	f_602 = 110;
	// (then a call to the empty 0x463890)
}

C3DTargetCursor::~C3DTargetCursor()							// 0x475cb0
{
}

void C3DTargetCursor::v260(void)							// 0x475cf0
{
	C3DPermanentSprite::v260();								// 0x45d7b0
	set_size((float)f_600, (float)f_602);					// 2002 +0xac / +0xb0
}
