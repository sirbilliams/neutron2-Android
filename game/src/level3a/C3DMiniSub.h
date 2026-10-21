/*  C3DMiniSub ('3SUB', tag "C3DMINISUB") - the mini sub Jimmy drives in Level3a: a C3DFlyingObject
    (animations DEFAULT "SUBMOVE.ase", GRAB "SUBclawpropeler.ase", STOP "SUBstop.ase", texture
    "sub.png", radius 300). It owns a glass C3DSubBubble round it, a headlight beam C3DSpotLight
    (toggled with L; its state is g_5388c4), a blue C3DLight that follows it and 10 C3DBub
    bubbles it trails (one every 0.25 s).
    Jimmy gets in through his menu item 9 (AI trigger "ENTERSUB"): f_9f4 becomes his element,
    f_9f8 Goddard's, and the sub the camera object. While it is the camera (v243):
      - Left / Right turn it (160 deg/s), the mouse x steers (dead zone 10, +-30 beyond 40),
        Go (or the mouse y) speeds it up by AccelRate / 11 per frame up to MaxSpeed (650; 1100
        with Speed, which costs 0.5 energy per s), Back dives (speeds up by AccelRate / 9; on the
        floor it brakes by DecelRate per s), Brake (0x4656f0) brakes to 10; nothing pressed,
        it slows by 40 / s (and creeps at 10); pitch AccelLean / DecelLean, roll -turn rate;
      - above the water surface (viewport y > 4390) the surface fog and "surfsound.omt", the
        bubbles hidden, "SPLASH"; below it the underwater fog and "musicunderwater.omt";
      - near a C3DMovableRock (520 in front) the claw camera (ev111); Action there grabs a rock
        620 in front (GRAB; radius 850 while carrying it), Action again drops it;
      - the chase camera (C3DPlayerObject ev95 / ev96 / ev98); Jimmy aims (his ESLOT 146).
    Each frame (Process) Jimmy and Goddard sit in it, the light, bubble and beam follow it; hit
    (v220: materials 130..139 hurt 10, 185..189 hurt 5, 253 turns it round; other walls by
    difficulty, "subcrash2"; AddTouching: a C3DMISSILE, "rock explosion2") it flashes red for
    f_a30 s (pushed down at first by C3DMISSILE hits).
    Reconstructed from Neutron2.exe: constructor 0x4523e0 (factory '3SUB', 0xa88 bytes),
    destructor 0x4528f0 (only resets the vtables, then ~C3DFlyingObject; deleting thunks 0x454c90
    / 0x454c80 / 0x4528c0 / 0x454ca0). Vtables: element 0x4e9a10 (113 slots: C3DFlyingObject's
    110 + 110..112; 67, 108 overridden), OMediaAnim 0x4e99c0, listener 0x4e99fc, game object
    (+0xc0) 0x4e9544 (0, 7, 10, 11, 16, 219, 220, 240, 243, 246, 260, 266, 267, 275, 284, 285
    overridden; 10, 11, 219 and 266 are code shared with C3DRocketShip / C3DGrannyObj),
    streamer 0x4e9534. Own members 0x9f0..0xa80, vtordisp 0xa80, streamer 0xa84.
    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                            */
#pragma once
#include "../C3DFlyingObject.h"

class C3DGoddard;
class C3DSubBubble;
class C3DSpotLight;
class C3DLight;
class OMediaStreamOperators;
class OMedia3DPolygon;

class C3DMiniSub : public C3DFlyingObject
{
	public:
	C3DMiniSub();															// 0x4523e0
	virtual ~C3DMiniSub();													// 0x4528f0

	// ---- element-family slots
	virtual void ev67(void);												// ESLOT 67  0x453770 animation ended: after GRAB, DEFAULT
	virtual void ev108(int state);											// ESLOT 108 0x453440 engine sound state (only ever stops f_a34)
	virtual void ev110(void) {}												// ESLOT 110 0x463890 (empty)
	virtual void ev111(unsigned char claw);									// ESLOT 111 0x454870 claw camera and handling on / off
	virtual CGameObject *ev112(void);										// ESLOT 112 0x453730 the next of the 10 trail bubbles

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x452a30 animations, texture, radius 300
	virtual void ResetObject(void);											// SLOT 10  0x46c840 (= C3DRocketShip's: speed 0 without a driver)
	virtual void InitPhysics(void);											// SLOT 11  0x43ba20 (= C3DGrannyObj's: unless f_4aa)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x452af0 hit by a C3DMISSILE
	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x4534f0 ceiling: vertical speed -120
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x453150 wall: hits by material
	virtual void Process(float dt);											// SLOT 240 0x452bd0 passengers, light, bubble, beam, hit flash
	virtual void v243(float dt);											// SLOT 243 0x4537b0 controls + camera
	virtual void RotateToDest(float dt);									// SLOT 246 0x453580 rates f_a40..f_a48
	virtual void v260(void);												// SLOT 260 0x452990 level start
	virtual void v266(void);												// SLOT 266 0x46c830 GetTaskState("LIBBY") (result unused)
	virtual void v267(int level);											// SLOT 267 0x453530 + with Jimmy: shown
	virtual void v275(void);												// SLOT 275 0x4534c0 passed on to Jimmy
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x454b10
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x454970

	// ---- data members (object offsets)
	int					f_9f0;			// 0x9f0  0
	C3DAnimated			*f_9f4;			// 0x9f4  the driver: Jimmy's element (NULL: nobody in it)
	C3DGoddard			*f_9f8;			// 0x9f8  Goddard's element, along for the ride
	unsigned char		f_9fc;			// 0x9fc  hit: flashing for f_a30 s
	float				f_a00;			// 0xa00  time since the hit
	CGameObject			*f_a04[10];		// 0xa04  the trail bubbles (C3DBub, game object views)
	int					f_a2c;			// 0xa2c  (not used)
	float				f_a30;			// 0xa30  flashing time (2 or 5 s)
	int					f_a34;			// 0xa34  engine sound channel (-1; never started)
	int					f_a38;			// 0xa38  engine sound state (ev108)
	unsigned char		f_a3c;			// 0xa3c  pushed down while flashing (missile hits)
	float				f_a40[4];		// 0xa40  RotateToDest rates x, y, z (2, 4, 1.1; w 1)
	unsigned char		f_a50;			// 0xa50  claw mode (near a movable rock, ev111)
	unsigned char		f_a51;			// 0xa51  0
	unsigned char		f_a52;			// 0xa52  Action held (once per press)
	CGameObject			*f_a54;			// 0xa54  the rock being carried
	C3DSubBubble		*f_a58;			// 0xa58  the glass bubble
	float				f_a5c;			// 0xa5c  time since the last trail bubble
	int					f_a60;			// 0xa60  last trail bubble used (0..9)
	C3DSpotLight		*f_a64;			// 0xa64  the headlight beam
	unsigned char		f_a68;			// 0xa68  L held (once per press)
	unsigned char		f_a69;			// 0xa69  at the surface (viewport above the water)
	C3DLight			*f_a74;			// 0xa74  the blue light (the original keeps its game object view)
	float				f_a7c;			// 0xa7c  time (the light's pulse)
};

