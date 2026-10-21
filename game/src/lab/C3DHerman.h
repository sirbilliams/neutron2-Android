/*  C3DHerman ('3HER') - Herman, Negatron's lab robot: a Granny AI (model "hermanbase.grn";
    WALK, FLY (= walk), TALK, LAUGH, STOP, ANALYZE, ATTACK1..3, BEFORERUN, DIES, HALT, HIT,
    LOOK, MAIL, HOLIDAY, RUN, THINKMOVE, THINKSTOP, TURNGRAB, TURNSPEAK, SPININ, SPINOUT),
    patrolling (AIState 2) at 160, VisibleRange 500.
    Touching Jimmy: he grabs (state 7, ATTACK1) and hurts him (-5 energy, -5 points). Hit by a
    water balloon: he stops (state 1), plays HIT, his health bar shows (ev144: health f_1d58 -=
    0.1 / 0.06 / 0.03 by skill, the bar for 15 s) and Jimmy scores 20; HIT ends -> LOOK -> state
    17, ATTACK1 ends -> flee for 3 s (state 5, then 17). Out of health: STOP, the bar hidden, 200
    points, and the trigger tagged DEFEAT is called. While playing, ev145 heals him slowly
    (0.001 / 0.0015 / 0.002 per second by skill) and draws the bar (two interface elements,
    canvases 411 and 462 of screens.omt) over his head, green above 0.34, red below.
    A cut scene that finds him stopped makes him THINKMOVE; so does a missing animation.
    Reconstructed from Neutron2.exe: constructor 0x43e2a0 (factory '3HER', 0x1d6c bytes),
    destructor 0x43e560 (deleting thunks 0x43e530 / 0x43efc0 / 0x43efd0 / 0x43efe0).

    Original layout: as C3DAI (element vtable 0x4e142c, 146 slots: C3DAI's 144 + 144, 145;
    listener 0x4e1418; OMediaAnim 0x4e13dc; CLocalGameObject vtable 0x4e0f60 at +0xc0), own
    members 0x1b50..0x1d64, vtordisp 0x1d64, streamer 0x1d68.                               */
#pragma once
#include "../C3DAI.h"
#include "../InterfaceElement.h"

class C3DHerman : public C3DAI
{
	public:
	C3DHerman();															// 0x43e2a0
	virtual ~C3DHerman();													// 0x43e560

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60  0x43ef90 no such animation: THINKMOVE
	virtual void ev67(void);												// ESLOT 67  0x43ea90 animation ended: HIT -> LOOK -> state 17; ATTACK1 -> flee
	virtual void ev69(void);												// ESLOT 69  0x43e620 load the model and the animations (once)
	virtual void ev144(void);												// ESLOT 144 0x43eb40 hit: health down (or defeated), bar for 15 s
	virtual void ev145(float dt);											// ESLOT 145 0x43ec10 heal; the health bar over his head

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43e920 (C3DAI's properties, no Talk*)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43e960 Jimmy: grab; a water balloon: hit
	virtual void Process(float dt);											// SLOT 240 0x43ea40 the bar while playing, else hidden
	virtual void v260(void);												// SLOT 260 0x417110 C3DAI's, then ev70(1)
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x43eeb0 "C3DHerman::ChangeState": THINKMOVE
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x468090 + f_1d58
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x43ee80 + f_1d58

	// ---- data members (object offsets)
	InterfaceElt		f_1b50;			// 0x1b50 health bar frame (screens.omt canvas 411)
	InterfaceElt		f_1c54;			// 0x1c54 health bar (canvas 462, width f_1d5c * health)
	float				f_1d58;			// 0x1d58 health (1.0)
	float				f_1d5c;			// 0x1d5c health bar length (50)
	float				f_1d60;			// 0x1d60 health bar shown for (15 s after a hit)
};
