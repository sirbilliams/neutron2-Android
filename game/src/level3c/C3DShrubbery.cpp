/*  C3DShrubbery - see C3DShrubbery.h. Reconstructed from Neutron2.exe 0x46fc60..0x46ff50. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DShrubbery.h"
#pragma GCC diagnostic pop
#include <string.h>

using namespace neg;

C3DShrubbery::C3DShrubbery()								// 0x46fc60
{
	SetName((char *)"C3DSHRUBBERY");
	Log("C3DShrubbery()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	RegisterProperty("Red",   &m_red,   PROP_FLOAT, 0);		// (InitObject's registrations, inlined)
	RegisterProperty("Green", &m_green, PROP_FLOAT, 0);
	RegisterProperty("Blue",  &m_blue,  PROP_FLOAT, 0);
	SetObjectID('3SHU');
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 65;
	m_sprite_size = 400;
	v42(0);
	v40(0);
	v52(1);
	v34(1);
	v107(1);
	v67(400.0f);
	v46(0);
	m_red = 1.0f;
	m_green = 1.0f;
	m_blue = 1.0f;
	C3DSprite::InitPhysics();								// 0x49ca30 (called directly)
	// (then a call to the empty 0x463890)
}

C3DShrubbery::~C3DShrubbery() {}							// 0x46fe60

void C3DShrubbery::InitObject(void)							// 0x46fea0
{
	C3DSprite::InitObject();
	RegisterProperty("Red",   &m_red,   PROP_FLOAT, 0);
	RegisterProperty("Green", &m_green, PROP_FLOAT, 0);
	RegisterProperty("Blue",  &m_blue,  PROP_FLOAT, 0);
}

namespace neg {
CGameObject *New_C3DShrubbery(void)	{ return new C3DShrubbery; }	// factory '3SHU' (0x67c bytes)
}
