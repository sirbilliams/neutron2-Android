/*  CJimmyGame - see CJimmyGame.h. Reconstructed from Neutron2.exe 0x4827f0..0x486570, with the
    texture-scroll helpers 0x497090 / 0x4970e0 / 0x4972b0 that only it uses.

    The original reaches C3DJimmy's members (music state +0x4c14.., surface flags +0x4ca6..) and
    many C3DJimmy / C3DPlayerObject slots; that class is not reconstructed yet, so those accesses
    are the small TODO functions below, named after what the original does with them.        */

#include "SaveGame.h"
#include "Physics.h"
#include "CJimmyGame.h"
#include "C3DObject.h"
#include "C3DJimmy.h"
#include "C3DRadar.h"
#include "C2DInGameMenu.h"
#include "CanvasWrapper.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "Sound.h"
#include "DatabaseCache.h"
#include "gen/globals.h"
#include "OMediaLayer.h"
#include "OMediaDataBase.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DShapeElement.h"
#include "OMediaCanvas.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaStreamOperators.h"
#include "OMediaError.h"
#include <math.h>
#include <string.h>
#include <strings.h>

namespace neg {
std::string					g_queued_sfx;
}
using namespace neg;

// --------------------------------------------------------------------------- not reconstructed yet

// Sound: Sound.h (PlayGameSound 0x48f420, StopGameSound 0x48f0d0, StopChannel 0x4b88e0,
// StopAllChannels 0x4b8990, UpdateSounds 0x4b8a30, PlayMusic 0x4b8850)
static OMediaDataBase *LoadMusic(const char *omt_name)	{ return GetDatabase(omt_name); }	// 0x4a4180
// 0x4a4730: close the cached database opened from `name` (object database cache 0x56aef4)
static void CloseDatabaseByName(const char *name)	{ neg::CloseDatabase(name); }	// DatabaseCache.cpp

// Tasks (CTaskList, 0x54e69c): SaveGame.cpp
static void SetTaskValue(const char *task, int v)	{ neg::SetTaskState(task, v); }		// 0x497980
static int  GetTaskValue(const char *task)		{ return neg::GetTaskState(task); }		// 0x497a40
static void LoadTask(const char *name, int a)	{ neg::LoadTask(name, a != 0); }		// 0x498af0

// Granny (RAD): granny.dll import 0x4ca464 GrannyAdvanceTime, game/granny/granny_compat.h.
#include "granny_compat.h"

// C2DInGameMenu 0x409b20: hand out the pickup announced by 0x545e64 (ANTIHOLO, inventory...).
static void MenuGivePendingPickup(void)			{ NEG_TODO(0x409b20, "C2DInGameMenu pending pickup (0x545e64)"); }

// The player's HUD object (C3DJimmy +0xd40, minus 200 to its element; NULL if none) and its
// hide (C3DRadar 0x468aa0).
static OMediaElement *PlayerHud(void)			{ C3DRadar *r = Jimmy()->f_d40; return r ? static_cast<OMediaElement *>(r) : NULL; }	// 0x483b9d
static void HudHide(OMediaElement *hud)			{ static_cast<C3DRadar *>(hud)->Hide(); }

// g_camera's element slot 105 (+0x1a4): called once when the health has run out.
static void CameraPlayerDead(CGameObject *camera) { (void)camera; NEG_TODO(0x4843d9, "camera element slot 105 (health ran out)"); }

// C3DJimmy's level-music members: +0x4c14 channel, +0x4c18 a value given to PlayMusic, +0x4c28
// the music .omt name. (port: without a C3DJimmy yet, a stand-in with its constructor's values.)
struct PlayerMusicState { int &channel; int &param; char *name; };
static PlayerMusicState PlayerMusic(void)
{
	if (C3DJimmy *j = Jimmy()) return PlayerMusicState{ j->f_4c14, j->f_4c18, j->f_4c28 };	// (g_player - 0xc0)
	static int channel = -1, param = 0;
	static char name[100] = "none";
	return PlayerMusicState{ channel, param, name };
}

// C3DJimmy members and slots TouchSurface uses (on the player's element).
static bool PlayerFlag(OMediaElement *p, int off)			{ (void)p; (void)off; NEG_TODO(0x4848ba, "C3DJimmy surface flags +0x4ca6 / +0x4ca7 / +0x4cb8"); return false; }
static void SetPlayerFlag(OMediaElement *p, int off, bool v)	{ (void)p; (void)off; (void)v; }
static void SetPlayerFloat(OMediaElement *p, int off, float v)	{ (void)p; (void)off; (void)v; NEG_TODO(0x48491b, "C3DJimmy +0xc08 = 0.5"); }
static void PlayerSlot(OMediaElement *p, int slot)			{ (void)p; (void)slot; NEG_TODO(0x4848ce, "C3DJimmy element slots 120 / 127..131"); }
static void PlayerSlot119(OMediaElement *p, float a, float b, float c, float d)
{
	(void)p; (void)a; (void)b; (void)c; (void)d;
	NEG_TODO(0x484aa2, "C3DJimmy element slot 119");
}
static void PlayerHurt(void *p, float a, int b, float c, bool d)	// slot 134 (+0x218) of the player (or what a '3COR' carries)
{
	(void)p; (void)a; (void)b; (void)c; (void)d;
	NEG_TODO(0x4847eb, "slot 134 (+0x218): hurt");
}
static void SetPlayerSurfaceSound(OMediaElement *p, int set)	{ (void)p; (void)set; NEG_TODO(0x484b32, "C3DJimmy element slot 153 (surface sound)"); }
// A '3COR' object: what it carries (+0x934) and its hit state (+0x1138 / +0x113c / +0x1178 / +0x1188).
static void *CorCarried(CGameObject *o)						{ (void)o; NEG_TODO(0x4847c1, "'3COR' +0x934"); return NULL; }
static void CorStartHit(CGameObject *o)						{ (void)o; NEG_TODO(0x4847f1, "'3COR' hit state"); }

