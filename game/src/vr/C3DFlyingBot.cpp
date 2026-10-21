/*  C3DFlyingBot - see C3DFlyingBot.h. Reconstructed from Neutron2.exe 0x432b20..0x433340, 0x467a80. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFlyingBot.h"
#include "../C3DSprite.h"
#include "../CGameType.h"
#include "../InterfaceElement.h"
#include "../Physics.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DVector.h"
#pragma GCC diagnostic pop
#include <string>

using namespace neg;

// A shot's sprite element (game object view - 0xc8).
static C3DSprite *ShotSprite(CGameObject *o)	{ return o ? dynamic_cast<C3DSprite *>(o) : NULL; }

// --------------------------------------------------------------------------- construction

C3DFlyingBot::C3DFlyingBot()								// 0x432b20
{
	SetName((char *)"C3DFLYINGBOT");
	Log("C3DBot()");										// (sic)
	f_649 = 0;												// the subclass's ev69 loads its animations
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0 (called directly)
	// (then a call to the empty 0x463890)
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 400.0f;											// speed
	f_96c = 11;												// wander and attack
	f_c08 = 11;
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 0;
	v42(1);
	v40(1);
	v34(0);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_aec, (char *)"RUN");						// 0x411210
	C3DAI::ev106(f_ac4, (char *)"RUN");
	C3DAI::ev106(f_a9c, (char *)"STOP");
	C3DAI::ev106(f_b14, (char *)"STOP");
	C3DAI::ev106(f_b3c, (char *)"SHOOT");
	f_1b60 = 135.0f;
	f_bd4 = 1;												// hovering
	f_1b50 = 0;
	f_1b54 = 5.0f;
	f_1b58 = 0;												// port: not set by the original (zeroed memory)
	f_1b5c = f_968;
	f_1b64 = 0;
	m_show_on_radar = 1;									// (element +0x580)
	// (then a call to the empty 0x463890)
}

C3DFlyingBot::~C3DFlyingBot() {}							// 0x432d40

// --------------------------------------------------------------------------- CGameObject family

void C3DFlyingBot::InitObject(void)							// 0x467a80
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	// (then CGameObject's empty slot 13)
}

void C3DFlyingBot::v260(void)								// 0x433220
{
	C3DAI::v260();											// 0x4143a0
	InitAnim();												// element +0x110
	f_968 = 900.0f;
	f_1b5c = 900.0f;
	ev71(0);												// element +0x11c: opaque
	f_9a8 = 1700.0f;										// VisibleRange
	f_b98 = 120.0f;											// FOV
}

unsigned char C3DFlyingBot::v231(CGameObject *other)		// 0x433280
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3JIM' || id == '3EPR' || id == '3GOD' || id == '3WBL';
}

void C3DFlyingBot::AddTouching(CGameObject *other)			// 0x433050
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other->NameContains((char *)"C3DWATERBALLOON") || other->NameContains((char *)"C3DENERGYPROJECTILE"))
	{
		C3DSprite *shot = ShotSprite(other);
		if (f_1b64 <= 0)									// first hit: lit for 10 s
		{
			PlayGameSound(-1, std::string("vrzap"), false);	// 0x48f420
			other->v136(0);									// +0x220
			other->v132(0);									// +0x210
			if (shot) shot->HideAgain();					// sprite element +0xd8
			f_1b64 += 10.0f;								// 0x4cb1cc
			ApplyTexture(f_590, 1);							// the wire grid
			ev72(1.0f);										// element +0x120
		}
		else												// hit again while lit: knocked out
		{
			other->v136(0);
			other->v132(0);
			if (shot) shot->HideAgain();
			f_bd4 = 0;										// falls
			f_1b50 = f_1b60;
			v124(0);										// +0x1f0
			f_968 = 0;
			ev58((char *)"STOP", 1);						// element +0xe8
			ev107(1);										// element +0x1ac
			AddScore(20);									// 0x482b50
			PlayGameSound(-1, std::string("vrzap"), false);
			v40(1);											// +0xa0
			f_00a = 0;
			f_005 = 0;
		}
	}
	if (other == g_camera)									// touched the player: away
	{
		ev107(5);
		f_c10 = 3000.0f;
		f_c3c = 20.0f;
	}
}

void C3DFlyingBot::Process(float dt)						// 0x432d90
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAI::Process(dt);										// 0x411240
	if (!v135()) return;									// +0x21c

	if (f_1b64 > 0)
	{
		f_1b64 -= dt;
		if (f_1b64 <= 0)
		{
			f_1b64 = 0;
			ApplyTexture(f_590, 0);							// element +0x100
		}
	}

	if (f_1b50 > 0)											// knocked out: spinning
	{
		f_1b50 -= dt;
		AddRotation(dt * 200.0f, dt * 300.0f, dt * -200.0f);	// +0x330; 0x4cb21c, 0x4caa7c, 0x4db928
		if (f_1b50 <= 0)									// up again
		{
			f_1b50 = 0;
			f_1b54 = 0;
			f_bd4 = 1;
			v124(1);
			f_968 = f_1b5c;
			ev58((char *)"RUN", 1);
			ev107(f_c08);
			ev71(0);
			v136(1);
			v132(1);
			ApplyTexture(f_590, 0);
			ShowAgain();									// element +0xd8
		}
	}

	if (f_bd4 && f_96c != 1)
	{
		f_1b54 -= dt;
		if (f_1b54 <= 0)									// the ground check
		{
			float p[4];
			v195(p);										// +0x30c
			const OMedia3DVector a(p[0], p[1], p[2], p[3]);
			const OMedia3DVector b(p[0], p[1] - 600.0f, p[2], p[3]);	// 0x4d0ca0
			if (PhysicsClosestHit(a, b, false))				// 0x4b7640
			{
				v196(a.x, a.y + dt * 90.0f, a.z, a.w);		// +0x310; 0x4d0c9c
				v162(0, 200.0f, 0);							// +0x288
			}
			f_1b54 = 5.0f;
		}
	}
}
