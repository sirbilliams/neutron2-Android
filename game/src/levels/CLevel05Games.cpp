/*  The level 5 game types - see CLevel05Games.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel05Games.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// --------------------------------------------------------------------------- CLevel05Game 'LEV5'

CLevel05Game::CLevel05Game()												// 0x48cb50
{
	static char name[] = "CLEVEL05GAME";
	SetName(name);
	Log("CLevel05Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV5');
	m_pickup_index_base = 2800;
}

CLevel05Game::~CLevel05Game()												// 0x48cc70
{
	Log("~CLevel05Game()");
}

void CLevel05Game::InitGame(void)											// 0x48cd00
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);												// not written to the .gam (f_00b)
	InitTerrainDatabase("level5.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.584f, 0.494f, 0.356f, 0.0f, 2000.0f, 10000.0f, 12000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel05AGame 'LV5A'

CLevel05AGame::CLevel05AGame()												// 0x48bd60
{
	static char name[] = "CLEVEL05AGAME";
	SetName(name);
	Log("CLevel05AGame()");
	CGameType::InitObject();
	CGameType::SetID('LV5A');
	m_pickup_index_base = 2500;
}

CLevel05AGame::~CLevel05AGame()												// 0x48be80
{
	Log("~CLevel05aGame()");
}

void CLevel05AGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070

void CLevel05AGame::InitGame(void)											// 0x48bf10
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level5a.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.827f, 0.76f, 0.545f, 0.0f, 1000.0f, 10000.0f, 13000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel05BGame 'LV5B'

CLevel05BGame::CLevel05BGame()												// 0x48c390
{
	static char name[] = "CLEVEL05BGAME";
	SetName(name);
	Log("CLevel05BGame()");
	CGameType::InitObject();
	CGameType::SetID('LV5B');
	m_pickup_index_base = 2600;
}

CLevel05BGame::~CLevel05BGame()												// 0x48c4b0
{
}

void CLevel05BGame::InitGame(void)											// 0x48c4f0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level5b.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 0.0f, 6000.0f, 16000.0f, 16000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel05B1Game 'L5B1'

CLevel05B1Game::CLevel05B1Game()											// 0x48c0a0
{
	static char name[] = "CLEVEL05B1GAME";
	SetName(name);
	Log("CLevel05B1Game()");
	CGameType::InitObject();
	CGameType::SetID('L5B1');
	m_pickup_index_base = 2600;					// (the same as LV5B's)
}

CLevel05B1Game::~CLevel05B1Game()											// 0x48c1c0
{
}

void CLevel05B1Game::InitGame(void)											// 0x48c200
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level5b1.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 0.0f, 6000.0f, 16000.0f, 16000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel05CGame 'LV5C'

CLevel05CGame::CLevel05CGame()												// 0x48c680
{
	static char name[] = "CLEVEL05CGAME";
	SetName(name);
	Log("CLevel05CGame()");
	CGameType::InitObject();
	CGameType::SetID('LV5C');
	m_pickup_index_base = 2700;
}

CLevel05CGame::~CLevel05CGame()												// 0x48c7a0
{
}

// 0x48c7e0. Unlike the other skies this one gets no element slot 51.
static void InitLevel5c(CJimmyGame *game)
{
	game->Log("InitGame()");
	game->CJimmyGame::InitGame();
	game->InitTerrainDatabase("level5c.omt");
	game->v107(0);											// not written to the .gam (f_00b)
	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL
	SkyLoad(sky, "omt\\skydowntown.omt", false);			// sky element +0xe8
	sky->place(0.0f, 0.0f, 0.0f);
	sky->InitPhysics();										// (empty for a C3DSky)
	sky->v67(1.0f);											// radius
	sky->v152(1.0f);
	game->CreateTerrainUnLit();
	ViewportToOrigin();
	game->SetFog(0.678f, 0.83f, 0.94f, 0.0f, 18000.0f, 20000.0f, 20000.0f, true);
	CaptureMouse(true);
}

void CLevel05CGame::InitGame(void)	{ InitLevel5c(this); }					// 0x48c7e0

// --------------------------------------------------------------------------- CLevel05FGame 'LV5F'

CLevel05FGame::CLevel05FGame()												// 0x48c9b0
{
	static char name[] = "CLEVEL05FGAME";
	SetName(name);
	Log("CLevel05FGame()");
	CGameType::InitObject();
	CGameType::SetID('LV5F');
	m_pickup_index_base = 2700;					// (the same as LV5C's)
}

CLevel05FGame::~CLevel05FGame()												// 0x48cad0
{
}

// Quirk: the exe has one function for both (0x48c7e0): level 5f loads level 5c's terrain, sky
// and fog.
void CLevel05FGame::InitGame(void)	{ InitLevel5c(this); }

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevel05Game(void)		{ return new CLevel05Game; }		// 'LEV5' (0x68c bytes)
CGameObject *New_CLevel05AGame(void)	{ return new CLevel05AGame; }		// 'LV5A'
CGameObject *New_CLevel05BGame(void)	{ return new CLevel05BGame; }		// 'LV5B'
CGameObject *New_CLevel05B1Game(void)	{ return new CLevel05B1Game; }		// 'L5B1'
CGameObject *New_CLevel05CGame(void)	{ return new CLevel05CGame; }		// 'LV5C'
CGameObject *New_CLevel05FGame(void)	{ return new CLevel05FGame; }		// 'LV5F'
}
