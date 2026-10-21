/*  C3DHumphrey - see C3DHumphrey.h. Reconstructed from Neutron2.exe 0x441cf0..0x4420d0 (and
    the shared 0x4368e0). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DHumphrey.h"
#include "../C3DRocketShip.h"				// neg::JimmyHurt
#include "../Game.h"
#include "../SaveGame.h"
#pragma GCC diagnostic pop
#include <string>

using namespace neg;

C3DHumphrey::C3DHumphrey()									// 0x441cf0
{
	SetName((char *)"C3DHUMPHREY");
	Log("C3DHUMPHREY()");
	C3DHumphrey::InitObject();								// 0x441f10 (called directly)
	C3DObject::InitPhysics();								// 0x49acb0
	v64(0.6f);												// 0x4ac0c0
	C3DAI::ev106(f_a9c, (char *)"STOP");
	C3DAI::ev106(f_ac4, (char *)"WALK");
	C3DAI::ev106(f_b14, (char *)"STOP");
	C3DAI::ev106(f_b3c, (char *)"ATTACK");
	C3DObject::SetObjectID('3HUM');							// 0x49ab00
	f_bd4 = 0;												// no hovering
	f_c28 = 1;
	f_008 = 1;
	f_c5d = 0;												// not held when touched
}

C3DHumphrey::~C3DHumphrey()									// 0x441e80
{
	// ("Deleting Humphrey": debug log)
}

namespace neg {
CGameObject *New_C3DHumphrey(void) { return new C3DHumphrey; }			// '3HUM' (0x1b58 bytes)
}

void C3DHumphrey::InitObject(void)							// 0x441f10
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	InitAnim();												// element +0x110
	CreateAnim((char *)"ATTACK", (char *)"humprun.ase");	// +0xe0
	CreateAnim((char *)"WALK", (char *)"humpwalk.ase");
	CreateAnim((char *)"STOP", (char *)"humpsleep.ase");
	CreateAnim((char *)"STOP2", (char *)"humpstop.ase");
	CreateTexture((char *)"humphrey.png", 0);				// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(20.0f);												// CGameObject +0x10c: radius
	ev58((char *)"STOP", 1);								// +0xe8
}

void C3DHumphrey::AddTouching(CGameObject *other)			// 0x441ff0
{
	C3DAI::AddTouching(other);								// 0x415150
	if (other->NameContains((char *)"C3DJIMMY"))			// +0x18
	{
		JimmyHurt(other, -10.0f, -10, 5.0f, 1);				// Jimmy's element +0x218
		ev107(5);											// element +0x1ac
	}
}

void C3DHumphrey::v260(void)								// 0x442050
{
	C3DAI::v260();											// 0x4143a0
	GetTaskState("SCENE");									// 0x497a40 (the result is not used)
	ev70(1);												// element +0x118
}

void C3DHumphrey::v266(void)								// 0x4368e0 (shared code)
{
	SetTaskState(m_task_name, GetTaskState(m_task_name));	// 0x497980 / 0x497a40 (writes back what it read)
}

void C3DHumphrey::v267(int level)							// 0x441fe0
{
	C3DAnimated::v267(level);								// 0x41c880
}
