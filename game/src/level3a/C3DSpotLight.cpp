/*  C3DSpotLight - see C3DSpotLight.h. Reconstructed from Neutron2.exe 0x4721f0..0x472860. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSpotLight.h"
#include "../objects/C3DCorona.h"
#include "../Physics.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>

using namespace neg;

C3DSpotLight::C3DSpotLight()								// 0x4721f0
{
	SetName((char *)"C3DSPOTLIGHT");						// 0x4ab1b0
	Log("C3DSpotLight()");
	C3DSpotLight::InitObject();								// (inlined in the original)
	// (then a call to the empty 0x463890)
	C3DObject::SetObjectID('3SPO');							// 0x49ab00
	v107(0);
	v52(0);
	v42(0);
	v40(0);
	v34(1);
	v46(0);
	f_924 = new C3DCorona;									// 0x428150 (0x694 bytes)
	f_924->v107(0);											// +0x1ac
	f_924->m_sprite_index = 36;								// element +0x594
	f_924->m_grow_speed = 0;								// +0x67c
	f_924->m_auto_mod = 0;									// +0x688
	C3DObject::InitPhysics();								// 0x49acb0
	f_928 = 0;
	// (then a call to the empty 0x463890)
}

C3DSpotLight::~C3DSpotLight() {}							// 0x472460

void C3DSpotLight::InitObject(void)							// 0x4724b0
{
	Log("C3DSpotLight::InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"LIGHT", (char *)"subspotlight.ase");
	CreateTexture((char *)"subspotlight.png", 0);
	ApplyTexture(f_590, 0);
	ev58((char *)"LIGHT", 1);
	ev72(0.8f);												// element +0x120 additive
}

void C3DSpotLight::Process(float dt)						// 0x472540
{
	C3DAnimated::Process(dt);								// 0x41c120
	if (!f_928)
	{
		HideAgain();										// element +0xdc
		f_924->HideAgain();									// corona element +0xd8 (port: the original does not check it)
		return;
	}
	ShowAgain();											// element +0xd8
	float p[4];
	v195(p);
	OMedia3DVector from(p[0], p[1], p[2], p[3]);
	OMedia3DVector to = v224(0, -2250.0f, 6000.0f);			// +0x380
	if (!PhysicsLineBlocked(from, to, false))				// 0x4b7170
	{
		f_924->HideAgain();
		return;
	}
	const OMedia3DVector &hit = g_phys_ray.end;				// 0x6311f8
	float dx = hit.x - from.x, dy = hit.y - from.y, dz = hit.z - from.z;
	float d = (float)(sqrt((double)dy * dy + (double)dz * dz + (double)dx * dx) * 0.3);	// 0x4dfdc8
	float alpha = (2000.0f - d) * 0.0005f;					// 0x4cb1ec, 0x4ccef4
	f_924->set_diffuse(OMediaFARGBColor(alpha, 0.9f, 0.9f, 0.1f));	// corona element +0x90..+0x9c
	OMedia3DVector back = v226(0, (float)(d * 0.45), (float)(d * -1.2));	// +0x388; 0x4fbd08 / 0x4fbd10
	f_924->v196(hit.x + back.x, hit.y + back.y, hit.z + back.z, hit.w);	// +0x310
	f_924->ShowAgain();										// corona element +0xd4
	f_924->set_size(d * 0.8f, d * 0.8f);					// corona element +0xac / +0xb0; 0x4caa58
}

namespace neg {
CGameObject *New_C3DSpotLight(void)	{ return new C3DSpotLight; }		// factory '3SPO' (0x934 bytes)
}
