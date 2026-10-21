/*  C3DCar - see C3DCar.h. Reconstructed from Neutron2.exe 0x4244c0..0x425840. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCar.h"
#include "C3DCamera.h"
#include "../level5a/C3DWheel.h"
#include "../C3DPlayer.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Physics.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaViewPort.h"
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DCar::C3DCar()											// 0x4244c0
{
	f_a50 = OMedia3DVector(0, 0, 0, 1.0f);					// (only the w are set)
	f_a60 = OMedia3DVector(0, 0, 0, 1.0f);
	SetName((char *)"C3DCAR");
	Log("C3DCar()");
	// (debug log "Test 1")
	v64(0.2f);
	C3DObject::SetObjectID('3CAR');							// 0x49ab00
	v62(0.42f);
	// (debug log "Test 2")
	f_a70 = 0;
	f_a74 = 0;
	f_a78 = -1;
	f_a7c = 11025;
	f_a80 = 0;
	f_a81 = 0;
	f_a82 = 0;
}

C3DCar::~C3DCar() {}										// 0x424640

// --------------------------------------------------------------------------- element slots

void C3DCar::ev110(float dt)								// 0x424dc0
{
	float pp[4];
	v195(pp);												// +0x30c
	OMedia3DVector P(pp[0], pp[1], pp[2], pp[3]);
	OMedia3DVector C(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 1.0f);	// viewport +0x44
	C3DCamera *probe = CameraProbe();
	probe->v196(C.x, C.y, C.z, 1.0f);						// +0x310: the probe at the camera
	OMedia3DVector T;
	if (f_a44 && !g_56af41)									// crashed: drift up above it
	{
		const float dx = P.x - C.x;
		const float dz = P.z - C.z;
		g_viewport->place(dx * dt * 0.3f + g_viewport->getx(), g_viewport->gety(), dz * dt * 0.3f + g_viewport->getz());	// 0x4caa64
		v195(pp);
		T = OMedia3DVector(pp[0], pp[1], pp[2], pp[3]);
		if (!getvisible()) ev106();							// element +0x70, +0x1a8
	}
	else
	{
		OMedia3DVector W;
		bool check = false;									// shown again if hidden
		switch (f_a38)
		{
			case 1:											// behind, catching up
			{
				W = ev136(-30.0f, 0, -200.0f);				// element +0x220
				W.y += 50.0f;								// 0x4ca4b0
				const float dx = W.x - C.x, dy = W.y - C.y, dz = W.z - C.z;
				double k;
				if (ev126()) k = 3.5;						// element +0x1f8: braking (0x4d2cd0)
				else
				{
					OMedia3DVector d(dx, dy, dz, 0);
					const float m = d.quick_magnitude();
					if (m > 400.0f) k = 2.8;				// 0x4ccf10, 0x4d2cc8
					else if (m > 200.0f) k = 1.7;			// 0x4cb21c, 0x4d2cc0
					else k = 1.0;
				}
				if (k == 1.0)
					g_viewport->place(dx * dt + g_viewport->getx(), dy * dt + g_viewport->gety(), dz * dt + g_viewport->getz());
				else
					g_viewport->place((float)(dx * dt * k) + g_viewport->getx(), (float)(dy * dt * k) + g_viewport->gety(),
									  (float)(dz * dt * k) + g_viewport->getz());
				break;
			}
			case 3:											// behind and above
			case 4:
			{
				const float s = v153();						// +0x264
				if (f_a38 == 3) W = ev136(-f_a24, 125.0f, -250.0f - s * 0.05f);	// 0x4d2cb8, 0x4d2cbc
				else W = ev136(-f_a24, 225.0f, -400.0f - s * 0.05f);			// 0x4d2cb4
				g_viewport->place(W.x, W.y, W.z);
				check = true;
				break;
			}
			case 5:											// behind at the heading plus the steering
				W = ev137(0, 0, -200.0f);					// element +0x224
				g_viewport->place(W.x, P.y + 100.0f, W.z);	// 0x4d2cb0
				check = true;
				break;
			case 6:											// in it
				g_viewport->place(P.x, P.y + 100.0f, P.z);
				if (getvisible()) HideAgain();				// element +0xdc
				break;
			case 7:											// just above it
				g_viewport->place(P.x, P.y + 30.0f, P.z);	// 0x4ca4b4
				check = true;
				break;
			case 8:											// in front, looking back
				W = ev136(0, 75.0f, 300.0f);
				g_viewport->place(W.x, W.y, W.z);
				break;
			case 9:											// beside a wheel
			case 11:
				W = ev136(f_988[2].x - 25.0f, f_988[2].y + 20.0f, f_988[2].z - 75.0f);	// 0x4d056c, 0x4caa40, 0x4d2cac
				g_viewport->place(W.x, W.y, W.z);
				break;
			case 10:
			case 12:
				W = ev136(f_988[3].x + 25.0f, f_988[3].y + 20.0f, f_988[3].z - 75.0f);
				g_viewport->place(W.x, W.y, W.z);
				break;
			case 13:										// rising high above it
			case 16:
			case 18:
				P.y = P.y + 4000.0f;						// 0x4d2ca8 (kept for the probe below)
				g_viewport->place((P.x - C.x) * 0.1f * dt + g_viewport->getx(), (P.y - C.y) * dt + g_viewport->gety(),
								  (P.z - C.z) * 0.1f * dt + g_viewport->getz());	// 0x4caa68
				check = true;
				break;
			case 15:										// towards (0, 3000, 1500)
			case 17:
			case 19:
				g_viewport->place(-C.x * dt + g_viewport->getx(), (3000.0f - C.y) * dt + g_viewport->gety(),
								  (1500.0f - C.z) * dt + g_viewport->getz());	// 0x4d2ca4, 0x4cb1e0
				check = true;
				break;
			case 14:										// far behind and above
			case 20:
			{
				const OMedia3DVector b = v224(0, 0, -500.0f);	// +0x380
				g_viewport->place(b.x, b.y + 1000.0f, b.z);	// 0x4cb1d8
				check = true;
				break;
			}
			default:										// 2: left where it is
				break;
		}
		if (check && !getvisible()) ev106();				// element +0x70, +0x1a8
		if (f_a4c)											// the rear-view mirror, facing back
		{
			const OMedia3DVector m = v224(0, 125.0f, -30.0f);
			f_a4c->place(m.x, m.y, m.z);					// +0x44
			f_a4c->set_angle(0, (omt_WAngle)(get_angley() + 0x2000), 0);	// +0x50
		}
		T = v224(0, 50.0f, 100.0f);							// what it looks at
	}
	C = OMedia3DVector(g_viewport->getx(), g_viewport->gety(), g_viewport->getz(), 0);
	if (g_56af42 && !g_56af41)								// camera collision: the probe from us to the camera
	{
		OMedia3DVector start = P;
		const OMedia3DVector move(C.x - P.x, C.y - P.y, C.z - P.z, 0);
		g_closest_face_t = 130000.0f;						// 0x6f72f4
		PhysicsMoveAgainstTerrain(probe, &start, move, false);	// 0x4b6500
		g_viewport->place(start.x, start.y, start.z);
		C = start;
	}
	OMedia3DVector L(T.x - C.x, T.y - C.y, T.z - C.z, 0);
	omt_Angle ax, ay;
	L.angles(ax, ay);
	const short bx = g_viewport->get_anglex();
	short dyaw = (short)((ay - g_viewport->get_angley()) & 0x3fff);
	if (dyaw > 0x2000) dyaw = (short)(dyaw - 0x4000);
	if (!f_a44)
	{
		switch (f_a38)
		{
			case 6: case 7: case 9: case 10: case 11: case 12:	// its own angles
				g_viewport->set_angle(get_anglex(), get_angley(), get_anglez());
				return;
			case 8:
				g_viewport->set_angle(0, (omt_WAngle)(get_angley() + 0x2000), 0);
				return;
			case 20:										// looking down
				g_viewport->set_angle(0x3800, get_angley(), 0);
				return;
			default:
				break;
		}
	}
	g_viewport->set_anglex((omt_WAngle)(g_viewport->get_anglex() + (short)(ax - bx)));
	g_viewport->set_angley((omt_WAngle)(g_viewport->get_angley() + dyaw));
}

unsigned char C3DCar::ev126(void)							// 0x424d90
{
	if (!g_joy_button1 && !PlayerKeyBack()) return 0;		// 0x465980
	return f_9ea > 0;
}

void C3DCar::ev132(void)									// 0x424cc0
{
	float r[4];
	v201(r);												// +0x324
	v203(r[0], r[1], f_a24 * -0.5f);						// +0x32c: lean into the turn (0x4d2ca0)
	C3DVehicle::ev132();									// 0x49f990
}

// One wheel of SetupCar4: made if needed ('3WHE'), its shape, its place under us.
static C3DObject *MakeWheel(C3DCar *car, int i, int shape, float x, float y, float z, bool always)
{
	if (always || !car->f_960[i])
	{
		CGameObject *o = CreateObject('3WHE');				// 0x4912d0
		car->f_960[i] = o ? dynamic_cast<C3DObject *>(o) : NULL;
	}
	C3DWheel *w = car->f_960[i] ? dynamic_cast<C3DWheel *>(car->f_960[i]) : NULL;
	if (!w) return car->f_960[i];
	w->ev56(shape);											// element +0xe0
	car->f_988[i].x = x;
	car->f_988[i].y = y;
	car->f_988[i].z = z;
	w->place(car->getx() + x, car->gety() + y, car->getz() + z);	// element +0x44
	w->InitPhysics();										// +0x2c
	return w;
}

void C3DCar::ev141(float zf, float zr, float xf, float xr, float y, int shape_f, int shape_r)	// 0x4246f0
{
	Log("SetupCar4(%f,%f,%f,%f,%f)", (double)zf, (double)zr, (double)xf, (double)xr, (double)y);
	for (int i = 0; i < 4; i++)
	{
		float x, z;
		int shape;
		switch (i)
		{
			case 0: x = -xf; z = zf; shape = shape_f; break;
			case 1: x = xf; z = zf; shape = shape_f; break;
			case 2: x = -xr; z = zr; shape = shape_r; break;
			default: x = xr; z = zr; shape = shape_r; break;
		}
		C3DObject *w = MakeWheel(this, i, shape, x, y, z, false);
		if (!w) continue;
		w->Link(this);										// +0x3a4: both ways
		Link(w);
	}
}

void C3DCar::ev142(float zf, float zm, float zr, float xf, float xm, float xr, float y,
				   int shape_f, int shape_m, int shape_r)	// 0x4248f0
{
	Log("SetupCar4(%f,%f,%f,%f,%f,%f,%f)", (double)zf, (double)zm, (double)zr, (double)xf, (double)xm, (double)xr, (double)y);
	for (int i = 0; i < 6; i++)
	{
		float x, z;
		int shape;
		switch (i)
		{
			case 0: x = -xf; z = zf; shape = shape_f; break;
			case 1: x = xf; z = zf; shape = shape_f; break;
			case 2: x = -xr; z = zr; shape = shape_r; break;
			case 3: x = xr; z = zr; shape = shape_r; break;
			case 4: x = -xm; z = zm; shape = shape_m; break;
			default: x = xm; z = zm; shape = shape_m; break;
		}
		C3DObject *w = MakeWheel(this, i, shape, x, y, z, true);
		if (!w) continue;									// port: the original goes on with NULL
		w->v46(0);											// +0xb8
		w->v67(80.0f);										// +0x10c: radius
	}
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DCar::ResetObject(void)								// 0x424d20
{
	C3DVehicle::ResetObject();								// 0x49d8b0
	v196(f_a50.x, f_a50.y, f_a50.z, f_a50.w);				// +0x310: position
	v202(f_a60.x, f_a60.y, f_a60.z, f_a60.w);				// +0x328: rotation
}

void C3DCar::Process(float dt)								// 0x424b30
{
	C3DVehicle::Process(dt);								// 0x49da30
	if (v135() && g_active_sound_db)						// +0x21c
	{
		if (f_a78 == -1)
		{
			f_a78 = PlayGameSound3D(this, -1, std::string("Scooter"), true);	// 0x48f200
			f_a7c = g_chan_freq[f_a78];						// 0x6f73d4
		}
		else
		{
			int f = (int)((double)v153() * 0.5263157894736842 * 51.0 + f_a7c);	// 0x4d2c98, 0x4d2c90
			if (f <= 7000) f = 7000;
			g_chan_freq[f_a78] = f;
		}
	}
	if (g_power_timers[0] > 0)								// power-up 0: more power
	{
		if (!f_a81)
		{
			f_a81 = 1;
			ev107(350.0f);									// element +0x1ac
		}
	}
	else if (f_a81)
	{
		ev107(0);
		f_a81 = 0;
	}
	if (g_power_timers[2] > 0)								// power-up 2
	{
		if (!f_a82)
		{
			f_a82 = 1;
			v62(0.61f);										// +0xf8
		}
	}
	else if (f_a82)
	{
		v62(0.21f);
		f_a82 = 0;
	}
	f_a74 += dt;
	f_a70 += dt;
}
