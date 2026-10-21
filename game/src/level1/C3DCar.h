/*  C3DCar - a C3DVehicle with its wheels and its chase camera: SetupCar4 makes four C3DWheel
    objects (ev141, linked both ways) or six (ev142), the engine plays "Scooter" (3D, looped)
    pitched by the speed. Power-up 0 raises its power to 350 (ev107), power-up 2 its v62 (0.61,
    else 0.21). It leans into turns (roll -steering / 2, ev132).
    The camera (ev110), by f_a38 (keys 1..9, C / V, B in C3DVehicle::v243):
      1  behind (-30, 0, -200) +50 up, catching up 1x / 1.7x / 2.8x a second (beyond 200 / 400
         from it; 3.5x while braking)          2  left where it is
      3, 4  behind and above, farther with speed   5  behind at the heading plus the steering
      6  in it (the vehicle hidden)   7  just above it   8  in front, looking back (B)
      9..12  beside a wheel   13, 16, 18  rising 4000 above it   15, 17, 19  towards (0, 3000, 1500)
      14, 20  500 behind and 1000 up (20: looking down)
    It looks at 100 in front of it (50 up), or with its own angles (6, 7, 9..12); crashed, it
    drifts up above it. With the camera collision on (g_56af42) the probe (C3DCamera) keeps it
    out of the walls. A rear-view mirror (C3DVehicle ev139) is placed 30 behind, facing back.
    Reconstructed from Neutron2.exe: constructor 0x4244c0 (no factory ID; ObjectID '3CAR'),
    destructor 0x424640 (deleting thunks 0x425800 / 0x4257f0 / 0x424610 / 0x425810).
    Vtables: element 0x4d2a48 (144 slots: C3DVehicle's 141 + 141..143; 110, 126, 132
    overridden; 114..122, 127, 128 = 0x4655c0, 0), game object (+0xc0) 0x4d257c (0, 10, 240
    overridden), streamer 0x4d256c. Own members 0xa50..0xa84, vtordisp 0xa84, streamer 0xa88.
    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                             */
#pragma once
#include "C3DVehicle.h"

class C3DCar : public C3DVehicle
{
	public:
	C3DCar();																// 0x4244c0
	virtual ~C3DCar();														// 0x424640

	// ---- element-family slots
	virtual void ev110(float dt);											// ESLOT 110 0x424dc0 the camera
	virtual unsigned char ev114(void) { return 0; }							// ESLOT 114..122 0x4655c0: no camera keys
	virtual unsigned char ev115(void) { return 0; }
	virtual unsigned char ev116(void) { return 0; }
	virtual unsigned char ev117(void) { return 0; }
	virtual unsigned char ev118(void) { return 0; }
	virtual unsigned char ev119(void) { return 0; }
	virtual unsigned char ev120(void) { return 0; }
	virtual unsigned char ev121(void) { return 0; }
	virtual unsigned char ev122(void) { return 0; }
	virtual unsigned char ev126(void);										// ESLOT 126 0x424d90 Back: joystick button 1 or PlayerKeyBack
	virtual unsigned char ev127(void) { return 0; }							// ESLOT 127 0x4655c0: no boost
	virtual unsigned char ev128(void) { return 0; }							// ESLOT 128 0x4655c0: no camera toggle
	virtual void ev132(void);												// ESLOT 132 0x424cc0 lean, then the wheels
	virtual void ev141(float zf, float zr, float xf, float xr, float y, int shape_f, int shape_r);	// ESLOT 141 0x4246f0 "SetupCar4": four wheels
	virtual void ev142(float zf, float zm, float zr, float xf, float xm, float xr, float y,
					   int shape_f, int shape_m, int shape_r);				// ESLOT 142 0x4248f0 "SetupCar4": six wheels (4, 5 in the middle)
	virtual void ev143(void) {}												// ESLOT 143 0x464ee0 (empty, one argument in the original)

	// ---- CGameObject-family overrides
	virtual void ResetObject(void);											// SLOT 10  0x424d20 back to f_a50 / f_a60
	virtual void Process(float dt);											// SLOT 240 0x424b30 engine sound, power-ups

	// ---- data members (object offsets)
	OMedia3DVector		f_a50;			// 0xa50  start position (ResetObject; w 1)
	OMedia3DVector		f_a60;			// 0xa60  start rotation (ResetObject; w 1)
	float				f_a70;			// 0xa70  time (+= dt)
	float				f_a74;			// 0xa74  time (+= dt)
	int					f_a78;			// 0xa78  the "Scooter" channel (-1)
	int					f_a7c;			// 0xa7c  its base frequency (11025)
	unsigned char		f_a80;			// 0xa80  0 (C3DJeep: Jump held)
	unsigned char		f_a81;			// 0xa81  power-up 0 applied
	unsigned char		f_a82;			// 0xa82  power-up 2 applied
};