// --------------------------------------------------------------------------- small helpers

// 0x4a38a0: was the key pressed? (clears it)
static bool KeyPressed(int vk)
{
	if (g_keys[vk]) { g_keys[vk] = 0; return true; }
	return false;
}


// C2DInGameMenu 0x40ed30 (this unused): show or hide the sprite g_5446ec (the pause picture).
static void ShowPauseSprite(bool show)
{
	if (show) g_5446ec->force_show();
	else g_5446ec->hide(true);
}

// The random number of the original (0x19660d LCG, 0..1000) as a fraction: < 0.5 picks the first sound.
static bool RandomHalf(void)
{
	*g_random_seed = *g_random_seed * 0x19660d + 0x7fff;
	const int r = (int)(((*g_random_seed >> 16) * 1001) >> 16) & 0xffff;
	return (double)((float)r * 0.001f) < 0.5;			// 0x4cb218, 0x4cb210
}

// 2002 layer flag 0x800 (fog on) is OMT 2.5's fog_enabled. (2.5's enable_fog() always sets it.)
struct LayerFogAccess : OMediaLayer { static bool OMediaLayer::*Enabled(void) { return &LayerFogAccess::fog_enabled; } };
static void SetLayerFog(OMediaLayer *l, bool on)	{ l->*LayerFogAccess::Enabled() = on; }

// A material's diffuse r, g, b; its alpha is kept (the original copies it onto itself).
static void SetDiffuseRGB(OMedia3DMaterial *m, float r, float g, float b)
{
	m->set_diffuse_r(r);
	m->set_diffuse_g(g);
	m->set_diffuse_b(b);
}

// The game's paths are DOS-style and relative.
static std::string PortPath(const char *dos)
{
	std::string s(dos);
	if (s.compare(0, 2, ".\\") == 0) s.erase(0, 2);
	for (size_t i = 0; i < s.size(); i++) if (s[i] == '\\') s[i] = '/';
	return s;
}

static bool FileExists(const char *dos_path)	// OMediaFilePath + OMediaFileStream::fileexists, as the original
{
	try
	{
		OMediaFilePath path(PortPath(dos_path));
		OMediaFileStream fs;
		fs.setpath(&path);
		bool e = fs.fileexists();
		fs.close();
		return e;
	}
	catch (OMediaError &) { return false; }
}

// name[i], lower case ("" past the end, as the original's string helpers give)
static int NameChar(const std::string &name, size_t i)
{
	return tolower(i < name.size() ? (unsigned char)name[i] : 0);
}

// --------------------------------------------------------------------------- texture scrolls (0x497xxx)

// The texture-scroll list 0x497090 / 0x4970e0 / 0x4972b0 is in SaveGame.cpp (neg::ClearTexScrolls,
// UpdateTexScrolls, AddTexScroll), with the other free functions of that address range.

// --------------------------------------------------------------------------- construction

CJimmyGame::CJimmyGame()													// 0x4827f0
{
	// nothing of its own: the members are set by InitGame
}

CJimmyGame::~CJimmyGame()													// 0x4828d0
{
}

// --------------------------------------------------------------------------- CLocalGameObject-family slots

void CJimmyGame::ResetObject(void)											// 0x482a70
{
	CLocalGameObject::ResetObject();
}

void CJimmyGame::Process(float dt)											// 0x484260
{
	if (m_no_texture_scroll == 0) neg::UpdateTexScrolls(dt);				// 0x4970e0
	if (m_refog)
	{
		m_refog = 0;
		SetFog(m_fog_color[0], m_fog_color[1], m_fog_color[2], m_fog_color[3],
			   m_fog_restore[0], m_fog_restore[1], m_fog_restore[2], true);	// element +0x194
	}
	if (KeyPressed(0x13) || KeyPressed(0x03))								// VK_PAUSE, VK_CANCEL
	{
		if (g_game_mode == 6 || g_game_mode == 5) PauseGame(!g_game_paused);	// element +0x110
	}
	CGameType::Process(dt);
	UpdateSounds();														// 0x4b8a30
	ev97(dt);																// element +0x184
	if (g_granny) GrannyAdvanceTime(g_granny, (double)dt);
	if (g_player_hud) static_cast<CLocalGameObject *>(dynamic_cast<CGameObject *>(g_player_hud))->v269(dt);	// HUD +0xc8, vtable +0x434
	if (m_sky && m_sky_rotates) m_sky->AddRotation(0.0f, dt, 0.0f);		// sky +0xc0, vtable +0x330
	if (g_camera && g_health < 1.0f)
	{
		g_health = 1.0f;
		CameraPlayerDead(g_camera);											// camera element +0x1a4
	}
	if (g_queued_sfx_pending && !IsChannelPlaying((int)g_queued_sfx_after))
	{
		g_queued_sfx_pending = 0;
		PlayGameSound(-1, g_queued_sfx, 0);
		g_queued_sfx = g_empty_string;										// 0x545f14 ""
		MenuGivePendingPickup();											// 0x409b20
	}
	if (g_voice_channel != -1 && !IsChannelPlaying(g_voice_channel)) g_voice_channel = -1;
	UpdateTimers(dt);														// element +0x17c
}

