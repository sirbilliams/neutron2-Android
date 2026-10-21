/*  C3DHoverCycle ('3HOV') - the hover cycle Jimmy rides in Level3c, ahead of the tidal wave: a
    morph C3DAI (animation STOP from "godflycycle.ase", texture "goddard02.png").
    Jimmy gets on by touching it (his ActivateMenuItem(this, this, 12, 13): menu item 12 makes
    it the camera object and sets f_1ba4 to him, the "DRIVE" pose) and gets off through the same
    menu item. While it is the camera object (v243):
      Left / Right (or the mouse x) turn it (+-25, the mouse up to +-45), TurnAround spins it
      half round in 1 s; Go or Action accelerate (f_1b98 1700 / s up to f_1ba0 1600, half that
      without Go; 2800 while the power timer runs and Jimmy sparkles) and cost neutron energy
      (0.3 / 0.6 / 0.8 per s by difficulty); Back brakes (2 * 500 / s); nothing pressed, the
      momentum turns towards the heading. The "harley1" loop's frequency follows the speed.
      Then the chase camera (ev144): behind and above, kept within 1200; close to the tidal wave
      and slow (or the brake key held) it looks back at the cycle from 1000 in front.
    It hovers (ev146, every frame): 200 above the ground found by a ray 1500 down, pushed up
    harder the lower it is, falling at 4.5 times that; Jump lifts it (+900, "boing", 0.5 s;
    hard: 1 energy). Fast into a wall (difficulty 1/2, head-on) it crashes: spins for 1.65 s,
    bounces off, "goddard bark", Jimmy is hurt (-5); over 700 it loses speed and slides.
    Shrubs slow it (0.8, "shrubs"); ev145 counts the time it heads towards the tidal wave.
    Reconstructed from Neutron2.exe: constructor 0x43f750 (factory '3HOV', 0x1bd8 bytes),
    destructor 0x43fa60 (only resets the vtables, then ~C3DAI; deleting thunks 0x43fa30 /
    0x4419a0 / 0x4419b0 / 0x4419c0). Vtables: element 0x4e2908 (147 slots: C3DAI's 144 +
    144..146), game object (+0xc0) 0x4e243c (0, 7, 16, 220, 240, 243, 260, 277, 278, 284, 285
    overridden), streamer 0x4e242c. Own members 0x1b50..0x1bd0, vtordisp 0x1bd0, streamer 0x1bd4.
    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                             */
#pragma once
#include "../C3DAI.h"
#include "OMedia3DPoint.h"

class C3DJimmy;
class C3DShadow;
class OMedia3DPolygon;
class OMediaStreamOperators;

class C3DHoverCycle : public C3DAI
{
	public:
	C3DHoverCycle();														// 0x43f750
	virtual ~C3DHoverCycle();												// 0x43fa60

	// ---- element-family slots (new)
	virtual void ev144(float dt);											// ESLOT 144 0x43fe70 the chase camera
	virtual void ev145(float dt);											// ESLOT 145 0x43fd60 time heading towards the tidal wave (f_1bc8)
	virtual void ev146(float dt);											// ESLOT 146 0x441130 hovering, the shadow, Jump

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43fab0 animation, texture
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43fc30 Jimmy gets on; shrubs; the tidal wave
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x440da0 wall: crash / slide
	virtual void Process(float dt);											// SLOT 240 0x43fb40 hovering; the rider follows
	virtual void v243(float dt);											// SLOT 243 0x440300 ridden: controls, sound, camera
	virtual void v260(void);												// SLOT 260 0x441110 properties loaded: find the tidal wave
	virtual void v277(void);												// SLOT 277 0x441900 pause: sound off
	virtual void v278(void);												// SLOT 278 0x441920 resume: sound on again
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x441870
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x441790

	// ---- data members (object offsets)
	int					f_1b50;			// 0x1b50 0 (unused)
	unsigned char		f_1b54;			// 0x1b54 0 (unused)
	OMedia3DPoint		f_1b58;			// 0x1b58 camera position, local (0, 10, -300)
	OMedia3DPoint		f_1b68;			// 0x1b68 camera target, local (0, 10, 200)
	float				f_1b78;			// 0x1b78 hover height (200, set by ev146)
	unsigned char		f_1b7c;			// 0x1b7c 0 (unused)
	float				f_1b80;			// 0x1b80 Jump: time until the next (0.5 s)
	float				f_1b84;			// 0x1b84 crashed: spin time left (1.65 s)
	float				f_1b88;			// 0x1b88 turn: degrees / s (+-25 keys, +-45 mouse); also the lean
	unsigned char		f_1b8c;			// 0x1b8c accelerating this frame (Go / Action)
	float				f_1b90;			// 0x1b90 pitch wanted (-35 at first; -26 braking, 27 accelerating; unused)
	float				f_1b94;			// 0x1b94 35 (unused)
	float				f_1b98;			// 0x1b98 acceleration (1700 / s)
	float				f_1b9c;			// 0x1b9c braking (500 / s)
	float				f_1ba0;			// 0x1ba0 top speed (1600; 1800 / 2800 by Jimmy's power)
	C3DJimmy			*f_1ba4;		// 0x1ba4 the rider (set by C3DJimmy ActivateMenuItem 12)
	C3DShadow			*f_1ba8;		// 0x1ba8 a shadow kept on the ground (never set: 0)
	CGameObject			*f_1bac;		// 0x1bac a Jimmy hurt by fast slides (never set: 0)
	int					f_1bb0;			// 0x1bb0 (unused)
	int					f_1bb4;			// 0x1bb4 the "harley1" channel (-1; C3DJimmy starts / stops it)
	int					f_1bb8;			// 0x1bb8 its base frequency (11025)
	unsigned char		f_1bbc;			// 0x1bbc touched by the tidal wave
	float				f_1bc0;			// 0x1bc0 20 while the power runs, else 0 (unused)
	CGameObject			*f_1bc4;		// 0x1bc4 the tidal wave (tag "C3DTSUNAMI")
	float				f_1bc8;			// 0x1bc8 time heading towards it (ev145)
	float				f_1bcc;			// 0x1bcc TurnAround: time left (1 s)
};
