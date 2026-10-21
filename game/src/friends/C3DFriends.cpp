/*  C3DFriends - see C3DFriends.h. Reconstructed from Neutron2.exe 0x436a50..0x436fe0 and the
    shared copies 0x44ebc0, 0x43e920, 0x417110, 0x4369f0, 0x4368e0, 0x436900, 0x471950, 0x44fa60,
    0x436920 / 0x427a40 / 0x45b120. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFriends.h"
#include "../C3DShadow.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DFriends::C3DFriends()									// 0x436a50
{
	f_1d60.w = 0;											// 0x1d6c
	f_bd4 = 0;												// no hovering
	v64(0.05f);												// 0x4ac0c0
	f_1b50[0] = 0;											// TalkState0..4
	f_1b50[1] = -1;
	f_1b50[2] = -1;
	f_1b50[3] = -1;
	f_1b50[4] = -1;
	for (int i = 0; i < 5; i++) strcpy(f_1b64[i], "none");	// TalkTrigger0..4
	f_1d58 = 0;
	f_1d5c = 100.0f;
	f_008 = 1;												// CGameObject +0x8
	f_c5d = 1;												// held when touched
	f_1d60.x = 0;											// the shadow 60 below our origin
	f_1d60.y = -60.0f;
	f_1d60.z = 0;
	if (g_windowed) f_1d70 = NULL;							// 0x56afab
	else f_1d70 = new C3DShadow;							// 0x60c bytes (the original keeps its CGameObject part)
	// (debug log "C3DShadow", 0x60c)
	if (f_1d70)
	{
		f_1d70->v36(1);										// CGameObject +0x90
		f_1d70->SetSize(90, 30);							// 0x46ec30
	}
}

C3DFriends::~C3DFriends()									// 0x436d20
{
	// (only the vtables; the shadow is not deleted)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DFriends::InitObject(void)							// 0x436da0
{
	C3DAI::InitObject();
	RegisterProperty("TalkState0", &f_1b50[0], PROP_LONG, 0);
	RegisterProperty("TalkTrigger0", f_1b64[0], PROP_ALPHA, 0);
	RegisterProperty("TalkState1", &f_1b50[1], PROP_LONG, 0);
	RegisterProperty("TalkTrigger1", f_1b64[1], PROP_ALPHA, 0);
	RegisterProperty("TalkState2", &f_1b50[2], PROP_LONG, 0);
	RegisterProperty("TalkTrigger2", f_1b64[2], PROP_ALPHA, 0);
	RegisterProperty("TalkState3", &f_1b50[3], PROP_LONG, 0);
	RegisterProperty("TalkTrigger3", f_1b64[3], PROP_ALPHA, 0);
	RegisterProperty("TalkState4", &f_1b50[4], PROP_LONG, 0);
	RegisterProperty("TalkTrigger4", f_1b64[4], PROP_ALPHA, 0);
}

void C3DFriends::ResetObject(void)							// 0x436d70
{
	C3DAI::ResetObject();
	f_1d58 = 0;
	f_1d5c = 100.0f;
}

void C3DFriends::Process(float dt)							// 0x436eb0
{
	C3DAI::Process(dt);
	if (f_8a8)												// culled this frame
	{
		if (f_1d70) f_1d70->HideAgain();					// sprite +0xd8
		return;
	}
	f_1d5c += dt;
	if (!f_1d70) return;
	f_1d70->ShowAgain();									// sprite +0xd4
	const OMedia3DVector p = v224(f_1d60.x, f_1d60.y, f_1d60.z);	// +0x380
	f_1d70->v196(p.x, p.y, p.z, p.w);						// +0x310
}

// --------------------------------------------------------------------------- element family

void C3DFriends::ev113(void)								// 0x436d90 (jmp 0x414630)
{
	C3DAI::ev113();
}

// --------------------------------------------------------------------------- shared copies

void C3DFriends::FriendInitObjectAnim(void)				// 0x44ebc0
{
	Log("InitObject()");
	C3DFriends::InitObject();								// 0x436da0
	InitAnim();												// element +0x110
	// (then a call to the empty 0x463890)
}

void C3DFriends::FriendInitObjectAIAnim(void)				// 0x43e920
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0 (sic: not C3DFriends', no Talk* properties)
	InitAnim();												// element +0x110
	// (then a call to the empty 0x463890)
}

void C3DFriends::FriendPropsLoadedShow(void)				// 0x417110
{
	C3DAI::v260();											// 0x4143a0
	ev70(1);												// element +0x118
}

void C3DFriends::FriendNoAnimIdle(void)					// 0x4369f0
{
	ev58((char *)"IDLE", 1);								// element +0xe8
}

void C3DFriends::FriendTaskStateRewrite(void)				// 0x4368e0
{
	// (debug log, empty)
	SetTaskState(m_task_name, GetTaskState(m_task_name));	// 0x497980 / 0x497a40 (as shipped: writes back what it read)
}

void C3DFriends::FriendLevelScene(int level)				// 0x436900
{
	GetTaskState("SCENE");									// 0x497a40 (the result is not used)
	C3DAnimated::v267(level);								// 0x41c880
}

void C3DFriends::FriendLevelSceneShow(int level)			// 0x471950
{
	C3DAnimated::v267(level);								// 0x41c880
	GetTaskState("SCENE");									// (the result is not used)
	ev70(1);												// element +0x118
}

void C3DFriends::FriendTouchScene(void)					// 0x44fa60
{
	// (debug log, empty)
	GetTaskState("SCENE");									// 0x497a40 (the result is not used)
}

// v270: when a cut scene starts (new mode 3) while we STOP, play <anim> instead and remember
// STOP in C3DPlayerObject's f_938 (f_934 = 1); when it ends (old mode 3), go back to it.
// C3DAnimated::v270 is not called.
void C3DFriends::FriendChangeState(int old_mode, int new_mode, const char *anim)	// 0x436920
{
	// (debug log "C3DFowl::ChangeState %d  %d", or Cindy's / Numey's)
	if (new_mode == 3 && f_934 == -1 && strcasecmp("STOP", GetAnim()) == 0)	// element +0xf4
	{
		strcpy(f_938, "STOP");
		f_934 = 1;
		ev58((char *)anim, 1);								// element +0xe8
	}
	if (old_mode == 3 && f_934 != -1)
	{
		ev58(f_938, 1);
		f_934 = -1;
	}
}