// 0x484640: per frame while the editor is editing (CGameObject::v4): no fog, grey clear colour,
// a far clip plane far away.
void CJimmyGame::v242(float t)
{
	CGameObject::v242(t);													// 0x464ee0 (empty)
	if (g_granny) GrannyAdvanceTime(g_granny, (double)t);
	SetLayerFog(g_layer_world, false);										// 0x4b1860(layer, 0x800)
	g_layer_clear->set_clear_color(OMediaFARGBColor(0.0f, 0.5f, 0.5f, 0.5f));	// 2002 +0x10c..0x118 = r, g, b, a
	g_layer_clear->set_fog_color(OMediaFARGBColor(0.0f, 0.5f, 0.5f, 0.5f));	// 2002 +0xc8..0xd4
	g_layer_world->set_far_clip(200000.0f);									// 2002 +0xc0
}

// 0x483b50: after a level load. Quirk: power timer [9] (0x54a448) is not reset.
void CJimmyGame::v260(void)
{
	m_pickup_index = 0;
	g_health = 100.0f;
	g_power_timers[0] = 0.0f;
	g_power_timers[1] = 0.0f;
	g_power_timers[2] = 0.0f;
	g_power_timers[3] = 0.0f;
	if (g_player)															// (and its element is not NULL)
	{
		g_player_hud = PlayerHud();
		HudHide(g_player_hud);												// 0x468aa0
	}
	g_power_timers[4] = 0.0f;
	g_power_timers[5] = 0.0f;
	g_power_timers[6] = 0.0f;
	g_power_timers[7] = 0.0f;
	g_power_timers[8] = 0.0f;
}

void CJimmyGame::v277(void)													// 0x485e80
{
	if (m_stopwatch_channel != -1) StopGameSound(m_stopwatch_channel, false);			// (the channel is kept)
}

void CJimmyGame::v278(void)													// 0x485ea0
{
	if (m_stopwatch_channel != -1) m_stopwatch_channel = PlayGameSound(-1, "stopwatch", 1);
}

// 0x486170. A missing player (g_player NULL) makes the original write through address 0x4c18.
void CJimmyGame::WriteState(OMediaStreamOperators &s)
{
	int level_items = (int)g_545efc, f00 = g_545f00, f04 = g_545f04, f08 = g_545f08;
	CLocalGameObject::WriteState(s);
	s << m_fog_start << m_fog_end << m_far_clip;
	s << m_fog_restore[0] << m_fog_restore[1] << m_fog_restore[2];
	s << m_fog_color[0] << m_fog_color[1] << m_fog_color[2];				// (alpha is not saved)
	PlayerMusicState pm = PlayerMusic();
	s << pm.param;
	s << pm.name;
	for (int i = 0; i < 10; i++) s << g_power_timers[i];
	s << g_difficulty;
	s << level_items << f00 << f04 << f08;
}

void CJimmyGame::ReadState(OMediaStreamOperators &s)						// 0x485ef0
{
	int level_items = 0, f00 = 0, f04 = 0, f08 = 0;
	CLocalGameObject::ReadState(s);
	s >> m_fog_start >> m_fog_end >> m_far_clip;
	s >> m_fog_restore[0] >> m_fog_restore[1] >> m_fog_restore[2];
	s >> m_fog_color[0] >> m_fog_color[1] >> m_fog_color[2];
	SetFog(m_fog_color[0], m_fog_color[1], m_fog_color[2], m_fog_color[3],
		   m_fog_restore[0], m_fog_restore[1], m_fog_restore[2], true);		// element +0x194
	PlayerMusicState pm = PlayerMusic();
	s >> pm.param;
	s >> pm.name;
	if (strcasecmp(pm.name, "none") != 0) PlayLevelMusic(pm.name, pm.param);	// element +0x198
	for (int i = 0; i < 10; i++) s >> g_power_timers[i];
	s >> g_difficulty;
	if (g_power_timers[6] <= 0.0f)
	{
		StopwatchEnd();														// element +0x1ac
		g_enemy_time_scale = 1.0f;
	}
	else g_enemy_time_scale = 0.25f;
	RestoreTimerEffects();													// element +0x1b0
	s >> level_items >> f00 >> f04 >> f08;
	if (g_menu)
	{
		g_545f04 = f04;
		g_545f00 = f00;
		g_545f08 = f08;
		SetLevelItems(level_items);											// 0x40a860
	}
}

