/*  CLevel01DGame - see CLevel01DGame.h. Reconstructed from Neutron2.exe 0x487b70..0x487f50. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLevel01DGame.h"
#include "LevelHelpers.h"
#pragma GCC diagnostic pop

using namespace neg;
using namespace levels;

CLevel01DGame::CLevel01DGame()												// 0x487b70
{
	static char name[] = "CLEVEL01DGAME";		// SetName upper-cases in place
	SetName(name);
	Log("CLevel01DGame()");
	CGameType::InitObject();					// (direct call) -> InitGame
	CGameType::SetID('LV1D');
	m_pickup_index_base = 500;
	// (debug log, empty in the shipped build)
}

CLevel01DGame::~CLevel01DGame()												// 0x487c90
{
	Log("~CLevel01DGame()");
}

// 0x487d20. The original then logs "UI L2 dest 1", "UI L2 dest 2", "UI L2 dest 3" (debug
// calls, empty in the shipped build).
void CLevel01DGame::UnInitObject(void)
{
	CGameType::UnInitObject();
}

void CLevel01DGame::InitGame(void)											// 0x487d50
{
	Log("InitGame()");
	CJimmyGame::InitGame();
	v107(0);												// not written to the .gam (f_00b)

	C3DObject *sky = CreateSky();							// a local: m_sky stays NULL (no sky rotation)
	if (sky)												// port: no C3DSky yet; the original does not test
	{
		SkyLoad(sky, "omt\\skylevel1d.omt", false);			// sky element +0xe8
		sky->place(0.0f, 0.0f, 0.0f);
		sky->InitPhysics();									// (empty for a C3DSky)
		sky->v67(1.0f);										// radius
		sky->v152(1.0f);
		sky->ev51();										// element +0xcc
	}

	InitTerrainDatabase("level1d.omt");						// element +0xe4
	CreateTerrainUnLit();									// element +0xf8
	ViewportToOrigin();
	SetFog(0.0f, 0.0f, 0.0f, 1.0f, 120000.0f, 130000.0f, 130000.0f, true);	// element +0x194
	CaptureMouse(true);										// 0x4a3930
	// (debug log)
}

namespace neg {
CGameObject *New_CLevel01DGame(void)	{ return new CLevel01DGame; }		// factory 'LV1D' (0x68c bytes)
}
