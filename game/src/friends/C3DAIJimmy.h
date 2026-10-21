/*  C3DAIJimmy ('3AIJ') - Jimmy as a computer-controlled friend, for the cut scenes: a Granny
    character with Jimmy's model ("jimmybase.grn") and his story animations (TALK, TYPE, DRIVE,
    WORRY*, EXCITE*, PROUD, YAWN, ORDER, HANGTALK, LEVER, TIME, TIMETRAV ...), patrolling
    (AIState 2) at 160. A cut scene that finds him stopped makes him IDLE; when it hands over to
    play (mode 3 -> 5) his fades are ended and he is shown, opaque.
    Reconstructed from Neutron2.exe: constructor 0x416af0 (factory '3AIJ', 0x1d7c bytes),
    destructor 0x416cc0 (inlined chain of the bases; deleting thunks 0x416c90 / 0x417150 /
    0x417160 / 0x417170).

    Original layout: as C3DFriends (element vtable 0x4cc56c, 147 slots: 60 overridden; listener
    0x4cc558; OMediaAnim 0x4cc51c; CLocalGameObject vtable 0x4cc0a0 at +0xc0), no members of
    its own (vtordisp 0x1d74, streamer 0x1d78).

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code of the original sees them at offset - 0xc0).            */
#pragma once
#include "C3DFriends.h"

class C3DAIJimmy : public C3DFriends
{
	public:
	C3DAIJimmy();															// 0x416af0
	virtual ~C3DAIJimmy();													// 0x416cc0

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60 0x4369f0 no such animation: IDLE

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x416d10 "C3DAIJimmy::InitObject()": the model and the animations
	virtual void v260(void);												// SLOT 260 0x417110
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x416fe0 "C3DAIJimmy::ChangeState"
};
