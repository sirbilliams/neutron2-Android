/*  C3DWheel ('3WHE') - a vehicle's wheel: a C3DObject whose shape is chunk <n> of the vehicle
    database (ev56), placed and linked by C3DCar's SetupCar4 (ev141 / ev142). It has its own
    physics (the vehicle counts the wheels on the floor) but collides with nothing, touches
    nothing and does not drive its element (v247 / v248 empty: the vehicle places it).
    Reconstructed from Neutron2.exe: constructor 0x4a0920 (factory '3WHE', 0x598 bytes),
    destructor 0x4a0ab0 (deleting thunks 0x4a0c50 / 0x4a0c40 / 0x4a0a80 / 0x4a0c60).
    Original layout: as C3DObject (element vtable 0x51aef0, 57 slots: + 56; CLocalGameObject
    vtable 0x51aa24 at +0xc0: 7, 11, 16, 231, 240, 247, 248 overridden), own member 0x58c,
    vtordisp 0x590, streamer 0x594.                                                           */
#pragma once
#include "../C3DObject.h"

class C3DWheel : public C3DObject
{
	public:
	C3DWheel();																// 0x4a0920
	virtual ~C3DWheel();													// 0x4a0ab0

	virtual void ev56(int id);												// ESLOT 56 0x4a0ba0 shape: vehicle database chunk <id>

	virtual void InitObject(void);											// SLOT 7   0x4a0b50
	virtual void InitPhysics(void);											// SLOT 11  0x4a0bd0 once
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x464ee0 (nothing)
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x434470 collides with nothing
	virtual void Process(float dt);											// SLOT 240 0x4a0c10 (C3DObject's)
	virtual void v247(void);												// SLOT 247 0x463890 (nothing)
	virtual void v248(void);												// SLOT 248 0x463890 (nothing)

	int					f_58c;			// 0x58c  0 (unused)
};