// --------------------------------------------------------------------------- element-family overrides

void CJimmyGame::InitGame(void)												// 0x482910
{
	Log("InitGame()");
	f_564 = 0;
	CGameType::InitGame();
	neg::ClearTexScrolls();													// 0x497090
	g_gravity.x = 0.0f;											// 0x631198 (Physics.cpp)
	g_gravity.y = -800.0f;
	g_gravity.z = 0.0f;
	f_554 = 1.0f;
	f_558 = 1.0f;
	g_layer_world->add_flags(omlayerf_SortElementBack2Front);				// 0x4b1880(layer, 0x20)
	v107(0);																// vtable +0x1ac
	m_surface_sound_next = 5;
	m_surface_sound = 5;
	m_pickup_index_base = 100;
	v128(0);																// vtable +0x200
	m_sky = NULL;
	m_sky_rotates = 1;
	f_5c8 = 0;
	m_refog = 0;
	f_4c4 = 1;																// CLocalGameObject +0x4c4
	m_xray_time = 0.0f;
	m_xray_flash = 0.0f;
	m_stopwatch_channel = -1;
	v52(0);																	// vtable +0xd0
	f_66c = 5.0f;
	f_670 = 20000.0f;
	f_674 = 20000.0f;
	f_678 = 15.0f;
	m_no_texture_scroll = 0;
	f_680 = 0.7f;
}

void CJimmyGame::UnInitGame(void)											// 0x482a30
{
	Log("UnInitGame()");
	CloseDatabaseByName("SoundEffects.omt");								// 0x4a4730
	g_active_sound_db = NULL;
	CGameType::UnInitGame();
}

// 0x4851a0: the path is first resolved by FileIsThere (omt\<path>, else c:\omt\<path>).
void CJimmyGame::InitTerrainDatabase(std::string path)
{
	char buf[256];
	strcpy(buf, path.c_str());
	// "Gona call Fileisthere" (debug log)
	FileIsThere(buf, "omt", "omt");											// element +0x164
	// "called Fileisthere"
	CGameType::InitTerrainDatabase(std::string(buf));						// (the same code, inline in the original)
}

// The three terrain builders (0x482bc0, 0x483040, 0x483780) differ in small ways; they are one
// function here with the differences spelled out.
enum TerrainMode { TERRAIN_LIT, TERRAIN_UNLIT, TERRAIN_TINTED };

static void BuildTerrain(CJimmyGame *game, TerrainMode mode, float r, float g, float b)
{
	if (!g_terrain_db) return;
	for (short i = 0; ShapeExists(g_terrain_db, 'E3ds', i); i++)
	{
		OMedia3DShape *shape = Load3DShape(g_terrain_db, i);
		std::string name;
		shape->get_chunk_name(name);
		if (name.find("3d") == 0) continue;								// 0x53b648 "3d"
		g_56af44 = 0;
		CGameObject *o = CreateObject('3TER');
		C3DObject *t = o ? dynamic_cast<C3DObject *>(o) : NULL;			// (a C3DTerrain)
		t->InitObject();													// (no NULL test in the original)
		t->set_shape(shape);												// element +0xac
		const bool nc = NameChar(name, 0) == 'n' && NameChar(name, 1) == 'c';	// "nc...": no collision
		if (mode == TERRAIN_LIT)
		{
			if (nc) MakeShapeUnLit(shape);
			else t->InitPhysics();
		}
		else
		{
			if (!nc) t->InitPhysics();
			MakeShapeUnLit(shape);
		}
		if (!g_windowed) t->ev51();											// element +0xcc (0x56afab)
		else if (!g_56af44) MakeShapeUnLit(shape);
		if (NameChar(name, 0) == 'i' && NameChar(name, 1) == 'k') t->HideAgain();	// "ik...": element +0xdc
		omt_PolygonList *polys = shape->get_polygons();
		for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
		{
			game->SetupTextureScroll(&*p);									// element +0x170
			OMedia3DMaterial *m = (*p).get_material();
			if (mode == TERRAIN_TINTED)
			{
				if (!m) continue;
				SetDiffuseRGB(m, r, g, b);
				if (nc && NameChar(name, 2) == '3' && !g_windowed) m->set_flags(m->get_flags() | ommatf_SecondPass);	// "nc3"
				continue;
			}
			if (mode == TERRAIN_LIT && !m) continue;						// (the unlit builder does not test m)
			// "nc1": additive; "nc2": additive, second pass. Windowed, they are hidden instead.
			if (nc && NameChar(name, 2) == '1')
			{
				if (!g_windowed) m->set_blend(omblendfc_Src_Alpha, omblendfc_One);
				else t->HideAgain();
			}
			if (nc && NameChar(name, 2) == '2')
			{
				if (!g_windowed)
				{
					m->set_blend(omblendfc_Src_Alpha, omblendfc_One);
					m->set_flags(m->get_flags() | ommatf_SecondPass);
				}
				else t->HideAgain();
			}
		}
	}
	game->SetFogColor();													// element +0xb0
}

