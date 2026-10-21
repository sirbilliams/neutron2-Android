/*  C3DPlanet ('3PLA') - a planet of the solar-system model (Level5d): a sprite (sprites.omt
    canvas 200, size 150) that circles the object tagged "SUN". When its properties are loaded it
    measures its distance to the sun (the orbit radius) and gets a push of 0.2 along (z, 0, x)
    of the unit vector to the sun; each frame it is pulled towards the sun (0.1 * dt of the
    offset) and gets (k, 0, k) more velocity, k = 0.1 * dt outside its orbit radius and
    -1.5 * dt inside it.
    Reconstructed from Neutron2.exe: constructor 0x45fa90 (factory '3PLA', 0x678 bytes),
    destructor 0x45fc40 (vtable resets only, then ~C3DSpriteType; deleting thunks 0x45fc10 /
    0x45fec0 / 0x45fed0).

    Original layout: as C3DSpriteType (element vtable 0x4f178c, 55 slots: C3DSprite's;
    CLocalGameObject vtable 0x4f12fc at +0xc8), own members 0x668..0x670, vtordisp 0x670,
    streamer 0x674. Members are named after their 2002 offset from the object start (the
    CGameObject-family methods of the original see them 0xc8 lower).                         */
#pragma once
#include "../C3DSpriteType.h"

class C3DPlanet : public C3DSpriteType
{
	public:
	C3DPlanet();															// 0x45fa90
	virtual ~C3DPlanet();													// 0x45fc40

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x45fd70 pulled to the sun
	virtual void v260(void);												// SLOT 260 0x45fc80 the sun, the orbit, the first push

	// FIELD 0x668 0x8
	CGameObject		*m_sun;				// 0x668  the object tagged "SUN" (= 0)
	float			m_orbit;			// 0x66c  distance to the sun when loaded (= 0)
};
