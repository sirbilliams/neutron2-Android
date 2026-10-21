/*  C3DVehicle ('3VEH') - the base of the wheeled vehicles (C3DCar -> C3DJeep, the Goddard
    scooter of level1_a): a C3DPlayerObject on up to six C3DWheel objects (f_960, placed at
    f_988[i] in the vehicle's frame; wheels 0 and 1 steer). While it is the camera object (v243):
      Left / Right (or the mouse x) steer (f_a24, +-45 degrees, 200 / s; the keys centre it
      again when let go); Go accelerates (f_9f8 / s up to the top speed f_a10 = 0.66 of the
      power f_a08; Boost: 0.825 of it), Back brakes (f_a00 / s) then reverses (down to -0.4 of
      the top speed, a rear-view mirror while reversing: ev139 / ev140); the joystick (ev112)
      steers and drives; the keys 1..9, C / V and B pick the camera (f_a38, ev110).
    Process: the wheels on the floor (front f_9e8, rear f_9ea) turn it (f_a28, by speed),
    friction, braking on slopes, and drive the velocity along the heading (or the way it slides).
    ev132 (from v248) puts the wheels under it: suspension (f_a18 down, f_a1c up), spin
    (rolling: wheel f_58c, degrees), steering. A hit wall head-on above speed 20 (with f_a43)
    crashes it (ev131): no control for 4 s (ev111), then upright again.
    Reconstructed from Neutron2.exe: constructor 0x49d370 (factory '3VEH', 0xa58 bytes),
    destructor 0x49d630 (deleting thunks 0x4a08e0 / 0x4a08d0 / 0x49d600 / 0x4a08f0).
    Vtables: element 0x51a798 (141 slots: C3DPlayerObject's 106 + 106..140; 55 overridden),
    game object (+0xc0) 0x51a2cc (0, 7, 10, 11, 218, 220, 222, 240, 243, 246, 247, 248
    overridden), streamer 0x51a2bc. Own members 0x960..0xa50, vtordisp 0xa50, streamer 0xa54.
    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                             */
#pragma once
#include "../C3DPlayerObject.h"
#include "OMedia3DVector.h"

class C3DObject;
class OMediaViewPort;
class OMedia3DPolygon;

class C3DVehicle : public C3DPlayerObject
{
	public:
	C3DVehicle();															// 0x49d370
	virtual ~C3DVehicle();													// 0x49d630

	// ---- element-family slots
	virtual void HideAgain(void);											// ESLOT 55  0x49d710 + the wheels
	virtual void ev106(void);												// ESLOT 106 0x49d740 shown again, with the wheels (hide(false))
	virtual void ev107(float power);										// ESLOT 107 0x49d770 power: accel, brake, top speed
	virtual void ev108(float d);											// ESLOT 108 0x49d7e0 power += d
	virtual void ev109(float x, float y, float z);							// ESLOT 109 0x49d980 move it and its wheels, the viewport too
	virtual void ev110(float dt);											// ESLOT 110 0x49e710 the camera (TODO: C3DCar's is reconstructed)
	virtual void ev111(float dt);											// ESLOT 111 0x49f300 crashed: 4 s, then upright
	virtual void ev112(void);												// ESLOT 112 0x49f440 joystick: steer, drive
	virtual void ev113(short *dx, short *dy);								// ESLOT 113 0x49f4f0 mouse movement (0x4a3990)
	virtual unsigned char ev114(void);										// ESLOT 114 0x49f510 key 1 (camera 6)
	virtual unsigned char ev115(void);										// ESLOT 115 0x49f520 key 2 (camera 3)
	virtual unsigned char ev116(void);										// ESLOT 116 0x49f530 key 3 (camera 1)
	virtual unsigned char ev117(void);										// ESLOT 117 0x49f540 key 4 (camera 2)
	virtual unsigned char ev118(void);										// ESLOT 118 0x49f550 key 5 (camera 11)
	virtual unsigned char ev119(void);										// ESLOT 119 0x49f560 key 6 (camera 12)
	virtual unsigned char ev120(void);										// ESLOT 120 0x49f570 key 7 (camera 20)
	virtual unsigned char ev121(void);										// ESLOT 121 0x49f580 key 8 (camera 13)
	virtual unsigned char ev122(void);										// ESLOT 122 0x49f590 key 9 (camera 15)
	virtual unsigned char ev123(void);										// ESLOT 123 0x49f5a0 Left
	virtual unsigned char ev124(void);										// ESLOT 124 0x49f5b0 Right
	virtual unsigned char ev125(void);										// ESLOT 125 0x49f5c0 Go (rear wheels down)
	virtual unsigned char ev126(void);										// ESLOT 126 0x49f5f0 Back (rear wheels down)
	virtual unsigned char ev127(void);										// ESLOT 127 0x49f630 Boost (rear wheels down)
	virtual unsigned char ev128(void);										// ESLOT 128 0x49f680 C / V pressed: next camera
	virtual unsigned char ev129(void);										// ESLOT 129 0x49f6b0 B held: look back (camera 8)
	virtual unsigned char ev130(void);										// ESLOT 130 0x49f6c0 Backspace pressed
	virtual void ev131(void);												// ESLOT 131 0x49f910 crash: stop, 4 s without control
	virtual void ev132(void);												// ESLOT 132 0x49f990 the wheels: place, spin, steer; tilt to the floor
	virtual void ev133(void) {}												// ESLOT 133 0x463890 (empty)
	virtual void ev134(void);												// ESLOT 134 0x4a02a0 in the air: pitch 340
	virtual OMedia3DVector ev135(float x, float y, float z);				// ESLOT 135 0x4a02d0 (x, y, z) turned by the heading only, w 0
	virtual OMedia3DVector ev136(float x, float y, float z);				// ESLOT 136 0x4a03b0 = v224: local point -> world
	virtual OMedia3DVector ev137(float x, float y, float z);				// ESLOT 137 0x4a0400 v224 with the steering added to the heading
	virtual OMedia3DVector ev138(float x, float y, float z);				// ESLOT 138 0x4a05a0 local point -> world by the element's angles
	virtual void ev139(void);												// ESLOT 139 0x4a06c0 the rear-view mirror on
	virtual void ev140(void);												// ESLOT 140 0x4a0880 the rear-view mirror off

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x49d840 '3VEH'
	virtual void ResetObject(void);											// SLOT 10  0x49d8b0 stopped, rotation kept in f_978
	virtual void InitPhysics(void);											// SLOT 11  0x49d880
	virtual void v218(float a, float b, float c);							// SLOT 218 0x49f400 + f_278 = f_228
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x49f6e0 walls: crash, slow down
	virtual void v222(void) {}												// SLOT 222 0x456d10 (empty)
	virtual void Process(float dt);											// SLOT 240 0x49da30 wheels, turning, friction, driving
	virtual void v243(float dt);											// SLOT 243 0x49e040 the controls, then the camera
	virtual void RotateToDest(float dt);									// SLOT 246 0x49f430 (CGameObject's)
	virtual void v247(void);												// SLOT 247 0x49f960 (C3DObject's)
	virtual void v248(void);												// SLOT 248 0x49f970 the wheels (ev132), then C3DObject's

