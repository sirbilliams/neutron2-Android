/*  C3DTrampoline ('3TRA') - a trampoline ("trampoline.ase", BOUNCE "trampoline2.ase",
    "trampoline.png"). Only the camera (Jimmy) collides with it. Landing on it ("boing", BOUNCE)
    holds him for the bounce (0.5 s): from 40 above it he sinks 330 / s and comes back up, then
    is thrown up at 0.8 x the speed he landed with (1.2 x with jump held), kept within
    MinStrength (= 400) .. MaxStrength (= 1000).
    Reconstructed from Neutron2.exe: constructor 0x476cc0 (factory '3TRA', 0x954 bytes),
    destructor 0x476e70 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x477410 /
    0x476e40 / 0x477400 / 0x477420).

    Original layout: as C3DAnimated (element vtable 94 slots; OMediaAnim vtable at +0x90;
    listener at +0x30; CLocalGameObject vtable 0x4fff4c at +0xc0), own members 0x924..0x94c,
    vtordisp 0x94c, streamer 0x950. Slot 17 (0x47fca0) only calls CGameObject's, so it is not
    overridden here.                                                                          */
#pragma once
#include "../C3DAnimated.h"

class C3DTrampoline : public C3DAnimated
{
	public:
	C3DTrampoline();														// 0x476cc0
	virtual ~C3DTrampoline();												// 0x476e70

	// ---- element-family slots
	virtual void ev67(void);												// ESLOT 67  0x477080 BOUNCE ended: DEFAULT

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x476ec0 properties, animations, texture
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x476f90 the camera lands: boing
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x4770c0 hold, then throw the camera

	float		m_timer;				// 0x924  bounce time left (= 0)
	float		m_bounce_time;			// 0x928  bounce length (= 0.5 s)
	float		m_min_strength;			// 0x92c  "MinStrength" (= 400)
	float		f_930;					// 0x930  (= 1, not used)
	float		m_pos[4];				// 0x934  where the camera is held (its position, y 40 above ours)
	float		m_vel_y;				// 0x944  the camera's vertical speed when it landed
	float		m_max_strength;			// 0x948  "MaxStrength" (= 1000)
};