// Quirk: unlike CGameType's, these do not set g_terrain.
void CJimmyGame::CreateTerrain(void)		{ BuildTerrain(this, TERRAIN_LIT, 0.0f, 0.0f, 0.0f); }		// 0x482bc0
void CJimmyGame::CreateTerrainUnLit(void)	{ BuildTerrain(this, TERRAIN_UNLIT, 0.0f, 0.0f, 0.0f); }	// 0x483040

void CJimmyGame::CreateTerrainTinted(float r, float g, float b, float a)	// 0x483780 (a unused)
{
	(void)a;
	BuildTerrain(this, TERRAIN_TINTED, r, g, b);
}

void CJimmyGame::PauseGame(bool pause)										// 0x485410
{
	Log("Inside PauseGame");
	if (!pause)
	{
		g_56af43 = 0;
		g_540590 = 0;
		CGameType::PauseGame(false);
		if (g_game_mode != 1) SetGameMode(5);								// 0x485360
		ShowPauseSprite(false);												// g_menu 0x40ed30
		PauseAll(false);													// vtable +0x45c
	}
	else if (g_game_mode != 6)
	{
		g_56af43 = 1;
		g_540590 = 0;
		g_mode_before_pause = g_game_mode;
		SetGameMode(6);
		CGameType::PauseGame(true);
		ShowPauseSprite(true);
		PauseAll(true);
		StopAllChannels();													// 0x4b8990
	}
}

void CJimmyGame::OnActivate(void)											// 0x482a80
{
	Log("GetFocus()");
	// "Gettting focus here!!!" (debug log)
	g_has_focus = 1;
	m_refog = 1;
	if (g_game_mode == 6) ShowPauseSprite(false);
}

// 0x482ad0. Quirk: the world layer's near and far clip go into the first two values SetFog is
// given again on the next focus (fog start / end), not the fog's own range.
void CJimmyGame::OnDeactivate(void)
{
	Log("LoseFocus()");
	g_has_focus = 0;
	ShowPauseSprite(false);
	// "Losing focus here!!!" (debug log)
	m_fog_restore[1] = g_layer_world->get_far_clip();						// 2002 +0xc0
	m_fog_restore[0] = g_layer_world->get_near_clip();						// 2002 +0xc4
}

// 0x484740: an object stands on a polygon with material `mat` (texture `tex`). The material's
// chunk ID picks the effect. Only Jimmy ('3JIM') and the '3COR' objects react.
void CJimmyGame::TouchSurface(CGameObject *o, OMedia3DMaterial *mat, OMediaCanvas *tex, float dt)
{
	(void)dt;
	if (!o || !tex) return;
	const int id = (int)mat->get_chunk_ID();								// (no NULL test of mat)
	if (o->m_object_id != '3JIM')
	{
		if (o->m_object_id != '3COR') return;
		if (id < 185 || id > 189) return;
		if (!o->v133()) return;												// vtable +0x214
		void *c = CorCarried(o);											// +0x934
		if (!c || PlayerFlag((OMediaElement *)c, 0x4cb8)) return;
		PlayerHurt(c, -5.0f, -5, 5.0f, true);								// its vtable +0x218
		CorStartHit(o);														// +0x1178 = 5.0, +0x1138 = 1, +0x1188 = 0, +0x113c = 0
		PlayGameSound(-1, RandomHalf() ? "pain ah agh aww" : "pain agh agh", 0);
		return;
	}
	OMediaElement *p = dynamic_cast<OMediaElement *>(o);						// Jimmy's element (o - 0xc0)

	if (id >= 110 && id <= 119)
	{
		if (!PlayerFlag(p, 0x4ca7)) { PlayerSlot(p, 130); SetPlayerFlag(p, 0x4ca7, true); }	// +0x208
		PlayerSlot(p, 120);													// +0x1e0
	}
	else if (PlayerFlag(p, 0x4ca7)) { PlayerSlot(p, 131); SetPlayerFlag(p, 0x4ca7, false); }	// +0x20c

	if (id >= 220 && id <= 229) SetPlayerFloat(p, 0xc08, 0.5f);

	if (id >= 185 && id <= 189 && o->v133() && !PlayerFlag(p, 0x4cb8))
	{
		LoadTask("autosave.tsk", 0);										// 0x498af0
		o->v261();															// vtable +0x414
		PlayerHurt(p, -5.0f, -5, 5.0f, true);								// element +0x218
		PlayGameSound(-1, RandomHalf() ? "pain ah agh aww" : "pain agh agh", 0);
	}

	if (id >= 100 && id <= 109)
	{
		if (!PlayerFlag(p, 0x4ca6)) { PlayerSlot(p, 128); SetPlayerFlag(p, 0x4ca6, true); }	// +0x200
		PlayerSlot(p, 127);													// +0x1fc
	}
	else if (PlayerFlag(p, 0x4ca6)) { PlayerSlot(p, 129); SetPlayerFlag(p, 0x4ca6, false); }	// +0x204

	if (id >= 120 && id <= 129 && o->v133()) PlayerSlot119(p, 0.0f, 1.0f, 0.0f, 0.0f);	// +0x1dc

	int set;
	if (id == 250)
	{
		if (o->v133())
		{
			SetPlayerFlag(p, 0x4db4, false);
			PlayerHurt(p, -300.0f, -1, -1.0f, true);
			PlayGameSound(-1, "SPLASH", 0);
		}
		set = 5;
	}
	else if (id == 251)
	{
		if (o->v133())
		{
			SetPlayerFlag(p, 0x4db4, false);
			PlayerHurt(p, -300.0f, -10, 5.0f, true);
			PlayGameSound(-1, "pain ahhouch", 0);
		}
		set = 5;
	}
	else if (id >= 185 && id <= 189) set = 0x23;
	else if (id >= 110 && id <= 119) set = 0x23;
	else if (id >= 50 && id <= 55) set = 0x7c;
	else if (id >= 56 && id <= 59) set = 0x7e;
	else if (id >= 130 && id <= 135) set = 0x7b;
	else if (id >= 136 && id <= 139) set = 0x80;
	else if (id >= 140 && id <= 149) set = 0x80;
	else if (id >= 150 && id <= 159) set = 0x7f;
	else if (id >= 70 && id <= 79) set = 0x81;
	else if (id == 90) set = 0x7d;
	else set = 5;
	m_surface_sound_next = set;
	if (m_surface_sound != m_surface_sound_next) SetPlayerSurfaceSound(p, m_surface_sound_next);	// element +0x264
	m_surface_sound = m_surface_sound_next;
}

