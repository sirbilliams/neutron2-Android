/*  CPtero ('3PTE') - a pterodactyl of the museum's prehistoric exhibit (Level5f): a morph AI
    ("PTerofly.ase" WALK, FALL, TALK, STOP; texture "Pterodactyl.png", radius 500) that flies
    its patrol (state 3, speed 500, VisibleRange 500, hovering) with the "pteroflap" loop, and a
    shock effect (a C3DASEObj, "pteroshock.ase").
    Hit by a water balloon: health f_1d58 down (0.24 / 0.15 / 0.09 by difficulty), "vrzap",
    "SPLASH", "balloon pop", the shock flashes 0.3 s and the health bar (canvases 411 / 462 of
    screens.omt over its head, 50 * health long, green above 0.34) shows for 8 s. Out of
    health: 100 points, it stops (state 1), drops (no more hovering), FALL, "pterodown"; it
    fades out between 5 and 1 s of the 8 s fall, then is gone; as for the dinosaurs, when 9 have
    fallen (g_545f48) the trigger tagged "KILLDINOS" is called (the radar hidden).
    No RTTI in the exe (named after its factory entry); its log name is "C3DPTERO".
    Reconstructed from Neutron2.exe: constructor 0x4671b0 (factory '3PTE', 0x1d78 bytes),
    destructor 0x4674a0 (deleting thunks 0x467470 / 0x4680e0 / 0x4680f0 / 0x468100).

    Original layout: as C3DAI (element vtable 0x4f36fc, 144 slots; CLocalGameObject vtable
    0x4f3230 at +0xc0), own members 0x1b50..0x1d70, vtordisp 0x1d70, streamer 0x1d74.        */
#pragma once
#include "../C3DAI.h"
#include "../InterfaceElement.h"

class C3DASEObj;

class CPtero : public C3DAI
{
	public:
	CPtero();																// 0x4671b0
	virtual ~CPtero();														// 0x4674a0

	// ---- element-family overrides
	virtual void ev69(void);												// ESLOT 69  0x4677b0 the model and the shock (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x467a80 (C3DAI's properties)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x467580 hit by a water balloon
	virtual void Process(float dt);											// SLOT 240 0x467b30 the fall, the shock, the bar
	virtual void v260(void);												// SLOT 260 0x467ab0 the flapping sound
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x468090 + health (shared with C3DHerman)
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x467fc0 + health

	// ---- data members (object offsets)
	InterfaceElt		f_1b50;			// 0x1b50 health bar frame (screens.omt canvas 411)
	InterfaceElt		f_1c54;			// 0x1c54 health bar (canvas 462)
	float				f_1d58;			// 0x1d58 health (1)
	float				f_1d5c;			// 0x1d5c health bar shown for (8 s after a hit)
	float				f_1d60;			// 0x1d60 fall time left (8 s once out of health)
	int					f_1d64;			// 0x1d64 "pteroflap" channel (-1)
	C3DASEObj			*f_1d68;		// 0x1d68 the shock effect
	float				f_1d6c;			// 0x1d6c shock flash time left (0)
};
