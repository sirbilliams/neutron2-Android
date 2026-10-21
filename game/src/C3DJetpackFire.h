/*  C3DJetpackFire ('3JFI') - the jet pack on Jimmy's back with its flames: an .ase morph object
    ("jetpack.png") with the animations STOP (folded), OPEN (unfolding, then ANIMATE: the flames
    loop) and CLOSE (folding, then hidden). Each C3DJetPack makes one (its f_9f0) and moves it
    onto Jimmy every frame while he flies with it.
    Reconstructed from Neutron2.exe: factory '3JFI', constructor 0x443500 (0x938 bytes),
    destructor 0x443640 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x4437f0 /
    0x443610 / 0x443800 / 0x4437e0).

    Original layout: as C3DAnimated (element vtable 0x4e4cdc, 94 slots; OMediaAnim vtable
    0x4e4c8c at +0x90; listener 0x4e4cc8 at +0x30; CLocalGameObject vtable 0x4e4810 at +0xc0),
    own members 0x924..0x930, vtordisp 0x930, streamer 0x934.                                */
#pragma once
#include "C3DAnimated.h"

class C3DJetpackFire : public C3DAnimated
{
	public:
	C3DJetpackFire();														// 0x443500
	virtual ~C3DJetpackFire();												// 0x443640

	// ---- element-family game slots
	virtual void ev67(void);												// ESLOT 67 0x443760 animation ended: OPEN -> ANIMATE, CLOSE -> hidden

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7  0x443690 the animations and the texture

	// ---- data members (object offsets)
	CGameObject			*f_924;			// 0x924  g_jimmy when the jet pack was made (NULL for Jimmy's own: g_jimmy is set after)
	float				f_928;			// 0x928  flame size (0; C3DJimmy / C3DPlayer set it, nothing reads it)
	int					f_92c;			// 0x92c  0
};
