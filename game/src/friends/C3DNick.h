/*  C3DNick ('3NIC') - Nick, the cool kid: an OMT morph friend (nickwalk / nickcoin / nickstop
    .ase, texture "nick.png") who rides a C3DSkateBoard while he patrols (state 3). Every 5 s
    of patrolling his "running" switches between SKATE and GLIDE, and his speed drops with each
    lap Jimmy has made him do (f_1d7c). He is shown only in the story scenes of his levels
    (v267), and talking to him moves the SCENE task on (ev145).
    Reconstructed from Neutron2.exe: constructor 0x45a350 (factory '3NIC', 0x1d88 bytes),
    destructor 0x45a580 (deleting thunks 0x45a550 / 0x45ab40 / 0x45ab50 / 0x45ab60).

    Original layout: as C3DFriends (element vtable 0x4edff0, 147 slots: 69, 126, 145 overridden;
    listener 0x4edfdc; OMediaAnim 0x4edfa0; CLocalGameObject vtable 0x4edb24 at +0xc0), own
    members 0x1d74..0x1d80, vtordisp 0x1d80, streamer 0x1d84. The streamer vtable's read_class /
    write_class are C3DDino's copies (0x42d640 / 0x42d650), the same jumps as C3DAI's.

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code of the original sees them at offset - 0xc0).            */
#pragma once
#include "C3DFriends.h"

class C3DSkateBoard;

class C3DNick : public C3DFriends
{
	public:
	C3DNick();																// 0x45a350
	virtual ~C3DNick();														// 0x45a580

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69  0x45a610 load the morph animations and the texture (once)
	virtual void ev126(void);												// ESLOT 126 0x45aac0 skate the NICPAT1 patrol
	virtual void ev145(void);												// ESLOT 145 0x45a9b0 talked to: the SCENE task moves on

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x44ebc0
	virtual void Process(float dt);											// SLOT 240 0x45a820 the skateboard, SKATE / GLIDE
	virtual void v266(void);												// SLOT 266 0x4368e0
	virtual void v267(int level);											// SLOT 267 0x45a6d0 shown in his story scenes only

	// ---- data members (object offsets)
	C3DSkateBoard		*f_1d74;		// 0x1d74 his skateboard (the original keeps its CGameObject part)
	float				f_1d78;			// 0x1d78 SKATE / GLIDE timer (5 s)
	int					f_1d7c;			// 0x1d7c laps done (SCENE 125 -> 120): each one slows him by 100
};
