/*  C3DVRTank ('3VRT') - the tank Jimmy drives in the VR level: a morph C3DAI (animations
    STRUGGLE from "tankfly.ase", STOP from "tankstop.ase"; texture "tank.png"). Jimmy gets in by
    touching it (his ActivateMenuItem(this, this, 5, 13): menu item 5 makes it the camera
    object, Jimmy DRIVE, Goddard SIT on board) and out through the same menu item.
    While it is the camera object (v243):
      Left / Right (or the mouse x) steer (+-15 degrees / s, a skid sideways with the speed),
      TurnAround spins it half round in 1 s; Go accelerates (800 / s up to 1200; 2000 while the
      first power-up timer g_54a424[0] runs; four times that while g_540594 < 1), Back reverses
      (500 / s, down to minus the top speed), the mouse y does both (+-250 per unit, at most
      +-500 / s); Brake takes 0.25 * 500 off each frame; above 80% of the top speed it slows by
      50 a frame. Action fires the next of its 10 energy projectiles (60, 70, 680 from its
      origin, 2.5 degrees up, at 2000, homing on the bug nearest its aim point) every 0.6 s
      (0.2 s while g_54a438 runs, which also makes the shots free; else -0.5 neutron energy).
      The chase camera sits at (0, 350, -500) looking at (0, 50, 200), eased (0.8) and kept out
      of walls; the "tank1" loop's frequency follows the speed; Jimmy's bubble shows over JIM1.
    Each frame (Process) Jimmy sits at (0, 100, -100) and Goddard at (135, 165, -90), turned
    with it, and Goddard's red-neutron cursor (C3DCursor) points at the target bug (ev144).
    Touching a bug or the giant wasp that is not knocked out hurts JIM1 (-1 / -3 / -5 by skill,
    "vrhit").
    Reconstructed from Neutron2.exe: constructor 0x47aaf0 (factory '3VRT', 0x1be8 bytes),
    destructor 0x47adb0 (only resets the vtables, then ~C3DAI; deleting thunks 0x47ad80 /
    0x47c370 / 0x47c380 / 0x47c390). Vtables: element 0x503b0c (C3DAI's 144 + 144, 145), game
    object (+0xc0) 0x503640 (0, 7, 16, 240, 243, 246, 260, 277, 278, 285 overridden; 285 only
    calls C3DAI's), streamer 0x503630. Own members 0x1b50..0x1be0, vtordisp 0x1be0, streamer
    0x1be4. Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the
    original object> (CGameObject-family code sees them at offset - 0xc0).                     */
#pragma once
#include "../C3DAI.h"
#include "OMedia3DPoint.h"

class C3DEnergyProjectile;

class C3DVRTank : public C3DAI
{
	public:
	C3DVRTank();															// 0x47aaf0
	virtual ~C3DVRTank();													// 0x47adb0

	// ---- element-family slots (new)
	virtual void ev144(float dt);											// ESLOT 144 0x47b490 the target bug and Goddard's cursor on it
	virtual CGameObject *ev145(OMedia3DVector pos);							// ESLOT 145 0x47b680 the active '3BUG' nearest pos

	// ---- not virtual
	void Fire(float x, float y, float z, float pitch, float speed, CGameObject *target, float delay, float unused);	// 0x47b260 the next shot from v224(x, y, z)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x47ae00 animations, texture, the sound channel
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x47b840 Jimmy gets in; bugs hurt JIM1
	virtual void Process(float dt);											// SLOT 240 0x47aec0 driven: speed and skid, the riders, the cursor
	virtual void v243(float dt);											// SLOT 243 0x47b9d0 driven: controls, firing, camera, sound
	virtual void RotateToDest(float dt);									// SLOT 246 0x47c110 ease towards the wanted angles (1.2, 4, 5.15)
	virtual void v260(void);												// SLOT 260 0x47b800 properties loaded: no speed, no target, JIM1
	virtual void v277(void);												// SLOT 277 0x47c2d0 pause: sound off
	virtual void v278(void);												// SLOT 278 0x47c2f0 resume: sound on again

	// ---- data members (object offsets)
	float				f_1b50;			// 0x1b50 top speed (1200; 2000 with the power-up)
	float				f_1b54;			// 0x1b54 steering: degrees / s (+-15; the mouse x)
	float				f_1b58;			// 0x1b58 TurnAround: time left (1 s)
	int					f_1b5c;			// 0x1b5c 0 (unused)
	unsigned char		f_1b60;			// 0x1b60 0 (unused)
	float				f_1b64;			// 0x1b64 time since the last shot
	unsigned char		f_1b68;			// 0x1b68 speed control used this frame (not read)
	float				f_1b6c;			// 0x1b6c pitch wanted (-35 at first; 10 forward, -7 back, 3 idle)
	float				f_1b70;			// 0x1b70 acceleration (800 / s)
	int					f_1b74;			// 0x1b74 (never set)
	float				f_1b78;			// 0x1b78 35 (unused)
	float				f_1b7c;			// 0x1b7c braking (500)
	OMedia3DPoint		f_1b80;			// 0x1b80 camera position, local (0, 350, -500)
	OMedia3DPoint		f_1b90;			// 0x1b90 camera target, local (0, 50, 200)
	CGameObject			*f_1ba0;		// 0x1ba0 "JIM1"
	CGameObject			*f_1ba4;		// 0x1ba4 Goddard (ev144)
	CGameObject			*f_1ba8;		// 0x1ba8 the target bug (ev144)
	int					f_1bac;			// 0x1bac the "tank1" channel (-1)
	int					f_1bb0;			// 0x1bb0 its base frequency (11025)
	C3DEnergyProjectile	*f_1bb4[10];	// 0x1bb4 the shots
	short				f_1bdc;			// 0x1bdc the next shot
};
