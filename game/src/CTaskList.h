/*  CTaskList - the game object that owns the story task states ("MOM", "CARLTASK", "SCENE" ...)
    and the self-running demo. Reconstructed from Neutron2.exe: vtable 0x516af4 (287 slots: the
    CLocalGameObject ones plus slot 286), constructor 0x497430, destructor 0x4978d0 (slot 0
    0x4978b0 is the compiler's deleting destructor around it).

    Created by C3DJimmy's constructor (0x443830: `new CTaskList`, 0x4c8 bytes, kept in the
    C3DJimmy at +0x4c90); it is not in the object factory and never saved to a .gam file.
    It has no members of its own (0x4c8 = sizeof CLocalGameObject).

    The task states themselves are global (neg::g_tasks, SaveGame.h): the constructor appends
    the 14 tasks of the game, objects follow them through their "TaskName" property
    (CLocalGameObject::m_task_name, slot 267 sets the state, slot 266 reacts to it), and the
    .tsk save files store them (SaveGame.cpp).

    Per frame (Process) the first frame after a reset pushes the task states to every object
    (v286), then the demo timer runs (UpdateDemo). The demo (neg::g_demo_mode 1..4, set
    elsewhere) plays the levels of NextDemoLevel in turn, each for a while, modes 3/4 pressing
    random keys.                                                                            */
#pragma once
#include "CLocalGameObject.h"

class CTaskList : public CLocalGameObject
{
	public:
	CTaskList();															// 0x497430
	virtual ~CTaskList();													// SLOT 0  0x4978b0 / 0x4978d0

	virtual void Process(float dt);											// SLOT 240 0x497d60
	virtual void v242(float t);												// SLOT 242 0x497d30
	virtual void v260();													// SLOT 260 0x497a80 (level reset: apply the tasks again)
	virtual void v270(int a2, int a3);										// SLOT 270 0x498f70 (game state change prev, new)
	// New in CTaskList (vtable +0x478): every object gets the state of its task (slot 267's
	// field f_4a0 directly, 0 when its task is unknown), then its slot 266.
	virtual void v286();													// SLOT 286 0x497a90

	// 0x497dc0 (thiscall, `this` unused): the demo's timers, every frame.
	void UpdateDemo(float dt);

#include "gen/CTaskList_slots.inc"
#include "gen/CTaskList_fields.inc"
};
