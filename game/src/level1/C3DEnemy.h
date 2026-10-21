/*  C3DEnemy ('3ENE') - the level enemies: a granny C3DAI whose "EnemyType" picks the model.
      0, 1  Nummey ("nummeybase.grn": TALK, WALK, STOP, SMALLSTOP, LAUGH, TYPE)
      2     a ghost (the kids in level1_b: "ghostbase.grn"; not on the radar). Jimmy (the
            camera object) touching it plays its line ("libbyghost", "carlghost", "cindyghost",
            "jimmynick28", "sheenhi" by ObjectTag), at most every 10 s. While power timer 7 runs
            it FALLs (ev67: back to state 3 at the end).
      3     a mummy: hits Jimmy (SWING; Jimmy ev134 -10 energy, -10 points, 5 s) unless power 1
            runs. A water balloon pops on it and sends it RUNning away (state 15, twice the
            speed, back to WALK after 20 s); hit again while fleeing it DIEs (20 points; DEAD,
            back after 6000 s). Every "NumToKill" (3) deaths call "ActivateWhenShot"'s NextState(-1).
      4     Nummey on a scooter, 5 flying, 6 the ghost Nummey (level1_a / level1_c Johnny).
            Jimmy touching it (every 5 s at most) costs it 30 of its 100 energy (ev144), gives
            100 points, stops power 0 and Jimmy's sparkles ("vrhit", "pickup2"); scooter and
            ghost Nummey make Jimmy drop his props. Its energy bar shows over its head for 20 s.
            Out of energy: state 1, Jimmy turned to face the way it faces, and the OUTOFENERGY
            trigger's NextTrigger. Fleeing (state 15) it slows down (by difficulty, not below
            400 / 200 / 250). A glow sprite (C3DSpriteObject, sprites.omt canvas 26) between it
            and the camera fades in within 600 of its target.
    Reconstructed from Neutron2.exe: constructor 0x42e610 (factory '3ENE', 0x1de8 bytes),
    destructor 0x42e8a0 (only resets the vtables, then ~C3DAI; deleting thunks 0x430600 /
    0x4305f0 / 0x42e870 / 0x430610). Vtables: element 0x4d9078 (146 slots: C3DAI's 144 + 144,
    145; 67, 69 overridden), game object (+0xc0) 0x4d8bac (0, 7, 16, 17, 231, 240, 258, 260,
    270, 284, 285 overridden), streamer 0x4d8b9c. Own members 0x1b50..0x1de0, vtordisp 0x1de0,
    streamer 0x1de4.
    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                             */
#pragma once
#include "../C3DAI.h"
#include "../InterfaceElement.h"

class C3DSpriteObject;
class OMediaStreamOperators;

class C3DEnemy : public C3DAI
{
	public:
	C3DEnemy();																// 0x42e610
	virtual ~C3DEnemy();													// 0x42e8a0

	// ---- element-family slots
	virtual void ev67(void);												// ESLOT 67  0x430490 animation ended: FALL -> back, DIE -> DEAD
	virtual void ev69(void);												// ESLOT 69  0x42f8b0 load the granny model and animations (once)
	virtual void ev144(float amount);										// ESLOT 144 0x42f6e0 lose energy; out of it: OUTOFENERGY
	virtual void ev145(void);												// ESLOT 145 0x42ea10 speed = InitialSpeed

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x42e960 properties
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x42f250 water balloons, Jimmy
	virtual void RemoveTouching(CGameObject *other);						// SLOT 17  0x42f860 the mummy walks again
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x430520 collides with
	virtual void Process(float dt);											// SLOT 240 0x42ea30
	virtual void v258(void);												// SLOT 258 0x42ea20 back to AIState, then v260
	virtual void v260(void);												// SLOT 260 0x42e9d0 properties loaded
	virtual void v270(int a, int msg);										// SLOT 270 0x430590 msg 4: the energy bar off
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x4303a0
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x430410

	// ---- data members (object offsets)
	int					f_1b50;			// 0x1b50 "EnemyType" (0)
	float				f_1b54;			// 0x1b54 mummy: dead, time until it gets up (6000 s)
	float				f_1b58;			// 0x1b58 mummy: running away, time left (20 s)
	C3DSpriteObject		*f_1b5c;		// 0x1b5c the glow sprite (types 4..6; the original keeps its CGameObject part)
	float				f_1b60;			// 0x1b60 energy (100)
	float				f_1b64;			// 0x1b64 ghost: time until it speaks again (10 s)
	// FIELD 0x1b68 0x208
	InterfaceElt		f_1b68;			// 0x1b68 the energy bar's frame (screens canvas 411)
	InterfaceElt		f_1c6c;			// 0x1c6c the energy bar (screens canvas 359)
	float				f_1d70;			// 0x1d70 time the bar stays up (20 s)
	char				f_1d74[100];	// 0x1d74 "ActivateWhenShot" ("none")
	int					f_1dd8;			// 0x1dd8 "NumToKill" (3)
	float				f_1ddc;			// 0x1ddc time until Jimmy can hit it again (5 s)
};
