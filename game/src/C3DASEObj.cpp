/*  C3DASEObj - see C3DASEObj.h. */
#include "C3DASEObj.h"
#include "CGameType.h"
#include "Game.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMedia3DShape.h"
#include "OMedia3DVector.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0 (as in
// C3DAI.cpp; the generated declaration of v226 has the wrong signature).
static OMedia3DVector ObjectDir(CGameObject *o, float x, float y, float z)
{
	float rot[4];
	o->v201(rot);											// vtable +0x324
	const omt_Angle ax = (omt_Angle)((int)(rot[0] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle ay = (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle az = (omt_Angle)((int)(rot[2] * kDegToAngle) & omc_MaxAngleMask);
	OMedia3DVector v(x, y, z, 0.0f);
	v.rotate(ax, ay, az);
	return v;
}

// 0x48fcb0 (thiscall, `this` unused): count a running timer *t down by dt; true only when it
// runs out now (then *t = 0).
static bool TimerExpired(float *t, float dt)
{
	if (*t > 0)
	{
		*t -= dt;
		if (*t <= 0)
		{
			*t = 0;
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------------- construction

C3DASEObj::C3DASEObj()										// 0x4207e0
{
	SetName((char *)"C3DASEOBJ");
	Log("C3DASEObj()");
	f_649 = 0;												// ev69 loads the animations
	C3DASEObj::InitObject();								// 0x420bd0 (called directly)
	C3DObject::SetObjectID('3ASE');							// 0x49ab00
	v42(0);
	v40(0);
	v34(0);
	v46(0);
	C3DObject::InitPhysics();								// 0x49acb0
	strcpy(f_924, "flamemove.ase");
	strcpy(f_988, "flamestop.ase");
	strcpy(f_9ec, "none");
	strcpy(f_a50, "none");
	strcpy(f_ab4, "flame.png");
	f_b88 = -1.0f;
	f_b8c = -1.0f;
	f_b7c = 0;
	f_664 = 1;												// "SecondPass"
	f_89c = 0;												// "Translucent"
	f_8a0 = 0;												// "Lit"
	f_b80 = 0;
	f_b84 = 0;
	strcpy(f_b18, "none");
	f_b90 = 0;
	f_b98 = 1.0f;
	f_b94 = 0;
	f_b9c = 0;
	f_ba0 = 0;
	f_ba4 = 0;
	// (then a call to the empty 0x463890)
}

C3DASEObj::~C3DASEObj()										// 0x420a80
{
}

namespace neg {
CGameObject *New_C3DASEObj(void)	{ return new C3DASEObj; }				// factory '3ASE' (0xbb0 bytes)
}

// --------------------------------------------------------------------------- element slots

void C3DASEObj::ev67(void)									// 0x4211a0
{
	if (f_b8c == 0 && f_b88 > 0)
	{
		f_b8c = f_b88;
		HideAgain();										// +0xdc
	}
}

void C3DASEObj::ev69(void)									// 0x420ad0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"ANIMATE", f_924);					// +0xe0
	if (strcasecmp(f_9ec, "none")) CreateAnim((char *)"ANIM1", f_9ec);	// _stricmp
	if (strcasecmp(f_a50, "none")) CreateAnim((char *)"ANIM2", f_a50);
	CreateAnim((char *)"STOP", f_988);
	CreateTexture(f_ab4, 0);								// +0xfc
	ev58((char *)"ANIMATE", 1);								// +0xe8
	ApplyTexture(f_590, 0);									// +0x100
	v67(100.0f);											// CGameObject +0x10c
	v64(0.1f);												// CGameObject +0x100
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DASEObj::InitObject(void)							// 0x420bd0
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("ASEWalk", f_924, PROP_ALPHA, 0);
	RegisterProperty("ASEStop", f_988, PROP_ALPHA, 0);
	RegisterProperty("ASEAnim1", f_9ec, PROP_ALPHA, 0);
	RegisterProperty("ASEAnim2", f_a50, PROP_ALPHA, 0);
	RegisterProperty("PNGFile", f_ab4, PROP_ALPHA, 0);
	RegisterProperty("DoubleSided", &f_b7c, PROP_LONG, 0);
	RegisterProperty("SlideXSpeed", &f_b80, PROP_FLOAT, 0);
	RegisterProperty("SlideYSpeed", &f_b84, PROP_FLOAT, 0);
	RegisterProperty("RepeatAnimTime", &f_b88, PROP_FLOAT, 0);
	RegisterProperty("RepeatSound", f_b18, PROP_ALPHA, 0);
	RegisterProperty("InOutDist", &f_b90, PROP_FLOAT, 0);
	RegisterProperty("PeriodSpeed", &f_b98, PROP_FLOAT, 0);
	RegisterProperty("RotateX", &f_b9c, PROP_FLOAT, 0);
	RegisterProperty("RotateY", &f_ba0, PROP_FLOAT, 0);
	RegisterProperty("RotateZ", &f_ba4, PROP_FLOAT, 0);
}

unsigned char C3DASEObj::v231(CGameObject *other)			// 0x420d80
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3GOD' || id == '3JIM' || id == '3BUS';
}

void C3DASEObj::Process(float dt)							// 0x420dc0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	if (!f_589 || f_8a8) return;							// hidden / culled
	if (TimerExpired(&f_b8c, dt))							// 0x48fcb0
	{
		ev58((char *)"ANIMATE", 0);							// +0xe8
		if (strcasecmp(f_b18, "none"))						// _stricmp
			PlayGameSound3DNoDoppler(this, -1, std::string(f_b18), false);	// 0x48f0f0
		ShowAgain();										// +0xd8
	}
	if (f_b80 != 0 || f_b84 != 0)
		get_shape()->offset_textures(-(dt * f_b80), -(dt * f_b84), false);	// 2002 +0x8c, shape +0xa4
	if (f_b90 > 0)
	{
		f_b94 = dt + f_b94;
		const float s = (float)sin((double)(f_b98 * f_430 * f_b94));	// x87 fsin
		const OMedia3DVector d = ObjectDir(this, 0, 0, 1.0f);			// +0x388
		float p[4];
		v88(p);												// +0x160: the start position
		v196(d.x * f_b90 * s + p[0], d.y * f_b90 * s + p[1], d.z * f_b90 * s + p[2], p[3]);	// +0x310
	}
	if (f_b9c != 0) AddRotation(f_430 * f_b9c * dt, 0, 0);	// +0x330
	if (f_ba0 != 0) AddRotation(0, f_430 * f_ba0 * dt, 0);
	if (f_ba4 != 0) AddRotation(0, 0, dt * f_430 * f_ba4);
}

void C3DASEObj::v260(void)									// 0x421080
{
	C3DAnimated::v260();									// 0x41cec0
	InitAnim();												// element +0x110
	if (f_b7c) get_shape()->set_two_sided(true);			// +0xb0, shape +0x74
	if (f_664 == 1) ev73();									// +0x124
	if (f_89c > 0) ev71(f_89c);								// +0x11c
	if (f_8a0 > 0) ev72(f_8a0);								// +0x120
	if (f_b88 > 0)
	{
		ShowAgain();										// +0xd8
		f_b8c = f_b88;
		ev58((char *)"ANIMATE", 0);
	}
	ev51();													// +0xcc texture filtering on
	if (f_b90 > 0)
	{
		v94(2);												// CGameObject +0x178
		set_shape(get_shape());								// +0xac (+0xb0)
	}
}

void C3DASEObj::WriteState(OMediaStreamOperators &s)		// 0x421240
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	bool visible = f_589 != 0;
	s << visible;
	if (visible) ShowAgain();								// +0xd8
	else HideAgain();										// +0xdc
}

void C3DASEObj::ReadState(OMediaStreamOperators &s)			// 0x4211f0
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	bool visible;
	s >> visible;
	if (visible) ShowAgain();								// +0xd8
	else HideAgain();										// +0xdc
}
