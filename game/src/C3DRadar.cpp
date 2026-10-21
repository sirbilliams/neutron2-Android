/*  C3DRadar - see C3DRadar.h.
    Constants: 0x4ca4b4 = 30 (needle spin, degrees / s), 0x4cb1f8 = 360 / 16384 (angle units to
    degrees), 0x4cb1f4 = 180; blip scale per range 0x4f3ee4 .. 0x4f3ed4.                       */
#include "C3DRadar.h"
#include "C3DJimmy.h"
#include "C3DRocketShip.h"				// neg::GoddardMarker
#include "C2DInGameMenu.h"
#include "CanvasWrapper.h"
#include "CGameType.h"
#include "InterfaceElement.h"
#include "Game.h"
#include "gen/globals.h"
#include "port/Widescreen.h"
#include "OMediaViewPort.h"
#include "OMediaTrigo.h"
#include "OMedia3DVector.h"
#include <string.h>

using namespace neg;

namespace neg {
InterfaceElt g_radar_blips[65];			// 0x5461d0 (static initialiser 0x468130)
}

static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// --------------------------------------------------------------------------- helpers

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'L1_A').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// 0x4b1ee0: a camera-relative direction (viewport angles), w = 0 (as in CEditor.cpp)
static float *CameraDir(float out[4], float x, float y, float z)
{
	const float sx = omd_Sin(g_viewport->get_anglex()), cx = omd_Cos(g_viewport->get_anglex());
	const float sy = omd_Sin(g_viewport->get_angley()), cy = omd_Cos(g_viewport->get_angley());
	const float sz = omd_Sin(g_viewport->get_anglez()), cz = omd_Cos(g_viewport->get_anglez());
	const float t = cz * y - sz * x;
	const float u = sz * y + cz * x;
	const float v = cx * z - t * sx;
	out[3] = 0.0f;
	out[0] = u * cy - v * sy;
	out[1] = sx * z + t * cx;
	out[2] = u * sy + v * cy;
	return out;
}

// 0x4b1de0: a point in front of the camera (viewport position + camera-relative offset), w = 1
static float *CameraPoint(float out[4], float x, float y, float z)
{
	float d[4];
	CameraDir(d, x, y, z);
	out[3] = 1.0f;
	out[0] = g_viewport->getx() + d[0];
	out[2] = g_viewport->getz() + d[2];
	out[1] = d[1] + g_viewport->gety();
	return out;
}

// 0x40b2a0: show / hide the interface element g_elt_545858.
static void ShowElt545858(bool show)
{
	if (show) g_elt_545858.Show();
	else g_elt_545858.Hide();
}

// --------------------------------------------------------------------------- construction

C3DRadar::C3DRadar()										// 0x468180
{
	SetName((char *)"C3DRADAR");
	Log("C3DRadar()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	C3DSprite::SetObjectID('3RAD');							// 0x49b550
	strcpy(m_sprite_db, "retainedsprites.omt");
	m_sprite_index = 27;
	m_sprite_size = 13;
	v46(0);
	C3DSprite::InitSpriteProperties(70.0f);					// 0x49ce10
	f_608 = 11000;
	f_614 = 99;
	f_600 = new C3DPermanentSprite;							// 0x608 bytes
	strcpy(f_600->m_sprite_db, "retainedsprites.omt");
	f_600->m_sprite_index = 30;
	f_600->m_sprite_size = 2;
	f_600->InitSpriteProperties(1.0f);						// element +0xd0
	f_600->v46(0);											// +0xb8
	f_600->v107(0);											// +0x1ac
	f_600->v36(0);											// +0x90
	v67(15.0f);
	v107(0);
	v36(0);
	v52(0);
	f_604 = 0;
	f_618 = 7.0f;
	f_61c = 0;
	C3DSprite::HideAgain();									// 0x49d080
	f_620 = NULL;
	f_624 = 0;
	f_628 = NULL;
	f_62c = 0;
	f_630 = 0;
	f_634 = 0;
	// the blips: picture 462 of the screens database, alpha blended, at (0, 0)
	for (int i = 0; i < 65; i++)
	{
		InterfaceElt &b = g_radar_blips[i];
		b.db = g_screens_db;
		b.canvas_id = 0x1ce;
		b.x = 0;
		b.y = 0;
		b.src_blend = omblendfc_Src_Alpha;					// 6
		b.dest_blend = omblendfc_Inv_Src_Alpha;				// 7
		b.home_x = 0;
		b.home_y = 0;
	}
	// (then a call to the empty 0x463890)
}

C3DRadar::~C3DRadar()										// 0x468480
{
	// (f_600 is not deleted)
}

namespace neg {
CGameObject *New_C3DRadar(void)	{ return new C3DRadar; }					// factory '3RAD' (0x640 bytes)
}

// --------------------------------------------------------------------------- slots

void C3DRadar::Process(float dt)							// 0x4684c0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	C3DJimmy *j = Jimmy();									// (g_545fd0 - 0xc0)
	if (f_61c)
	{
		if (!j->f_d5c) return;
		CGameObject *m = GoddardMarker(static_cast<CGameObject *>(j->f_d5c));	// Goddard +0x1b80
		j->ev149(dt, f_628, m ? dynamic_cast<C3DObject *>(m) : NULL);			// element +0x254
		return;
	}
	j->ev161(0, 1);											// element +0x284
	j->f_d48 = 0;
}

