/*  CGameType - see CGameType.h. Reconstructed from Neutron2.exe (0x4af5a0..0x4b0d70 and the shape
    helpers 0x4b2460..0x4b2c80 it uses).                                                        */

#include "CGameType.h"
#include "Mouse.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "TextureAnims.h"
#include "Physics.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include "OMediaViewPort.h"
#include "OMediaDataBase.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DShapeElement.h"
#include <SDL.h>

namespace neg {
CGameObject			*g_camera = NULL;
unsigned int		 g_song_id = 0;
bool				 g_game_paused = false;
bool				 g_help_shown = false;
OMediaElement		*g_terrain = NULL;
unsigned int		 g_input_device[4] = { 'KEYB', 'KEYB', 'KEYB', 'KEYB' };
OMediaDataBase		*g_active_sound_db = NULL;

OMediaDataBase		*g_effects_db = NULL;
OMediaDataBase		*g_object_db = NULL;
OMediaDataBase		*g_song_db = NULL;
OMediaDataBase		*g_sound_db = NULL;
OMediaDataBase		*g_terrain_db = NULL;
OMediaDataBase		*g_vehicle_db = NULL;
OMediaFileStream	*g_effects_stream = NULL;
OMediaFileStream	*g_object_stream = NULL;
OMediaFileStream	*g_song_stream = NULL;
OMediaFileStream	*g_sound_stream = NULL;
OMediaFileStream	*g_terrain_stream = NULL;
OMediaFileStream	*g_vehicle_stream = NULL;
}
using namespace neg;

// --------------------------------------------------------------------------- not reconstructed yet

// 0x49b260 C3DParticle::Reset (debug logs "+C3DParticle::Reset()" / "-..."): puts every particle of
// the list 0x54e848 back to its start state (hidden, 'PART', ...). Nothing in the shipped game
// adds to that list (only its static constructor / destructor and this function use it), so the
// loop never runs.
static void ResetParticles(void)			{}
// 0x4a3930 (as in CEditor.cpp): capture centres the pointer (hiding the Windows cursor has no
// equivalent on the Thor); release only clears the flag.
static void CaptureMouse(bool capture)
{
	if (capture && !g_56aed0) { neg::CenterMouse(); g_mouse_captured = 1; return; }
	g_mouse_captured = 0;
}
// OMediaMouseCursor::hide() (OMT2.dll); the SDL build of OMT has no OMediaMouseCursor.
static void HideMouseCursor(void)			{ SDL_ShowCursor(SDL_DISABLE); }
// --------------------------------------------------------------------------- shape helpers (0x4b2xxx)

// 0x4b2d70. Quirk: the chunk type argument is ignored, the test is always made for '3DSh'.
bool neg::ShapeExists(OMediaDataBase *db, unsigned int type, int id)
{
	(void)type;
	return db->object_exists('3DSh', id);
}

// 0x4b2c80 "Load3DShape(%d)"
OMedia3DShape *neg::Load3DShape(OMediaDataBase *db, int id)
{
	OMedia3DShape *shape = NULL;
	if (db)
	{
		shape = (OMedia3DShape *)db->get_object('3DSh', id);
		if (g_56afaa) NEG_TODO(0x4b2c80, "debug database call (db vtable +0x4c)");
		if (shape) shape->db_unlock();
		AddShapeTextureAnims(db, shape);					// 0x4a23e0
		AddShapeMaterialAnims(db, shape);					// 0x490300
		g_56afac = 0;
		// Translucent materials (diffuse alpha != 1) are drawn in the second pass, alpha blended.
		omt_PolygonList *polys = shape->get_polygons();			// (no NULL test in the original)
		for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
		{
			OMedia3DMaterial *m = (*p).get_material();
			if (m && m->get_diffuse_a() != 1.0f)
			{
				m->set_flags(m->get_flags() | ommatf_SecondPass);
				m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
				g_56afac = 1;
			}
		}
	}
	return shape;
}

static void SetShapeShadeMode(OMedia3DShape *shape, omt_ShadeMode mode)		// 0x4b24a0 (flat) / 0x4b2460 (gouraud)
{
	if (!shape) return;
	omt_PolygonList *polys = shape->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
		if ((*p).get_material()) (*p).get_material()->set_shade_mode(mode);
}

static void SetShapeLightMode(OMedia3DShape *shape, omt_MaterialLightMode mode)	// 0x4b24e0 (light) / 0x4b25c0 (vertex colour)
{
	if (!shape) return;
	omt_PolygonList *polys = shape->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
		if ((*p).get_material()) (*p).get_material()->set_light_mode(mode);
}

