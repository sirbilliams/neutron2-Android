/*  C3DPendulum ('3PEN') - a swinging morph object. In the level1 run (L1_A, L1_B, L1_C) it is the
    swing hood ("swinghood.ase" / ".png", radius-flag v34(1)): it hangs still until Jimmy (the
    camera object) touches it, then swings about x, 2 * strength * sin(5 * t), the strength set to
    7 and fading by 3 per s. Elsewhere it is the pendulum ("pendulum.ase" / ".png", radius 100):
    it swings for ever about z, "SwingHeight" (60, degrees) * sin(t), the phase starting at a
    random 0..2.5; while the stopwatch slows the enemies (time scale not 1) at a quarter speed.
    Collides with the camera object and '3JIM' only.
    Reconstructed from Neutron2.exe: constructor 0x45cc60 (factory '3PEN', 0x940 bytes),
    destructor 0x45ce20 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x45d200 /
    0x45d1f0 / 0x45cdf0 / 0x45d210). Vtables: element 0x4efa58 (C3DAnimated's 94 slots), game
    object (+0xc0) 0x4ef58c (0, 7, 16, 17, 231, 240 overridden), streamer 0x4ef57c.
    Own members 0x924..0x934, vtordisp 0x938, streamer 0x93c.                                  */
#pragma once
#include "../C3DAnimated.h"

class C3DPendulum : public C3DAnimated
{
	public:
	C3DPendulum();															// 0x45cc60
	virtual ~C3DPendulum();													// 0x45ce20

	virtual void InitObject(void);											// SLOT 7   0x45cea0 property, animation, texture
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x45cfa0 the camera: swing (strength 7)
	virtual void RemoveTouching(CGameObject *other);						// SLOT 17  0x47fca0 (just CGameObject's)
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x45ce70 collides with the camera, '3JIM' only
	virtual void Process(float dt);											// SLOT 240 0x45cfd0 swinging

	// ---- data members (object offsets)
	float				f_924;			// 0x924  phase (s)
	float				f_928;			// 0x928  "SwingHeight" (60)
	float				f_92c;			// 0x92c  (unused)
	float				f_930;			// 0x930  swing hood: strength (7 when touched, -3 / s)
	float				f_934;			// 0x934  (unused)
};
