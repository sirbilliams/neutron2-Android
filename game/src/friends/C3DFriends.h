/*  C3DFriends - the base of Jimmy's friends and the other talking characters of the story
    (C3DFowl '3FOW', C3DSheen '3SHE', C3DLibby '3LIB', C3DCarl '3CAR', C3DCindy '3CIN',
    C3DNick '3NIC', C3DBenny '3BEN', C3DAIJimmy '3AIJ', C3DNumey '3NUM'): a C3DAI that does not
    hover, with a blob shadow (a C3DShadow, 90 x 30, placed at v224(f_1d60) every frame, not made
    in windowed mode) and five TalkState / TalkTrigger property pairs (read by the cut scene code).
    No factory ID of its own.
    Reconstructed from Neutron2.exe: constructor 0x436a50, destructor 0x436d20 (only resets the
    vtables, then ~C3DAI; deleting thunks 0x436cf0 / 0x436fb0 / 0x436fc0 / 0x436fd0).

    Original layout (classes.json): as C3DAI (element vtable 0x4de008, 147 slots: C3DAI's 144 +
    144..146; OMediaAnim at +0x90, vtable 0x4ddfb8; listener +0x30, vtable 0x4ddff4;
    CLocalGameObject at +0xc0, vtable 0x4ddb3c, 286 slots), own members 0x1b50..0x1d74,
    vtordisp 0x1d74, streamer 0x1d78 (size 0x1d7c). Derived classes put theirs from 0x1d74 on.
    Element slots 114 and 126 point at 0x456d10 (a jump to the empty 0x463890): the same as
    C3DAI's, so they are not overridden here; slot 113 is a jump to C3DAI's (declared anyway).
    The streamer vtable's read_class / write_class (0x417180 / 0x417190) are jumps to
    OMedia3DShapeElement's, as in C3DAI.

    Shared compiled code: several subclass overrides are one function in the exe, used by
    several classes (MSVC folded identical code). They are the protected helpers below
    (Friend*), each named after what it does with its address; a subclass override that is such
    a copy just calls the helper (e.g. C3DFowl::InitObject() { FriendInitObjectAnim(); }).

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code of the original sees them at offset - 0xc0).            */
#pragma once
#include "../C3DAI.h"
#include "OMedia3DVector.h"

class C3DShadow;

class C3DFriends : public C3DAI
{
	public:
	C3DFriends();															// 0x436a50
	virtual ~C3DFriends();													// 0x436d20

	// ---- element-family slots
	virtual void ev113(void);												// ESLOT 113 0x436d90 (a jump to C3DAI's)
	virtual unsigned char ev144(void) { return 0; }							// ESLOT 144 0x436f80 (0)
	virtual void ev145(void) {}												// ESLOT 145 0x463890 (empty; Sheen/Libby/Benny: FriendTouchScene, Nick his own)
	virtual void ev146(int unused) { (void)unused; }						// ESLOT 146 0x464ee0 (empty, one stack argument: type unknown)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x436da0 TalkState0..4 / TalkTrigger0..4
	virtual void ResetObject(void);											// SLOT 10  0x436d70
	virtual void Process(float dt);											// SLOT 240 0x436eb0 timer f_1d5c, the shadow under us

	protected:
	// ---- shared compiled copies (see above)
	void FriendInitObjectAnim(void);										// 0x44ebc0 SLOT 7: C3DFriends::InitObject, then InitAnim
	void FriendInitObjectAIAnim(void);										// 0x43e920 SLOT 7: C3DAI::InitObject (skips C3DFriends'!), then InitAnim
	void FriendPropsLoadedShow(void);										// 0x417110 SLOT 260: C3DAI::v260, then ev70(1)
	void FriendNoAnimIdle(void);											// 0x4369f0 ESLOT 60: ev58("IDLE", 1)
	void FriendTaskStateRewrite(void);										// 0x4368e0 SLOT 266: SetTaskState(TaskName, GetTaskState(TaskName))
	void FriendLevelScene(int level);										// 0x436900 SLOT 267: GetTaskState("SCENE") (unused), C3DAnimated::v267
	void FriendLevelSceneShow(int level);									// 0x471950 SLOT 267: C3DAnimated::v267, GetTaskState("SCENE") (unused), ev70(1)
	void FriendTouchScene(void);											// 0x44fa60 ESLOT 145: GetTaskState("SCENE") (unused)
	void FriendChangeState(int old_mode, int new_mode, const char *anim);	// 0x436920 (Fowl "TALK"), 0x427a40 (Cindy "IDLE"),
																			// 0x45b120 (Numey "SCHEME") SLOT 270: a cut scene interrupts STOP with <anim>

	public:
	// ---- data members (object offsets)
	int					f_1b50[5];		// 0x1b50 "TalkState0".."TalkState4" (0, -1, -1, -1, -1)
	char				f_1b64[5][100];	// 0x1b64 "TalkTrigger0".."TalkTrigger4" ("none"; 0x1b64 0x1bc8 0x1c2c 0x1c90 0x1cf4)
	short				f_1d58;			// 0x1d58 0 (ResetObject)
	float				f_1d5c;			// 0x1d5c timer: 100 (ResetObject), += dt while not culled
	OMedia3DVector		f_1d60;			// 0x1d60 where the shadow goes, in our frame (0, -60, 0; w 0 at 0x1d6c)
	C3DShadow			*f_1d70;		// 0x1d70 blob shadow (90 x 30; NULL in windowed mode)
};
