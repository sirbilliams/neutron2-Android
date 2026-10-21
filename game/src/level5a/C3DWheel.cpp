/*  C3DWheel - see C3DWheel.h. Reconstructed from Neutron2.exe 0x4a0920..0x4a0c60. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DWheel.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop

using namespace neg;

C3DWheel::C3DWheel()										// 0x4a0920
{
	SetName((char *)"C3DWHEEL");							// 0x4ab1b0
	Log("C3DWheel()");
	C3DWheel::InitObject();									// (inlined in the original)
	// (then a call to the empty 0x463890)
	f_58c = 0;
	v32(0);
	v50(0);
	v42(0);
	v40(0);
	v46(0);
	v58(0);
	v64(1.0f);
	// (then a call to the empty 0x463890)
}

C3DWheel::~C3DWheel() {}									// 0x4a0ab0

void C3DWheel::ev56(int id)									// 0x4a0ba0
{
	set_shape(Load3DShape(g_vehicle_db, id));				// 0x4b2c80, element +0xac
}

void C3DWheel::InitObject(void)								// 0x4a0b50
{
	Log("InitObject()");
	C3DObject::InitObject();								// 0x49ac20
	SetObjectID('3WHE');									// element +0xc4
	v107(0);												// +0x1ac
}

void C3DWheel::InitPhysics(void)							// 0x4a0bd0
{
	Log("InitPhysics()");
	if (f_4aa) return;
	C3DObject::InitPhysics();								// 0x49acb0
	f_4aa = 1;
	f_005 = 0;
	f_00a = 0;
}

void C3DWheel::Process(float dt)							// 0x4a0c10
{
	C3DObject::Process(dt);									// 0x49ae00
}

namespace neg {
CGameObject *New_C3DWheel(void)	{ return new C3DWheel; }		// factory '3WHE' (0x598 bytes)
}
