/*  C3DTriggerType - see C3DTriggerType.h. */
#include "DatabaseCache.h"
#include "C3DTriggerType.h"
#include "CJimmyGame.h"
#include "C3DJimmy.h"
#include "objects/C3DAITrigger.h"
#include "objects/C3DCutSceneCamera.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "objects/C3DMultiCutSceneCamera.h"
#pragma GCC diagnostic pop
#include "levels/CLoadLevel.h"
#include "C3DPlayer.h"
#include "C3DJimmy.h"
#include "CJimmyGame.h"
#include "CGameType.h"
#include "CEditor.h"
#include "C2DInGameMenu.h"
#include "InterfaceElement.h"
#include "Game.h"
#include "Todo.h"
#include "gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaStringField.h"
#include "OMediaTrigo.h"
#include "level1/C3DJeep.h"
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x4a4180: the level's database cache (DatabaseCache.cpp).
using neg::GetDatabase;

// --------------------------------------------------------------------------- not reconstructed yet

// C2DInGameMenu 0x40c0f0 (on g_menu): cut-scene letterbox in/out.
using neg::CutsceneLetterbox;

// CGameObject slot 224 (+0x380, 0x4ad800): a point relative to the object (struct return).
static float *ObjectRelativePoint(CGameObject *o, float *out, float a, float b, float c)
{
	if (!o) return NULL;								// port: the original calls through NULL
	const OMedia3DVector v = o->v224(a, b, c);
	out[0] = v.x; out[1] = v.y; out[2] = v.z; out[3] = v.w;
	return out;
}

// The next trigger's own entry points. Each is a virtual its class introduces (the element
// vtable of the next object); they are called once its type is known from NameContains.
namespace neg { void TextTriggerTrigger(CGameObject *o); }
static void TextTriggerFire(CGameObject *next)		{ neg::TextTriggerTrigger(next); }	// objects/C3DTextTrigger.cpp
namespace neg { void MusicTriggerTrigger(CGameObject *o); }
static void MusicTriggerFire(CGameObject *next)		{ neg::MusicTriggerTrigger(next); }	// objects/C3DMusicTrigger.cpp
namespace neg { void FadeObjectTrigger(CGameObject *o); }
static void FadeObjectFire(CGameObject *next)		{ neg::FadeObjectTrigger(next); }	// objects/C3DFadeObject.cpp
static void AITriggerFire(CGameObject *next, CGameObject *caller, unsigned char flag)
{
	static_cast<C3DAITrigger *>(next)->Trigger(caller, flag);			// element +0xe4
}
static void MultiCutSceneFire(CGameObject *next, CGameObject *caller, unsigned char flag)
{
	static_cast<C3DMultiCutSceneCamera *>(next)->TriggerCallback(caller, flag);	// element +0xe4 (0x457be0)
}
static void CutSceneCameraFire(CGameObject *next, unsigned char flag)
{
	static_cast<C3DCutSceneCamera *>(next)->TriggerCamera(flag);		// element +0xe0
}
// C3DFlyingCamera is a C3DObject (CGameObject at +0xc0): element slot 144 (+0x240).
namespace neg { void FlyingCameraTrigger(CGameObject *o); }
static void FlyingCameraFire(CGameObject *next)	{ neg::FlyingCameraTrigger(next); }	// objects/C3DFlyingCamera.cpp
// CLoadLevel (a C3DSprite): element slot 55 (+0xdc).
static void LoadLevelFire(CGameObject *next)		{ static_cast<CLoadLevel *>(next)->ev55(); }

// The player (Jimmy: a C3DPlayer) behind a JIM1 tag.
static C3DPlayer *PlayerOf(CGameObject *jim)		{ return jim ? dynamic_cast<C3DPlayer *>(jim) : NULL; }

// 0x4795cf: element byte +0xa99 = 0.
static void JimmyClearA99(CGameObject *jim)
{
	if (C3DPlayer *p = PlayerOf(jim)) p->f_a99 = 0;
}