void neg::MakeShapeUnLit(OMedia3DShape *shape)							// 0x4b2520 "MakeShapeUnLit()"
{
	if (!shape) return;
	omt_PolygonList *polys = shape->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		OMediaFRGBColor white(1.0f, 1.0f, 1.0f);
		m->set_color(white, 0.0f, 0.0f, 1.0f, 0.0f);			// emission 0, ambient 0, diffuse 1, specular 0
		m->set_light_mode(ommlmc_Color);
	}
}

// --------------------------------------------------------------------------- construction

CGameType::CGameType()														// 0x4af5a0
{
	static char name[] = "CGAMETYPE";		// SetName upper-cases in place (the original's string is in .data)
	SetName(name);
	Log("CGameType()");
	set_id('GAME');							// element +0x78
	m_object_id = 'GAME';					// +0x490
	link(g_world);
	HideMouseCursor();						// OMediaMouseCursor::hide()
	g_camera = NULL;
	g_gametype = this;
}

CGameType::~CGameType()														// 0x4af6e0
{
	Log("~CGameType()");
	SetCamera(NULL);						// vtable +0x124
	// g_gametype is not cleared (the next game type's constructor replaces it).
}

// --------------------------------------------------------------------------- slots

void CGameType::update_logic(float millisecs_elapsed)						// 0x4af7b0
{
	OMediaElement::update_logic(millisecs_elapsed);
}

void CGameType::InitObject(void)											// 0x4af9b0
{
	Log("InitObject()");
	// Quirk: CLocalGameObject::InitObject is not called, so a game type registers no properties.
	ResetParticles();						// 0x49b260
	ResetTextureAnims();					// 0x4a2270
	InitGame();								// element vtable +0xb4
}

void CGameType::UnInitObject(void)											// 0x4af9f0
{
	Log("UnInitObject()");
	UnInitGame();							// element vtable +0xb8
}

void CGameType::UnInitDatabases(void)										// 0x4afa90
{
	Log("UnInitDatabases()");
	UnInitEffectsDatabase();
	UnInitObjectDatabase();
	UnInitSongDatabase();
	UnInitSoundDatabase();
	UnInitTerrainDatabase();
	UnInitVehicleDatabase();
}

void CGameType::Process(float dt)											// 0x4b0a90
{
	CLocalGameObject::Process(dt);
	ev76();									// element vtable +0x130
	if (g_camera && (!g_56af43 || g_540590))	// GameIsRunning() inline
		g_camera->v243(dt);					// vtable +0x3cc
}

void CGameType::SetID(int id)												// 0x4af7a0
{
	set_id(id);
	m_object_id = id;
}

// 0x4af7c0: the fog colour of the world layer is the diffuse colour of the terrain database's
// material named "fogcolor" ("fogcoloralt" when 0x54e860 is set; C3DSky sets it).
void CGameType::SetFogColor(void)
{
	omt_DBIterator it;
	OMediaDBObject *o = g_terrain_db->get_first_object('3DMa', it);
	while (o)
	{
		std::string name;
		o->get_chunk_name(name);
		if ((g_54e860 && name == "fogcoloralt") || (!g_54e860 && name == "fogcolor"))
		{
			OMediaFARGBColor c;
			((OMedia3DMaterial *)o)->get_diffuse(c);
			g_layer_world->set_fog_color(c);					// layer +0xc8 (2002)
			return;
		}
		o = g_terrain_db->get_next_object(it);
	}
	// (each material stays db-locked by the iteration, as in the original)
}

void CGameType::InitGame(void)												// 0x4afa20
{
	Log("InitGame()");
	CaptureMouse(true);						// 0x4a3930
	g_viewport->sety(10000.0f);
	g_viewport->setx(0);
	g_viewport->setz(0);
	g_56af41 = 0;
}

void CGameType::UnInitGame(void)											// 0x4afa60
{
	Log("UnInitGame()");
	ResetParticles();						// 0x49b260
	ResetTextureAnims();					// 0x4a2270
	StopAllChannels();						// 0x4b8990
	PhysicsClearTerrain();					// 0x4b49c0 ClearPolyTerrains
}

void CGameType::StartSong(void)												// 0x4afb00
{
	if (g_sound && g_song_db)
	{
		OMediaDBObject *song = g_song_db->get_object('Wave', (omt_FSChunkID)0);
		if (song)
		{
			song->db_unlock();
			g_song_id = song->get_chunk_ID();
			PlaySong();						// element vtable +0x12c
		}
	}
}

