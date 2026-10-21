/*  C3DSheen - see C3DSheen.h. Reconstructed from Neutron2.exe 0x46ecd0..0x46f120. */
#include "C3DSheen.h"
#include "../Game.h"
#include <string>

using namespace neg;

C3DSheen::C3DSheen()										// 0x46ecd0
{
	SetName((char *)"C3DSHEEN");
	Log("C3DSheen()");
	f_649 = 0;												// ev69 loads the animations later
	f_6d0 = 1;												// Granny
	FriendInitObjectAnim();									// 0x44ebc0 inlined: InitObject() called directly
	C3DObject::SetObjectID('3SHE');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 160.0f;											// speed
	f_96c = 2;												// patrolling
	f_c08 = 2;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 300.0f;
	if (!f_4aa) C3DObject::InitPhysics();					// 0x49acb0 (0x43ba20 inlined)
	C3DAI::ev106(f_ac4, (char *)"IDLE");					// running, waiting and sitting: IDLE
	C3DAI::ev106(f_a9c, (char *)"IDLE");
	C3DAI::ev106(f_b14, (char *)"IDLE");
	// (then a call to the empty 0x463890)
}

C3DSheen::~C3DSheen()										// 0x46eea0
{
}

namespace neg {
CGameObject *New_C3DSheen(void) { return new C3DSheen; }				// '3SHE' (0x1d7c bytes)
}

void C3DSheen::InitObject(void)		{ FriendInitObjectAnim(); }			// 0x44ebc0
void C3DSheen::InitPhysics(void)	{ if (!f_4aa) C3DObject::InitPhysics(); }	// 0x43ba20
void C3DSheen::v266(void)			{ FriendTaskStateRewrite(); }		// 0x4368e0
void C3DSheen::v267(int level)		{ FriendLevelSceneShow(level); }	// 0x471950
void C3DSheen::ev145(void)			{ FriendTouchScene(); }				// 0x44fa60

void C3DSheen::ResetObject(void)							// 0x46f020
{
	float p[4];
	v88(p);													// +0x160: the start position
	v196(p[0], p[1], p[2], p[3]);							// +0x310
	float q[4];
	v195(q); const float qz = q[2];							// +0x30c, three times (as shipped)
	v195(q); const float qy = q[1];
	v195(q);
	x = q[0];												// the element's position (2002 +0x44..)
	y = qy;
	z = qz;
	InitPhysics();											// +0x2c (slot 11)
	C3DFriends::ResetObject();								// 0x436d70
	f_b8c = NULL;											// no destination
}

void C3DSheen::ev69(void)									// 0x46eef0
{
	if (f_649) return;
	f_649 = 1;
	InitModel(std::string("sheenbase.grn"));				// element +0x144
	CreateAnim((char *)"IDLE", (char *)"sheenidle.grn");	// +0xe0
	ev58((char *)"IDLE", 1);								// +0xe8
	v67(150.0f);											// CGameObject +0x10c: radius
	v64(0.5f);												// CGameObject +0x100
}
