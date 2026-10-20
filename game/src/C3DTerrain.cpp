/*  C3DTerrain - see C3DTerrain.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DTerrain.h"
#include "Game.h"
#include "Physics.h"
#include "OMediaLayer.h"
#pragma GCC diagnostic pop

using namespace neg;


C3DTerrain::C3DTerrain()									// 0x49d0e0
{
	SetName((char *)"C3DTERRAIN");
	Log("C3DTerrain()");
	SetObjectID('3TER');									// 0x49ab00 (direct call)
	CGameObject::v46(0);									// 0x4abf10 (direct calls)
	CGameObject::v52(0);									// 0x4abfa0
	CGameObject::v107(0);									// 0x4ad4d0: not written to the .gam (f_00b)
	// (debug log, empty in the shipped build)
}

C3DTerrain::~C3DTerrain()									// 0x49d210
{
	Log("~C3DTerrain()");
	// (debug log)
}

void C3DTerrain::InitObject(void)							// 0x49d2b0
{
	Log("InitObject()");
	C3DObject::InitObject();
	link_layer(g_layer_world);								// element +0x34 (0x56afd4: the world layer again)
	// (debug log)
}

// Quirk: the base InitPhysics is not called; a terrain is not a physics object, only its
// polygons go into the terrain grid.
void C3DTerrain::InitPhysics(void)							// 0x49d2f0
{
	Log("InitPhysics()");
	PhysicsAddTerrain(f_414);								// 0x4b4210 Add3DTerrain(shape)
	// (debug log)
}

void C3DTerrain::Process(float dt)							// 0x464ee0 (shared empty function)
{
	(void)dt;												// a terrain does nothing per frame
}

namespace neg {
CGameObject *New_C3DTerrain(void)	{ return new C3DTerrain; }				// factory '3TER' (0x594 bytes)
}
