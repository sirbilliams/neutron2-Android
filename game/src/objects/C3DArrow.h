/*  C3DArrow ('3ARR') - a pulsing marker sprite ("sprites.omt" canvas 33, 200 wide): its height
    bobs between 150 and 250 and its colour from red to yellow, half transparent. It shows while
    the task RequiredTask has reached RequiredLevel (or equals ExactLevel when that is set).
    Reconstructed from Neutron2.exe: constructor 0x420350 (factory '3ARR', 0x6cc bytes),
    destructor 0x420590 (only the vtables, then ~C3DSpriteType; deleting thunks 0x420560 /
    0x4207a0 / 0x4207b0). Vtables 0x4ce558 (element, 55 slots) / 0x4ce544 (listener) / 0x4ce0c8
    (game object) / 0x4ce0b8 (streamer: C3DSprite's read_class / write_class copies 0x4207c0 /
    0x4207d0). C3DSpriteType's 0x668, own members 0x668..0x6c4, vtordisp 0x6c4, the streamer
    at 0x6c8.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DArrow : public C3DSpriteType
{
	public:
	C3DArrow();																// 0x420350
	virtual ~C3DArrow();													// 0x420590

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4205d0 properties
	virtual void Process(float dt);											// SLOT 240 0x420640 the pulse
	virtual void v266(void);												// SLOT 266 0x4368e0 SetTaskState(TaskName, GetTaskState(TaskName))
	virtual void v267(int level);											// SLOT 267 0x420710 shown by RequiredTask's state

	// FIELD 0x668 0x5c
	float		m_time;					// 0x668  (= 0) += dt
	char		m_required_task[80];	// 0x66c  "RequiredTask" (= "none")
	int			m_required_level;		// 0x6bc  "RequiredLevel" (= 0)
	int			m_exact_level;			// 0x6c0  "ExactLevel" (= -1: use RequiredLevel)
};
