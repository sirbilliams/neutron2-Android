/*  C3DNeuCar ('3NCA') - one of the cars on Retroville's streets (Level5c): a C3DAICar with the
    morph model "car.ase" and the texture "carred.png" (red), radius 300, speed 20 until its patrol
    points or the C3DAICar timer give it another. It plays "DEFAULT" (looped).
    Reconstructed from Neutron2.exe: constructor 0x459210 (factory '3NCA', 0x1b74 bytes; it inlines
    InitObject), InitObject 0x459410, destructor (vtable resets only, then ~C3DAICar; deleting
    thunks 0x459390 / 0x4594c0 / 0x4594d0 / 0x4594e0).

    Original layout: as C3DAICar (element vtable 0x4ec68c, 144 slots, all inherited), no own members.
*/
#pragma once
#include "C3DAICar.h"

class C3DNeuCar : public C3DAICar
{
	public:
	C3DNeuCar();																// 0x459210
	virtual ~C3DNeuCar();

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x459410 the model
};
