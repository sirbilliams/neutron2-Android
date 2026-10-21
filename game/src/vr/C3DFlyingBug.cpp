/*  C3DFlyingBug - see C3DFlyingBug.h. Reconstructed from Neutron2.exe 0x433340..0x433720. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFlyingBug.h"
#include "../CGameType.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DFlyingBug::C3DFlyingBug()								// 0x433340
{
	SetName((char *)"C3DFLYINGBUG");
	C3DObject::SetObjectID('3BUG');							// 0x49ab00
	CGameObject::v67(300.0f);								// 0x4ac130: radius
	CGameObject::v40(0);									// 0x4abe80
	f_1b60 = 60000.0f;										// knocked out for good
	f_1b68 = 1.0f;
	m_show_on_radar = 1;									// (element +0x580)
}

C3DFlyingBug::~C3DFlyingBug() {}							// 0x433480

namespace neg {
CGameObject *New_C3DFlyingBug(void) { return new C3DFlyingBug; }		// '3BUG' (0x1b74 bytes)
}

// --------------------------------------------------------------------------- element family

void C3DFlyingBug::ev69(void)								// 0x4334d0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"RUN", (char *)"bugfly.ase");		// element +0xe0
	CreateAnim((char *)"STOP", (char *)"bugstop.ase");
	CreateTexture((char *)"bug01.png", 0);					// element +0xfc
	CreateTexture((char *)"bug01grid.png", 1);
	ev58((char *)"RUN", 1);									// element +0xe8
	ApplyTexture(f_590, 0);									// element +0x100
	v67(200.0f);											// +0x10c: radius
	v64(0.7f);												// +0x100
	m_show_on_radar = 1;
}

// --------------------------------------------------------------------------- CGameObject family

unsigned char C3DFlyingBug::v231(CGameObject *other)		// 0x433590
{
	if (other == g_camera) return 1;
	const int id = other->m_object_id;
	return id == '3PRO' || id == '3BUG' || id == '3EPR' || id == '3VRT';
}

void C3DFlyingBug::Process(float dt)						// 0x4335e0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DFlyingBot::Process(dt);								// 0x432d90
	if (f_1b50 <= 0) return;
	const float a = f_1b68 - dt * 0.33333334f;				// 0x4dc054
	f_1b68 = a;
	if (a <= 0)
	{
		HideAgain();										// element +0xdc
		v136(0);											// +0x220
		v132(0);											// +0x210
		return;
	}
	ev71(a);												// element +0x11c
	ev91(1);												// element +0x16c
	if (v133())												// +0x214
	{
		v136(0);
		v132(0);
	}
}
