/*  C3DWaterPlane ('3WAT') - the water surface (tag "C3DWATERPLANE"): a morph object, animation
    DEFAULT from "watertest.ase" / "watertest.png" in Level3a ('LV3A'), "cavewater.ase" /
    ".png" elsewhere; alpha 0.7, second pass, two-sided.
    In Level3a it only sets its alpha every frame ((cos(4 t) + 1) / 2 with t never advancing).
    Elsewhere, while there is a camera (Process):
      - its level moves to a target (ev94: relative, ev95: absolute with the "waterfalls"
        loop and the WATERSLOW/FASTLEFT/RIGHT objects shown) at f_92c per s; there the four
        objects are hidden and the loop stops;
      - the camera (Jimmy) going below 6 under the surface swims: SWIM, "SPLASH" when falling
        faster than 400; with the viewport below the surface the "musictomb1.omt" music and a
        blue fog (ev97), above it the normal fog (ev96); in play mode, after 3 s an oxygen
        bar (the gadget bar) runs down over "f_940" (40 s), then the energy drains and Jimmy
        is hurt (the OXYGEN pickup puts f_93c back to 3); speed kept to 300, under 100 FLOAT
        (+ "sloshing"), else SWIM (+ "swim"); buoyancy 1.4 g (0.9 g with Jump held), the
        vertical speed kept to -200 / 90 (-80 with Jump); more than 7 above it he is out
        (top speed 600 again).
    Reconstructed from Neutron2.exe: constructor 0x47e420 (factory '3WAT', 0x950 bytes),
    destructor 0x47e5d0 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x47f7c0 /
    0x47f7b0 / 0x47e5a0 / 0x47f7d0). Vtables 0x505b08 (element, 98 slots: C3DAnimated's 94 +
    94..97) / 0x505af4 (listener) / 0x505ab8 / 0x50563c (game object at +0xc0: 0, 7, 240, 260,
    284, 285 overridden) / 0x50562c (streamer). Own members 0x924..0x948, vtordisp 0x948,
    streamer 0x94c. Element-family slots are `ev<n>` (`// ESLOT n`).                          */
#pragma once
#include "../C3DAnimated.h"

class C3DWaterPlane : public C3DAnimated
{
	public:
	C3DWaterPlane();														// 0x47e420
	virtual ~C3DWaterPlane();												// 0x47e5d0

	// ---- element-family slots of its own
	virtual void ev94(float dy, float speed);								// ESLOT 94 0x47f420 level to here + dy (when not moving)
	virtual void ev95(float y, float speed);								// ESLOT 95 0x47f470 level to y, "waterfalls" (when not moving)
	virtual void ev96(void);												// ESLOT 96 0x47f390 viewport above the water: music off, normal fog
	virtual void ev97(void);												// ESLOT 97 0x47f2d0 viewport under the water: music, blue fog

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x47e620 the water animation
	virtual void Process(float dt);											// SLOT 240 0x47e6e0
	virtual void v260(void);												// SLOT 260 0x47f590 (re)load: JIM1, alpha, hide the waterfalls
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x47f710
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x47f680

	// FIELD 0x924 0x24
	float				f_924;			// 0x924  time (+= dt; not in Level3a)
	float				f_928;			// 0x928  level being moved to (-99999: none)
	float				f_92c;			// 0x92c  its speed (50)
	unsigned char		f_930;			// 0x930  the camera is in the water
	unsigned char		f_931;			// 0x931  the underwater music / fog is on
	int					f_934;			// 0x934  the "swim" / "sloshing" channel (-1)
	int					f_938;			// 0x938  the "waterfalls" channel (-1)
	float				f_93c;			// 0x93c  time under water in play mode (the oxygen)
	float				f_940;			// 0x940  oxygen time after the first 3 s (40)
	CGameObject			*f_944;			// 0x944  the object tagged "JIM1" (not used)
};
