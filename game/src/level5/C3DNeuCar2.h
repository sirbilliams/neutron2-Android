/*  C3DNeuCar2 ('3NC2') - one of the cars on Retroville's streets (Level5c): a C3DAICar with the
    morph model "car.ase" and the texture "cargreen.png" (green), radius 300, speed 20 until its patrol
    points or the C3DAICar timer give it another. It plays "DEFAULT" (looped).
    Reconstructed from Neutron2.exe: constructor 0x4594f0 (factory '3NC2', 0x1b74 bytes; it inlines
    InitObject), InitObject 0x4596f0, destructor (vtable resets only, then ~C3DAICar; deleting
    thunks 0x459670 / 0x4597a0 / 0x4597b0 / 0x4597c0).

    Original layout: as C3DAICar (element vtable 0x4ecdb4, 144 slots, all inherited), no own members.
*/
#pragma once
#include "C3DAICar.h"

class C3DNeuCar2 : public C3DAICar
{
	public:
	C3DNeuCar2();																// 0x4594f0
	virtual ~C3DNeuCar2();

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4596f0 the model
};
