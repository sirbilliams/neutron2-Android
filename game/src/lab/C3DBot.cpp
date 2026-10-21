/*  C3DBot - see C3DBot.h. Reconstructed from Neutron2.exe 0x422250..0x422cf0, 0x45b1f0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DBot.h"
#include "C3DShock.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Physics.h"
#include "../gen/globals.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop

using namespace neg;

C3DBot::C3DBot()											// 0x422250
{
	SetName((char *)"C3DBOT");
	Log("C3DBot()");
	f_649 = 0;												// ev69 loads the animations later
	C3DBot::InitObject();									// 0x422690 inlined
	C3DObject::SetObjectID('3BOT');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 400.0f;											// speed
	f_96c = 11;												// wandering, attacking what it sees
	f_c08 = 11;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 0;
	v42(1);
	v40(1);
	v34(0);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_aec, (char *)"RUN");						// flying is running
	C3DAI::ev106(f_ac4, (char *)"RUN");
	C3DAI::ev106(f_a9c, (char *)"STOP");
	C3DAI::ev106(f_b14, (char *)"STOP");
	C3DAI::ev106(f_b3c, (char *)"SHOOT");					// walking is shooting
	f_1b58 = new C3DShock;									// 0x6b4 bytes
	f_1b58->v107(0);										// CGameObject +0x1ac
	f_1b5c = f_968;
	f_bd4 = 1;												// hovering
	f_1b50 = 0;
	f_1b54 = 5.0f;
	m_show_on_radar = 1;									// CLocalGameObject +0x4c0
	v136(1);
	v132(1);
	// (then a call to the empty 0x463890)
}

C3DBot::~C3DBot()											// 0x422530
{
	// (the empty InterfaceElt destructors, then ~C3DAI; the shock is not deleted)
}

namespace neg {
CGameObject *New_C3DBot(void) { return new C3DBot; }					// '3BOT' (0x1d70 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DBot::InitObject(void)								// 0x422690
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	RegisterProperty("TargOffX", &f_978.x, PROP_FLOAT, 0);
	RegisterProperty("TargOffY", &f_978.y, PROP_FLOAT, 0);
	RegisterProperty("TargOffZ", &f_978.z, PROP_FLOAT, 0);
	// (then a call to the empty 0x463890)
}

void C3DBot::v260(void)										// 0x422700
{
	C3DAI::v260();											// 0x4143a0
	InitAnim();												// element +0x110
	if (g_difficulty == 0)									// 0x53b5b0
	{
		f_1b5c = f_1b3c * 0.4f;								// InitialSpeed; 0x4d0c98
		f_968 = f_1b5c;
	}
	else if (g_difficulty == 1)
	{
		f_1b5c = f_1b3c * 0.7f;								// 0x4cceec
		f_968 = f_1b5c;
	}
	else if (g_difficulty == 2)
	{
		f_1b5c = f_1b3c;
		f_968 = f_1b3c;
	}
}

void C3DBot::AddTouching(CGameObject *other)				// 0x422780
{
	C3DAI::AddTouching(other);								// 0x415150
	if (!other->NameContains((char *)"C3DWATERBALLOON") || f_1b50 > 0) return;
	f_bd4 = 0;												// it drops
	f_1b50 = 60000.0f;										// stunned
	v124(0);												// +0x1f0
	f_968 = 0;
	ev107(f_c08);											// element +0x1ac: AIState
	ev58((char *)"SHOOT", 1);								// element +0xe8
	f_1b58->f_694 = 1;										// the shock plays (port: the original does not check the pointer)
	f_1b58->ShowAgain();									// sprite +0xd4
	AddScore(20);											// 0x482b50
	f_00a = 0;
	f_005 = 0;
}

void C3DBot::v222(void)										// 0x422860
{
	// (debug log, empty)
	if (f_1b50 <= 0) return;
	v136(0);												// +0x220
	HideAgain();											// element +0xdc
	if (!f_1b58->f_694) return;
	f_1b58->HideAgain();									// sprite +0xd8
	f_1b58->f_694 = 0;
}

unsigned char C3DBot::v231(CGameObject *other)				// 0x422b90
{
	if (g_camera == other) return 1;
	switch (other->m_object_id)
	{
		case '3BOT': case '3EPR': case '3SWN':	return 1;
		case '3JIM': case '3GOD': case '3WBL':	break;
		default:								return 0;
	}
	if (f_1b50 > 0) return 0;								// stunned: no collisions with these
	return 1;
}

void C3DBot::Process(float dt)								// 0x4228f0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAI::Process(dt);										// 0x411240
	if (f_1b50 > 0)
	{
		f_1b50 -= dt;
		if (f_1b50 <= 0)									// back on its feet
		{
			f_1b50 = 0;
			f_1b54 = 5.0f;
			f_bd4 = 1;
			v124(1);
			f_968 = f_1b5c;
			ev58((char *)"RUN", 1);
			f_00a = 1;
			f_005 = 1;
			if (f_c0c != -1) ev107(f_c0c);					// FallbackAIState
		}
	}
	if (f_1b58)
	{
		float p[4];
		v195(p);											// +0x30c
		f_1b58->v196(p[0], p[1], p[2], p[3]);				// the shock with us
		if (f_1b58->f_694) AddRotation(dt * 80.0f, dt * 400.0f, dt * 20.0f);	// +0x330: spinning
	}
	if (f_bd4 && f_96c != 1 && f_1b54 > -3.0f)
	{
		f_1b54 -= dt;
		if (f_1b54 < 0)
		{
			float q[4];
			v195(q);
			const OMedia3DVector a(q[0], q[1], q[2], q[3]);
			const OMedia3DVector b(q[0], q[1] - 600.0f, q[2], q[3]);	// 0x4d0ca0
			if (PhysicsClosestHit(a, b, false))				// 0x4b7640: ground within 600 below
				v196(q[0], dt * 90.0f + q[1], q[2], q[3]);	// rise (0x4d0c9c)
			if (f_1b54 < -3.0f) f_1b54 = 5.0f;				// 0x4d0ca4
		}
	}
}

void C3DBot::WriteState(OMediaStreamOperators &s)			// 0x422c40
{
	C3DAI::WriteState(s);									// 0x415760
	s << f_1b50;
	bool h = f_bd4 != 0;									// (a bool in the stream)
	s << h;
}

void C3DBot::ReadState(OMediaStreamOperators &s)			// 0x422c00
{
	C3DAI::ReadState(s);									// 0x4155e0
	s >> f_1b50;
	bool h;
	s >> h;
	f_bd4 = h;
}

// --------------------------------------------------------------------------- element family

void C3DBot::ev58(char *name, unsigned char loop)			// 0x45b1f0
{
	C3DAnimated::ev58(name, loop);							// 0x41bea0
}

void C3DBot::ev69(void)										// 0x4225d0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"SHOOT", (char *)"goofbotshoot.ase");	// +0xe0
	CreateAnim((char *)"RUN", (char *)"goofbootfly.ase");
	CreateAnim((char *)"STOP", (char *)"goofbotstop.ase");
	CreateTexture((char *)"goofbot.png", 0);				// +0xfc
	ev58((char *)"RUN", 1);									// +0xe8
	ApplyTexture(f_590, 0);									// +0x100
	v67(100.0f);											// CGameObject +0x10c: radius
	v64(0.7f);												// CGameObject +0x100
}