// 0x406a40: start the in-game menu's fade (mode, seconds). (A static copy, as in CEditor.cpp.)
static void StartFade(int mode, float t)
{
	if (g_5324fc == 0) { g_5324ec = 1; return; }
	if (mode == 6) mode = 4;
	if (t < 0.1f) t = 0.1f;				// 0x4caa68
	g_5324e8 = t;
	g_545dcc = t;
	g_545dd0 = mode;
	g_5324ec = 0;
	g_545e5c = 1;
}

// 0x479610..0x47973d: the LoadLevel trigger hands its level over to Jimmy: his promotion, then
// PlayerLoadLevel (element slot 121) with copies of the four strings; with a FadeType the load
// waits for the fade (and Jimmy's morph animation is paused, element slot 79).
static void LoadLevelHandOver(CGameObject *loadlevel, CGameObject *jim)
{
	CLoadLevel *ll = static_cast<CLoadLevel *>(loadlevel);
	C3DPlayer *p = PlayerOf(jim);
	if (!p) return;										// port: a JIM1 that is no player (the original has no check)
	p->f_c18 = ll->m_promotion;
	char level[80], start[80], ret_level[100], ret_start[100];	// (stack copies, as in the original)
	strcpy(level, ll->m_level_name);
	strcpy(start, ll->m_start_point);
	strcpy(ret_level, p->f_c25);
	strcpy(ret_start, p->f_b24);
	p->ev121(level, start, ret_level, ret_start, ll->m_load_screen);	// element +0x1e4
	if (ll->m_fade_type != -1)
	{
		g_editor->m_load_pending = 0;					// the fade loads the level when it ends
		g_view_restore = 1;								// 0x54a498
		p->ev79(1);										// element +0x13c
		StartFade(ll->m_fade_type, ll->m_fade_time);	// 0x406a40
	}
}

// C3DJeep +0x9c4 (CGameObject-relative): the jeep can take the camera when it has a driver.
static int JeepField9c4(CGameObject *jeep)
{
	C3DJeep *j = jeep ? dynamic_cast<C3DJeep *>(jeep) : NULL;
	return j && j->f_a84;										// the driver (object 0xa84)
}

// CJimmyGame element slot 102 (+0x198): PlayLevelMusic(jim element + 0x4c28, *(jim element + 0x4c18)).
// (The original reads those even when there is no JIM1: a crash.)
static void JimmyGameRestore(CGameObject *jim)
{
	C3DJimmy *j = jim ? dynamic_cast<C3DJimmy *>(jim) : NULL;
	CJimmyGame *g = g_gametype ? dynamic_cast<CJimmyGame *>(g_gametype) : NULL;
	if (j && g) g->PlayLevelMusic(j->f_4c28, j->f_4c18);	// port: no call without them
}

// --------------------------------------------------------------------------- construction

C3DTriggerType::C3DTriggerType()							// 0x478c40
{
	SetName((char *)"C3DTRIGGERTYPE");
	m_toggle = -1;
	strcpy(m_toggle_object, "none");
	strcpy(m_next_trigger, "none");
	g_cutscene_skipped = 0;
	m_fade_type = -1;
	m_fade_time = 1.0f;
	strcpy(m_default_to, "none");
}

C3DTriggerType::~C3DTriggerType() {}						// 0x478dd0

void C3DTriggerType::InitObject(void)						// 0x479000
{
	C3DSprite::InitObject();
	RegisterProperty("Toggle",       &m_toggle,       PROP_LONG,  0);
	RegisterProperty("ToggleObject", m_toggle_object, PROP_ALPHA, 0);
	RegisterProperty("NextTrigger",  m_next_trigger,  PROP_ALPHA, 0);
	RegisterProperty("FadeType",     &m_fade_type,    PROP_LONG,  0);
	RegisterProperty("FadeTime",     &m_fade_time,    PROP_FLOAT, 0);
	RegisterProperty("DefaultTo",    m_default_to,    PROP_ALPHA, 0);
}

// --------------------------------------------------------------------------- v242

