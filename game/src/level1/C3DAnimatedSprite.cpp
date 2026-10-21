/*  C3DAnimatedSprite - see C3DAnimatedSprite.h. Reconstructed from Neutron2.exe 0x41f910..0x420350. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DAnimatedSprite.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>

using namespace neg;

C3DAnimatedSprite::C3DAnimatedSprite()						// 0x41f910
{
	SetName((char *)"C3DANIMATEDSPRITE");
	Log("C3DAnimatedSprite()");
	C3DAnimatedSprite::InitObject();						// 0x41fbb0 (called directly)
	SetObjectID('3ANI');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 36;
	m_sprite_size = 100;
	v42(0);
	v40(0);
	v52(0);
	v107(1);
	v46(0);
	CGameObject::v232();									// 0x4adea0 (called directly)
	f_7bc = 1.0f;											// Alpha
	f_7b0 = 1.0f;											// Red
	f_7b4 = 1.0f;											// Green
	f_7b8 = 1.0f;											// Blue
	f_7c0 = 1;												// Activated
	f_7c4 = 0;
	f_7c8 = 0;
	f_7cc = -1;												// OnSoundIndex
	f_7d0 = -1;												// OffSoundIndex
	f_7d4 = 0;
	f_7d8 = 10.0f;											// FPS
	f_7e0[0] = 60;
	f_7e0[1] = 61;
	f_7e0[2] = 62;
	for (int i = 3; i < 9; i++) f_7e0[i] = -1;
	f_7dc = 9;
	m_sprite_index = 60;
	f_808 = 1;												// Loop
	m_db = GetDatabase("sprites.omt");						// 0x4a4180
	f_80c = 1;
	f_804 = 0;
	f_810 = -1;												// InitallyVisible
	// (then a call to the empty 0x463890)
}

C3DAnimatedSprite::~C3DAnimatedSprite() {}					// 0x41fb70

namespace neg {
CGameObject *New_C3DAnimatedSprite(void) { return new C3DAnimatedSprite; }	// '3ANI' (0x81c bytes)
}

void C3DAnimatedSprite::InitObject(void)					// 0x41fbb0
{
	Log("InitObject()");
	CPickupType::InitObject();								// 0x496d60
	RegisterProperty("Alpha", &f_7bc, PROP_FLOAT, 0);
	RegisterProperty("Red", &f_7b0, PROP_FLOAT, 0);
	RegisterProperty("Green", &f_7b4, PROP_FLOAT, 0);
	RegisterProperty("Blue", &f_7b8, PROP_FLOAT, 0);
	RegisterProperty("Activated", &f_7c0, PROP_LONG, 0);
	RegisterProperty("OnSoundIndex", &f_7cc, PROP_LONG, 0);
	RegisterProperty("OffSoundIndex", &f_7d0, PROP_LONG, 0);
	RegisterProperty("FPS", &f_7d8, PROP_FLOAT, 0);
	RegisterProperty("Loop", &f_808, PROP_LONG, 0);
	static const char *const names[9] = { "Sprite1", "Sprite2", "Sprite3", "Sprite4", "Sprite5",
										  "Sprite6", "Sprite7", "Sprite8", "Sprite9" };
	for (int i = 0; i < 9; i++) RegisterProperty(names[i], &f_7e0[i], PROP_LONG, 0);
	RegisterProperty("InitallyVisible", &f_810, PROP_LONG, 0);	// (sic)
	v67(10.0f);												// +0x10c: radius
}

void C3DAnimatedSprite::Process(float dt)					// 0x41fdd0
{
	C3DSpriteType::Process(dt);								// 0x473700
	if (g_gametype->IsPaused()) return;						// +0x114
	f_7c4 += dt;
	f_7d4 += dt;
	if (!f_80c || f_7c0 == 0) return;
	if (f_7c4 <= 1.0f / f_7d8) return;
	f_7c4 = 0;
	if (f_804 >= 0 && f_804 < 9)							// (a 9-case switch)
		InitSpriteID(m_db, f_7e0[f_804], (float)m_sprite_size);	// element +200
	f_804++;
	if (f_7dc <= f_804)
	{
		f_804 = 0;
		if (f_808 != 0)
		{
			if (f_808 != 2) return;							// Loop 1: again
			HideAgain();									// element +0xd8
		}
		f_80c = 0;
	}
}

void C3DAnimatedSprite::v260(void)							// 0x420020
{
	CPickupType::v260();									// 0x496de0
	f_7c4 = 0;
	for (int i = 8; i >= 0; i--)							// the first -1 ends the list
		if (f_7e0[i] == -1) f_7dc = i;
	m_sprite_index = f_7e0[0];
	if (f_7c0 == 0) HideAgain();							// element +0xd8
	else
	{
		set_diffuse(OMediaFARGBColor(f_7bc, f_7b0, f_7b4, f_7b8));	// +0x90..+0x9c
		ShowAgain();										// element +0xd4
		f_80c = 1;
	}
	if (f_810 == 1)
	{
		set_diffuse(OMediaFARGBColor(f_7bc, f_7b0, f_7b4, f_7b8));
		ShowAgain();
	}
	else if (f_810 == 0) HideAgain();
	v52(0);													// +0xd0
}

void C3DAnimatedSprite::v268(int state)						// 0x420200
{
	if (state == -1)
	{
		if (f_7d4 < 1.0f) return;
		f_7d4 = 0;
		if (f_7c0 == 0) state = 1;
		else state = 0;
	}
	if (state == 0)
	{
		if (f_7d0 != -1) PlayGameSoundIndex(-1, f_7d0, false);	// 0x48f030
		f_7c0 = 0;
		HideAgain();
		return;
	}
	if (state != 1) return;
	f_804 = 0;
	if (f_7cc != -1) PlayGameSoundIndex(-1, f_7cc, false);
	f_7c0 = 1;
	f_7c4 = 0;
	f_80c = 1;
	ShowAgain();
}