// --------------------------------------------------------------------------- CJimmyGame's own slots

int CJimmyGame::NextPickupIndex(void)										// 0x484700
{
	m_pickup_index++;
	if (m_pickup_index > 100)
		ErrorLog("ERROR: PickupIndexCounter > 100  %d + %d", m_pickup_index_base, m_pickup_index);
	return m_pickup_index_base + m_pickup_index;
}

// 0x484f40: `name` becomes "<dir>\<name>" when that file exists, else "c:\<cd_dir>\<name>".
bool CJimmyGame::FileIsThere(char *name, const char *dir, const char *cd_dir)
{
	// "Inside fileisthere" (debug log)
	char local[256];
	strcpy(local, dir);
	strcat(local, "\\");													// 0x534a64
	strcat(local, name);
	if (!FileExists(local))
	{
		char cd[256];
		strcpy(cd, "c:\\");													// 0x53dc50
		strcat(cd, cd_dir);
		strcat(cd, "\\");
		strcat(cd, name);
		strcpy(name, cd);
	}
	else strcpy(name, local);
	return true;
}

// 0x483bf0: the stage of the level's story from task "SCENE": 0 below 250, 1 below 320, else 2.
char CJimmyGame::GetSceneStage(void)
{
	const int v = GetTaskValue("SCENE");
	if (v < 250) return 0;
	return (char)((319 < v) + 1);
}

// 0x484cc0: textures 800..879 scroll (870..879 wobble), 900..909 scroll diagonally at
// increasing speeds. True when the polygon got a scroll.
bool CJimmyGame::SetupTextureScroll(OMedia3DPolygon *p)
{
	OMedia3DMaterial *m = p->get_material();
	if (!m || !m->get_texture()) return false;
	const int id = (int)m->get_texture()->get_chunk_ID();
	if (!id) return false;
	if (id >= 800 && id < 880)
	{
		if (id < 810) neg::AddTexScroll(p, 0.0f, 0.2f);
		else if (id < 820) neg::AddTexScroll(p, 0.2f, 0.2f);
		else if (id < 830) neg::AddTexScroll(p, 0.2f, -0.2f);
		else if (id < 840) neg::AddTexScroll(p, 0.0f, -0.2f);
		else if (id < 850) neg::AddTexScroll(p, -0.2f, -0.2f);
		else if (id < 860) neg::AddTexScroll(p, -0.2f, 0.0f);
		else if (id < 870) neg::AddTexScroll(p, -0.2f, 0.2f);
		else neg::AddTexScroll(p, -99999.0f, -99999.0f);					// wobble
		return true;
	}
	if (id >= 900 && id < 910)
	{
		static const float speed[10] = { 0.05f, 0.08f, 0.1f, 0.13f, 0.17f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f };
		neg::AddTexScroll(p, speed[id - 900], speed[id - 900]);
		return true;
	}
	return false;
}

// 0x4834b0: diffuse r, g, b of every material of the terrain database's shapes.
void CJimmyGame::SetTerrainDBColor(float r, float g, float b, float a)
{
	(void)a;
	if (!g_terrain_db) return;
	for (short i = 0; ShapeExists(g_terrain_db, 'E3ds', i); i++)
	{
		OMedia3DShape *shape = Load3DShape(g_terrain_db, i);
		omt_PolygonList *polys = shape->get_polygons();
		for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
			if ((*p).get_material()) SetDiffuseRGB((*p).get_material(), r, g, b);
	}
}

