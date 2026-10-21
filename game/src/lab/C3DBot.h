/*  C3DBot ('3BOT') - Negatron's goof bot: a hovering morph AI (.ase animations SHOOT
    "goofbotshoot", RUN "goofbootfly", STOP "goofbotstop"; texture "goofbot.png"; FLY, SIT and
    WALK animations off), wandering (AIState 11) at InitialSpeed scaled by the skill (x0.4,
    x0.7, x1), VisibleRange 500, its target offset "TargOffX/Y/Z". Every 5 s it rises for up to
    3 s (90 / s) while there is ground within 600 below.
    A water balloon stuns it (f_1b50 = 60000 s: it drops, spins in its C3DShock, shoots
    nothing; Jimmy scores 20); on landing while stunned it is hidden. Touching Jimmy (see
    C3DJimmy::AddTouching): an unstunned bot hurts him and backs off (state 5).
    Reconstructed from Neutron2.exe: constructor 0x422250 (factory '3BOT', 0x1d70 bytes),
    destructor 0x422530 (deleting thunks 0x422500 / 0x422ca0 / 0x422cb0 / 0x422cc0).

    Original layout: as C3DAI (element vtable 0x4d0a50, 144 slots; listener 0x4d0a3c; OMediaAnim
    0x4d0a00; CLocalGameObject vtable 0x4d0584 at +0xc0), own members 0x1b50..0x1d68,
    vtordisp 0x1d68, streamer 0x1d6c.                                                       */
#pragma once
#include "../C3DAI.h"
#include "../InterfaceElement.h"

class C3DShock;

class C3DBot : public C3DAI
{
	public:
	C3DBot();																// 0x422250
	virtual ~C3DBot();														// 0x422530

	// ---- element-family slots
	virtual void ev58(char *name, unsigned char loop);						// ESLOT 58  0x45b1f0 (calls C3DAnimated's)
	virtual void ev69(void);												// ESLOT 69  0x4225d0 load the animations and the texture (once)
	virtual void ev117(void) {}												// ESLOT 117 0x463890 (no FLY animation)
	virtual void ev119(void) {}												// ESLOT 119 0x463890 (no SIT animation)
	virtual void ev120(void) {}												// ESLOT 120 0x463890 (no WALK animation)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x422690 + TargOffX/Y/Z
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x422780 a water balloon: stunned
	virtual void v222(void);												// SLOT 222 0x422860 landed: hidden while stunned
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x422b90 collides with the camera, '3BOT', '3EPR', '3SWN'; '3JIM', '3GOD', '3WBL' unless stunned
	virtual void Process(float dt);											// SLOT 240 0x4228f0 stun time, the shock, hovering up
	virtual void v260(void);												// SLOT 260 0x422700 speed by skill
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x422c40 + f_1b50, f_bd4
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x422c00 + f_1b50, f_bd4

	// ---- data members (object offsets)
	float				f_1b50;			// 0x1b50 stunned for (0; a water balloon: 60000 s)
	float				f_1b54;			// 0x1b54 hover timer (5: rise when below 0, down to -3)
	C3DShock			*f_1b58;		// 0x1b58 the shock sprite (the original keeps its CGameObject part)
	float				f_1b5c;			// 0x1b5c speed to go back to (v260: InitialSpeed by skill)
	InterfaceElt		f_1b60;			// 0x1b60 (constructed, not used)
	InterfaceElt		f_1c64;			// 0x1c64 (constructed, not used)
};
