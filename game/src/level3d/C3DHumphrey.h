/*  C3DHumphrey ('3HUM') - Humphrey, a guard animal: an ASE AI (humprun / humpwalk / humpsleep /
    humpstop as ATTACK / WALK / STOP / STOP2, texture "humphrey.png"), radius 20. It sleeps
    (STOP) until its AITriggers move it; four of C3DAI's state animations are renamed
    (STOP / WALK / STOP / ATTACK).
    Touching Jimmy it bites (-10 energy, -10 points, 5 s) and goes to AI state 5 (flee).
    Reconstructed from Neutron2.exe: constructor 0x441cf0 (factory '3HUM', 0x1b58 bytes),
    destructor 0x441e80 ("Deleting Humphrey" (debug log), then ~C3DAI; deleting thunks 0x4420b0
    / 0x441e50 / 0x4420a0 / 0x4420c0).

    Original layout: as C3DAI (element vtable 0x4e37cc, 144 slots, all C3DAI's; listener
    0x4e37b8; OMediaAnim 0x4e377c; CLocalGameObject vtable 0x4e3300 at +0xc0), no members of
    its own. Slot 266 is C3DArrow's code (0x4368e0).                                         */
#pragma once
#include "../C3DAI.h"

class C3DHumphrey : public C3DAI
{
	public:
	C3DHumphrey();															// 0x441cf0
	virtual ~C3DHumphrey();													// 0x441e80

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x441f10 animations, texture
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x441ff0 Jimmy: bite
	virtual void v260(void);												// SLOT 260 0x442050 C3DAI's, then ev70(1)
	virtual void v266(void);												// SLOT 266 0x4368e0 SetTaskState(TaskName, GetTaskState(TaskName))
	virtual void v267(int level);											// SLOT 267 0x441fe0 C3DAnimated's
};
