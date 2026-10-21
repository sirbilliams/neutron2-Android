/*  C3DSubBubble ('3SBB') - the glass bubble round the mini sub (created at run time; never in a
    .gam file): a morph object, animation STOP from "subsphere.ase", texture "subsphere2.png",
    radius 1, no collision. Each frame its heading is kept in 0..360 and its texture scrolls
    with its turning (u: -0.009 per degree) and its movement (v: -0.01 per unit, times dt).
    Reconstructed from Neutron2.exe: constructor 0x4747f0 (factory '3SBB', 0x944 bytes),
    destructor 0x474940 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x474d80 /
    0x474d70 / 0x474910 / 0x474d90). Vtables 0x4fd8fc (element, 94 slots, all C3DAnimated's) /
    0x4fd8e8 (listener) / 0x4fd8ac / 0x4fd430 (game object at +0xc0: 0, 7, 240 overridden; 260
    is 0x46fc00, a jump to C3DAnimated's) / 0x4fd420 (streamer). Own members 0x924..0x93c,
    vtordisp 0x93c, streamer 0x940.                                                           */
#pragma once
#include "../C3DAnimated.h"

class C3DSubBubble : public C3DAnimated
{
	public:
	C3DSubBubble();															// 0x4747f0
	virtual ~C3DSubBubble();												// 0x474940

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x474990 the bubble animation
	virtual void Process(float dt);											// SLOT 240 0x474a70 heading 0..360, texture scroll

	// FIELD 0x924 0x18
	float				f_924;			// 0x924  heading (angle y) last frame
	float				f_928[4];		// 0x928  position last frame (w = 1)
	int					f_938;			// 0x938  0 (not used)
};
