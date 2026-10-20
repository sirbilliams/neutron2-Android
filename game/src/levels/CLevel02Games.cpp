/*  The level 2 game types - see CLevel02Games.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel02Games.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// --------------------------------------------------------------------------- CLevel02Game 'LEV2'

CLevel02Game::CLevel02Game()												// 0x4890d0
{
	static char name[] = "CLEVEL02GAME";
	SetName(name);
	Log("CLevel02Game()");
	CGameType::InitObject();
	CGameType::SetID('LEV2');
	m_pickup_index_base = 1200;
}

CLevel02Game::~CLevel02Game()												// 0x4891f0
{
	Log("~CLevel02Game()");
}

// 0x48a070 (shared by LEV2, LV3A..LV3E, LV4B, LV5A, VR01, VR02): a jump to CGameType's.
void CLevel02Game::UnInitObject(void)	{ CGameType::UnInitObject(); }

void CLevel02Game::InitGame(void)											// 0x489280
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level2.omt");
	v107(0);												// not written to the .gam (f_00b)
	float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
	m_sky = CreateSky();
	if (m_sky)
	{
		switch (GetSceneStage())							// element +0x16c
		{
			case 0:
				SkyLoad(m_sky, "omt\\bluesky3.omt", false);
				break;
			case 1:
				SkyLoad(m_sky, "omt\\dusksky.omt", false);
				r = 1.0f; g = 0.9f; b = 0.7f;
				break;
			case 2:
				m_sky_rotates = 0;
				SkyLoad(m_sky, "omt\\nightsky.omt", false);
				r = 0.7f; g = 0.7f; b = 1.0f;
				break;
		}
		m_sky->ev51();										// element +0xcc
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();								// (empty for a C3DSky)
		m_sky->v67(1.0f);									// radius
		m_sky->v152(1.0f);
	}
	CreateTerrainTinted(r, g, b, a);						// element +0x168
	ViewportToOrigin();
	CaptureMouse(true);
	// (no SetFog here)
}

// --------------------------------------------------------------------------- CLevel02AGame 'LV2A'

CLevel02AGame::CLevel02AGame()												// 0x488950
{
	static char name[] = "CLEVEL02AGAME";
	SetName(name);
	Log("CLevel02AGame()");
	CGameType::InitObject();
	CGameType::SetID('LV2A');
	m_pickup_index_base = 1000;
}

CLevel02AGame::~CLevel02AGame()												// 0x488a70
{
}

void CLevel02AGame::InitGame(void)											// 0x488ab0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);
	InitTerrainDatabase("level2a.omt");
	v107(0);												// (twice)
	CreateTerrainTinted(1.0f, 1.0f, 1.0f, 1.0f);
	ViewportToOrigin();
	g_layer_clear->set_clear_color(RGBA(0.1f, 0.1f, 0.1f, 0.0f));	// 2002 +0x10c (inline)
	SetFog(0.1f, 0.1f, 0.1f, 0.0f, 600.0f, 3300.0f, 3500.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevel02BGame 'LV2B'

CLevel02BGame::CLevel02BGame()												// 0x488c70
{
	static char name[] = "CLEVEL02BGAME";
	SetName(name);
	Log("CLevel02BGame()");
	CGameType::InitObject();
	CGameType::SetID('LV2B');
	m_pickup_index_base = 1100;
}

CLevel02BGame::~CLevel02BGame()												// 0x488d90
{
}

void CLevel02BGame::InitGame(void)											// 0x488dd0
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("level2b.omt");
	v107(0);
	float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
	m_sky = CreateSky();
	if (m_sky)
	{
		switch (GetSceneStage())
		{
			case 0:
				SkyLoad(m_sky, "omt\\bluesky3.omt", false);
				m_sky->ev51();
				break;
			case 1:
				SkyLoad(m_sky, "omt\\dusksky.omt", false);
				m_sky->ev51();
				r = 1.0f; g = 0.9f; b = 0.9f;
				break;
			case 2:
				m_sky_rotates = 0;
				SkyLoad(m_sky, "omt\\nightsky.omt", false);
				m_sky->ev51();
				r = 0.7f; g = 0.7f; b = 0.8f;
				break;
		}
		m_sky->place(0.0f, 0.0f, 0.0f);
		m_sky->InitPhysics();
		m_sky->v67(1.0f);
		m_sky->v152(1.0f);
	}
	CreateTerrainTinted(r, g, b, a);
	ViewportToOrigin();
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevel02Game(void)		{ return new CLevel02Game; }		// 'LEV2' (0x68c bytes)
CGameObject *New_CLevel02AGame(void)	{ return new CLevel02AGame; }		// 'LV2A'
CGameObject *New_CLevel02BGame(void)	{ return new CLevel02BGame; }		// 'LV2B'
}
