/*  The level 3 game types - see CLevel03Games.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel03Games.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// --------------------------------------------------------------------------- CLevel03Game 'LEV3'

CLevel03Game::CLevel03Game()												// 0x48a500
{
	static char name[] = "CLEVEL03GAME";
	SetName(name);
	Log("CLevel03Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV3');
	m_pickup_index_base = 1900;
}

CLevel03Game::~CLevel03Game()												// 0x48a620
{
	Log("~CLevel03Game()");
}

void CLevel03Game::InitGame(void)											// 0x48a6b0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);												// not written to the .gam (f_00b)
	InitTerrainDatabase("level3.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.05f, 0.227f, 0.533f, 0.0f, 500.0f, 10000.0f, 11500.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel03AGame 'LV3A'

CLevel03AGame::CLevel03AGame()												// 0x489560
{
	static char name[] = "CLEVEL03AGAME";
	SetName(name);
	Log("CLevel03AGame()");
	CGameType::InitObject();
	CGameType::SetID('LV3A');
	m_pickup_index_base = 1400;
}

CLevel03AGame::~CLevel03AGame()												// 0x489680
{
}

void CLevel03AGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070

void CLevel03AGame::InitGame(void)											// 0x4896c0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level3A.omt");
	v107(0);
	CreateTerrain();										// element +0xf4 (lit)
	m_sky = CreateSky();
	SkyLoad(m_sky, "omt\\bluesky3.omt", false);				// sky element +0xe8
	m_sky->place(0.0f, 0.0f, 0.0f);
	m_sky->InitPhysics();									// (empty for a C3DSky)
	m_sky->v67(1.0f);										// radius
	m_sky->v152(1.0f);
	m_sky->ev51();											// element +0xcc
	m_sky->HideAgain();										// element +0xdc: the sky is not drawn
	ViewportToOrigin();
	SetFog(0.3f, 0.76f, 0.85f, 0.0f, 500.0f, 9500.0f, 11500.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel03BGame 'LV3B'

CLevel03BGame::CLevel03BGame()												// 0x4898e0
{
	static char name[] = "CLEVEL03BGAME";
	SetName(name);
	Log("CLevel03BGame()");
	CGameType::InitObject();
	CGameType::SetID('LV3B');
	m_pickup_index_base = 1500;
}

CLevel03BGame::~CLevel03BGame()												// 0x489a00
{
}

void CLevel03BGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070

void CLevel03BGame::InitGame(void)											// 0x489a40
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level3B.omt");
	v107(0);
	CreateTerrain();
	ViewportToOrigin();
	SetFog(0.05f, 0.227f, 0.533f, 0.0f, 500.0f, 5000.0f, 7000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel03CGame 'LV3C'

CLevel03CGame::CLevel03CGame()												// 0x489bd0
{
	static char name[] = "CLEVEL03CGAME";
	SetName(name);
	Log("CLevel03CGame()");
	CGameType::InitObject();
	CGameType::SetID('LV3C');
	m_pickup_index_base = 1600;
}

CLevel03CGame::~CLevel03CGame()												// 0x489cf0
{
}

void CLevel03CGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070

void CLevel03CGame::InitGame(void)											// 0x489d30
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level3c.omt");
	v107(0);
	CreateTerrainUnLit();
	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL
	SkyLoad(sky, "omt\\skylevel5.omt", false);
	sky->place(0.0f, 0.0f, 0.0f);
	sky->InitPhysics();
	sky->v67(1.0f);
	sky->v152(1.0f);
	sky->ev51();
	ViewportToOrigin();
	SetFog(0.71f, 0.75f, 0.76f, 0.0f, 1000.0f, 7000.0f, 10000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel03DGame 'LV3D'

CLevel03DGame::CLevel03DGame()												// 0x489f10
{
	static char name[] = "CLEVEL03DGAME";
	SetName(name);
	Log("CLevel03DGame()");
	CGameType::InitObject();
	CGameType::SetID('LV3D');
	m_pickup_index_base = 1700;
}

CLevel03DGame::~CLevel03DGame()												// 0x48a030
{
}

void CLevel03DGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070

void CLevel03DGame::InitGame(void)											// 0x48a080
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level3D.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.71f, 0.75f, 0.76f, 0.0f, 1000.0f, 9000.0f, 9100.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel03EGame 'LV3E'

CLevel03EGame::CLevel03EGame()												// 0x48a210
{
	static char name[] = "CLEVEL03EGAME";
	SetName(name);
	Log("CLevel03EGame()");
	CGameType::InitObject();
	CGameType::SetID('LV3E');
	m_pickup_index_base = 1800;
}

CLevel03EGame::~CLevel03EGame()												// 0x48a330
{
}

void CLevel03EGame::UnInitObject(void)	{ CGameType::UnInitObject(); }		// 0x48a070

// 0x48a370. Quirk: level 3's terrain (level3.omt), as CLevel01EGame.
void CLevel03EGame::InitGame(void)
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level3.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.05f, 0.227f, 0.533f, 0.0f, 500.0f, 10000.0f, 11500.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevel03Game(void)		{ return new CLevel03Game; }		// 'LEV3' (0x68c bytes)
CGameObject *New_CLevel03AGame(void)	{ return new CLevel03AGame; }		// 'LV3A'
CGameObject *New_CLevel03BGame(void)	{ return new CLevel03BGame; }		// 'LV3B'
CGameObject *New_CLevel03CGame(void)	{ return new CLevel03CGame; }		// 'LV3C'
CGameObject *New_CLevel03DGame(void)	{ return new CLevel03DGame; }		// 'LV3D'
CGameObject *New_CLevel03EGame(void)	{ return new CLevel03EGame; }		// 'LV3E'
}
