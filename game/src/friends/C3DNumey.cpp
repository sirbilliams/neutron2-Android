/*  C3DNumey - see C3DNumey.h. Reconstructed from Neutron2.exe 0x45ab70..0x45b270. */
#include "C3DNumey.h"
#include "../Game.h"
#include "../CGameType.h"
#include "../Globals.h"
#include <string>

using namespace neg;

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'L1_A').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

C3DNumey::C3DNumey()										// 0x45ab70
{
	SetName((char *)"C3DNUMEY");
	Log("C3DNumey()");
	f_6d0 = 1;												// Granny
	f_649 = 0;												// ev69 loads the animations later
	FriendInitObjectAnim();									// 0x44ebc0 inlined: InitObject() called directly
	C3DObject::SetObjectID('3NUM');							// 0x49ab00
	f_9a8 = 1000.0f;										// VisibleRange
	f_968 = 160.0f;											// speed
	f_96c = 1;												// waiting
	f_c08 = 1;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 300.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_a9c, (char *)"STOP");					// waiting and sitting are stopping
	C3DAI::ev106(f_b14, (char *)"STOP");
	v40(1);													// 0x4abe80
	v42(1);													// 0x4abeb0
	v50(0);													// 0x4abf70
	v77(0, 1.0f, 0);										// 0x4ac290
	// (then a call to the empty 0x463890)
}

C3DNumey::~C3DNumey()										// 0x45ad60
{
	// (debug log "Deleting C3DNumey")
}

namespace neg {
CGameObject *New_C3DNumey(void) { return new C3DNumey; }				// '3NUM' (0x1d7c bytes)
}

void C3DNumey::InitObject(void)		{ FriendInitObjectAnim(); }			// 0x44ebc0
void C3DNumey::v260(void)			{ FriendPropsLoadedShow(); }		// 0x417110
void C3DNumey::v266(void)			{ FriendTaskStateRewrite(); }		// 0x4368e0
void C3DNumey::v267(int level)		{ FriendLevelScene(level); }		// 0x436900

void C3DNumey::ev58(char *name, unsigned char loop)			// 0x45b1f0
{
	C3DAnimated::ev58(name, loop);							// 0x41bea0
}

void C3DNumey::ev60(void)									// 0x45b210
{
	ev58((char *)"SCHEME", 1);								// element +0xe8
}

void C3DNumey::Process(float dt)							// 0x45b0d0
{
	C3DFriends::Process(dt);								// 0x436eb0
}

void C3DNumey::AddTouching(CGameObject *other)				// 0x45b0e0
{
	C3DAI::AddTouching(other);								// 0x415150
	// (debug log "MEBTEMP Numey is touching %s", other's name)
}

void C3DNumey::v270(int old_mode, int new_mode)				// 0x45b120
{
	FriendChangeState(old_mode, new_mode, "SCHEME");		// "C3DNumey::ChangeState %d  %d"
}

void C3DNumey::ev69(void)									// 0x45adf0
{
	if (f_649) return;
	f_649 = 1;
	InitModel(std::string("nummeybase.grn"));				// element +0x144
	CreateAnim((char *)"TALK", (char *)"nummeytalk.grn");	// +0xe0
	if (LevelID() == 'LV4B')								// g_gametype +0x490
		CreateAnim((char *)"FLAIL", (char *)"NummyFlail.grn");
	else
		CreateAnim((char *)"WALK", (char *)"nummeywalk.grn");
	CreateAnim((char *)"STOP", (char *)"nummeystop.grn");
	CreateAnim((char *)"SCHEME", (char *)"nummeystop.grn");
	CreateAnim((char *)"SMALLSTOP", (char *)"nummeystop.grn");
	CreateAnim((char *)"LAUGH", (char *)"nummeylaugh.grn");
	CreateAnim((char *)"TYPE", (char *)"nummeypushbuttons.grn");
	CreateAnim((char *)"TALKAROG", (char *)"nummyaroganttalk.grn");
	CreateAnim((char *)"IDLEAROG", (char *)"nummyarogant.grn");
	CreateAnim((char *)"FLIP", (char *)"nummeyflipsinair.grn");
	CreateAnim((char *)"TALKMAD", (char *)"nummytalkmad.grn");
	CreateAnim((char *)"IDLEMAD", (char *)"nummyreallymad.grn");
	CreateAnim((char *)"LAUGHEVIL", (char *)"nummylaughfowl.grn");
	CreateAnim((char *)"FLIPS", (char *)"nummeyflipsinair.grn");
	CreateAnim((char *)"FLYAIR", (char *)"numflyair.grn");
	CreateAnim((char *)"IDLEMAD2", (char *)"nummymadidle.grn");
	CreateAnim((char *)"FOWLLAUGH", (char *)"nummylaughfowl.grn");
	CreateAnim((char *)"GIVE", (char *)"nummygiveflurp.grn");
	CreateAnim((char *)"DOOM", (char *)"nummeydoom.grn");
	ev58((char *)"STOP", 1);								// +0xe8
	v67(60.0f);												// CGameObject +0x10c: radius
	v64(1.265f);											// CGameObject +0x100
}