// 0x478e10: in the editor, holding Page Down while this trigger is selected puts the camera
// next to the NextTrigger object: an offset (20, 20, -100) turned by the camera's own angles
// (taken to degrees and back, as shipped).
void C3DTriggerType::v242(float t)
{
	C3DSpriteType::v242(t);
	if (!g_keys[0x22] || g_editor->m_sel_object != static_cast<CGameObject *>(this)) return;	// VK_NEXT (0x56adc2)
	if (strcasecmp(m_next_trigger, "none") == 0) return;
	CGameObject *next = FindObjectByTag(m_next_trigger);
	if (!next) return;

	const double to_deg = 360.0f / 16384.0f;				// 0x4cb1f8 (x87: no rounding in between)
	const double to_angle = 16384.0f / 360.0f;				// 0x4cb228
	const int ix = (int)((double)g_viewport->get_anglex() * to_deg * to_angle) & 0x3fff;
	const int iy = (int)((double)g_viewport->get_angley() * to_deg * to_angle) & 0x3fff;
	const int iz = (int)((double)g_viewport->get_anglez() * to_deg * to_angle) & 0x3fff;
	const float *sn = OMediaCosSinTable::global.get_sintab();	// OMediaCosSinTable +0
	const float *cs = OMediaCosSinTable::global.get_costab();	// OMediaCosSinTable +0x10004
	const float sx = sn[ix], cx = cs[ix], sy = sn[iy], cy = cs[iy], sz = sn[iz], cz = cs[iz];

	const float a = 20.0f * cz - -20.0f * sz;				// 0x4caa40 / 0x4d65cc
	const float b = 20.0f * sz - 20.0f * cz;
	const float p = -100.0f * cx - a * sx;					// 0x4df748
	const float ox = b * cy - p * sy;
	const float oz = p * cy + b * sy;
	const float oy = a * cx - 100.0f * sx;					// 0x4d2cb0

	float pos[4];
	next->v195(pos); g_viewport->setx(ox + pos[0]);			// CGameObject +0x30c, once per axis
	next->v195(pos); g_viewport->sety(oy + pos[1]);
	next->v195(pos); g_viewport->setz(oz + pos[2]);
}

// --------------------------------------------------------------------------- CallNextTrigger

