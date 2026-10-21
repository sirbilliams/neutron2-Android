/*  C3DCraneBoom - see C3DCraneBoom.h. Reconstructed from Neutron2.exe 0x428940..0x4293c0, and
    the CAweReal mouse helper 0x4a39f0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCraneBoom.h"
#include "C3DCraneParts.h"
#include "../C3DPlayer.h"					// PlayerKey*
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Mouse.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaViewPort.h"
#include <string>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4a39f0 (CAweReal): the mouse movement for steering. The pointer's offset from the
// viewport's centre, beyond a dead zone (limx / limy), divided by kx / ky, is held as the
// output (g_56aebe / g_56aec0, the extremes g_56aeba / g_56aebc) until both the offset and the
// held value are back within the dead zone; the pointer is re-centred every 0.2 s.
static void MouseSteer(short *dx, short *dy, float dt, float kx, float ky, short limx, short limy)
{
	if (g_56aed0) return;									// (mouse steering off)
	short mx = 0, my = 0;
	GetMousePos(&mx, &my);									// 0x4b1d10
	const short ox = (short)(MouseCentreX() - mx);			// 0x4b1da0
	const short oy = (short)(MouseCentreY() - my);			// 0x4b1dc0
	short cur = g_56aebe;
	if (ox > limx || ox < -limx || cur > limx || cur < -limx)
	{
		if (kx <= 0.0f) kx = 0.0001f;						// 0x38d1b717
		if (ox > limx)
		{
			g_56aeba = (short)(int)((float)ox / kx);		// _ftol
			if (g_56aeba > cur) { cur = g_56aeba; g_56aebe = cur; }
		}
		if (ox < -limx)
		{
			g_56aeba = (short)(int)((float)ox / kx);
			if (g_56aeba < cur) { cur = g_56aeba; g_56aebe = cur; }
		}
		*dx = cur;
	}
	else
	{
		g_56aebe = 0;
		*dx = 0;
	}
	cur = g_56aec0;
	if (oy > limy || oy < -limy || cur > limy || cur < -limy)
	{
		if (ky <= 0.0f) ky = 0.0001f;
		if (oy > limy)
		{
			g_56aebc = (short)(int)((float)oy / ky);
			if (g_56aebc > cur) { cur = g_56aebc; g_56aec0 = cur; }
		}
		if (oy < -limy)
		{
			g_56aebc = (short)(int)((float)oy / ky);
			if (g_56aebc < cur) { cur = g_56aebc; g_56aec0 = cur; }
		}
		*dy = cur;
	}
	else
	{
		g_56aec0 = 0;
		*dy = 0;
	}
	if (g_56aea8 > 0.0f)									// 0x4d17c0
	{
		g_56aea8 -= dt;
		return;
	}
	CenterMouse();											// 0x4b1d50
	g_56aea8 = 0.2f;
}

// --------------------------------------------------------------------------- construction

C3DCraneBoom::C3DCraneBoom()								// 0x428940
{
	m_chain_pos.w = 1.0f;									// (OMedia3DVector constructor)
	Log("C3DCraneBoom()");
	SetName((char *)"C3DCRANEBOOM");
	C3DAnimated::InitObject();								// 0x41b0a0 (it has no InitObject of its own)
	C3DObject::SetObjectID('3CRB');							// 0x49ab00
	v52(0);
	v46(0);
	v42(0);
	v40(0);
	v44(0);
	m_db = GetDatabase("objects.omt");						// 0x4a4180 (element +0x574)
	f_648 = 0;												// no animations
	f_6d0 = 2;												// (neither morph nor Granny)
	C3DObject::set_shape(Load3DShape(m_db, 6));				// 0x4b2c80, 0x49ab10
	MakeShapeUnLit(get_shape());							// 0x4b2520
	v67(15.0f);												// radius
	m_reach = 400.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	m_chain = new C3DChain;									// 0x92c bytes
	m_chain->v107(0);										// CGameObject +0x1ac
	m_chain->ShowAgain();									// element +0xd8
	m_claw = new C3DCraneClaw;								// 0x93c bytes
	m_claw->v107(0);
	m_claw->m_boom = this;
	m_drop = -1300.0f;
	m_up = 1;
	m_grab_time = 0.5f;
	m_driver = NULL;
	m_channel = -1;
	// (then a call to the empty 0x463890)
}

C3DCraneBoom::~C3DCraneBoom()								// 0x428bf0
{
}

namespace neg {
CGameObject *New_C3DCraneBoom(void) { return new C3DCraneBoom; }		// '3CRB' (0x998 bytes)
}

// --------------------------------------------------------------------------- slots

void C3DCraneBoom::v260(void)								// 0x428c40
{
	C3DPlayerObject::v260();								// 0x464ed0
	const OMedia3DVector q = v224(0, 0, m_reach);			// +0x380: the trolley
	m_chain->v196(q.x, q.y, q.z, q.w);						// +0x310
	float p[4];
	m_chain->v195(p);										// +0x30c
	m_chain_pos = OMedia3DVector(p[0], p[1], p[2], p[3]);
}

void C3DCraneBoom::Process(float dt)						// 0x428cd0
{
	C3DAnimated::Process(dt);								// 0x41c120
	float r[4];
	if (m_driver)
	{
		// Jimmy rides in the cab
		const OMedia3DVector q = v224(0, 150.0f, 0);
		m_driver->v196(q.x, q.y, q.z, q.w);					// +0x310
		v201(r);											// +0x324
		m_driver->v202(r[0], r[1], r[2], r[3]);				// +0x328
		C3DAnimated *j = dynamic_cast<C3DAnimated *>(g_jimmy);	// (g_jimmy - 0xc0), not checked
		if (j && strcasecmp(j->GetAnim(), "DRIVE") != 0)	// element +0xf4; 0x4c4520
			j->ev86((char *)"DRIVE", NULL, NULL, NULL, 1, 0.2f);	// element +0x158
	}
	if (m_claw)
	{
		const OMedia3DVector q = v224(0, m_drop, m_reach);
		m_claw->v196(q.x, q.y, q.z, q.w);
		v201(r);
		m_claw->v202(r[0], r[1], r[2], r[3]);
	}
	if (m_chain)
	{
		const OMedia3DVector q = v224(0, 10.0f, m_reach);
		m_chain->v196(q.x, q.y, q.z, q.w);
	}
}

void C3DCraneBoom::v243(float dt)							// 0x428eb0
{
	C3DPlayerObject::v243(dt);								// 0x464ee0 (empty)
	if (m_channel == -1)
	{
		m_channel = PlayGameSound(-1, std::string("cranerun"), true);	// 0x48f420
		if (m_claw) m_claw->m_active = 1;
	}
	m_grab_time += dt;
	if (PlayerKeyAction() && m_grab_time >= 0.5)			// 0x465750; 0x4cb210 (double)
	{
		PlayGameSound(-1, std::string("cranegrab"), false);
		m_up = !m_up;
		m_grab_time = 0;
	}
	short mx = 0, my = 0;
	MouseSteer(&mx, &my, dt, 8.0f, 6.0f, 10, 10);			// 0x4a39f0
	if (PlayerKeyLeft() || mx > 0)							// 0x4659f0
		AddRotation(0, dt * 20.0f, 0);						// +0x330; 0x4caa40
	else if (PlayerKeyRight() || mx < 0)					// 0x465a60
		AddRotation(0, dt * -20.0f, 0);						// 0x4d65cc
	const float my_speed = (1.0f / dt) * (float)my;
	if (PlayerKeyGo() || my_speed > 40.0f)					// 0x465910; 0x4d65c8
	{
		m_reach += dt * 230.0f;								// 0x4d65c4
		if (m_reach > 1290.0f) m_reach = 1290.0f;			// 0x4d65c0
	}
	if (PlayerKeyBack() || my_speed < -40.0f)				// 0x465980; 0x4d65bc
	{
		m_reach -= dt * 230.0f;
		if (m_reach < 300.0f) m_reach = 300.0f;				// 0x4caa7c
	}
	if (m_up == 1)
	{
		m_drop += dt * 200.0f;								// 0x4cb21c
		if (!(m_drop < -1300.0f)) m_drop = -1300.0f;		// 0x4d65b4
	}
	else
	{
		m_drop -= dt * 200.0f;
		if (m_drop < -1750.0f)								// 0x4d65b8: at the bottom
		{
			m_drop = -1750.0f;
			if (m_claw && m_claw->m_held) m_claw->Drop(m_claw->m_held);	// 0x429980
			m_up = 1;
		}
	}
	// the cable hangs towards the trolley (50 / s; the original keeps the step in dt's slot)
	float c[4];
	m_chain->v195(c);										// +0x30c (twice)
	m_chain->v195(c);
	const float step = dt * 50.0f;							// 0x4ca4b0
	if (m_chain_pos.x <= c[0]) m_chain_pos.x = step + m_chain_pos.x;
	else m_chain_pos.x = m_chain_pos.x - step;
	m_chain->v195(c);
	if (m_chain_pos.z <= c[2]) m_chain_pos.z = step + m_chain_pos.z;
	else m_chain_pos.z = m_chain_pos.z - step;
	// the viewport: behind and above the boom, looking along it
	const OMedia3DVector eye = v224(-1000.0f, 1000.0f, -400.0f);	// +0x380
	const OMedia3DVector at = v224(0, -1000.0f, 900.0f);
	g_viewport->place(eye.x, eye.y, eye.z);					// viewport +0x44..+0x4c
	OMedia3DVector d(at.x - eye.x, at.y - eye.y, at.z - eye.z, 0);
	omt_Angle ax, ay;
	d.angles(ax, ay);										// OMedia3DVector::angles
	short wy = (short)((ay - g_viewport->get_angley()) & 0x3fff);
	if (wy > 0x2000) wy = (short)(wy - 0x4000);
	g_viewport->set_anglex((omt_Angle)(g_viewport->get_anglex() + (short)(ax - g_viewport->get_anglex())));	// +0x50
	g_viewport->set_angley((omt_Angle)(g_viewport->get_angley() + wy));									// +0x52
}

void C3DCraneBoom::v277(void)								// 0x4292e0
{
	C3DAnimated::v277();									// 0x41ce80
	if (m_channel != -1) StopGameSound(m_channel, false);	// 0x48f0d0
}

void C3DCraneBoom::v278(void)								// 0x429300
{
	C3DAnimated::v278();									// 0x41cea0
	if (m_channel != -1) m_channel = PlayGameSound(-1, std::string("cranerun"), true);
}
