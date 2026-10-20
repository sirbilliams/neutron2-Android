/*  The level 6 and 7 game types - see CLevel06Games.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel06Games.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// --------------------------------------------------------------------------- CLevel06Game 'LEV6'

CLevel06Game::CLevel06Game()												// 0x48d250
{
	static char name[] = "CLEVEL06GAME";
	SetName(name);
	Log("CLevel06Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV6');
	m_pickup_index_base = 3000;
}

CLevel06Game::~CLevel06Game()												// 0x48d370
{
	Log("~CLevel06Game()");
}

void CLevel06Game::InitGame(void)											// 0x48d400
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level6.omt");
	v107(0);												// not written to the .gam (f_00b)
	CreateTerrainUnLit();
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.18f, 0.23f, 0.3f, 0.0f);
	g_layer_clear->set_clear_color(c);						// 2002 +0x10c
	WorldFog(g_layer_world, c, 4000.0f, 2000.0f, 4000.0f);	// (0x56afc8)
	WorldFog(g_layer_world, c, 4000.0f, 2000.0f, 4000.0f);	// (0x56afd4)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel06AGame 'LV6A'

CLevel06AGame::CLevel06AGame()												// 0x48ce90
{
	static char name[] = "CLEVEL06AGAME";
	SetName(name);
	Log("CLevel06AGame()");
	CGameType::InitObject();
	CGameType::SetID('LV6A');
	m_pickup_index_base = 2900;
}

CLevel06AGame::~CLevel06AGame()												// 0x48cfb0
{
	Log("~CLevel06AGame()");
}

void CLevel06AGame::InitGame(void)											// 0x48d040
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level6a.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.18f, 0.23f, 0.3f, 0.0f);
	g_layer_clear->set_clear_color(c);
	WorldFog(g_layer_world, c, 4000.0f, 2000.0f, 4000.0f);	// (0x56afc8)
	WorldFog(g_layer_world, c, 4000.0f, 2000.0f, 4000.0f);	// (0x56afd4)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel07Game 'LEV7'

CLevel07Game::CLevel07Game()												// 0x48d610
{
	static char name[] = "CLEVEL06GAME";		// sic: level 6's name
	SetName(name);
	Log("CLevel07Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV7');
	m_pickup_index_base = 3100;
}

CLevel07Game::~CLevel07Game()												// 0x48d730
{
	Log("~CLevel07Game()");
}

void CLevel07Game::InitGame(void)											// 0x48d7c0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level7.omt");
	v107(0);
	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL
	if (sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(sky, "omt\\space.omt", false);				// sky element +0xe8
		sky->place(0.0f, 0.0f, 0.0f);
		sky->InitPhysics();									// (empty for a C3DSky)
		sky->v67(1.0f);										// radius
		sky->v152(1.0f);
		sky->ev51();										// element +0xcc
	}
	CreateTerrainUnLit();
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.0f, 0.0f, 0.0f, 0.0f);
	g_layer_clear->set_clear_color(c);
	WorldFog(g_layer_world, c, 10000.0f, 9000.0f, 10000.0f);	// (0x56afc8)
	WorldFog(g_layer_world, c, 10000.0f, 9000.0f, 10000.0f);	// (0x56afd4)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevel06Game(void)		{ return new CLevel06Game; }		// 'LEV6' (0x68c bytes)
CGameObject *New_CLevel06AGame(void)	{ return new CLevel06AGame; }		// 'LV6A'
CGameObject *New_CLevel07Game(void)		{ return new CLevel07Game; }		// 'LEV7'
}