void C3DRadar::v260(void)									// 0x4686e0
{
	C3DPermanentSprite::v260();								// 0x45d7b0
	InitPhysics();											// vtable +0x2c
	v67(15.0f);												// +0x10c
	f_628 = NULL;
	if (!g_windowed) set_diffuse(OMediaFARGBColor(0.8f, 0.1f, 0.7f, 0.1f));	// +0x90.. (r, g, b, a)
	if (f_600) f_600->HideAgain();							// element +0xd8
	HideAgain();											// element +0xd8
	set_flags((get_flags() & ~(omelf_DisableRotate | omelf_FaceViewport)) | omelf_SecondPass);	// +0x7c &= ~0x240, |= 8
}

void C3DRadar::v269(float dt)								// 0x4685b0
{
	if (g_game_mode == 5 || g_game_mode == 6)				// playing / paused
	{
		if (LevelID() == 'LV5F')
		{
			ShowElt545858(true);							// 0x40b2a0
			int n = 0;
			for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
			{
				CLocalGameObject *o = static_cast<CLocalGameObject *>(*i);
				if (o && o->m_show_on_radar == 1 && o->v123() && o->f_140) n++;	// +0x4c0, +0x1ec, +0x140
			}
			DrawText(320, 5, 'MED', "%d", n);				// 0x4a1a80 (0x532a04)
		}
		else ShowElt545858(false);
		if (f_61c)
		{
			Place();										// 0x468790
			if (g_gametype->IsPaused()) return;				// +0x114
			AddRotation(0, 0, dt * 30.0f);					// +0x330; 0x4ca4b4
			return;
		}
	}
	else ShowElt545858(false);
	for (int i = 0; i < 65; i++) Blip(i, 1000, 0, false, 0, 1.0f);
	f_600->HideAgain();										// element +0xd8
}

// --------------------------------------------------------------------------- methods

void C3DRadar::Open(CGameObject *centre, int unused, int x, int y, int z, CGameObject *target)	// 0x4689d0
{
	(void)unused;
	if (f_61c) return;
	f_628 = target;
	f_620 = centre;
	f_62c = x;
	f_61c = 1;
	f_630 = y;
	f_634 = z;
	f_600->ShowAgain();										// element +0xd4
	g_menu->f_5b0->force_show();							// (inline: hide count 1, hide(false))
}

void C3DRadar::Show(void)									// 0x468a50
{
	if (!g_545f0e) return;
	f_61c = 1;
	g_menu->f_5b0->force_show();							// (inline)
	f_600->ShowAgain();										// element +0xd4
	g_545f0e = 0;
}

void C3DRadar::Hide(void)									// 0x468aa0
{
	if (!f_61c) return;
	f_61c = 0;
	g_menu->f_5b0->hide(true);								// +0x58
	f_600->HideAgain();										// element +0xd8
	for (int i = 0; i < 65; i++) Blip(i, 1000, 0, false, 0, 1.0f);
}

void C3DRadar::Place(void)									// 0x468790
{
	if (!f_620 || !f_61c) return;
	f_600->set_flags(get_flags() | omelf_SecondPass);		// +0x7c: our flags | 8
	const int l = LevelID();
	// (the original's compare tree: 'LV5f' with a small f and anything else get 1)
	if ((l >= 'L1_A' && l <= 'L1_C') || l == 'VR01') f_618 = 2.0f;
	else if (l == 'LV4B') f_618 = 6.0f;
	else f_618 = 1.0f;

	float pos[4];
	// port: widescreen - the radar's camera-space x scaled so it moves out to the right edge with
	// its disc (f_5b0) and blips (centred at x 550 of the 640 space: 230 right of the centre).
	const float wide_k = (230.0f + WideMargin()) / 230.0f;
	CameraPoint(pos, (float)f_62c * wide_k, (float)f_630, (float)f_634);	// 0x4b1de0
	v197(pos[0], pos[1], pos[2]);							// +0x314
	const short ax = g_viewport->get_anglex(), ay = g_viewport->get_angley();	// 2002 +0x50 / +0x52
	float r[4];
	v201(r);												// +0x324
	v203((float)ax * kAngleToDeg, (float)ay * kAngleToDeg, r[2]);	// +0x32c
	f_600->v197(pos[0], pos[1], pos[2]);
	const short ax2 = g_viewport->get_anglex(), ay2 = g_viewport->get_angley();
	f_620->v201(r);
	f_600->v203((float)ax2 * kAngleToDeg, (float)ay2 * kAngleToDeg, -(r[1] + 180.0f));	// 0x4cb1f4
	f_60c = 550;
	f_610 = 73;
	UpdateBlips(f_620, f_624, pos);							// 0x468b00 (pos by value)
}

