/*  C3DDino ('3DIN') - a dinosaur of the museum's prehistoric exhibit (Level5f): a Granny AI
    ("EnemyType" 0 or 3: the T. rex, "trexbase.grn" with WALK, TALK, FALL, STOP, SIZE, PLAY,
    PICKUP; otherwise a raptor, "raptorbase.grn" with WALK (run), TALK (attack), FALL (death),
    STOP, SIZE), speed 600, walking sound "dinowalk"/2/3 (random) or "raptorwalk".
    Unless windowed it gets a shock effect (a C3DASEObj, "dinoshock.ase" / "raptorshock.ase")
    and a shadow (a C3DAIGranny playing the matching "...shadow" model, in step with ev58:
    WALK -> WALK, anything else -> STOP).
    Hit by a water balloon: health f_1d58 down (T. rex 0.15 / 0.1 / 0.05, raptor 0.26 / 0.16 /
    0.1 by difficulty), "SPLASH", "balloon pop", "vrzap", the shock flashes 0.3 s and the health
    bar (as Herman's: canvases 411 / 462 over its head, green above 0.34) shows for 8 s. Out of
    health: 100 points, it stops (state 1; so do Sporky for "SPORKEATER" and Carl for "DINO6"),
    FALL, "dinodown" / "dinodown2", the walking sound stops (EnemyType 3 also silences
    "FOWLYELLS" and plays DINOCAR's ANIM1); 8 s later it is gone (hidden, no collision), and
    when 9 have fallen (g_545f48) the trigger tagged "KILLDINOS" is called (the radar hidden).
    Reconstructed from Neutron2.exe: constructor 0x42b7a0 (factory '3DIN', 0x1d88 bytes),
    destructor 0x42ba50 (deleting thunks 0x42ba20 / 0x42d610 / 0x42d620 / 0x42d630).

    Original layout: as C3DAI (element vtable 0x4d7d34, 144 slots; CLocalGameObject vtable
    0x4d7868 at +0xc0), own members 0x1b50..0x1d80, vtordisp 0x1d80, streamer 0x1d84.        */
#pragma once
#include "../C3DAI.h"
#include "../InterfaceElement.h"

class C3DASEObj;
class C3DAIGranny;

class C3DDino : public C3DAI
{
	public:
	C3DDino();																// 0x42b7a0
	virtual ~C3DDino();														// 0x42ba50

	// ---- element-family overrides
	virtual void ev58(char *name, unsigned char loop);						// ESLOT 58  0x42d140 play an animation (the shadow follows)
	virtual void ev69(void);												// ESLOT 69  0x42bb30 the model, the shock and the shadow (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x42cef0 + "EnemyType"
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x42c4e0 hit by a water balloon
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x42d250 collides with the camera, '3DIN', '3WBL'
	virtual void Process(float dt);											// SLOT 240 0x42c850 the shock, the fall, the shadow, the bar
	virtual void v260(void);												// SLOT 260 0x42cf30 walking sound, count the dinosaurs
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x42d590 + health, fall time, animation, g_545f48
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x42d430

	// ---- data members (object offsets)
	InterfaceElt		f_1b50;			// 0x1b50 health bar frame (screens.omt canvas 411)
	InterfaceElt		f_1c54;			// 0x1c54 health bar (canvas 462, width f_1d5c * health)
	float				f_1d58;			// 0x1d58 health (1)
	float				f_1d5c;			// 0x1d5c health bar length (50)
	float				f_1d60;			// 0x1d60 health bar shown for (8 s after a hit)
	float				f_1d64;			// 0x1d64 fall time left (8 s once out of health)
	int					f_1d68;			// 0x1d68 "EnemyType" (0: T. rex, 3: T. rex with the fowl, else raptor)
	int					f_1d6c;			// 0x1d6c walking sound channel (-1)
	C3DASEObj			*f_1d70;		// 0x1d70 the shock effect (0 when windowed)
	C3DAIGranny			*f_1d74;		// 0x1d74 the shadow (0 when windowed)
	float				f_1d78;			// 0x1d78 shock flash time left
	unsigned char		f_1d7c;			// 0x1d7c the shadow's blending is set
};