// 0x4835b0 / 0x4836a0: diffuse r, g, b of the shapes of every '3TER' object.
static void TintTerrainObjects(float r, float g, float b)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o || o->m_object_id != '3TER') continue;
		OMedia3DShapeElement *e = dynamic_cast<OMedia3DShapeElement *>(o);	// o - 0xc0
		omt_PolygonList *polys = e->get_shape()->get_polygons();			// element +0xb0 (no NULL test)
		for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
			if ((*p).get_material()) SetDiffuseRGB((*p).get_material(), r, g, b);
	}
}

void CJimmyGame::SetTerrainColor(float r, float g, float b, float a)		// 0x4835b0 (a unused)
{
	(void)a;
	TintTerrainObjects(r, g, b);
}

void CJimmyGame::SetTerrainGrey(float v)									// 0x4836a0
{
	TintTerrainObjects(v, v, v);
}

// 0x483c50: count the power-up timers down; when one runs out its effect ends (RefreshObjects).
// Quirk: timer [1] (0x54a428) is not counted down.
void CJimmyGame::UpdateTimers(float dt)
{
	if (IsPaused()) return;													// element +0x114
	float *t = g_power_timers;
	if (t[3] > 0.0f && (t[3] -= dt) <= 0.0f)
	{
		t[3] = 0.0f;
		HudHide(g_player_hud);												// 0x468aa0
		RefreshObjects();
	}
	if (t[0] > 0.0f && (t[0] -= dt) <= 0.0f) { t[0] = 0.0f; RefreshObjects(); }
	if (t[2] > 0.0f && (t[2] -= dt) <= 0.0f) { t[2] = 0.0f; RefreshObjects(); }
	if (t[4] > 0.0f)
	{
		// x-ray: the terrain flashes green / yellow while m_xray_time runs, then goes back to grey-green
		if (m_xray_time > 0.0f)
		{
			m_xray_time -= dt;
			if (m_xray_flash > 0.0f)
			{
				m_xray_flash -= dt;
				if (m_xray_flash < 0.0f)
				{
					SetTerrainColor(0.1f, 1.0f, 0.1f, 1.0f);				// element +0x180
					m_xray_flash = -0.2f;
				}
			}
			else
			{
				m_xray_flash += dt;
				if (m_xray_flash > 0.0f)
				{
					SetTerrainColor(1.0f, 1.0f, 0.1f, 1.0f);
					m_xray_flash = 0.2f;
				}
			}
			if (m_xray_time <= 0.0f)
			{
				m_xray_time = 0.0f;
				SetTerrainColor(0.4f, 1.0f, 0.4f, 1.0f);
			}
		}
		if ((t[4] -= dt) <= 0.0f)
		{
			t[4] = 0.0f;
			SetTerrainColor(1.0f, 1.0f, 1.0f, 1.0f);
			RefreshObjects();
		}
	}
	if (t[5] > 0.0f && (t[5] -= dt) <= 0.0f) { t[5] = 0.0f; RefreshObjects(); }
	if (t[7] > 0.0f && (t[7] -= dt) <= 0.0f) { t[7] = 0.0f; RefreshObjects(); }
	if (t[8] > 0.0f && (t[8] -= dt) <= 0.0f) { t[8] = 0.0f; RefreshObjects(); }
	if (t[9] > 0.0f && (t[9] -= dt) <= 0.0f) { t[9] = 0.0f; RefreshObjects(); }
	// stopwatch: ticks while it runs
	if (t[6] > 0.0f)
	{
		t[6] -= dt;
		if (m_stopwatch_channel == -1) m_stopwatch_channel = PlayGameSound(-1, "stopwatch", 1);
		if (t[6] <= 0.0f)
		{
			StopwatchEnd();													// element +0x1ac
			return;
		}
	}
	else if (m_stopwatch_channel != -1)
	{
		StopGameSound(m_stopwatch_channel, false);
		m_stopwatch_channel = -1;
	}
}

// 0x485570: promote when the rank has gone up (task "PROMOTION" = 10 x rank, score box moves).
bool CJimmyGame::CheckPromotion(void)
{
	const int rank = GetRank();												// element +0x18c
	if (rank <= g_promotion_rank) return false;
	static const int box_y[10] = { 0, 0x182, 0x151, 0x122, 0xf1, 0xc1, 0x91, 0x61, 0x30, 1 };
	int value;
	switch (rank)
	{
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
			SetTaskValue("PROMOTION", rank * 10);							// 0x497980
			g_promotion_rank = rank;
			g_score_box_y = box_y[rank];
			return true;
		case 10:
		case 11:
			g_score_box2_on = 1;
			value = 100;
			break;
		default:
			value = 120;
			break;
	}
	SetTaskValue("PROMOTION", value);
	g_promotion_rank = rank;
	return true;
}

int CJimmyGame::GetRank(void)												// 0x4854e0
{
	if (g_score < 3000) return 0;
	if (g_score < 6000) return 1;
	if (g_score < 9000) return 2;
	if (g_score < 12000) return 3;
	if (g_score < 15000) return 4;
	if (g_score < 18000) return 5;
	if (g_score < 21000) return 6;
	if (g_score < 25000) return 7;
	if (g_score < 30000) return 8;
	return 34999 < g_score ? 10 : 9;
}

void CJimmyGame::RefreshObjects(void)										// 0x483c20: slot 271 of every object
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) static_cast<CLocalGameObject *>(*i)->v271();				// vtable +0x43c
}