// Returns a float in the original (the last x distance; no caller uses it).
void C3DRadar::UpdateBlips(CGameObject *centre, int unused, float pos[4])	// 0x468b00
{
	(void)unused;
	(void)pos;												// (not used)
	int range = 5;
	f_604 = 0;
	if (g_game_mode != 5 && g_game_mode != 6) return;
	int i = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); it++)
	{
		CLocalGameObject *o = static_cast<CLocalGameObject *>(*it);
		if (!o) continue;
		const int kind = o->m_show_on_radar;				// +0x4c0
		if (kind != 1 && kind != 2 && kind != 3) continue;
		Blip(i, 1000, 1000, false, 0, 1.0f);
		if (o->v123() && o->f_140)							// +0x1ec, +0x140
		{
			float a[4], b[4];
			o->v195(a);										// +0x30c
			centre->v195(b);
			OMedia3DVector d(a[0] - b[0], 0, a[2] - b[2], 0);
			const float dist = d.quick_magnitude();
			if (o->m_show_on_radar == 1)
			{
				const int u = f_608;
				const float sd = dist * f_618;
				// (sic: range starts at 5, so the "== 99" test never passes)
				if (sd < (float)(u * 5) && range == 99) range = 5;
				if (sd < (float)(u * 4) && range >= 5) range = 4;
				if (sd < (float)(u * 3) && range >= 4) range = 3;
				if (sd < (float)(u * 2) && range >= 3) range = 2;
				if (sd < (float)u && range >= 2) range = 1;
				f_604++;
			}
			if (dist * f_618 < (float)(f_608 * f_614))
			{
				float k;
				int size;
				switch (f_614)
				{
					case 1:  k = 0.0055f;  size = 8; break;	// 0x4f3ee4
					case 2:  k = 0.0025f;  size = 6; break;	// 0x4f3ee0
					case 3:  k = 0.0015f;  size = 4; break;	// 0x4f3edc
					case 4:  k = 0.00115f; size = 3; break;	// 0x4f3ed8
					default: k = 0.00085f; size = 3; break;	// 0x4f3ed4
				}
				const int y = (int)(d.z * f_618 * k + (float)f_610);	// _ftol
				const int x = (int)((float)f_60c - d.x * f_618 * k);	// _ftol
				Blip(i, x, y, true, o->m_show_on_radar, (float)size);
			}
		}
		i++;
	}
	switch (range)
	{
		case 1: f_614 = 1; break;
		case 2: f_614 = 2; break;
		case 3: f_614 = 3; break;
		case 4: f_614 = 4; break;
		default: f_614 = 5; break;
	}
	DrawText(600, 10, 'MED', "%dX", f_614);					// 0x4a1a80 (0x539ec0)
	f_600->ShowAgain();										// element +0xd4
}

// Blip i (< 65) at (x, y) in the 640x480 HUD space, coloured by kind (1 red, 2 green, 3 blue),
// size x size; hidden (moved off screen to (-20, -20)) when !show.
void C3DRadar::Blip(int i, int x, int y, bool show, int kind, float size)	// 0x468e70
{
	if (i >= 65) return;
	InterfaceElt *b = &g_radar_blips[i];
	if (!b->canvas_elt) b->Show();							// 0x482520
	b->SetPos((float)x, (float)y);							// 0x4823f0
	if (!show)
	{
		b->SetPos(-20.0f, -20.0f);
		return;
	}
	float r, g, bl;
	if (kind == 1)		{ r = 1.0f; g = 0.2f; bl = 0.2f; }
	else if (kind == 2)	{ r = 0.2f; g = 1.0f; bl = 0.2f; }
	else if (kind == 3)	{ r = 0.2f; g = 0.2f; bl = 1.0f; }
	else return;
	if (!b->canvas_elt) return;
	b->canvas_elt->set_diffuse(OMediaFARGBColor(1.0f, r, g, bl));	// 2002 +0x90.. (r, g, b, a = 1)
	b->Scale(size, size);									// 0x482390
}
