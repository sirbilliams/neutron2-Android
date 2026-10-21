/*  C3DAIJimmy - see C3DAIJimmy.h. Reconstructed from Neutron2.exe 0x416af0..0x417180. */
#include "C3DAIJimmy.h"
#include "../Game.h"
#include <string>
#include <string.h>
#include <strings.h>

using namespace neg;

C3DAIJimmy::C3DAIJimmy()									// 0x416af0
{
	SetName((char *)"C3DAIJIMMY");
	Log("C3DAIJimmy()");
	f_649 = 0;
	f_6d0 = 1;												// Granny
	C3DAIJimmy::InitObject();								// 0x416d10 (called directly)
	C3DObject::SetObjectID('3AIJ');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 160.0f;											// speed
	f_96c = 2;												// patrolling
	f_c08 = 2;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 300.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_a9c, (char *)"IDLE");					// waiting and sitting are idling
	C3DAI::ev106(f_b14, (char *)"IDLE");
	v67(60.0f);												// CGameObject +0x10c: radius
	v64(1.365f);											// CGameObject +0x100
	f_1d60.y = -65.0f;										// the shadow 65 below our origin
	// (then a call to the empty 0x463890)
}

C3DAIJimmy::~C3DAIJimmy()									// 0x416cc0
{
}

namespace neg {
CGameObject *New_C3DAIJimmy(void) { return new C3DAIJimmy; }			// '3AIJ' (0x1d7c bytes)
}

void C3DAIJimmy::v260(void)			{ FriendPropsLoadedShow(); }		// 0x417110
void C3DAIJimmy::ev60(void)			{ FriendNoAnimIdle(); }				// 0x4369f0

void C3DAIJimmy::InitObject(void)							// 0x416d10
{
	Log("C3DAIJimmy::InitObject()");
	C3DAI::InitObject();									// 0x4110e0 (sic: not C3DFriends', no Talk* properties)
	InitModel(std::string("jimmybase.grn"));				// element +0x144
	static const char *const anims[][2] = {					// element +0xe0 CreateAnim(name, file)
		{ "WALK", "jimmywalk.grn" },
		{ "RUN", "jimmywalk.grn" },
		{ "TALK", "jimmyfulltalk.grn" },
		{ "TYPE", "jimmybuttons.grn" },
		{ "DRIVE", "jimmysit.grn" },
		{ "ANIMSTOP", "jimmyfullstop.grn" },
		{ "STOP", "jimmyfullstop.grn" },
		{ "TALKBODY", "jimmytalkbody.grn" },
		{ "TALKHERMAN", "jimmytalkherman.grn" },
		{ "WORRYIDLE", "jimmyscared.grn" },
		{ "WORRYTALK", "jimmyscaredtalk.grn" },
		{ "PROUD", "jimmyproud.grn" },
		{ "EXCITETALK", "jimmytalkexcited.grn" },
		{ "EXCITEIDLE", "jimmyExcited.grn" },
		{ "ARGGH", "jimmyAhhrrgg.grn" },
		{ "YAWN", "jimmyawn.grn" },
		{ "ORDER", "jimmyorderpizza.grn" },
		{ "IDLE", "jimmyidle.grn" },
		{ "HANGTALK", "jimmyspork.grn" },
		{ "LEVER", "jimmylever.grn" },
		{ "TIME", "jimmytime.grn" },
		{ "TIMETRAV", "jimmytimetravel.grn" },
	};
	for (size_t i = 0; i < sizeof(anims) / sizeof(anims[0]); i++)
		CreateAnim((char *)anims[i][0], (char *)anims[i][1]);
	ev58((char *)"STOP", 1);								// +0xe8
	f_649 = 1;												// the animations are loaded
	// (then a call to the empty 0x463890)
}

void C3DAIJimmy::v270(int old_mode, int new_mode)			// 0x416fe0
{
	// (debug log "C3DAIJimmy::ChangeState %d  %d"; C3DAnimated::v270 is not called)
	if (new_mode == 3 && f_934 == -1 && strcasecmp("STOP", GetAnim()) == 0)	// a cut scene: IDLE meanwhile
	{
		strcpy(f_938, "STOP");
		f_934 = 1;
		ev58((char *)"IDLE", 1);							// element +0xe8
	}
	if (old_mode != 3) return;
	if (new_mode == 5)										// back to play: end any fade, shown and opaque
	{
		f_8b0 = 0;											// no fades
		ShowAgain();										// element +0xd8
		f_891 = 0;
		f_890 = 0;
		f_898 = 0;
		ShowAgain();
		ev71(0);											// +0x11c: opaque
		ev91(0);											// +0x16c: first pass
	}
	if (f_934 != -1)
	{
		ev58(f_938, 1);
		f_934 = -1;
	}
}
