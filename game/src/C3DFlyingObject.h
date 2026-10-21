/*  C3DFlyingObject - a C3DPlayerObject the player flies: the base of the vehicles that fly (the
    rocket ship C3DRocketShip '3ROC', and others such as the mini sub). It has no factory ID of its
    own (InitObject sets ObjectID '3FLY').
    Reconstructed from Neutron2.exe: constructor 0x4346b0, destructor 0x434930 (deleting thunks
    0x434900 / 0x435f80 / 0x435f90 / 0x435fa0).

    Original layout (classes.json): as C3DPlayerObject (element vtable 0x4dcc70, 110 slots:
    C3DPlayerObject's 106 + 106..109; OMediaAnim vtable 0x4dcc20 at +0x90; listener 0x4dcc5c at
    +0x30; CLocalGameObject vtable 0x4dc7a4 at +0xc0), own members 0x960..0x9f0, vtordisp 0x9f0,
    streamer 0x9f4. Derived classes put their members from 0x9f0 on.

    Flying (only while it is the camera object, g_camera):
      v243     controls: Left/Right (or the mouse x) turn (f_978, up to 45 deg/s, 80 after 1.5 s
               held), Speed boosts the top speed to 2100, Go/Back (or the mouse y) climb / dive
               and speed up / brake; then the chase camera (C3DPlayerObject ev95/ev96/ev98).
      Process  velocity = forward speed along the heading plus a sideways part from the turn
               (lean), vertical acceleration f_9c0 that cancels gravity (UpRate / DownRate, a
               ceiling at MaxHeight), vertical speed clamped to MaxVertVelocity; turn-around
               (TurnAround key / ev109: 180 deg/s for f_9d8 seconds) and barrel rolls (f_9d4
               seconds of 300 deg/s roll, from the BARRELROLL pickups and triggers).
    ev108(state) is told each frame whether it is climbing (2), cruising (1) or slowing (0):
    C3DRocketShip plays its engine sounds from it.

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                            */
#pragma once
#include "C3DPlayerObject.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"

class C3DFlyingObject : public C3DPlayerObject
{
	public:
	C3DFlyingObject();														// 0x4346b0
	virtual ~C3DFlyingObject();												// 0x434930

	// ---- element-family slots
	virtual OMedia3DVector ev106(float x, float y, float z);				// ESLOT 106 0x435d20 position + (x,y,z) turned by (0, heading + f_978, roll), w = 1 (= C3DPlayer ev113)
	virtual void ev107(int a) { (void)a; }									// ESLOT 107 0x464ee0 (empty)
	virtual void ev108(int state) { (void)state; }							// ESLOT 108 0x464ee0 (empty) 2 climbing, 1 cruising, 0 slowing / stopped (v243)
	virtual void ev109(void);												// ESLOT 109 0x434ba0 turn around (if moving): f_9dd, f_9d8 = 1 s

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4349d0 properties, ObjectID '3FLY'
	virtual void ResetObject(void);											// SLOT 10  0x434b10
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x435eb0 BARRELROLL(2) pickups / AI triggers
	virtual void Process(float dt);											// SLOT 240 0x434c30 flight
	virtual void v243(float dt);											// SLOT 243 0x435230 controls + camera
	virtual void RotateToDest(float dt);									// SLOT 246 0x435b60 rates 2 / 4 / 3 (none while rolling)
	virtual void v258(void);												// SLOT 258 0x435ea0 (CLocalGameObject's)

	// ---- data members (object offsets)
	float				f_960;			// 0x960  "AccelRate" (400): speed += AccelRate * 20 * dt
	float				f_964;			// 0x964  "DecelRate" (1): braking (per second on the floor, * 0.1 per frame in the air)
	float				f_968;			// 0x968  "MaxSpeed" (1400): top speed now (2100 with Speed; by difficulty otherwise)
	float				f_96c;			// 0x96c  forward speed
	float				f_970;			// 0x970  35
	float				f_974;			// 0x974  0
	float				f_978;			// 0x978  turn rate, degrees / s (+ left)
	float				f_97c;			// 0x97c  turn rate the movement used (lean)
	float				f_980;			// 0x980  heading at ResetObject
	float				f_984;			// 0x984  "MaxHeight" (2500)
	float				f_988;			// 0x988  "UpRate" (650): vertical acceleration climbing
	float				f_98c;			// 0x98c  "DownRate" (-650): diving
	float				f_990;			// 0x990  "MaxVertVelocity" (650)
	float				f_994;			// 0x994  "NewGravity" (0; not used here)
	int					f_998;			// 0x998  1
	unsigned char		f_99c;			// 0x99c  climbing this frame (Go / mouse up)
	unsigned char		f_99d;			// 0x99d  diving this frame (Back / mouse down)
	unsigned char		f_99e;			// 0x99e  turning by keys (DEFAULT animation and f_978 easing after)
	unsigned char		f_99f;			// 0x99f  0
	OMedia3DPoint		f_9a0;			// 0x9a0  camera offset (0, 200, -500)
	OMedia3DPoint		f_9b0;			// 0x9b0  camera look offset (0, 50, 200)
	float				f_9c0;			// 0x9c0  vertical acceleration wanted (+ gravity cancelled)
	float				f_9c4;			// 0x9c4  "AccelLean" (20): pitch when climbing
	float				f_9c8;			// 0x9c8  "DecelLean" (-20): pitch when diving
	float				f_9cc;			// 0x9cc  time in the air (0 on the floor)
	float				f_9d0;			// 0x9d0  mouse y, summed (> 15 climbs, < -15 dives; eases back)
	float				f_9d4;			// 0x9d4  barrel roll time left
	float				f_9d8;			// 0x9d8  turn-around time left
	unsigned char		f_9dc;			// 0x9dc  play "barrelroll" when the roll starts
	unsigned char		f_9dd;			// 0x9dd  turning around (ev109)
	unsigned char		f_9de;			// 0x9de  turn around next frame
	unsigned char		f_9df;			// 0x9df  barrel roll direction: 1 one way (+300 deg/s), 0 the other (1)
	int					f_9e0;			// 0x9e0  0
	float				f_9e4;			// 0x9e4  Left (or mouse left) held, s
	float				f_9e8;			// 0x9e8  Right (or mouse right) held, s
	unsigned char		f_9ec;			// 0x9ec  slow speeds (C3DRocketShip: parachute open)
};
