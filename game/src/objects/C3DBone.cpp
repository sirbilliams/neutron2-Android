/*  C3DBone - see C3DBone.h. Reconstructed from Neutron2.exe 0x421f70..0x422240, 0x47e3a0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DBone.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMedia3DShape.h"
#pragma GCC diagnostic pop

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DBone::C3DBone()											// 0x421f70
{
	SetName((char *)"C3DBONE");								// 0x4ab1b0
	Log("C3DBONE()");
	C3DAnimated::InitObject();								// 0x41b0a0 (called directly)
	C3DObject::SetObjectID('3BON');							// 0x49ab00
	v52(1);
	v46(1);
	v42(1);
	v40(1);
	v44(0);
	m_db = GetPermanentDatabase("permanentobjects.omt");	// 0x4a4560
	f_648 = 0;												// no animations
	C3DObject::set_shape(Load3DShape(m_db, 0));				// 0x4b2c80, 0x49ab10 (called directly)
	f_6d0 = 2;												// neither morph (0) nor Granny (1)
	MakeShapeUnLit(get_shape());							// 0x4b2520
	v67(15.0f);												// radius
	v107(0);
	v36(0);
	f_924 = 0;
	get_shape()->scale(3.0f, 3.0f, 3.0f);					// shape +0x34
	v50(0);
	// (then a call to the empty 0x463890)
}

C3DBone::~C3DBone() {}										// 0x422140

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DBone::Process(float dt)								// 0x422190
{
	C3DAnimated::Process(dt);								// 0x41c120
	AddRotation(dt * 120.0f, dt * 25.0f, dt * 10.0f);		// +0x330; 0x4d0568, 0x4d056c, 0x4cb1cc
}

void C3DBone::v260(void)									// 0x47e3a0
{
	C3DAnimated::v260();
	InitPhysics();											// +0x2c
}

namespace neg {
CGameObject *New_C3DBone(void)	{ return new C3DBone; }			// factory '3BON' (0x930 bytes)
}
