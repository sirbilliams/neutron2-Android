/*  C3DShock - see C3DShock.h. Reconstructed from Neutron2.exe 0x46f120..0x46f800. */
#include "C3DShock.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMediaCanvas.h"

using namespace neg;

C3DShock::C3DShock()										// 0x46f120
{
	SetName((char *)"C3DSHOCK");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3SHO');							// 0x49b550
	m_db = GetDatabase("sprites.omt");						// 0x4a4180, CLocalGameObject +0x4b4
	f_670[0] = 226;
	f_670[1] = 227;
	f_670[2] = 228;
	for (int i = 3; i < 9; i++) f_670[i] = -1;
	f_668 = 10.0f;
	f_66c = 3;
	f_698 = 0;
	f_69c = 0;
	m_sprite_size = 220;
	v107(0);
	v42(0);
	v40(0);
	v46(0);
	v52(0);
	v34(1);
	f_694 = 0;
	f_6a0 = 1;
	f_6a4 = -1.0f;
	f_6a8 = 1.0f;
	C3DSprite::InitPhysics();								// 0x49ca30
}

C3DShock::~C3DShock()										// 0x46f300
{
}

namespace neg {
CGameObject *New_C3DShock(void) { return new C3DShock; }				// '3SHO' (0x6b4 bytes)
}

void C3DShock::v260(void)									// 0x46f680
{
	C3DSpriteType::v260();									// 0x473650
	set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);	// +0xb4 = 6, +0xb8 = 7 / 1
	f_69c = 0;
	for (int i = 8; i >= 0; i--)							// the first -1 ends the animation
		if (f_670[i] == -1) f_66c = i;
	m_sprite_index = f_670[0];
	InitSpriteProperties(0);								// element +0xd0
	if (f_694) ShowAgain();									// +0xd4
	else HideAgain();										// +0xd8
}

void C3DShock::Process(float dt)							// 0x46f340
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	f_69c = dt + f_69c;
	if (!f_694) return;
	if (f_6a4 > 0)
	{
		set_diffuse(OMediaFARGBColor(f_6a4 / f_6a8, 1.0f, 1.0f, 1.0f));	// 2002 +0x90.. (r, g, b, a)
		f_6a4 -= dt;
		if (f_6a4 <= 0)
		{
			f_6a4 = -1.0f;
			HideAgain();									// element +0xd8
			f_694 = 0;
		}
	}
	if (f_69c <= 1.0f / f_668) return;
	f_69c = 0;
	// (debug log "Playing Sprite %d for %s")
	if (f_698 >= 0 && f_698 <= 8)
	{
		InitSpriteID(m_db, f_670[f_698], (float)m_sprite_size);	// element +200
		OMediaCanvas *c = get_canvas();						// element +0x8c
		if (c) c->set_filtering(omtfc_Linear, omtfc_Linear);	// +0x60, +0x64 = 1, +0x24 vtable +0x10 (port: the original does not check)
		set_blend(omblendfc_Src_Alpha, g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One);
	}
	f_698++;
	if (f_698 < f_66c) return;
	f_698 = 0;
	if (f_6a0 != 0)
	{
		if (f_6a0 != 2) return;								// 1: loop
		HideAgain();
	}
	f_694 = 0;
}