// The six database pairs: the file stream is opened first, then the database on it.

void CGameType::InitEffectsDatabase(std::string path)						// 0x4afb70
{
	if (!g_effects_stream)
	{
		g_effects_stream = InitFileStream(path.c_str());
		if (!g_effects_stream) return;
	}
	if (!g_effects_db) g_effects_db = InitDatabase(g_effects_stream);
}

void CGameType::UnInitEffectsDatabase(void)									// 0x4afcd0
{
	Log("UnInitEffectsDatabase()");
	UnInitDatabase(g_effects_db);
	UnInitFileStream(g_effects_stream);
	g_effects_db = NULL;
	g_effects_stream = NULL;
}

void CGameType::InitObjectDatabase(std::string path)						// 0x4afd10
{
	if (!g_object_stream)
	{
		g_object_stream = InitFileStream(path.c_str());
		if (!g_object_stream) return;
	}
	if (!g_object_db) g_object_db = InitDatabase(g_object_stream);
}

void CGameType::UnInitObjectDatabase(void)									// 0x4afe70
{
	Log("UnInitObjectDatabase()");
	UnInitDatabase(g_object_db);
	UnInitFileStream(g_object_stream);
	g_object_db = NULL;
	g_object_stream = NULL;
}

void CGameType::InitSongDatabase(std::string path)							// 0x4afeb0
{
	if (!g_song_stream)
	{
		g_song_stream = InitFileStream(path.c_str());
		if (!g_song_stream) return;
	}
	if (!g_song_db) g_song_db = InitDatabase(g_song_stream);
}

void CGameType::UnInitSongDatabase(void)									// 0x4b0010
{
	Log("UnInitSongDatabase()");
	UnInitDatabase(g_song_db);
	UnInitFileStream(g_song_stream);
	g_song_db = NULL;
	g_song_stream = NULL;
}

void CGameType::InitSoundDatabase(std::string path)							// 0x4b0050
{
	Log("InitSoundDatabase(%s)", path.c_str());
	if (!g_sound_stream)
	{
		g_sound_stream = InitFileStream(path.c_str());
		if (!g_sound_stream) return;
	}
	if (!g_sound_db)
	{
		g_sound_db = InitDatabase(g_sound_stream);
		g_active_sound_db = g_sound_db;
	}
}

void CGameType::UnInitSoundDatabase(void)									// 0x4b01d0
{
	Log("UnInitSoundDatabase()");
	UnInitDatabase(g_sound_db);
	UnInitFileStream(g_sound_stream);
	g_active_sound_db = NULL;
	g_sound_db = NULL;
	g_sound_stream = NULL;
}

void CGameType::InitTerrainDatabase(std::string path)						// 0x4b0220
{
	if (!g_terrain_stream)
	{
		g_terrain_stream = InitFileStream(path.c_str());
		if (!g_terrain_stream) return;
	}
	if (!g_terrain_db) g_terrain_db = InitDatabase(g_terrain_stream);
}

void CGameType::UnInitTerrainDatabase(void)									// 0x4b0380
{
	Log("UnInitTerrainDatabase()");
	UnInitDatabase(g_terrain_db);
	UnInitFileStream(g_terrain_stream);
	g_terrain_db = NULL;
	g_terrain_stream = NULL;
}

void CGameType::InitVehicleDatabase(std::string path)						// 0x4b03c0
{
	if (!g_vehicle_stream)
	{
		g_vehicle_stream = InitFileStream(path.c_str());
		if (!g_vehicle_stream) return;
	}
	if (!g_vehicle_db) g_vehicle_db = InitDatabase(g_vehicle_stream);
}

void CGameType::UnInitVehicleDatabase(void)									// 0x4b0520
{
	Log("UnInitVehicleDatabase()");
	UnInitDatabase(g_vehicle_db);
	UnInitFileStream(g_vehicle_stream);
	g_vehicle_db = NULL;
	g_vehicle_stream = NULL;
}

// 0x4b0560 / 0x4b07b0: one '3TER' object per shape of the terrain database, except the shapes whose
// name starts with "3d". The unlit variant also makes each shape's materials unlit.
static void CreateTerrainObjects(bool unlit)
{
	for (short i = 0; ShapeExists(g_terrain_db, 'E3ds', i); i++)
	{
		OMedia3DShape *shape = Load3DShape(g_terrain_db, i);
		std::string name;
		shape->get_chunk_name(name);
		if (name.find("3d") != 0)
		{
			CGameObject *o = CreateObject('3TER');
			g_terrain = o ? dynamic_cast<OMediaElement *>(o) : NULL;
			o->InitObject();					// (no NULL test in the original)
			static_cast<OMedia3DShapeElement *>(g_terrain)->set_shape(shape);	// element vtable +0xac
			o->InitPhysics();					// vtable +0x2c
			if (unlit) MakeShapeUnLit(shape);	// 0x4b2520
		}
	}
}

