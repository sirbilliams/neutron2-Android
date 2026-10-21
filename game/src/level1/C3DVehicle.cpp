/*  C3DVehicle - see C3DVehicle.h. Reconstructed from Neutron2.exe 0x49d370..0x4a0920. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DVehicle.h"
#include "C3DCamera.h"
#include "../level5a/C3DWheel.h"
#include "../Bridge.h"
#include "../CGameType.h"
#include "../CViewPort.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Mouse.h"
#include "../Physics.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DPolygon.h"
#include "OMediaTrigo.h"
#include "OMediaViewPort.h"
#include "OMediaWindow.h"
#include "OMediaRect.h"
#include <math.h>
#include <stdlib.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4a38a0: was the key pressed? (clears it)
static bool KeyPressed(int vk)
{
	if (g_keys[vk]) { g_keys[vk] = 0; return true; }
	return false;
}

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360

// The original's fold of an angle in degrees into 0..360 (x87 value, then stored as a float):
// below 0 it becomes 360 - |(int)a| (the fraction is lost), above 360 it loses 360.
static float Fold(double a)
{
	if (a < 0.0) return (float)(360 - abs((int)a));
	if (a > 360.0) return (float)(a - 360.0);
	return (float)a;
}

static omt_Angle ToAngle(float deg)
{
	return (omt_Angle)((int)(deg * kDegToAngle) & omc_MaxAngleMask);
}

// The element angles (2002 slot 249) from the object's rotation, as the vehicle code does for
// each wheel: v249(int(x * k), int(y * k), int(z * k)).
static void AnglesFromRotation(CGameObject *o)
{
	float r[4];
	o->v201(r);												// +0x324
	o->v249((short)(int)(r[0] * kDegToAngle), (short)(int)(r[1] * kDegToAngle), (short)(int)(r[2] * kDegToAngle));	// +0x3e4
}

// --------------------------------------------------------------------------- construction

C3DVehicle::C3DVehicle()									// 0x49d370
{
	f_978.w = 1.0f;
	for (int i = 0; i < 6; i++) f_988[i].w = 1.0f;
	SetName((char *)"C3DVEHICLE");
	Log("C3DVehicle()");
	v30(0);
	v38(1);
	v50(0);
	v56(1.0f);
	v64(0.7f);
	for (int i = 0; i < 6; i++)
	{
		f_960[i] = NULL;
		f_988[i].x = 0;
		f_988[i].y = 0;
		f_988[i].z = 0;
	}
	f_9f0 = 100.0f;
	f_9f4 = 100.0f;
	f_9f8 = 700.0f;
	f_a10 = 1485.0f;
	f_a14 = 1485.0f;
	f_9fc = 700.0f;
	f_a00 = 1200.0f;
	f_a04 = 0;
	f_a08 = 2250.0f;
	f_a0c = 0;
	f_a18 = 15.0f;
	f_a1c = 15.0f;
	f_a24 = 0;
	f_a28 = 0;
	f_a2c = 0;
	f_a30 = 0;
	f_a34 = 0;
	f_9e8 = 0;
	f_9ea = 0;
	f_9ec = 0;
	f_a38 = 1;
	f_a3c = 0;
	f_a40 = 0;
	f_a41 = 0;
	f_a42 = 0;
	f_a43 = 1;
	f_a44 = 0;
	f_a45 = 0;
	f_a46 = 0;
	f_a47 = 0;
	f_a49 = 0;
	f_a4a = 0;
	f_a48 = 0;
	v66(115.0f);
	f_a4c = NULL;
	if (f_a46) ev139();
	if (!g_54e844) new C3DCamera;							// 0x4b3aa0(0x598), 0x499420: sets g_54e844
}

C3DVehicle::~C3DVehicle() {}								// 0x49d630

namespace neg {
CGameObject *New_C3DVehicle(void) { return new C3DVehicle; }	// '3VEH' (0xa58 bytes)
}

// --------------------------------------------------------------------------- element slots

void C3DVehicle::HideAgain(void)							// 0x49d710
{
	C3DObject::HideAgain();									// 0x49b120
	for (int i = 0; i < 6; i++)
		if (f_960[i]) f_960[i]->HideAgain();				// +0xdc
}

void C3DVehicle::ev106(void)								// 0x49d740
{
	hide(false);											// element +0x58
	for (int i = 0; i < 6; i++)
		if (f_960[i]) f_960[i]->hide(false);
}

void C3DVehicle::ev107(float power)							// 0x49d770
{
	f_9f8 = power + 600.0f;									// 0x4d0ca0
	f_a00 = 1200.0f;
	f_9f4 = power;
	f_a08 = power * 4.5f + 1800.0f;							// 0x4d92dc, 0x4d986c
	f_a04 = 0;
	f_a0c = 0;
	f_a10 = f_a08 * 0.66f;									// 0x4d7f94
	f_9fc = f_9f8;
	f_a14 = f_a10;
}

void C3DVehicle::ev108(float d)								// 0x49d7e0
{
	f_9f4 += d;
	f_9f8 += d;
	f_a08 = d * 4.5f + f_a08;
	f_a10 = f_a08 * 0.66f;
	f_9fc = f_9f8;
	f_a14 = f_a10;
}

void C3DVehicle::ev109(float x, float y, float z)			// 0x49d980
{
	for (int i = 0; i < 6; i++)
		if (f_960[i]) f_960[i]->v197(x + f_988[i].x, y + f_988[i].y, z + f_988[i].z);	// +0x314
	v90(x, y, z);											// +0x168
	float p[4];
	v195(p);												// +0x30c
	g_viewport->place(p[0], p[1], p[2]);					// viewport +0x44
}

void C3DVehicle::ev110(float dt)							// 0x49e710
{
	// The camera of a plain vehicle: the same twenty views as C3DCar's (level1/C3DCar.cpp), with
	// other distances. No level places a '3VEH' and C3DCar overrides it.
	(void)dt;
	NEG_TODO(0x49e710, "C3DVehicle::ev110, the camera of a plain vehicle");
}

void C3DVehicle::ev111(float dt)							// 0x49f300
{
	f_a34 += dt;
	if (!(f_a34 < 4.0f))									// 0x4dce64
	{
		if (!v135()) return;								// +0x21c
		f_a44 = 0;
		v172(0, 0, 0);										// +0x2b0
		v176(0, 0, 0);										// +0x2c0
		v154(0);											// +0x268
		f_a04 = 0;
		return;
	}
	float v[4];
	v174(v);												// +0x2b8: velocity
	v178(-(v[0] * dt), 0, -(v[2] * dt));					// +0x2c8: velocity +=
	float r[4];
	v170(r);												// +0x2a8
	v173(-(r[0] * dt), -(r[1] * dt), -(r[2] * dt));			// +0x2b4
}

void C3DVehicle::ev112(void)								// 0x49f440
{
	f_a24 = g_joy_x * -45.0f;								// 0x4dce40
	if (g_joy_button0 || g_joy_button2) return;
	if (g_joy_y > 0.1f)										// 0x4caa68
	{
		float t = g_joy_y * 1.67f;							// 0x51aa08
		if (!(t < 1.0f)) t = 1.0f;
		f_a49 = 1;
		f_a10 = t * f_a08 * 0.66f;
	}
	if (g_joy_y < -0.5f)									// 0x4d2ca0
	{
		f_a4a = 1;
		f_a10 = f_a08 * 0.66f;
	}
}

void C3DVehicle::ev113(short *dx, short *dy)				// 0x49f4f0
{
	GetMouseDelta(dx, dy);									// 0x4a3990
}

unsigned char C3DVehicle::ev114(void) { return KeyPressed('1'); }	// 0x49f510
unsigned char C3DVehicle::ev115(void) { return KeyPressed('2'); }	// 0x49f520
unsigned char C3DVehicle::ev116(void) { return KeyPressed('3'); }	// 0x49f530
unsigned char C3DVehicle::ev117(void) { return KeyPressed('4'); }	// 0x49f540
unsigned char C3DVehicle::ev118(void) { return KeyPressed('5'); }	// 0x49f550
unsigned char C3DVehicle::ev119(void) { return KeyPressed('6'); }	// 0x49f560
unsigned char C3DVehicle::ev120(void) { return KeyPressed('7'); }	// 0x49f570
unsigned char C3DVehicle::ev121(void) { return KeyPressed('8'); }	// 0x49f580
unsigned char C3DVehicle::ev122(void) { return KeyPressed('9'); }	// 0x49f590
unsigned char C3DVehicle::ev123(void) { return g_keys[0x25] != 0; }	// 0x49f5a0 VK_LEFT
unsigned char C3DVehicle::ev124(void) { return g_keys[0x27] != 0; }	// 0x49f5b0 VK_RIGHT

unsigned char C3DVehicle::ev125(void)						// 0x49f5c0
{
	if (!g_mouse_down && !g_keys[0x26] && !g_joy_button0) return 0;	// VK_UP
	return f_9ea > 0;
}

unsigned char C3DVehicle::ev126(void)						// 0x49f5f0
{
	if (!((g_mouse_rdown && !g_mouse_down) || (!g_joy_button0 && g_joy_button1) || g_keys[0x28])) return 0;	// VK_DOWN
	return f_9ea > 0;
}

unsigned char C3DVehicle::ev127(void)						// 0x49f630
{
	if (!((g_mouse_down && g_mouse_rdown) || g_keys['X'] || g_joy_button2 || (g_joy_button0 && g_joy_button1))) return 0;
	return f_9ea > 0;
}

unsigned char C3DVehicle::ev128(void)						// 0x49f680
{
	if (g_keys[0x11]) return 0;								// VK_CONTROL
	return KeyPressed('C') || KeyPressed('V');
}

unsigned char C3DVehicle::ev129(void) { return g_keys['B'] != 0; }	// 0x49f6b0

unsigned char C3DVehicle::ev130(void)						// 0x49f6c0
{
	if (g_keys[0x11]) return 0;
	return KeyPressed(0x08);								// VK_BACK
}

void C3DVehicle::ev131(void)								// 0x49f910
{
	v154(0);												// +0x268
	v157(0, 0, 0);											// +0x274
	f_a44 = 1;
	f_a48 = 0;
	f_a34 = 0;
}

void C3DVehicle::ev132(void)								// 0x49f990
{
	const OMedia3DVector fwd = v226(0, 0, 1.0f);			// +0x388
	float rot[4];
	v201(rot);												// +0x324
	const float speed = v153();								// +0x264
	for (int i = 0; i < 6; i++)
	{
		C3DObject *w = f_960[i];
		if (!w) continue;
		w->v176(0, 0, 0);									// +0x2c0
		w->v162(0, 0, 0);									// +0x288
		float wp[4];
		w->v195(wp);										// +0x30c: where it was
		w->v190(wp[0], wp[1], wp[2], wp[3]);				// +0x2f8
		const OMedia3DVector &off = f_988[i];
		OMedia3DVector at;
		if (!f_a44 && v113())								// +0x1c4: physics on
		{
			const OMedia3DVector free = v224(off.x, off.y, off.z);	// +0x380
			w->v196(free.x, free.y, free.z, free.w);		// +0x310
			if (!w->v113())
			{
				w->v134(0);									// +0x218
				w->v188(0, -25000.0f, 0);					// +0x2f0
				PhysicsSnapToGround(w);						// 0x4b68f0
			}
			float g[4];
			w->v186(g);										// +0x2e8: the floor under it
			float h = g[1] - free.y + off.y;				// suspension
			const float lo = off.y - f_a18;
			if (h < lo) h = lo;
			else
			{
				const float hi = f_a1c + off.y;
				if (h > hi) h = hi;
			}
			at = v224(off.x, h, off.z);
		}
		else at = v224(off.x, off.y, off.z);
		w->v196(at.x, at.y, at.z, at.w);					// +0x310
		w->place(at.x, at.y, at.z);							// element +0x44..
		// spin: the distance it rolled
		OMedia3DVector d(wp[0] - at.x, 0, wp[2] - at.z, 0);
		const float mag = d.quick_magnitude();
		if (mag != 0)
		{
			const float dot = fwd.y * d.y + d.z * fwd.z + d.x * fwd.x;
			const float a = mag / w->v151() * 57.29578f;	// +0x25c radius; 0x51aa0c
			if (speed < 0 || !f_a41)
			{
				C3DWheel *ww = static_cast<C3DWheel *>(w);
				const double s = dot > 0 ? (double)a + ww->f_58c : (double)ww->f_58c - a;
				ww->f_58c = Fold(s);
			}
		}
		float r[4];
		v201(r);
		const float spin = static_cast<C3DWheel *>(w)->f_58c;
		if (i == 0 || i == 2 || i == 4)						// left
		{
			const float rz = Fold((double)r[0] - spin);
			const float ry = Fold((double)r[1] + 90.0f);	// 0x4d0c9c
			w->v203(r[2], ry, rz);							// +0x32c
		}
		else
		{
			const float rz = Fold((double)r[0] + spin);
			const float ry = Fold((double)r[1] - 90.0f);
			const float rx = Fold(-(double)r[2]);
			w->v203(rx, ry, rz);
		}
		AnglesFromRotation(w);								// +0x3e4
	}
	// the front wheels steer
	const OMedia3DVector st = v226(0, f_a24, 0);			// +0x388
	for (int i = 0; i < 2; i++)
	{
		if (!f_960[i]) continue;
		f_960[i]->AddRotation(st.x, st.y, st.z);			// +0x330
		AnglesFromRotation(f_960[i]);
	}
	if (!GameIsRunning() || !v135()) return;				// 0x4b0ca0, +0x21c
	float ax = rot[0], ay = rot[1], az = rot[2];
	if (f_9e8 <= 0 && f_9ea <= 0)
	{
		if (v131()) ev134();								// +0x20c, element +0x218: no wheel down
		return;
	}
	OMedia3DPolygon *poly = v205();							// +0x334
	if (!poly) return;
	const OMedia3DVector n = poly->get_normal();			// polygon +8
	FloorNormalToAngles(n, &ax, &ay, &az);					// 0x4b1fc0
	v218(ax, ay, az);										// +0x368
}

void C3DVehicle::ev134(void)								// 0x4a02a0
{
	v218(340.0f, 0, 0);										// +0x368
	f_29c = 2.0f;
}

OMedia3DVector C3DVehicle::ev135(float x, float y, float z)	// 0x4a02d0
{
	float r[4];
	v201(r);												// +0x324
	const omt_Angle ay = ToAngle(r[1]);
	OMediaCosSinTable &t = OMediaCosSinTable::global;
	const float sy = t.sin(ay), cy = t.cos(ay);
	// v226 with the x and z angles 0
	return OMedia3DVector(x * cy - z * sy, y, z * cy + x * sy, 0.0f);
}

OMedia3DVector C3DVehicle::ev136(float x, float y, float z)	// 0x4a03b0
{
	return v224(x, y, z);									// +0x380
}

OMedia3DVector C3DVehicle::ev137(float x, float y, float z)	// 0x4a0400
{
	float r[4];
	v201(r);												// +0x324
	double h = (double)r[1] + f_a24;						// the heading plus the steering
	if (h < 0.0) h = (double)(360 - abs((int)h));
	else if (h > 360.0) h = h - 360.0;
	const omt_Angle ax = ToAngle(r[0]);
	const omt_Angle ay = (omt_Angle)((int)(h * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle az = ToAngle(r[2]);
	OMediaCosSinTable &t = OMediaCosSinTable::global;
	const float sx = t.sin(ax), cx = t.cos(ax), sy = t.sin(ay), cy = t.cos(ay), sz = t.sin(az), cz = t.cos(az);
	const float a = cz * y - sz * x;
	const float b = sz * y + cz * x;
	const float c = cx * z - a * sx;
	float p[4];
	v195(p);												// +0x30c
	return OMedia3DVector(p[0] + (b * cy - c * sy), p[1] + (a * cx + sx * z), p[2] + (c * cy + b * sy), 1.0f);
}

OMedia3DVector C3DVehicle::ev138(float x, float y, float z)	// 0x4a05a0
{
	OMediaCosSinTable &t = OMediaCosSinTable::global;
	const omt_Angle ax = get_anglex() & omc_MaxAngleMask, ay = get_angley() & omc_MaxAngleMask, az = get_anglez() & omc_MaxAngleMask;
	const float sx = t.sin(ax), cx = t.cos(ax), sy = t.sin(ay), cy = t.cos(ay), sz = t.sin(az), cz = t.cos(az);
	const float a = cz * y - sz * x;
	const float b = sz * y + cz * x;
	const float c = cx * z - a * sx;
	float p[4];
	v195(p);												// +0x30c
	return OMedia3DVector(p[0] + (b * cy - c * sy), p[1] + (a * cx + sx * z), p[2] + (c * cy + b * sy), 1.0f);
}

void C3DVehicle::ev139(void)								// 0x4a06c0
{
	if (f_a4c) return;
	OMediaViewPort *vp = new OMediaViewPort(g_window);		// 0x120 bytes; g_window's supervisor (+0x14), NULL without a window
	f_a4c = vp;
	if (!vp) return;
	vp->link(g_world);										// +0x2c
	vp->link_layer(g_layer_world);							// +0x34 (0x56afc8)
	vp->link_window(g_window);								// +0xb0
	OMediaRect r(0, 0, (int)((float)(short)g_screen_w * 0.3f), (int)((float)(short)g_screen_h * 0.2f + 1.0f));	// 0x4caa64, 0x4e562c
	vp->setbounds(&r);										// +0xac
	OMediaRect b;
	vp->getbounds(&b);										// 2002 +0xe4
	const int dx = (int)((float)(short)g_screen_w * 0.5f - (float)(b.right - b.left) * 0.5f) - b.left;	// centred
	const int dy = 4 - b.top;								// 4 from the top
	b.left += dx; b.right += dx;
	b.top += dy; b.bottom += dy;
	vp->setbounds(&b);
}

void C3DVehicle::ev140(void)								// 0x4a0880
{
	if (!f_a4c) return;
	delete f_a4c;											// (deleting destructor through the listener at +0x30)
	f_a4c = NULL;
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DVehicle::InitObject(void)							// 0x49d840
{
	Log("InitObject()");
	C3DObject::InitObject();								// 0x49ac20
	SetObjectID('3VEH');									// element +0xc4
}

void C3DVehicle::ResetObject(void)							// 0x49d8b0
{
	Log("ResetObject()");
	CLocalGameObject::ResetObject();						// 0x48ee80
	f_a0c = 0;
	f_29c = 5.0f;
	f_a24 = 0;
	f_a28 = 0;
	v154(0);												// +0x268
	f_a04 = 0;
	float r[4];
	v201(r);												// +0x324
	f_978 = OMedia3DVector(r[0], r[1], r[2], r[3]);
	v201(r);
	f_a2c = r[1];
	ev107(f_9f0);											// element +0x1ac
	f_a44 = 0;
	f_a48 = 0;
}

void C3DVehicle::InitPhysics(void)							// 0x49d880
{
	Log("InitPhysics()");
	C3DObject::InitPhysics();								// 0x49acb0
}

void C3DVehicle::v218(float a, float b, float c)			// 0x49f400
{
	CGameObject::v218(a, b, c);								// 0x4ad4f0
	f_278 = f_228;
}

void C3DVehicle::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x49f6e0
{
	if (!f_a44 && f_a43)
	{
		float v[4];
		v174(v);											// +0x2b8
		OMedia3DVector h(v[0], 0, v[2], v[3]);
		if (h.quick_magnitude() > 20.0f)					// 0x4caa40: fast
		{
			const OMedia3DVector fwd = v226(0, 0, 1.0f);	// +0x388
			if (!(fabsf(fwd.z * c + fwd.y * b + fwd.x * a) < 0.9f))	// 0x4e4158: head-on
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
	s = s - fabsf(dot) * f_a00 * g_frame_time;				// 0x6f75c8
	if (f_004) { if (!(s > 100.0f)) s = 100.0f; }			// 0x4e4148
	else if (!(s > 0.0f)) s = 0.0f;							// 0x4d17c0
	v154(s);												// +0x268
}

void C3DVehicle::Process(float dt)							// 0x49da30
{
	C3DObject::Process(dt);									// 0x49ae00
	if (!GameIsRunning() || !v131()) return;				// 0x4b0ca0, +0x20c
	f_9e8 = 0;
	f_9ea = 0;
	for (int i = 0; i < 6; i++)
	{
		if (!f_960[i] || !f_960[i]->v133()) continue;		// +0x214: on the floor
		if (i >= 0 && i <= 1) f_9e8++;
		else f_9ea++;
	}
	f_9ec = f_9e8 + f_9ea;
	float rot[4];
	v201(rot);												// +0x324
	float vv[4];
	v174(vv);												// +0x2b8
	OMedia3DVector vel(vv[0], vv[1], vv[2], vv[3]);
	const float vy = vel.y;
	if (f_a44)
	{
		ev111(dt);											// element +0x1bc
		return;
	}
	if (f_004)
	{
		union { float f; unsigned int u; } arg;
		arg.f = dt;
		v244(arg.u);										// +0x3d0 (empty)
	}
	vel.y = 0;
	const float hs = vel.quick_magnitude();					// horizontal speed
	float s = v153();										// +0x264
	if (hs > 0)
	{
		if (f_9e8 > 0)										// front wheels down: turn
		{
			const float inv = 1.0f / vel.quick_magnitude();
			vel.x *= inv; vel.y *= inv; vel.z *= inv;
			const OMedia3DVector fwd = v226(0, 0, 1.0f);	// +0x388
			const float dot = fwd.x * vel.x + fwd.z * vel.z + fwd.y * vel.y;
			float t = hs * 0.005f;							// 0x51a9f0
			if (!(t < 1.0f)) t = 1.0f;
			const float k = f_a48 ? 1.5f : 1.0f;			// 0x4caa60
			if (f_004) f_a28 = 2.0f * (k * f_a24 * t);
			else f_a28 = (float)f_9e8 * f_a24 * k * t;
			if (!(dot < 0)) AddRotation(0, dt * f_a28, 0);	// +0x330
			else AddRotation(0, -(dt * f_a28), 0);
		}
		if (f_9ec != 0)
		{
			if (!f_a40 && !f_a47 && (f_9e8 != 0 || f_9ea != 0))	// rolling out
			{
				s = s - s * 0.5f * dt;						// 0x4caa5c
				if (!(s < -1.0f) && !(s > 1.0f)) s = 0;		// 0x4cb1dc
			}
			if (!f_004 && !f_a41)							// steering slows it
				s = s - (float)(fabsf(f_a24) * (1.0 / 45.0) * (s * 0.8f) * dt);	// 0x51a9e8, 0x4caa58
			s = s - (float)(sin(rot[0] * 0.017453292f) * 500.0 * dt);	// 0x51a9e0, 0x51a9d8: slopes
		}
	}
	if (v133() && f_9ec >= 2)								// +0x214
	{
		const OMedia3DVector fwd = v226(0, 0, 1.0f);
		v174(vv);
		OMedia3DVector dir(vv[0], vv[1], vv[2], vv[3]);
		const float mag = dir.quick_magnitude();
		if (s > 0 && mag != 0)
		{
			const float inv = 1.0f / dir.quick_magnitude();
			dir.x *= inv; dir.y *= inv; dir.z *= inv;
			if (dir.z * fwd.z + dir.y * fwd.y + dir.x * fwd.x < 0) dir = fwd;	// going backwards
			if (!f_a41)
			{
				float g;
				if (f_a48) g = 0.9f;						// 0x4e4158
				else
				{
					const float k = f_a08 * 0.66f * 0.2f / s;	// 0x4d7f94, 0x4e562c
					if (v145() > v145() * k) g = v145() * k;	// +0x244
					else g = v145();
				}
				dir.x = fwd.x * g + dir.x;					// grip: towards the heading
				dir.y = fwd.y;
				dir.z = fwd.z * g + dir.z;
				f_a20 = g;
			}
			f_a30 = 0;
		}
		else
		{
			dir = fwd;
			if (f_a41)
			{
				f_a30 += dt;
				if (f_a30 < 1.0f) s = 0;
			}
		}
		dir.x *= s; dir.y *= s; dir.z *= s;
		v176(dir.x, dir.y, dir.z);							// +0x2c0: velocity
		if (vy < 0) v178(0, vy, 0);							// +0x2c8: keep falling
	}
	v154(s);												// +0x268
}

void C3DVehicle::v243(float dt)								// 0x49e040
{
	float s = v153();										// +0x264
	if (!f_004)
	{
		f_a0c = s;
		f_a49 = 0;
		f_a4a = 0;
		short mx = 0, my = 0;
		ev113(&mx, &my);									// element +0x1c4
		float turn = (float)mx;
		if (turn == 0)
		{
			bool key = false;
			if (ev123()) { turn = turn + dt * 200.0f; key = true; }		// 0x4cb21c: Left
			else if (ev124()) { turn = turn - dt * 200.0f; key = true; }	// Right
			else if (f_a45)									// let go: centre again
			{
				if (f_a24 < 0)
				{
					f_a24 = dt * 200.0f + f_a24;
					if (f_a24 > 0) f_a24 = 0;
				}
				else if (f_a24 > 0)
				{
					f_a24 = f_a24 - dt * 200.0f;
					if (f_a24 < 0) f_a24 = 0;
				}
			}
			if (key)
			{
				f_a45 = 1;
				if (!(s > 130.0f) && !ev126() && s < f_a10)	// 0x4da46c: slow, steering speeds it up
				{
					s = dt * f_9f8 + s;
					if (s > f_a10) s = f_a10;
				}
			}
		}
		else
		{
			turn = dt + dt;									// the mouse
			turn = turn * (float)mx;
			f_a45 = 0;
		}
		f_a24 = turn + f_a24;
		if (f_a24 < -45.0f) f_a24 = -45.0f;					// 0x4dce40
		else if (f_a24 > 45.0f) f_a24 = 45.0f;				// 0x4dce3c
		f_a40 = 0;
		if (!f_a44)
		{
			if (ev127())									// element +0x1fc: boost
			{
				f_a10 = f_a08 * 0.825f;						// 0x51a9f8
				f_a48 = 1;
				f_a49 = 1;
			}
			else
			{
				f_a10 = f_a08 * 0.66f;
				f_a48 = 0;
			}
			if (g_56b084) ev112();							// element +0x1c0: the joystick
			if (ev125() || f_a49)							// element +0x1f4: Go
			{
				f_a40 = 1;
				if (s < f_a10)
				{
					s = dt * f_9f8 + s;
					if (s > f_a10) s = f_a10;
				}
				else if (f_a48)
				{
					s = s - dt * f_a00;
					if (s < f_a10)
					{
						f_a48 = 0;
						s = f_a10;
					}
				}
				if (f_a47)
				{
					if (!f_a46) ev140();					// element +0x230
					f_a47 = 0;
				}
			}
			if (!ev126() && !f_a4a)							// element +0x1f8: Back
			{
				f_a41 = 0;
				if (f_a47)
				{
					if (!f_a46) ev140();
					f_a47 = 0;
				}
			}
			else if (!f_a47)								// first frame: brake
			{
				f_a47 = 1;
				if (s > 0)
				{
					s = s - dt * f_a00;
					if (!(s > 0)) s = 0;
					f_a41 = 1;
				}
				else if (s < 0)
				{
					s = dt * f_a00 + s;
					if (!(s < 0)) s = 0;
					f_a41 = 1;
				}
			}
			else											// held: brake, then reverse
			{
				s = s - dt * f_a00;
				if (s < 0)
				{
					if (!f_a4c) ev139();					// element +0x22c: the mirror
					const float lim = -(f_a10 * 0.4f);		// 0x4d0c98
					if (s < lim) s = lim;
				}
			}
		}
		if (f_a40) f_a04 = f_a08;
		else f_a04 = s * 1.5151515f;						// 0x51a9f4
		if (!g_keys[0x11])									// VK_CONTROL: no camera keys
		{
			if (ev128()) f_a38 = (f_a38 == 6) ? 1 : (f_a38 == 1 ? 2 : 6);	// element +0x200: C / V
			else if (ev114()) f_a38 = 6;
			else if (ev115()) f_a38 = 3;
			else if (ev116()) f_a38 = 1;
			else if (ev117()) f_a38 = 2;
			else if (ev118()) f_a38 = 11;
			else if (ev119()) f_a38 = 12;
			else if (ev120()) f_a38 = 20;
			else if (ev121()) f_a38 = 13;
			else if (ev122()) f_a38 = 15;
			if (ev129())									// element +0x204: B, look back
			{
				if (f_a3c == 0) f_a3c = f_a38;
				f_a38 = 8;
			}
			else if (f_a38 == 8 && f_a3c != 0 && !f_004)
			{
				f_a38 = f_a3c;
				f_a3c = 0;
			}
		}
	}
	v154(s);												// +0x268
	ev110(dt);												// element +0x1b8: the camera
}

void C3DVehicle::RotateToDest(float dt)						// 0x49f430
{
	CGameObject::RotateToDest(dt);							// 0x4ae180
}

void C3DVehicle::v247(void)									// 0x49f960
{
	C3DObject::v247();										// 0x49ad90
}

void C3DVehicle::v248(void)									// 0x49f970
{
	ev132();												// element +0x210
	C3DObject::v248();										// 0x49add0
}
