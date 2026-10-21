/*  C3DBus ('3SBU') - the school bus (Level5c): a C3DAICar whose morph model "bus.ase"
    ("HIDEFAULT", texture "bus.png") is only loaded once its animations are set up (ev69), and
    then plays "STOP" (looped). Speed 400, v64 0.05, v66 1000, v94 2, v34 0.
    Reconstructed from Neutron2.exe: constructor 0x4235f0 (factory '3SBU', 0x1b78 bytes; it
    inlines InitObject), destructor 0x423790 (vtable resets only, then ~C3DAICar; deleting
    thunks 0x423760 / 0x423890 / 0x4238a0 / 0x4238b0).

    Original layout: as C3DAICar (element vtable 0x4d1cb8, 144 slots; CLocalGameObject vtable
    0x4d17ec at +0xc0), own member 0x1b6c, vtordisp 0x1b70, streamer 0x1b74. Slots 8
    (0x4162b0), 260 (0x423840) and 267 (0x441fe0) are jumps to / copies of the inherited code,
    so they are not overridden.                                                               */
#pragma once
#include "C3DAICar.h"

class C3DBus : public C3DAICar
{
	public:
	C3DBus();																// 0x4235f0
	virtual ~C3DBus();														// 0x423790

	// ---- element-family overrides
	virtual void ev69(void);												// ESLOT 69 0x4237e0 load the model once

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43e920 C3DAI's, then InitAnim
	virtual void v277(void);												// SLOT 277 0x423850 ev79(1)
	virtual void v278(void);												// SLOT 278 0x423860 ev79(0)

	int					f_1b6c;			// 0x1b6c (= -1, not used)
};
