/*  The Retroville level game types - see CLevel01Games.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel01Games.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// --------------------------------------------------------------------------- CLevel01Game 'LEV1'

CLevel01Game::CLevel01Game()												// 0x488560
{
	static char name[] = "CLEVEL01GAME";
	SetName(name);
	Log("CLevel01Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV1');
	f_684 = 0;
	m_pickup_index_base = 800;
}

CLevel01Game::~CLevel01Game()												// 0x488690
{
	Log("~CLevel01Game()");
}

// 0x487160 (shared with L1_A, L1_B, L1_C, LV4B): only CJimmyGame's.
void CLevel01Game::Process(float dt)	{ CJimmyGame::Process(dt); }

void CLevel01Game::InitGame(void)											// 0x488720
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);												// not written to the .gam (f_00b)
	m_sky = CreateSky();
	if (m_sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(m_sky, "omt\\wintersky.omt", false);		// sky element +0xe8
		m_sky->ev51();										// element +0xcc
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();								// (empty for a C3DSky)
		m_sky->v67(1.0f);									// radius
		m_sky->v152(1.0f);
	}
	InitTerrainDatabase("level1.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.2f, 0.2f, 0.2f, 0.0f, 13000.0f, 17000.0f, 28000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01AGame 'LV1A'

CLevel01AGame::CLevel01AGame()												// 0x4871b0
{
	static char name[] = "CLEVEL01AGAME";
	SetName(name);
	Log("CLevel01AGame()");
	CGameType::InitObject();
	CGameType::SetID('LV1A');
	m_pickup_index_base = 200;
}

CLevel01AGame::~CLevel01AGame()												// 0x4872d0
{
	Log("~CLevel01AGame()");
}

void CLevel01AGame::InitGame(void)											// 0x487360
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	InitTerrainDatabase("level1a.omt");
	CreateTerrain();										// element +0xf4 (lit)
	ViewportToOrigin();
	SetFog(0.28f, 0.3f, 0.4f, 0.0f, 600.0f, 4500.0f, 5000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01BGame 'LV1B'

CLevel01BGame::CLevel01BGame()												// 0x4874f0
{
	static char name[] = "CLEVEL01AGAME";		// sic: level 1a's name
	SetName(name);
	Log("CLevel01BGame()");
	CGameType::InitObject();
	CGameType::SetID('LV1B');
	m_pickup_index_base = 300;
}

CLevel01BGame::~CLevel01BGame()												// 0x487610
{
	Log("~CLevel01BGame()");
}

void CLevel01BGame::InitGame(void)											// 0x4876a0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	InitTerrainDatabase("level1b.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 1.0f, 10000.0f, 12000.0f, 12000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01CGame 'LV1C'

CLevel01CGame::CLevel01CGame()												// 0x487830
{
	static char name[] = "CLEVEL01CGAME";
	SetName(name);
	Log("CLevel01CGame()");
	CGameType::InitObject();
	CGameType::SetID('LV1C');
	m_pickup_index_base = 400;
}

CLevel01CGame::~CLevel01CGame()												// 0x487950
{
	Log("~CLevel01Game()");						// sic
}

void CLevel01CGame::InitGame(void)											// 0x4879e0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	InitTerrainDatabase("level1c.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 1.0f, 10000.0f, 12000.0f, 12000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01EGame 'LV1E'

CLevel01EGame::CLevel01EGame()												// 0x487f60
{
	static char name[] = "CLEVEL01EGAME";
	SetName(name);
	Log("CLevel01EGame()");
	CGameType::InitObject();
	CGameType::SetID('LV1E');
	m_pickup_index_base = 1900;					// (the same as CLevel03Game's)
}

CLevel01EGame::~CLevel01EGame()												// 0x488080
{
}

// 0x4880c0. Quirk: it loads level 3's terrain, and sets no fog.
void CLevel01EGame::InitGame(void)
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	InitTerrainDatabase("level3.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01FGame 'LV1F'

CLevel01FGame::CLevel01FGame()												// 0x4881f0
{
	static char name[] = "CLEVEL01DGAME";		// sic: level 1d's name and log
	SetName(name);
	Log("CLevel01DGame()");
	CGameType::InitObject();
	CGameType::SetID('LV1F');
	m_pickup_index_base = 700;
}

CLevel01FGame::~CLevel01FGame()												// 0x488310
{
}

void CLevel01FGame::InitGame(void)											// 0x488350
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	C3DObject *sky = CreateSky();							// a local: no sky rotation
	if (sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(sky, "omt\\space.omt", false);
		sky->place(0.0f, 0.0f, 0.0f);
		sky->InitPhysics();
		sky->v67(1.0f);
		sky->v152(1.0f);
		sky->ev51();
	}
	InitTerrainDatabase("level1f.omt");
	CreateTerrain();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 0.0f, 2000.0f, 14000.0f, 15000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01_AGame 'L1_A'

CLevel01_AGame::CLevel01_AGame()											// 0x486580
{
	static char name[] = "CLEVEL01_AGAME";
	SetName(name);
	Log("CLevel01_AGame()");
	CGameType::InitObject();
	CGameType::SetID('L1_A');
	m_pickup_index_base = 800;
}

CLevel01_AGame::~CLevel01_AGame()											// 0x4866a0
{
	Log("~CLevel01_AGame()");
}

void CLevel01_AGame::Process(float dt)	{ CJimmyGame::Process(dt); }		// 0x487160

void CLevel01_AGame::InitGame(void)											// 0x486730
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	m_sky = CreateSky();
	if (m_sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(m_sky, "omt\\wintersky.omt", false);
		m_sky->ev51();
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();
		m_sky->v67(1.0f);
		m_sky->v152(1.0f);
	}
	m_sky_rotates = 0;
	InitTerrainDatabase("level1_A.omt");
	CreateTerrainTinted(1.0f, 1.0f, 1.0f, 1.0f);			// element +0x168
	ViewportToOrigin();
	SetFog(0.2f, 0.2f, 0.2f, 0.0f, 13000.0f, 17000.0f, 28000.0f, false);	// (fog off)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01_BGame 'L1_B'

CLevel01_BGame::CLevel01_BGame()											// 0x486990
{
	static char name[] = "CLEVEL01_BGAME";
	SetName(name);
	Log("CLevel01_BGame()");
	CGameType::InitObject();
	CGameType::SetID('L1_B');
	m_pickup_index_base = 800;
}

CLevel01_BGame::~CLevel01_BGame()											// 0x486ab0
{
	Log("~CLevel01_BGame()");
}

void CLevel01_BGame::Process(float dt)	{ CJimmyGame::Process(dt); }		// 0x487160

void CLevel01_BGame::InitGame(void)											// 0x486b40
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	m_sky = CreateSky();
	if (m_sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(m_sky, "omt\\dusksky.omt", false);
		m_sky->ev51();
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();
		m_sky->v67(1.0f);
		m_sky->v152(1.0f);
	}
	InitTerrainDatabase("level1_B.omt");
	CreateTerrainTinted(1.0f, 1.0f, 1.0f, 1.0f);
	ViewportToOrigin();
	SetFog(0.6f, 0.37f, 0.25f, 0.0f, 1000.0f, 5000.0f, 7500.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel01_CGame 'L1_C'

CLevel01_CGame::CLevel01_CGame()											// 0x486d90
{
	static char name[] = "CLEVEL01_CGAME";
	SetName(name);
	Log("CLevel01_CGame()");
	CGameType::InitObject();
	CGameType::SetID('L1_C');
	m_pickup_index_base = 800;
}

CLevel01_CGame::~CLevel01_CGame()											// 0x486eb0
{
	Log("~CLevel01_CGame()");
}

void CLevel01_CGame::Process(float dt)	{ CJimmyGame::Process(dt); }		// 0x487160

void CLevel01_CGame::InitGame(void)											// 0x486f40
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	m_sky = CreateSky();
	m_sky_rotates = 0;
	if (m_sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(m_sky, "omt\\nightsky.omt", false);
		m_sky->ev51();
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();
		m_sky->v67(1.0f);
		m_sky->v152(1.0f);
	}
	InitTerrainDatabase("level1_C.omt");
	CreateTerrainTinted(0.7f, 0.7f, 0.8f, 1.0f);
	ViewportToOrigin();
	SetFog(0.2f, 0.2f, 0.2f, 0.0f, 13000.0f, 17000.0f, 28000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevel01Game(void)		{ return new CLevel01Game; }		// 'LEV1' (0x690 bytes)
CGameObject *New_CLevel01AGame(void)	{ return new CLevel01AGame; }		// 'LV1A' (0x68c bytes)
CGameObject *New_CLevel01BGame(void)	{ return new CLevel01BGame; }		// 'LV1B'
CGameObject *New_CLevel01CGame(void)	{ return new CLevel01CGame; }		// 'LV1C'
CGameObject *New_CLevel01EGame(void)	{ return new CLevel01EGame; }		// 'LV1E'
CGameObject *New_CLevel01FGame(void)	{ return new CLevel01FGame; }		// 'LV1F'
CGameObject *New_CLevel01_AGame(void)	{ return new CLevel01_AGame; }		// 'L1_A'
CGameObject *New_CLevel01_BGame(void)	{ return new CLevel01_BGame; }		// 'L1_B'
CGameObject *New_CLevel01_CGame(void)	{ return new CLevel01_CGame; }		// 'L1_C'
}
