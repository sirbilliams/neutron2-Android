/*  C3DHugh - see C3DHugh.h. Reconstructed from Neutron2.exe 0x4419f0..0x441cf0. */
#include "C3DHugh.h"
#include "../Game.h"

using namespace neg;

C3DHugh::C3DHugh()											// 0x4419f0
{
	SetName((char *)"C3DHUGH");
	Log("C3DHugh()");
	C3DHugh::InitObject();									// 0x441bd0 called directly
	C3DObject::SetObjectID('3HUG');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 160.0f;											// speed
	f_96c = 2;												// patrolling
	f_c08 = 2;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 300.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_a9c, (char *)"STOP");					// waiting and sitting are stopping
	C3DAI::ev106(f_b14, (char *)"STOP");
	// (then a call to the empty 0x463890)
}

C3DHugh::~C3DHugh()											// 0x441b80
{
}

namespace neg {
CGameObject *New_C3DHugh(void) { return new C3DHugh; }					// '3HUG' (0x1d7c bytes)
}

void C3DHugh::v266(void)			{ FriendTaskStateRewrite(); }		// 0x4368e0

void C3DHugh::InitObject(void)								// 0x441bd0
{
	Log("InitObject()");
	C3DFriends::InitObject();								// 0x436da0
	InitAnim();												// element +0x110
	CreateAnim((char *)"TALK", (char *)"hughtalk.ase");		// +0xe0
	CreateAnim((char *)"WALK", (char *)"hughwalk.ase");
	CreateAnim((char *)"STOP", (char *)"hughstop.ase");
	CreateTexture((char *)"hugh.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"STOP", 1);								// +0xe8
	v67(150.0f);											// +0x10c: radius
	v64(0.5f);												// +0x100
	// (then a call to the empty 0x463890)
}

void C3DHugh::v267(int level)								// 0x425e60
{
	C3DAnimated::v267(level);								// 0x41c880
	ev70(1);												// element +0x118: shown
}
