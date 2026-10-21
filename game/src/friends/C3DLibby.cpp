/*  C3DLibby - see C3DLibby.h. Reconstructed from Neutron2.exe 0x44f720..0x44fbb0. */
#include "C3DLibby.h"
#include "../Game.h"

using namespace neg;

C3DLibby::C3DLibby()										// 0x44f720
{
	SetName((char *)"C3DLIBBY");
	Log("C3DLibby()");
	f_649 = 0;												// ev69 loads the animations later (morph: f_6d0 stays 0)
	FriendInitObjectAnim();									// 0x44ebc0 inlined: InitObject() called directly
	C3DObject::SetObjectID('3LIB');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 200.0f;											// speed
	f_96c = 3;												// patrolling only
	f_c08 = 3;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 300.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"RUN");
	C3DAI::ev106(f_a9c, (char *)"STOP");					// waiting and sitting are stopping
	C3DAI::ev106(f_b14, (char *)"STOP");
	// (then a call to the empty 0x463890)
}

C3DLibby::~C3DLibby()										// 0x44f8e0
{
}

namespace neg {
CGameObject *New_C3DLibby(void) { return new C3DLibby; }				// '3LIB' (0x1d7c bytes)
}

void C3DLibby::InitObject(void)		{ FriendInitObjectAnim(); }			// 0x44ebc0
void C3DLibby::v266(void)			{ FriendTaskStateRewrite(); }		// 0x4368e0
void C3DLibby::v267(int level)		{ FriendLevelSceneShow(level); }	// 0x471950
void C3DLibby::ev145(void)			{ FriendTouchScene(); }				// 0x44fa60

void C3DLibby::ev147(void)									// 0x44fb20
{
	ApplyTexture(f_590, 1);									// element +0x100: libbywire.png
}

void C3DLibby::ev148(void)									// 0x44fb40
{
	ApplyTexture(f_590, 0);									// libby.png
}

void C3DLibby::v260(void)									// 0x44fa30
{
	C3DAI::v260();											// 0x4143a0
	ApplyTexture(f_590, 0);									// element +0x100
}

void C3DLibby::ResetObject(void)							// 0x44fa80 (as C3DSheen's, without InitPhysics)
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
	C3DFriends::ResetObject();								// 0x436d70
	f_b8c = NULL;											// no destination
}

void C3DLibby::ev69(void)									// 0x44f930
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"TALK", (char *)"libytalk.ase");		// +0xe0
	CreateAnim((char *)"WALK", (char *)"libywalk.ase");
	CreateAnim((char *)"RUN", (char *)"libyrun.ase");
	CreateAnim((char *)"DIGITIZE", (char *)"libbyvox.ase");
	CreateAnim((char *)"FREEZE", (char *)"libyvoxstop.ase");
	CreateAnim((char *)"STOP", (char *)"libystop.ase");
	CreateTexture((char *)"libby.png", 0);					// +0xfc
	CreateTexture((char *)"libbywire.png", 1);
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"STOP", 1);								// +0xe8
	v67(150.0f);											// CGameObject +0x10c: radius
	v64(0.5f);												// CGameObject +0x100
}