// 0x4790b0: fire the trigger called `name`: "none" does nothing, "DEFAULT" ends the scripted
// sequence (camera back to Jimmy or the vehicle he is in, or to DefaultTo), anything else is
// an object tag whose class decides what firing means.
void C3DTriggerType::CallNextTrigger(const char *name, unsigned char flag)
{
	if (strcasecmp(name, "none") == 0)
	{
		// (debug log "NextTrigger is none for %s", m_tag)
		return;
	}

	if (strcasecmp(name, "DEFAULT") != 0)
	{
		CGameObject *next = FindObjectByTag(name);
		if (!next)
		{
			ErrorLog("ERROR: No NextTrigger %s found", name);
			return;
		}
		// (each case also writes a debug log "Calling <type> %s, NextTrigger=%s")
		if (next->NameContains((char *)"C3DTEXTTRIGGER"))			{ TextTriggerFire(next); return; }
		if (next->NameContains((char *)"C3DMUSICTRIGGER"))		{ MusicTriggerFire(next); return; }
		if (next->NameContains((char *)"C3DAITRIGGER"))			{ AITriggerFire(next, this, flag); return; }
		if (next->NameContains((char *)"C3DCUTSCENECAMERA"))		{ CutSceneCameraFire(next, flag); return; }
		if (next->NameContains((char *)"C3DMULTICUTSCENE"))		{ MultiCutSceneFire(next, this, flag); return; }
		if (next->NameContains((char *)"C3DFLYINGCAMERA"))		{ FlyingCameraFire(next); return; }
		if (next->NameContains((char *)"C3DFADEOBJECT"))			{ FadeObjectFire(next); return; }
		if (next->NameContains((char *)"C3DLOADLEVEL"))
		{
			g_elt_545c80.Hide();
			CGameObject *jim = FindObjectByTag("JIM1");
			if (!jim)
			{
				ErrorLog("ERROR: Unable to find JIM1 in level");
				return;
			}
			// (debug log "Loading LoadLevel trigger %s for %s", name, m_tag)
			JimmyClearA99(jim);
			g_editor->f_7f8 = 1;
			g_gametype->SetCamera(jim);
			LoadLevelFire(next);
			LoadLevelHandOver(next, jim);					// (debug logs "Callnexttrig 1" / "2" around)
			g_active_sound_db = GetDatabase("SoundEffects.omt");
			if (g_active_sound_db) g_54a4b8 = true;
			return;
		}
		ErrorLog("ERROR: Invalid type for NextTrigger %s", name);
		return;
	}

	// "DEFAULT"
	// (debug logs "Reseting to DEFAULT in C3DTriggerType", "\tReseting to DEFAULT (Good job ;)")
	CutsceneLetterbox(false);
	g_545ef8 = 0;
	CGameObject *jim = FindObjectByTag("JIM1");
	g_music_volume = g_6f7518;
	if (g_cutscene_element)
	{
		g_elt_545c80.Hide();
		if (g_cutscene_element) delete g_cutscene_element;	// listener vtable +8: deleting destructor
		g_cutscene_element = NULL;
	}
	if (g_camera == jim)
	{
		g_active_sound_db = GetDatabase("SoundEffects.omt");
		if (g_active_sound_db) g_54a4b8 = true;
		CLocalGameObject *l = static_cast<CLocalGameObject *>(jim);	// sic: NULL without a JIM1
		if (l->f_4ab) l->f_4ab = 0;
		g_cutscene_skipped = 0;
		return;
	}

	CGameObject *o = jim;
	if (jim)
	{
		if (g_menu->IsCurrentGadget(4))
		{
			o = FindObjectByTag("C3DJEEP");
			if (o)
			{
				if (JeepField9c4(o)) g_gametype->SetCamera(o);
				else o = FindObjectByTag("JIM1");
			}
		}
		else if (g_menu->IsCurrentGadget(1))
		{
			o = FindObjectByTag("C3DROCKETSHIP");
			if (o) g_gametype->SetCamera(o);
		}
		if (strcasecmp(m_default_to, "none") != 0) o = FindObjectByTag(m_default_to);
		g_gametype->SetCamera(o);

		float buf[4];
		const float *p = ObjectRelativePoint(o, buf, 0.0f, 500.0f, -500.0f);	// +0x380
		if (p)
		{
			const float dx = g_viewport->getx() - p[0];
			const float dy = g_viewport->gety() - p[1];
			const float dz = g_viewport->getz() - p[2];
			if (sqrtf(dx * dx + dz * dz + dy * dy) > 1000.0f)	// 0x4cb1d8
				g_viewport->place(p[0], p[1], p[2]);
		}
		o = FindObjectByTag("JIM1");
		g_music_volume = g_6f7518;
		JimmyGameRestore(o);
	}
	g_music_volume = g_6f7518;
	g_active_sound_db = GetDatabase("SoundEffects.omt");
	if (g_active_sound_db) g_54a4b8 = true;
	CLocalGameObject *l = static_cast<CLocalGameObject *>(o);	// sic: NULL without a JIM1
	if (l->f_4ab) l->f_4ab = 0;
	if (g_cutscene_skipped)
	{
		g_545e57 = 1;
		g_cutscene_skipped = 0;
		return;
	}
	g_545e57 = 0;
	g_545ef8 = 0;
	g_cutscene_skipped = 0;
}

// --------------------------------------------------------------------------- v280

// 0x479800: the editor's list shows this trigger and the chain of NextTriggers after it (at most
// 100 names; a chain of more than 200 links is reported as a loop).
void C3DTriggerType::v280(void)
{
	if (!g_editor) return;
	CGameObject *o = this;
	int shown = 0;
	for (int n = 1; ; n++)
	{
		if (n > 200)
		{
			DrawText(300, 200, 'SMAL', "WARNING: ENDLESS LOOP FOR %s", m_tag);	// 0x4a1a80 (a copy of DrawText)
			return;
		}
		if (shown < 100)									// g_editor +0x28c .. +0x41c
		{
			OMediaStringField *f = g_editor->m_list_fields[shown++];
			f->set_string(std::string(o->m_tag));			// element +0xc8
			f->force_show();
		}
		if (!o->NameContains((char *)"C3DTRIGGERTYPE")) return;
		C3DTriggerType *t = static_cast<C3DTriggerType *>(o);
		if (strcasecmp(t->m_next_trigger, "none") == 0) return;
		o = FindObjectByTag(t->m_next_trigger);
		if (!o) return;
	}
}