	// ---- data members (object offsets)
	C3DObject			*f_960[6];		// 0x960  the wheels (C3DWheel, element pointers; C3DCar makes them)
	OMedia3DVector		f_978;			// 0x978  rotation at ResetObject (w = 1 at first)
	OMedia3DVector		f_988[6];		// 0x988  wheel positions in our frame (w 1)
	short				f_9e8;			// 0x9e8  front wheels (0, 1) on the floor
	short				f_9ea;			// 0x9ea  other wheels on the floor
	short				f_9ec;			// 0x9ec  all wheels on the floor
	float				f_9f0;			// 0x9f0  power at ResetObject (100)
	float				f_9f4;			// 0x9f4  power (100; ev107)
	float				f_9f8;			// 0x9f8  acceleration / s (700)
	float				f_9fc;			// 0x9fc  = f_9f8
	float				f_a00;			// 0xa00  braking / s (1200)
	float				f_a04;			// 0xa04  engine level (speed * 1.515, or f_a08 accelerating; the engine sound)
	float				f_a08;			// 0xa08  top power (2250)
	float				f_a0c;			// 0xa0c  speed this frame (v243)
	float				f_a10;			// 0xa10  top speed (1485 = 0.66 * f_a08; boost 0.825)
	float				f_a14;			// 0xa14  = f_a10
	float				f_a18;			// 0xa18  suspension: down (15)
	float				f_a1c;			// 0xa1c  suspension: up (15)
	float				f_a20;			// 0xa20  grip this frame (Process)
	float				f_a24;			// 0xa24  steering, degrees (+-45)
	float				f_a28;			// 0xa28  turn rate (Process)
	float				f_a2c;			// 0xa2c  heading at ResetObject
	float				f_a30;			// 0xa30  sliding time (Process)
	float				f_a34;			// 0xa34  crashed time (ev111)
	int					f_a38;			// 0xa38  camera (1; ev110)
	int					f_a3c;			// 0xa3c  camera before B (looking back)
	unsigned char		f_a40;			// 0xa40  accelerating
	unsigned char		f_a41;			// 0xa41  braking (no grip)
	unsigned char		f_a42;			// 0xa42  0 (unused)
	unsigned char		f_a43;			// 0xa43  walls crash it (1)
	unsigned char		f_a44;			// 0xa44  crashed
	unsigned char		f_a45;			// 0xa45  steered by the keys (centre again)
	unsigned char		f_a46;			// 0xa46  the rear-view mirror always on
	unsigned char		f_a47;			// 0xa47  Back held
	unsigned char		f_a48;			// 0xa48  boost
	unsigned char		f_a49;			// 0xa49  joystick: accelerate
	unsigned char		f_a4a;			// 0xa4a  joystick: back
	OMediaViewPort		*f_a4c;			// 0xa4c  the rear-view mirror
};
