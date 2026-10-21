/*  C3DLunarLander - see C3DLunarLander.h. Reconstructed from Neutron2.exe 0x4509a0..0x451700. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DLunarLander.h"
#include "../C3DPlayer.h"					// PlayerKey*, JimmyActivateMenuItem
#include "../CanvasWrapper.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../InterfaceElement.h"				// AddScore
#include "../Mouse.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <list>
#include <math.h>
#include <string>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x465b60: a key of the "deactivate item" list (no joystick button); as the key tests of
// C3DPlayer.cpp (as in C3DGoddard.cpp).
static bool KeyDeactivateItem(void)
{
	for (const int *list = g_key_deactivate_item; *list; list++)
	{
		const int k = *list;
		if (k == 0x1f5) { if (g_mouse_down) return true; }
		else if (k == 0x1f4) { if (g_mouse_rdown) return true; }
		else if ((unsigned)k < 256 && g_keys[k])			// port: index checked (the original reads g_keys[k] for any k)
		{
			g_56aebe = 0;
			g_56aec0 = 0;
			return true;
		}
	}
	return false;
}

// 0x404d60: the gadget bar's size from a count (height = count * 1.04, scaled to the screen).
// (As in C3DRocketShip.cpp.)
static void SetGadgetBarSize(float n)
{
	if (n < 0) n = 0;
	if (!g_gadget_bar) return;
	const float h = (float)(short)g_screen_h * 0.00208333344f;	// 0x4ca4b8: 1 / 480
	g_gadget_bar->set_size((float)(short)g_screen_w * 0.00156250002f * 20.0f,	// 0x4ca4bc: 1 / 640
						   (float)((double)h * n * 1.04));
}

// 0x4a39f0 (CAweReal): the mouse steering (as in C3DHoverCycle.cpp). Per axis, the pointer's
// distance from the viewport's centre: inside the dead zone (+-lim), with the held value inside
// it too, 0; outside, the distance / k is kept when it is further out than the held value
// (g_56aebe x, g_56aec0 y). Every 0.2 s (g_56aea8) the pointer goes back to the centre.
static void MouseSteer(short *dx, short *dy, float dt, float kx, float ky, short limx, short limy)
{
	if (g_56aed0) return;
	short x = 0, y = 0;
	GetMousePos(&x, &y);									// 0x4b1d10
	const short rx = (short)(MouseCentreX() - x);			// 0x4b1da0
	const short ry = (short)(MouseCentreY() - y);			// 0x4b1dc0
	short held = g_56aebe;
	if (rx > limx || rx < -limx || held > limx || held < -limx)
	{
		if (kx <= 0.0f) kx = 0.0001f;
		if (rx > limx)
		{
			g_56aeba = (short)(int)((float)rx / kx);
			if (g_56aeba > held) { held = g_56aeba; g_56aebe = held; }
		}
		if (rx < -limx)
		{
			g_56aeba = (short)(int)((float)rx / kx);
			if (g_56aeba < held) { held = g_56aeba; g_56aebe = held; }
		}
		*dx = held;
	}
	else
	{
		g_56aebe = 0;
		*dx = 0;
	}
	held = g_56aec0;
	if (ry > limy || ry < -limy || held > limy || held < -limy)
	{
		if (ky <= 0.0f) ky = 0.0001f;
		if (ry > limy)
		{
			g_56aebc = (short)(int)((float)ry / ky);
			if (g_56aebc > held) { held = g_56aebc; g_56aec0 = held; }
		}
		if (ry < -limy)
		{
			g_56aebc = (short)(int)((float)ry / ky);
			if (g_56aebc < held) { held = g_56aebc; g_56aec0 = held; }
		}
		*dy = held;
	}
	else
	{
		g_56aec0 = 0;
		*dy = 0;
	}
	if (g_56aea8 > 0.0)
	{
		g_56aea8 = g_56aea8 - dt;
		return;
	}
	CenterMouse();											// 0x4b1d50
	g_56aea8 = 0.2f;
}

// --------------------------------------------------------------------------- construction

C3DLunarLander::C3DLunarLander()							// 0x4509a0
{
	m_cam_pos.w = 1.0f;										// (OMedia3DVector constructor)
	SetName((char *)"C3DLUNARLANDER");
	Log("C3DLUNARLANDER()");
	C3DLunarLander::InitObject();							// 0x450b90 (direct call)
	C3DObject::SetObjectID('3LUN');							// 0x49ab00
	CGameObject::v46(1);									// 0x4abf10 (direct calls)
	CGameObject::v52(1);
	CGameObject::v42(1);
	CGameObject::v40(1);
	CGameObject::v44(0);
	CGameObject::v94(2);
	CGameObject::v111(0);
	C3DObject::InitPhysics();								// 0x49acb0
	m_jim = 0;
	m_cam_pos.x = 0;
	m_cam_pos.y = 0;
	m_cam_pos.z = 0;
	m_landed = 1;
	m_fuel = 100.0f;
	m_channel = -1;
	m_landings = 0;
}

C3DLunarLander::~C3DLunarLander()							// 0x450b40
{
}

namespace neg {
CGameObject *New_C3DLunarLander(void) { return new C3DLunarLander; }	// '3LUN' (0x1b7c bytes)
}

void C3DLunarLander::InitObject(void)						// 0x450b90
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0 (not C3DAI's)
	InitAnim();												// +0x110
	CreateAnim((char *)"FLY", (char *)"lunerlander.ase");	// +0xe0
	CreateAnim((char *)"CRASH", (char *)"lunerlandercrash.ase");
	CreateAnim((char *)"THRUST", (char *)"lunerlanderthrust.ase");
	CreateAnim((char *)"LAND", (char *)"lunerlanderland.ase");
	CreateAnim((char *)"HOVER", (char *)"lunerlanderhover.ase");
	CreateAnim((char *)"STOP", (char *)"lunerlanderstop.ase");
	CreateTexture((char *)"lander.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(100.0f);											// radius
	ev58((char *)"LAND", 1);								// +0xe8 looped
}

void C3DLunarLander::v260(void)								// 0x46fc00
{
	C3DAnimated::v260();									// 0x41cec0
}

// --------------------------------------------------------------------------- flying

void C3DLunarLander::ev144(void)							// 0x450d20
{
	CGameObject *t = FindObjectByTag("LANDINGTARGET");		// 0x4aefb0
	if (!t) return;
	float p[4];
	t->v195(p);												// +0x30c
	v196(p[0], p[1], p[2], p[3]);							// +0x310 position
}

void C3DLunarLander::Process(float dt)						// 0x450c90
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120 (no AI)
	if (g_camera == this || m_landed) return;
	if (!IsAnim((char *)"HOVER")) ev58((char *)"HOVER", 1);	// 0x41c6d0, +0xe8
	v178(0, -3.0f, 0);										// +0x2c8 velocity +=
}

void C3DLunarLander::v243(float dt)							// 0x450d80
{
	C3DObject::v243(dt);									// 0x49af30
	if (m_landings > 10 || KeyDeactivateItem())				// 0x465b60
	{
		ev144();											// element +0x240
		if (g_jimmy) JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 0, 0.0f);	// Jimmy element +0x228: out
		return;
	}
	// (a debug print of the vertical speed, compiled empty)
	if (!m_jim)
	{
		m_jim = FindObjectByTag("JIM1");					// 0x4aefb0
		if (m_jim) m_cam_pos = m_jim->v224(0, 300.0f, -600.0f);	// +0x380
	}
	else
		SetGadgetBarSize(m_fuel);							// 0x404d60
	if (!v133()) m_landed = 0;								// +0x214 in the air
	short mx = 0, my = 0;
	MouseSteer(&mx, &my, dt, 8.0f, 6.0f, 10, 10);			// 0x4a39f0
	float v[4];
	if (PlayerKeyLeft() || mx > 10)							// 0x4659f0
	{
		v174(v);											// +0x2b8 velocity
		v176(0, v[1], 100.0f);								// +0x2c0
		m_fuel = m_fuel - dt * 5.0f;						// 0x4caa90
	}
	else if (PlayerKeyRight() || mx < -10)					// 0x465a60
	{
		v174(v);
		v176(0, v[1], -100.0f);
		m_fuel = m_fuel - dt * 5.0f;
	}
	else
	{
		v174(v);
		if (v[2] > 10.0f)									// 0x4cb1cc
		{
			float a[4], b[4];
			v174(a);
			v174(b);
			v176(b[0], a[1], v[2] - dt * 20.0f);			// 0x4caa40
		}
		else
		{
			v174(v);
			if (v[2] < -10.0f)								// 0x4dce44
			{
				float a[4], b[4];
				v174(a);
				v174(b);
				v176(b[0], a[1], v[2] + dt * 20.0f);
			}
			else
			{
				float a[4];
				v174(a);
				v174(v);
				v176(v[0], a[1], 0);
			}
		}
	}
	if (PlayerKeyGo() || PlayerKeyAction() || PlayerKeyJump())	// 0x465910, 0x465750, 0x4658a0: thrust
	{
		if (m_fuel > 0.0f)
		{
			if (!IsAnim((char *)"THRUST")) ev58((char *)"THRUST", 1);
			v178(0, 10.0f, 0);								// +0x2c8 (per frame)
			m_fuel = m_fuel - dt * 11.0f;					// 0x4cb1c8
		}
		else
		{
			m_fuel = 0;
			v178(0, -5.0f, 0);
			if (!IsAnim((char *)"HOVER")) ev58((char *)"HOVER", 1);
		}
		if (m_channel == -1)
			m_channel = PlayGameSound(-1, std::string("rocket blast"), true);	// 0x48f420
	}
	else if (PlayerKeyBack())								// 0x465980: get out
	{
		if (g_jimmy) JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 0, 0.0f);
		return;
	}
	else
	{
		if (!IsAnim((char *)"HOVER")) ev58((char *)"HOVER", 1);
		v178(0, -3.0f, 0);
		if (m_channel != -1)
		{
			StopGameSound(m_channel, false);				// 0x48f0d0
			m_channel = -1;
		}
	}
	const OMedia3DVector target = v224(0, 100.0f, 0);		// +0x380
	ev97(m_cam_pos, target);								// element +0x184: the camera at m_cam_pos looks at us
}

void C3DLunarLander::v222(void)								// 0x451270
{
	if (m_landed) return;
	if (g_camera != this)
	{
		m_landed = 1;
		if (!IsAnim((char *)"CRASH"))
		{
			ev58((char *)"CRASH", 0);
			PlayGameSound(-1, std::string("rock explosion2"), false);
		}
		v176(0, 0, 0);
		m_fuel = 100.0f;
		ev144();											// element +0x240
		return;
	}
	m_landings++;
	CGameObject *t = FindObjectByTag("LANDINGTARGET");		// 0x4aefb0
	float dist = 0;
	if (t)
	{
		float p[4], q[4];
		v195(p);											// +0x30c
		t->v195(q);
		const float dx = p[0] - q[0], dy = p[1] - q[1], dz = p[2] - q[2];
		dist = sqrtf(dz * dz + dy * dy + dx * dx);
	}
	float v[4];
	v174(v);												// +0x2b8 velocity
	if (v[1] > -40.0f)										// 0x4d65bc: soft
	{
		if (!IsAnim((char *)"LAND")) ev58((char *)"LAND", 1);
		if (dist < 120.0f)									// 0x4d0568
		{
			AddScore(500);									// g_gametype 0x482b50
			PlayGameSound(-1, std::string("cheering"), false);
		}
	}
	else
	{
		v174(v);
		if (v[1] > -150.0f)									// 0x4e83b0
		{
			if (dist < 180.0f)								// 0x4cb1f4
			{
				AddScore(100);
				PlayGameSound(-1, std::string("pickup3"), false);
			}
			if (!IsAnim((char *)"LAND")) ev58((char *)"LAND", 1);
		}
		else
		{
			v174(v);
			if (v[1] > -250.0f)								// 0x4d2cb8
			{
				if (!IsAnim((char *)"LAND")) ev58((char *)"LAND", 1);
				if (dist < 310.0f)							// 0x4e83ac
				{
					AddScore(20);
					PlayGameSound(-1, std::string("pickup2"), false);
				}
			}
			else
			{
				if (!IsAnim((char *)"CRASH"))
				{
					ev58((char *)"CRASH", 0);
					PlayGameSound(-1, std::string("rock explosion2"), false);
				}
				v176(0, 0, 0);
				m_fuel = 100.0f;
			}
		}
	}
	if (g_jimmy)
	{
		JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 0, 0.0f);	// Jimmy element +0x228: out
		m_landed = 1;
	}
	if (m_channel != -1)
	{
		StopGameSound(m_channel, false);
		m_channel = -1;
	}
}

void C3DLunarLander::v277(void)								// 0x451610
{
	C3DAnimated::v277();									// 0x41ce80
	if (m_channel != -1) StopGameSound(m_channel, false);	// 0x48f0d0 (the channel is kept)
}

void C3DLunarLander::v278(void)								// 0x451630
{
	C3DAnimated::v278();									// 0x41cea0
	if (m_channel != -1)
		m_channel = PlayGameSound3D(this, -1, std::string("rocket blast"), true);	// 0x48f200
}
