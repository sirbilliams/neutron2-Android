/*  C3DEnergyProjectile ('3EPR') - the VR tank's shot (created at run time; C3DVRTank's
    constructor makes 10): sprite 20 of "retainedsprites.omt" (size 65) spinning at 250 degrees
    / s, with a trail sprite (31, size 100) that is left where it hits a wall (200 back along
    its flight) and fades out in 1 s. It may home on a target (f_604, after f_608 seconds, by
    skill within 1800 / 1400 / 800, keeping the speed f_610). It collides with bugs, the giant
    wasp and the moving targets only, and pops ("vrzap") on walls, ceilings, floors, C3DBOTs and
    moving targets.
    Reconstructed from Neutron2.exe: factory '3EPR', constructor 0x430640 (0x628 bytes),
    destructor 0x430980 (only resets the vtables, then ~C3DPermanentSprite; deleting thunks
    0x430950 / 0x431140 / 0x431150). Vtables: element 0x4d9b7c?, game object (+0xc8) (0, 16, 219,
    220, 222, 231, 240, 260 overridden). Own members 0x600..0x620, vtordisp 0x620, streamer 0x624.
    The non-virtual 0x430a10 (Pop), 0x430c10 (Reset), 0x430cd0 (Fire) and 0x430d40 (Home) are
    element-family functions the tank calls too.                                              */
#pragma once
#include "../C3DPermanentSprite.h"
#include "OMedia3DVector.h"

class OMedia3DPolygon;

class C3DEnergyProjectile : public C3DPermanentSprite
{
	public:
	C3DEnergyProjectile();													// 0x430640
	virtual ~C3DEnergyProjectile();											// 0x430980

	// ---- not virtual
	void Pop(void);															// 0x430a10 "vrzap", stopped, switched off, hidden
	void Reset(void);														// 0x430c10 the same without the sound
	void Fire(OMedia3DVector dir, float speed);								// 0x430cd0 velocity dir * speed, on, shown
	void Home(CGameObject *target, float dt);								// 0x430d40 pull towards target, speed f_610

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x430c90 pops on C3DBOT / C3DMOVINGTARGET
	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x430ad0 ceiling: pop
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x430ae0 wall: the trail stays, pop
	virtual void v222(void);												// SLOT 222 0x430ac0 floor: pop
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x4310f0 collides with '3BUG', '3GWA', '3TAR' only
	virtual void Process(float dt);											// SLOT 240 0x430f60 spin, homing, the trail fades
	virtual void v260(void);												// SLOT 260 0x4309c0 sprite, physics, hidden, faces the viewport

	// ---- data members (object offsets)
	C3DPermanentSprite	*f_600;			// 0x600 the trail sprite (the original keeps its CGameObject part)
	CGameObject			*f_604;			// 0x604 target to home on (NULL: none)
	float				f_608;			// 0x608 time before the homing starts
	unsigned char		f_60c;			// 0x60c homing
	float				f_610;			// 0x610 homing speed
	float				f_614;			// 0x614 (cleared, not read)
	float				f_618;			// 0x618 (set by the tank, not read)
	float				f_61c;			// 0x61c the trail's fade time left (1 when it is left behind)
};
