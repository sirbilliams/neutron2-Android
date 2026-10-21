/*  C3DLunarLander ('3LUN') - the lunar lander of the museum's moon exhibit (Level5d). Jimmy gets
    in through the AITrigger "LUNARLANDER" (his ActivateMenuItem item 13: it is launched upwards,
    "rocket blast", becomes the camera object, its fuel 100) and flies it to the object tagged
    "LANDINGTARGET". While it is the camera object (v243):
      the camera stays where it was put on the first frame (300 above and 600 behind JIM1) and
      looks at the lander (100 above it); the gadget bar shows the fuel;
      Left / Right (or the mouse) push it sideways at 100 along z and cost 5 fuel / s, else its z
      speed decays by 20 / s to 0; Go, Action or Jump thrust (THRUST, +10 up per frame, 11 fuel
      / s, the "rocket blast" loop; out of fuel: -5 and HOVER); nothing: HOVER, -3 per frame;
      Back (or the "deactivate item" key, or more than 10 landings) gets Jimmy out (item 0).
    Touching the ground (v222) it lands: slower than 40 down: LAND (within 120 of the target:
    500 points, "cheering"); slower than 150: within 180 100 points ("pickup3"); slower than
    250: within 310 20 points ("pickup2"); faster: CRASH ("rock explosion2", stopped). Each
    landing gets Jimmy out. Not driven, it hovers down (HOVER, -3) and a touch-down crashes it
    back onto the landing target (ev144).
    Reconstructed from Neutron2.exe: constructor 0x4509a0 (factory '3LUN', 0x1b7c bytes),
    destructor 0x450b40 (vtable resets only, then ~C3DAI; deleting thunks 0x450b10 / 0x4516b0 /
    0x4516c0 / 0x4516d0).

    Original layout: as C3DAI (element vtable 0x4e8160, 145 slots: C3DAI's 144 + 144;
    CLocalGameObject vtable 0x4e7c94 at +0xc0), own members 0x1b50..0x1b74, vtordisp 0x1b74,
    streamer 0x1b78. Its InitObject and slot 260 skip C3DAI's (C3DAnimated's are called).      */
#pragma once
#include "../C3DAI.h"
#include "OMedia3DVector.h"

class C3DLunarLander : public C3DAI
{
	public:
	C3DLunarLander();														// 0x4509a0
	virtual ~C3DLunarLander();												// 0x450b40

	// ---- element-family slots (new)
	virtual void ev144(void);												// ESLOT 144 0x450d20 back onto the landing target

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x450b90 the model (C3DAnimated's InitObject)
	virtual void v222(void);												// SLOT 222 0x451270 touched down: land or crash
	virtual void Process(float dt);											// SLOT 240 0x450c90 not driven: hover down
	virtual void v243(float dt);											// SLOT 243 0x450d80 driven (the camera object)
	virtual void v260(void);												// SLOT 260 0x46fc00 (a jump to C3DAnimated's)
	virtual void v277(void);												// SLOT 277 0x451610 pause: + the thrust sound
	virtual void v278(void);												// SLOT 278 0x451630 resume: + the thrust sound

	OMedia3DVector		m_cam_pos;		// 0x1b50 the camera's place (JIM1's v224(0, 300, -600)) (0, 0, 0, 1)
	CGameObject			*m_jim;			// 0x1b60 JIM1, found on the first driven frame (0)
	unsigned char		m_landed;		// 0x1b64 down (1); cleared in the air while driven
	float				m_fuel;			// 0x1b68 (100)
	int					m_channel;		// 0x1b6c "rocket blast" (-1)
	int					m_landings;		// 0x1b70 landings while driven (0)
};
