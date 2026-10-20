/*  The bonus level game types - see CLevelBonusGames.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevelBonusGames.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

// --------------------------------------------------------------------------- CLevelPromotion 'LPRO'

CLevelPromotion::CLevelPromotion()											// 0x48da20
{
	static char name[] = "CLEVELPROMOTION";
	SetName(name);
	Log("CLevelPromotion()");
	CGameType::InitObject();
	CGameType::SetID('LPRO');
	m_pickup_index_base = 800;
}

CLevelPromotion::~CLevelPromotion()											// 0x48db40
{
}

void CLevelPromotion::InitGame(void)										// 0x48db80
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);												// not written to the .gam (f_00b)
	InitTerrainDatabase("Bonus01.omt");
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 0.0f, 13000.0f, 17000.0f, 28000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevelVR01 'VR01'

CLevelVR01::CLevelVR01()													// 0x48dcc0
{
	static char name[] = "CLEVELVR01";
	SetName(name);
	Log("CLevelVR01()");
	CGameType::InitObject();
	CGameType::SetID('VR01');
	m_pickup_index_base = 3200;
}

CLevelVR01::~CLevelVR01()													// 0x48dde0
{
}

void CLevelVR01::UnInitObject(void)	{ CGameType::UnInitObject(); }			// 0x48a070

void CLevelVR01::InitGame(void)												// 0x48de20
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("vr01.omt");
	v107(0);
	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL
	SkyLoad(sky, "omt\\skyvr01.omt", false);				// sky element +0xe8
	sky->place(0.0f, 0.0f, 0.0f);
	sky->InitPhysics();										// (empty for a C3DSky)
	sky->v67(1.0f);											// radius
	sky->v152(1.0f);
		// (no element slot 51 here, as for level 5c's sky)
	CreateTerrainUnLit();
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 0.0f, 16000.0f, 19500.0f, 20000.0f, true);
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- CLevelVR02 'VR02'

CLevelVR02::CLevelVR02()													// 0x48dff0
{
	static char name[] = "CLEVELVR02";
	SetName(name);
	Log("CLevelVR02()");
	CGameType::InitObject();
	CGameType::SetID('VR02');
	m_pickup_index_base = 3300;
}

CLevelVR02::~CLevelVR02()													// 0x48e110
{
}

void CLevelVR02::UnInitObject(void)	{ CGameType::UnInitObject(); }			// 0x48a070

// 0x48e150. Quirk: the fog ends at 33000 through one world-layer global and at 30000 through
// the other; both are the same layer, so the second (30000) stays.
void CLevelVR02::InitGame(void)
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	InitTerrainDatabase("vr02.omt");
	v107(0);
	CreateTerrainUnLit();
	ViewportToOrigin();
	const OMediaFARGBColor c = RGBA(0.031f, 0.34f, 0.58f, 0.0f);
	g_layer_clear->set_clear_color(c);						// 2002 +0x10c
	WorldFog(g_layer_world, c, 50000.0f, 20000.0f, 33000.0f);	// (0x56afc8)
	WorldFog(g_layer_world, c, 50000.0f, 20000.0f, 30000.0f);	// (0x56afd4)
	CaptureMouse(true);
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLevelPromotion(void)	{ return new CLevelPromotion; }	// 'LPRO' (0x68c bytes)
CGameObject *New_CLevelVR01(void)		{ return new CLevelVR01; }			// 'VR01'
CGameObject *New_CLevelVR02(void)		{ return new CLevelVR02; }			// 'VR02'
}
