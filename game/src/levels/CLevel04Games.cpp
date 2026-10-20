/*  The level 4 game types - see CLevel04Games.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel04Games.h"
#include "LevelHelpers.h"
#include <string.h>
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// The sky of LEV4 and LV4C by the story stage; the night sky stands still.
// (Inline in both; the compiler shares the SkyLoad call between the cases.)
static void LoadStageSky(CJimmyGame *game, char stage)
{
	switch (stage)
	{
		case 0:	SkyLoad(game->m_sky, "omt\\bluesky3.omt", false); break;
		case 1:	SkyLoad(game->m_sky, "omt\\dusksky.omt", false); break;
		case 2:
			game->m_sky_rotates = 0;
			SkyLoad(game->m_sky, "omt\\nightsky.omt", false);
			break;
	}
}

// --------------------------------------------------------------------------- CLevel04Game 'LEV4'

CLevel04Game::CLevel04Game()												// 0x48b8b0
{
	static char name[] = "CLEVEL04GAME";
	SetName(name);
	Log("CLevel04Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV4');
	m_pickup_index_base = 2400;
}

CLevel04Game::~CLevel04Game()												// 0x48b9d0
{
	Log("~CLevel04Game()");
}

void CLevel04Game::InitGame(void)											// 0x48ba60
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);												// not written to the .gam (f_00b)
	m_sky = CreateSky();
	if (m_sky)
	{
		LoadStageSky(this, GetSceneStage());				// element +0x16c, sky element +0xe8
		m_sky->ev51();										// element +0xcc
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();								// (empty for a C3DSky)
		m_sky->v67(1.0f);									// radius
		m_sky->v152(1.0f);
	}
	InitTerrainDatabase("level4.omt");
	CreateTerrain();										// element +0xf4 (lit)
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.24f, 0.56f, 0.75f, 0.0f);
	g_layer_clear->set_fog_color(c);						// 2002 +0xc8
	g_layer_world->set_far_clip(30000.0f);					// (0x56afc8) 2002 +0xc0
	g_layer_world->set_fog_color(c);
	g_layer_world->set_far_clip(30000.0f);					// (0x56afd4)
	g_layer_world->set_fog_color(c);
	SetLayerFog(g_layer_clear, false);						// 0x4b1860(layer, 0x800)
	SetLayerFog(g_layer_world, false);						// (0x56afd4)
	SetLayerFog(g_layer_world, false);						// (0x56afc8)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel04AGame 'LV4A'

CLevel04AGame::CLevel04AGame()												// 0x48a840
{
	static char name[] = "CLEVEL04AGAME";
	SetName(name);
	Log("CLevel04AGame()");
	CGameType::InitObject();
	CGameType::SetID('LV4A');
	m_pickup_index_base = 2000;
}

CLevel04AGame::~CLevel04AGame()												// 0x48a960
{
	Log("~CLevel04aGame()");
}

void CLevel04AGame::InitGame(void)											// 0x48a9f0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL
	SkyLoad(sky, "omt\\nightsky.omt", false);
	sky->place(0.0f, 0.0f, 0.0f);
	sky->InitPhysics();
	sky->v67(1.0f);
	sky->v152(1.0f);
	sky->ev51();
	InitTerrainDatabase("level4a.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.24f, 0.96f, 0.25f, 0.0f, 1000.0f, 9600.0f, 10000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel04BGame 'LV4B'

CLevel04BGame::CLevel04BGame()												// 0x48ac00
{
	static char name[] = "CLEVEL04BGAME";
	SetName(name);
	Log("CLevel04BGame()");
	CGameType::InitObject();
	CGameType::SetID('LV4B');
	strcpy(m_task_name, "POWER");
	f_684 = 0;
	f_688 = 0;
	m_pickup_index_base = 2100;
}

CLevel04BGame::~CLevel04BGame()												// 0x48ad50
{
	Log("~CLevel04bGame()");
}

void CLevel04BGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070
void CLevel04BGame::Process(float dt)	{ CJimmyGame::Process(dt); }		// 0x487160

// 0x48af40: CJimmyGame's, then the task name again (a level load may have replaced it).
void CLevel04BGame::v260(void)
{
	CJimmyGame::v260();
	strcpy(m_task_name, "POWER");
}

// 0x48af30: only CLocalGameObject's (SetTaskState("POWER", state) reaches the game type here).
void CLevel04BGame::v267(int a2)	{ CLocalGameObject::v267(a2); }

void CLevel04BGame::InitGame(void)											// 0x48ade0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	InitTerrainDatabase("level4b.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.75f, 0.6f, 0.4f, 0.0f, 500.0f, 7000.0f, 7200.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel04CGame 'LV4C'

CLevel04CGame::CLevel04CGame()												// 0x48afe0
{
	static char name[] = "CLEVEL04CGAME";
	SetName(name);
	Log("CLevel04CGame()");
	CGameType::InitObject();
	CGameType::SetID('LV4C');
	m_pickup_index_base = 2200;
}

CLevel04CGame::~CLevel04CGame()												// 0x48b100
{
	Log("~CLevel04cGame()");
}

void CLevel04CGame::InitGame(void)											// 0x48b190
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	m_sky = CreateSky();
	if (m_sky)
	{
		LoadStageSky(this, GetSceneStage());
		m_sky->ev51();
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();
		m_sky->v67(1.0f);
		m_sky->v152(1.0f);
	}
	InitTerrainDatabase("level4c.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.031f, 0.34f, 0.58f, 0.0f);
	g_layer_clear->set_fog_color(c);						// 2002 +0xc8
	g_layer_clear->set_clear_color(c);						// 2002 +0x10c
	g_layer_world->set_far_clip(30000.0f);					// (0x56afc8)
	g_layer_world->set_fog_color(c);
	g_layer_world->set_far_clip(30000.0f);					// (0x56afd4)
	g_layer_world->set_fog_color(c);
	SetLayerFog(g_layer_world, false);						// (0x56afd4) 0x4b1860(layer, 0x800)
	SetLayerFog(g_layer_world, false);						// (0x56afc8)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel04DGame 'LV4D'

CLevel04DGame::CLevel04DGame()												// 0x48b490
{
	static char name[] = "CLEVEL04DGAME";
	SetName(name);
	Log("CLevel04DGame()");
	CGameType::InitObject();
	CGameType::SetID('LV4D');
	m_pickup_index_base = 2300;
}

CLevel04DGame::~CLevel04DGame()												// 0x48b5b0
{
	Log("~CLevel04DGame()");
}

// 0x48b640. Quirk: the fog range is set (on 0x56afd4 only), then the fog is turned off.
void CLevel04DGame::InitGame(void)
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL
	SkyLoad(sky, "omt\\nightsky.omt", false);
	sky->place(0.0f, 0.0f, 0.0f);
	sky->InitPhysics();
	sky->v67(1.0f);
	sky->v152(1.0f);
	sky->ev51();
	InitTerrainDatabase("level4d.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.24f, 0.96f, 0.25f, 0.0f);
	g_layer_clear->set_fog_color(c);						// 2002 +0xc8
	g_layer_world->set_far_clip(10000.0f);					// (0x56afc8)
	g_layer_world->set_fog_color(c);
	g_layer_world->set_far_clip(10000.0f);					// (0x56afd4)
	g_layer_world->set_fog_color(c);
	g_layer_world->set_fog_range(1000.0f, 9600.0f);			// (0x56afd4) 2002 +0xd8 / +0xdc
	SetLayerFog(g_layer_world, false);						// (0x56afc8) 0x4b1860(layer, 0x800)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevel04Game(void)		{ return new CLevel04Game; }		// 'LEV4' (0x68c bytes)
CGameObject *New_CLevel04AGame(void)	{ return new CLevel04AGame; }		// 'LV4A'
CGameObject *New_CLevel04BGame(void)	{ return new CLevel04BGame; }		// 'LV4B' (0x694 bytes)
CGameObject *New_CLevel04CGame(void)	{ return new CLevel04CGame; }		// 'LV4C'
CGameObject *New_CLevel04DGame(void)	{ return new CLevel04DGame; }		// 'LV4D'
}