// 0x484520: fog colour and range, the clear colour (both layers), the far clip plane.
void CJimmyGame::SetFog(float r, float g, float b, float a, float start, float end, float far_clip, bool on)
{
	m_fog_start = start;
	m_fog_end = end;
	m_far_clip = far_clip;
	m_fog_restore[0] = start;
	m_fog_restore[1] = end;
	m_fog_restore[2] = far_clip;
	const OMediaFARGBColor c(a, r, g, b);
	g_layer_clear->set_fog_color(c);										// 2002 +0xc8
	g_layer_clear->set_clear_color(c);										// 2002 +0x10c
	g_layer_world->set_far_clip(m_far_clip);								// 2002 +0xc0
	g_layer_world->set_clear_color(c);
	g_layer_world->set_fog_color(c);
	g_layer_world->set_fog_range(m_fog_start, m_fog_end);					// 2002 +0xd8 / +0xdc
	SetLayerFog(g_layer_world, true);										// 2002 flags |= 0x800
	m_fog_color[0] = r;
	m_fog_color[1] = g;
	m_fog_color[2] = b;
	m_fog_color[3] = a;
	if (!on) SetLayerFog(g_layer_world, false);								// 0x4b1860(layer, 0x800)
}

// 0x485d00: stop the level music, then load and play `name` ("none": no music).
void CJimmyGame::PlayLevelMusic(const char *name, int a)
{
	PlayerMusicState pm = PlayerMusic();									// (g_player - 0xc0)->+0x4c14..
	if (g_level_music)
	{
		StopChannel(pm.channel, 0);											// 0x4b88e0
		if (strcasecmp(pm.name, "none") != 0)
		{
			CloseDatabaseByName(pm.name);									// 0x4a4730
			pm.channel = -1;
		}
		g_level_music = NULL;
	}
	if (strcasecmp(name, "none") != 0)
	{
		strcpy(pm.name, name);
		g_level_music = LoadMusic(name);									// 0x4a4180
		if (g_level_music && pm.channel == -1 && a != -1)
		{
			pm.param = a;
			pm.channel = PlayMusic(g_level_music, -1, a, true);				// 0x4b8850
		}
	}
}

// 0x485b80. Quirk: the channel is forgotten even when the music database stays open.
void CJimmyGame::StopLevelMusic(void)
{
	if (!g_level_music) return;
	PlayerMusicState pm = PlayerMusic();
	StopChannel(pm.channel, 0);
	pm.channel = -1;
	if (strcasecmp(pm.name, "none") != 0)
	{
		CloseDatabaseByName(pm.name);
		strcpy(pm.name, "none");
	}
	g_level_music = NULL;
}

// 0x485c60: as StopLevelMusic, but the name stays (so ReadState / PlayLevelMusic can restart it).
void CJimmyGame::StopLevelMusicKeepName(void)
{
	if (!g_level_music) return;
	PlayerMusicState pm = PlayerMusic();
	StopChannel(pm.channel, 0);
	pm.channel = -1;
	if (strcasecmp(pm.name, "none") != 0) CloseDatabaseByName(pm.name);
	g_level_music = NULL;
}

void CJimmyGame::StartGadgetEffect(const char *gadget)						// 0x483570
{
	if (strcasecmp("XRAY", gadget) == 0)									// 0x539608
	{
		m_xray_time = 3.0f;
		m_xray_flash = 0.2f;
	}
}

// 0x485b40: a running average of the frame rate (2% per frame). The x87 keeps the sum in a
// register; double does the same here.
float CJimmyGame::SmoothFrameRate(float dt)
{
	g_frame_rate_now = 1.0f / dt;
	const double sum = (double)g_frame_rate_acc + g_frame_rate_now;
	g_frame_rate = (float)(sum * 0.02f);									// 0x503620
	g_frame_rate_acc = (float)(sum - g_frame_rate);
	return g_frame_rate;
}

// 0x4841f0: the stopwatch has run out: everything runs at normal speed again.
void CJimmyGame::StopwatchEnd(void)
{
	if (m_stopwatch_channel != -1)
	{
		StopGameSound(m_stopwatch_channel, false);
		m_stopwatch_channel = -1;
	}
	g_power_timers[6] = 0.0f;
	RefreshObjects();
	g_enemy_time_scale = 1.0f;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) (*i)->f_430 = 1.0f;											// each object's time scale
}

// 0x484110: after a savegame was read, end the effects whose timers are not running.
void CJimmyGame::RestoreTimerEffects(void)
{
	if (g_power_timers[3] <= 0.0f)
	{
		g_power_timers[3] = 0.0f;
		HudHide(g_player_hud);
	}
	if (g_power_timers[4] <= 0.0f)
	{
		g_power_timers[4] = 0.0f;
		SetTerrainColor(1.0f, 1.0f, 1.0f, 1.0f);							// element +0x180
	}
	if (g_power_timers[6] <= 0.0f && m_stopwatch_channel != -1)
	{
		StopGameSound(m_stopwatch_channel, false);
		m_stopwatch_channel = -1;
	}
	RefreshObjects();														// element +0x190
}
