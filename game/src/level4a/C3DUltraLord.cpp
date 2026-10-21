/*  C3DUltraLord - see C3DUltraLord.h. Reconstructed from Neutron2.exe 0x47a330..0x47a6d0,
    0x44ebc0 / 0x4368e0 / 0x44fa60 (shared). */
#include "C3DUltraLord.h"
#include "../Game.h"
#include "../SaveGame.h"

using namespace neg;

C3DUltraLord::C3DUltraLord()								// 0x47a330
{
	SetName((char *)"C3DULTRALORD");
	Log("C3DUltraLord()");
	f_649 = 0;												// ev69 loads the animations later
	FriendInitObjectAnim();									// 0x44ebc0 inlined: InitObject() called directly
	C3DObject::SetObjectID('3ULT');							// 0x49ab00
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

C3DUltraLord::~C3DUltraLord()								// 0x47a4f0
{
}

namespace neg {
CGameObject *New_C3DUltraLord(void) { return new C3DUltraLord; }		// '3ULT' (0x1d7c bytes)
}

void C3DUltraLord::InitObject(void)		{ FriendInitObjectAnim(); }		// 0x44ebc0
void C3DUltraLord::v266(void)			{ FriendTaskStateRewrite(); }	// 0x4368e0

void C3DUltraLord::v267(int level)							// 0x47a660
{
	C3DAnimated::v267(level);								// 0x41c880
	GetTaskState("SCENE");									// 0x497a40 (the result is not used)
}

void C3DUltraLord::ev69(void)								// 0x47a540
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"WALK", (char *)"ultrawalk.ase");	// +0xe0
	CreateAnim((char *)"FLEX1", (char *)"ultraflex.ase");
	CreateAnim((char *)"FLEX2", (char *)"ultraflex2.ase");
	CreateAnim((char *)"FLEX3", (char *)"ultraflex3.ase");
	CreateAnim((char *)"TALK", (char *)"ultratalk.ase");
	CreateAnim((char *)"GIVE", (char *)"ultragive.ase");
	CreateAnim((char *)"WHISPER", (char *)"ultrawhisper.ase");
	CreateAnim((char *)"STOP", (char *)"ultrastop.ase");
	CreateTexture((char *)"ultralord.png", 0);				// +0xfc
	ev58((char *)"STOP", 1);								// +0xe8
	ApplyTexture(f_590, 0);									// +0x100
	v67(150.0f);											// CGameObject +0x10c: radius
	v64(0.5f);												// CGameObject +0x100
}

void C3DUltraLord::ev145(void)								// 0x44fa60
{
	FriendTouchScene();										// GetTaskState("SCENE") (unused)
}
