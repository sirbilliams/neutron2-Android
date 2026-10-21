/*  C3DCindy - see C3DCindy.h. Reconstructed from Neutron2.exe 0x427600..0x427b60. */
#include "C3DCindy.h"
#include "../Game.h"
#include <string>

using namespace neg;

C3DCindy::C3DCindy()										// 0x427600
{
	SetName((char *)"C3DCINDY");
	Log("C3DCindy()");
	f_649 = 0;												// ev69 loads the animations later
	f_6d0 = 1;												// Granny
	FriendInitObjectAnim();									// 0x44ebc0 inlined: InitObject() called directly
	C3DObject::SetObjectID('3CIN');							// 0x49ab00
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
	f_934 = -1;												// (already -1)
	f_1d60.y = -80.0f;										// the shadow
	// (then a call to the empty 0x463890)
}

C3DCindy::~C3DCindy()										// 0x4277e0
{
}

namespace neg {
CGameObject *New_C3DCindy(void) { return new C3DCindy; }				// '3CIN' (0x1d7c bytes)
}

void C3DCindy::InitObject(void)		{ FriendInitObjectAnim(); }			// 0x44ebc0
void C3DCindy::v260(void)			{ FriendPropsLoadedShow(); }		// 0x417110
void C3DCindy::ev60(void)			{ FriendNoAnimIdle(); }				// 0x4369f0

void C3DCindy::v270(int old_mode, int new_mode)				// 0x427a40
{
	FriendChangeState(old_mode, new_mode, "IDLE");			// "C3DCindy::ChangeState %d  %d"
}

void C3DCindy::ev69(void)									// 0x427830
{
	if (f_649) return;
	f_649 = 1;
	InitModel(std::string("cindybase.grn"));				// element +0x144
	CreateAnim((char *)"WALK", (char *)"cindywalk.grn");	// +0xe0
	CreateAnim((char *)"TALK", (char *)"cindytalk.grn");
	CreateAnim((char *)"STOP", (char *)"cindystop.grn");
	CreateAnim((char *)"LOVE", (char *)"cindyinlove.grn");
	CreateAnim((char *)"LOVETALK", (char *)"cindylovetalk.grn");
	CreateAnim((char *)"LOVEIDLE", (char *)"cindyloveidle.grn");
	CreateAnim((char *)"DISGTALK", (char *)"cindytalkdis.grn");
	CreateAnim((char *)"IDLE", (char *)"cindyidle.grn");
	CreateAnim((char *)"TALKFLURP", (char *)"cindytalkflurp.grn");
	CreateAnim((char *)"DRINKFLURP", (char *)"cindydrinkflurp.grn");
	CreateAnim((char *)"HOLDFLURP", (char *)"cindyholdflurp.grn");
	ev58((char *)"STOP", 1);								// +0xe8
	v67(60.0f);												// CGameObject +0x10c: radius
	v64(1.45f);												// CGameObject +0x100
}
