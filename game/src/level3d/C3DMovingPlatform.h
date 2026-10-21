/*  C3DMovingPlatform ('3MOP') - a moving platform (an elevator or, with IsBoat, a rocking
    boat): shape 26 of "objects.omt" (shape 37 and radius 235 in the Level5 B levels LV5B /
    LV5A / L5B1), unlit, driven by its AITriggers like any C3DAI (no animations, speed 250,
    state 3, no gravity). "elevator" loops (3D) while it is in state 3.
    The camera (Jimmy) touching it within its radius (200) boards it: his speed is zeroed, he
    is set to its height, his camera axis factors f_af0 are saved and set to (3, 5, 3). While
    he rides, he gets its velocity, stays at its height, his forward speed decays, his shadow
    follows 50 below him; he gets off (the factors restored) when he is farther than the
    radius or presses jump (a press held from boarding does not count). A boat rocks about x
    and z with the global pulse (g_545ddc).
    Reconstructed from Neutron2.exe: constructor 0x455810 (factory '3MOP', 0x1b7c bytes),
    destructor 0x455b30 (only resets the vtables, then ~C3DAI; deleting thunks 0x456300 /
    0x455b00 / 0x4562f0 / 0x456310).

    Original layout: as C3DAI (element vtable 144 slots, all C3DAI's; CLocalGameObject vtable
    at +0xc0), own members 0x1b50..0x1b74, vtordisp 0x1b74, streamer 0x1b78. Slots 15 and 17
    (0x456280 / 0x456270) only call CGameObject's / C3DAI's, so they are not overridden.     */
#pragma once
#include "../C3DAI.h"

class C3DMovingPlatform : public C3DAI
{
	public:
	C3DMovingPlatform();													// 0x455810
	virtual ~C3DMovingPlatform();											// 0x455b30

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x455b80 + "IsBoat"
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x456090 the camera boards
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x456290 collides with the camera
	virtual void Process(float dt);											// SLOT 240 0x455bc0 the elevator sound, the rider, rocking
	virtual void v260(void);												// SLOT 260 0x456060 C3DAI's, then v42(0), v40(0)

	unsigned char		m_riding;		// 0x1b50 the camera is on board (= 0)
	int					m_is_boat;		// 0x1b54 "IsBoat" (= 0)
	float				m_radius;		// 0x1b58 boarding distance (200 / 235)
	float				m_saved_axis[3];// 0x1b5c the rider's f_af0 before boarding (= 0)
	float				f_1b68;			// 0x1b68 (= 1, not used)
	int					m_channel;		// 0x1b6c "elevator" (= -1)
	unsigned char		m_jump_held;	// 0x1b70 jump was held when boarding (= 0)
};