void CGameType::CreateTerrain(void)											// 0x4b0560
{
	if (!g_terrain_db) return;
	CreateTerrainObjects(false);
	SetFogColor();							// element vtable +0xb0
}

void CGameType::CreateTerrainUnLit(void)									// 0x4b07b0
{
	if (!g_terrain_db) return;
	CreateTerrainObjects(true);
	SetFogColor();
}

// 0x4b0b10..0x4b0c50: apply a material mode to the shape of every terrain element of the world layer.
static void ForEachTerrainShape(void (*apply)(OMedia3DShape *, int), int mode)
{
	omt_ElementList *l = g_layer_world->get_element_list();
	for (omt_ElementList::iterator i = l->begin(); i != l->end(); i++)
	{
		OMediaElement *e = *i;
		if (e->get_id() != '3TER') continue;
		OMedia3DShapeElement *se = static_cast<OMedia3DShapeElement *>(e);
		if (se->get_shape()) apply(se->get_shape(), mode);		// get_shape (element vtable +0xb0) twice, as in the original
	}
}
static void ApplyShade(OMedia3DShape *s, int mode) { SetShapeShadeMode(s, (omt_ShadeMode)mode); }
static void ApplyLight(OMedia3DShape *s, int mode) { SetShapeLightMode(s, (omt_MaterialLightMode)mode); }
static void ApplyUnLit(OMedia3DShape *s, int) { MakeShapeUnLit(s); }

void CGameType::SetTerrainFlatShading(void)		{ ForEachTerrainShape(ApplyShade, omshademc_Flat); }		// 0x4b0b10 -> 0x4b24a0
void CGameType::SetTerrainGouraudShading(void)	{ ForEachTerrainShape(ApplyShade, omshademc_Gouraud); }	// 0x4b0b60 -> 0x4b2460
void CGameType::SetTerrainLit(void)				{ ForEachTerrainShape(ApplyLight, ommlmc_Light); }		// 0x4b0bb0 -> 0x4b24e0
void CGameType::SetTerrainVertexLit(void)		{ ForEachTerrainShape(ApplyLight, ommlmc_VertexColor); }	// 0x4b0c00 -> 0x4b25c0
void CGameType::SetTerrainUnLit(void)			{ ForEachTerrainShape(ApplyUnLit, 0); }					// 0x4b0c50 -> 0x4b2520

void CGameType::PauseGame(bool pause)										// 0x4b0a10
{
	Log("PauseGame(%d)", (int)pause);
	g_game_paused = pause;
}

bool CGameType::IsPaused(void)												// 0x4803b0
{
	return g_game_paused;
}

void CGameType::ev70(void)													// 0x4b0a50 (InitGame clears it; C3DCar/C3DVehicle test it)
{
	g_56af41 = 1;
}

void CGameType::SetCamera(CGameObject *camera)								// 0x4b0a60
{
	g_camera = camera;
}

void CGameType::SetInputDevice(short player, unsigned int device)			// 0x4b0a70
{
	g_input_device[player] = device;
}

void CGameType::PlaySong(void)												// 0x4b0ae0
{
	if (g_music_on && g_song_db)
		PlayMusic(g_song_db, g_music_channel, (int)g_song_id, true);	// 0x4b8850
}

void CGameType::ToggleHelp(void)											// 0x4b0cd0
{
	Log("ToggleHelp()");
	if (g_help_shown)
	{
		HideHelp();							// element vtable +0x148
		g_help_shown = false;
	}
	else
	{
		ShowHelp();							// element vtable +0x144
		g_help_shown = true;
	}
}

// --------------------------------------------------------------------------- free functions (Bridge.h)

bool neg::GameIsRunning(void)												// 0x4b0ca0
{
	if (g_56af43 && !g_540590) return false;
	return true;
}

int neg::GameTypeTouchStart(CGameObject *o, CGameObject *other)	{ return g_gametype->TouchStart(o, other); }	// vtable +0x14c
int neg::GameTypeTouchEnd(CGameObject *o, CGameObject *other)	{ return g_gametype->TouchEnd(o, other); }		// vtable +0x150
int neg::GameTypeTouch154(CGameObject *o, CGameObject *other)	{ return g_gametype->Touch154(o, other); }	// vtable +0x154
