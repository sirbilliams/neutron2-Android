/*  C3DSubBubble - see C3DSubBubble.h. Reconstructed from Neutron2.exe 0x4747f0..0x474d90. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSubBubble.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMedia3DShape.h"
#pragma GCC diagnostic pop
#include <math.h>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DSubBubble::C3DSubBubble()								// 0x4747f0
{
	f_928[3] = 1.0f;										// 0x934 (the point's w)
	SetName((char *)"C3DSUBBUBBLE");						// 0x4ab1b0
	Log("C3DSubBubble()");
	C3DSubBubble::InitObject();								// 0x474990 (called directly)
	C3DObject::SetObjectID('3SBB');							// 0x49ab00
	C3DObject::InitPhysics();								// 0x49acb0
	v107(0);
	v52(0);
	v132(0);
	f_938 = 0;
	// (then a call to the empty 0x463890)
}

C3DSubBubble::~C3DSubBubble() {}							// 0x474940

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DSubBubble::InitObject(void)							// 0x474990
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"STOP", (char *)"subsphere.ase");
	CreateTexture((char *)"subsphere2.png", 0);
	ApplyTexture(f_590, 0);
	v67(1.0f);												// radius
	ev58((char *)"STOP", 1);
	float a[4], p[4];
	v201(a);												// +0x324 angles
	f_924 = a[1];
	v195(p);												// +0x30c position
	f_928[0] = p[0]; f_928[1] = p[1]; f_928[2] = p[2]; f_928[3] = p[3];
}

void C3DSubBubble::Process(float dt)						// 0x474a70
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120

	float a[4];
	v201(a);
	if (a[1] > 360.0f) { v201(a); v203(a[0], a[1] - 360.0f, a[2]); }	// +0x32c
	v201(a);
	if (a[1] < 0.0f) { v201(a); v203(a[0], a[1] + 360.0f, a[2]); }

	// the turn since last frame, across 0 / 360
	float turn;
	v201(a);
	if (a[1] > 270.0f)
	{
		if (f_924 < 90.0f) { v201(a); turn = f_924 - (a[1] - 360.0f); }
		else { v201(a); turn = f_924 - a[1]; }
	}
	else
	{
		v201(a);
		if (a[1] < 90.0f && f_924 > 270.0f) { float prev = f_924 - 360.0f; v201(a); turn = prev - a[1]; }
		else { v201(a); turn = f_924 - a[1]; }
	}

	float p[4];
	v195(p);
	float dx = p[0] - f_928[0];
	float dy = p[1] - f_928[1];
	float dz = p[2] - f_928[2];
	const double k = 0.01f;									// 0x4fda88
	double x = dx * k, y = dy * k, z = (float)(dz * k);	// (x87; z stored as a float)
	double dist = sqrt(y * y + z * z + x * x);
	get_shape()->offset_textures((float)(-turn * 0.009), (float)-(dist * dt), false);	// 2002 element +0x8c, shape +0xa4

	v195(p);
	f_928[0] = p[0]; f_928[1] = p[1]; f_928[2] = p[2]; f_928[3] = p[3];
	v201(a);
	f_924 = a[1];
}

namespace neg {
CGameObject *New_C3DSubBubble(void)	{ return new C3DSubBubble; }		// factory '3SBB' (0x944 bytes)
}
