/*  C3DJeep - see C3DJeep.h. Reconstructed from Neutron2.exe 0x4420d0..0x442f10. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DJeep.h"
#include "../Bridge.h"
#include "../C3DAnimated.h"
#include "../C3DPlayer.h"
#include "../CGameType.h"
#include "../CViewPort.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string>
#include <string.h>
#include <strings.h>

using namespace neg;

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DJeep::C3DJeep()											// 0x4420d0
{
	SetName((char *)"C3DJEEP");
	Log("C3DJeep()");
	f_9f0 = 0;												// power at ResetObject
	C3DJeep::InitObject();									// 0x442410 (inlined)
	f_a84 = NULL;
	C3DObject::SetObjectID('3JEE');							// 0x49ab00
	v67(150.0f);											// radius
	f_a38 = 1;												// camera
	v107(1);
	v44(0);
	f_a43 = 0;												// walls do not crash it
	f_6d0 = 2;												// a plain shape
	f_a88 = 0;
	f_4c4 = 1;
}

C3DJeep::~C3DJeep() {}										// 0x442370

namespace neg {
CGameObject *New_C3DJeep(void) { return new C3DJeep; }		// '3JEE' (0xa94 bytes)
}

// --------------------------------------------------------------------------- element slots

void C3DJeep::ShowAgain(void)								// 0x442990
{
	C3DObject::ShowAgain();									// 0x49b100
	for (int i = 0; i < 6; i++)
		if (f_960[i]) f_960[i]->ShowAgain();				// +0xd8
}

void C3DJeep::ev107(float power)							// 0x4428f0
{
	f_9f8 = power + 600.0f;									// 0x4d0ca0
	f_a00 = 1200.0f;
	f_9f4 = power;
	f_a08 = power * 4.5f + 1800.0f;							// 0x4d92dc, 0x4d986c
	f_a04 = 0;
	f_a0c = 0;
	f_a10 = f_a08 * 0.66f;									// 0x4d7f94
	f_9fc = f_9f8;
	if (g_power_timers[0] > 0) f_a14 = f_a10 * 1.4f;		// 0x4e413c
	else f_a14 = f_a10;
}

unsigned char C3DJeep::ev123(void) { return PlayerKeyLeft(); }	// 0x442da0 (0x4659f0)
unsigned char C3DJeep::ev124(void) { return PlayerKeyRight(); }	// 0x442db0 (0x465a60)

unsigned char C3DJeep::ev125(void)							// 0x442dc0
{
	if (!PlayerKeyGo() && !PlayerKeyAction() && !g_joy_button0) return 0;	// 0x465910, 0x465750
	return f_9ea > 0;
}

unsigned char C3DJeep::ev126(void)							// 0x442df0
{
	if (g_joy_button0) return 0;
	return PlayerKeyBack();									// 0x465980
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DJeep::InitObject(void)								// 0x442410
{
	Log("InitObject()");
	C3DVehicle::InitObject();								// 0x49d840
	g_gametype->InitVehicleDatabase(std::string("omt\\scooter.omt"));	// +0xec
	set_shape(Load3DShape(g_vehicle_db, 0));				// 0x4b2c80, element +0xac
	v66(5515.0f);											// +0x108
	MakeShapeUnLit(get_shape());							// 0x4b2520
	InitPhysics();											// +0x2c
}

void C3DJeep::UnInitObject(void)							// 0x4162b0
{
	C3DAnimated::UnInitObject();
}

void C3DJeep::InitPhysics(void)								// 0x442550
{
	if (f_4aa) return;
	Log("InitPhysics()");
	C3DObject::InitPhysics();								// 0x49acb0
}

void C3DJeep::AddTouching(CGameObject *other)				// 0x4429c0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (g_jimmy && other == g_jimmy)
		JimmyActivateMenuItem(g_jimmy, this, this, 4, 13.0f);	// Jimmy element +0x228: ride it
}

void C3DJeep::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x442aa0
{
	if (!f_a44 && f_a43)
	{
		float v[4];
		v174(v);											// +0x2b8
		OMedia3DVector h(v[0], 0, v[2], v[3]);
		if (h.quick_magnitude() > 20.0f)					// 0x4caa40
		{
			const OMedia3DVector fwd = v226(0, 0, 1.0f);	// +0x388
			if (!(fabsf(fwd.z * c + fwd.y * b + fwd.x * a) < 0.9f))	// 0x4e4158
				ev131();									// element +0x20c: crash
		}
	}
	if (f_a44)
	{
		CLocalGameObject::v220(a, b, c, d, poly);			// 0x48ef40
		return;
	}
	float s = v153();										// +0x264
	if (!(s > 0)) return;
	float v[4];
	v174(v);
	OMedia3DVector n(v[0], v[1], v[2], v[3]);
	const float inv = 1.0f / n.quick_magnitude();
	n.x *= inv; n.y *= inv; n.z *= inv;
	float dot = n.z * c + n.y * b + n.x * a;
	if (dot < -0.85f) { if (!f_004) dot *= 3.5f; }			// 0x4e4154, 0x4e4150
	else if (!f_004) dot *= 1.5f;							// 0x4caa60
	if (!(dot < 0)) return;
	if (f_004)
	{
		s = s - fabsf(dot) * f_a00 * g_frame_time;			// 0x6f75c8
		if (!(s > 100.0f)) s = 100.0f;						// 0x4e4148
	}
	else if (PlayerKeyGo())									// 0x465910
	{
		s = s - fabsf(dot) * f_a00 * g_frame_time;
		if (!(s > 230.0f)) s = 230.0f;						// 0x4e4140
	}
	v154(s);												// +0x268
}

// The driver kept on it: his position (object and element) and rotation are ours.
static void KeepDriver(C3DJeep *j)
{
	C3DAnimated *drv = j->f_a84;
	float p[4];
	j->v195(p);												// +0x30c
	drv->v197(p[0], p[1], p[2]);							// +0x314
	drv->place(p[0], p[1], p[2]);							// element +0x44
	float r[4];
	j->v201(r);												// +0x324
	drv->v203(r[0], r[1], r[2]);							// +0x32c
}

void C3DJeep::Process(float dt)								// 0x442560
{
	if (!f_589) return;										// hidden (HideAgain)
	if (g_gametype->IsPaused())								// +0x114
	{
		if (f_a84) KeepDriver(this);
		ev140();											// element +0x230: mirror off
		return;
	}
	C3DCar::Process(dt);									// 0x424b30
	if (!f_a84)												// nobody on it
	{
		f_a0c = 0;
		f_29c = 10.0f;
		f_a24 = 0;
		f_a28 = 0;
		v154(0);											// +0x268
		f_a04 = 0;
		return;
	}
	if (!PlayerKeyJump()) f_a80 = 0;						// 0x4658a0
	else if (!f_a80 && v133())								// +0x214: a hop
	{
		v162(0, 700.0f, 0);									// +0x288
		f_a80 = 1;
	}
	KeepDriver(this);
	f_a88 += dt;
	if (!(f_a88 > 0.3)) return;								// 0x4dfdc8
	if (v153() < 10.0f) f_a84->ev58((char *)"SCOOTSTOP", 1);	// 0x4cb1cc, element +0xe8
	else f_a84->ev58((char *)"SCOOT", 1);
	f_a88 = 0;
}

void C3DJeep::v243(float dt)								// 0x442a30
{
	if (GameIsRunning()) C3DVehicle::v243(dt);				// 0x4b0ca0
}

void C3DJeep::v260(void)									// 0x4428c0
{
	C3DPlayerObject::v260();								// 0x464ed0
	ev107(f_9f0);											// element +0x1ac
}

void C3DJeep::v275(void)									// 0x442a50
{
	FindObjectByTag("JIM1");								// (the result is not used)
	if (g_jimmy) static_cast<CLocalGameObject *>(g_jimmy)->v275();	// +0x44c
	ev140();												// element +0x230
}

void C3DJeep::v277(void)									// 0x442d00
{
	C3DAnimated::v277();									// 0x41ce80
	if (f_a78 != -1) StopChannel(f_a78, false);				// 0x4b88e0 (the channel is kept)
	ev140();
}

void C3DJeep::v278(void)									// 0x442d40
{
	C3DAnimated::v278();									// 0x41cea0
	if (f_a78 != -1) f_a78 = PlayGameSound3D(this, -1, std::string("Scooter"), true);	// 0x48f200
}

void C3DJeep::WriteState(OMediaStreamOperators &s)			// 0x46cc40 (C3DRocketShip's)
{
	C3DPlayerObject::WriteState(s);							// 0x4651b0
	float v[4], a, b, c;
	v195(v); a = v[0];
	v195(v); b = v[1];
	v195(v); c = v[2];
	s << a << b << c;
	v201(v); a = v[0];
	v201(v); b = v[1];
	v201(v); c = v[2];
	s << a << b << c;
}

void C3DJeep::ReadState(OMediaStreamOperators &s)			// 0x442e10
{
	C3DPlayerObject::ReadState(s);							// 0x4651a0
	float a, b, c;
	s >> a >> b >> c;
	v197(a, b, c);											// +0x314: position
	s >> a >> b >> c;
	v203(a, b, c);											// +0x32c: rotation
}
