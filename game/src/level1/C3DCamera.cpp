/*  C3DCamera - see C3DCamera.h. Reconstructed from Neutron2.exe 0x499420..0x499630. */
#include "C3DCamera.h"
#include "../gen/globals.h"

using namespace neg;

C3DCamera *neg::CameraProbe(void)
{
	return (C3DCamera *)g_54e844;
}

C3DCamera::C3DCamera()										// 0x499420
{
	v46(0);
	v52(0);
	InitCube(OMedia3DVector(1.0f, 1.0f, 1.0f, 1.0f), 75.0f);	// element +0xe4
	HideAgain();											// 0x49b120
	v107(0);
	g_54e844 = (C3DObject *)this;
}

C3DCamera::~C3DCamera()										// 0x499510
{
	g_54e844 = NULL;
}

void C3DCamera::v219(float, float, float, float, OMedia3DPolygon *) {}	// 0x4995d0
void C3DCamera::v220(float, float, float, float, OMedia3DPolygon *) {}	// 0x4995d0
