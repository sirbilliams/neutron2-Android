/*  C3DRope and C3DHookSlide - see C3DRope.h. Reconstructed from Neutron2.exe 0x46cd90..0x46e040,
    0x43f3a0..0x43f750 (and the shared 0x43d610, 0x45ce70). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DRope.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"					// PlayerKey*, JimmyActivateMenuItem
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DShape.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string>
#include <strings.h>

using namespace neg;

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// =========================================================================== C3DRope

C3DRope::C3DRope()											// 0x46cd90
{
	m_anchor.w = 1.0f;										// (OMedia3DPoint / OMedia3DVector constructors)
	m_grab_pos.w = 1.0f;
	m_dir.w = 0;
	SetName((char *)"C3DROPE");
	Log("C3DRope()");
	f_649 = 0;												// ev69 loads the animation later
	Log("C3DRope::InitObject()");							// 0x46d0e0 inlined
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("CanSwing", &m_can_swing, PROP_LONG, 0);
	RegisterProperty("RopeScale", &m_scale, PROP_FLOAT, 0);
	C3DObject::SetObjectID('3ROP');							// 0x49ab00
	v107(1);
	v52(1);
	v42(0);
	v40(0);
	v34(1);
	v94(2);
	v46(0);
	v92(270.0f, 0, 0);										// turned to hang down
	m_attached = 0;
	m_regrab = 0;
	m_can_swing = 1;
	f_6d0 = 2;
	C3DObject::InitPhysics();								// 0x49acb0
	m_scale = 1.0f;
	m_sliding = 0;
	m_anchor.x = 0;
	m_anchor.y = 0;
	m_anchor.z = 0;
	m_grab_pos.x = 0;
	m_grab_pos.y = 0;
	m_grab_pos.z = 0;
	m_dir.x = 0;
	m_dir.y = 0;
	m_dir.z = 0;
	m_rider = NULL;
	m_hook = NULL;
	m_hook_dist = 0;
	m_hook_speed = 0;
	m_hook_moving = 0;
	m_swing_channel = -1;
	m_freq = 0;												// port: the original never sets it
	m_slide_channel = -1;
	m_released = 0;
}

C3DRope::~C3DRope()											// 0x46d000
{
}

namespace neg {
CGameObject *New_C3DRope(void) { return new C3DRope; }					// '3ROP' (0x990 bytes)
}

// --------------------------------------------------------------------------- element family

void C3DRope::ev69(void)									// 0x46d050
{
	if (f_649) return;
	f_649 = 1;
	if (!(m_scale > 1.0f)) return;							// 0x4caa6c
	CreateAnim((char *)"ROPE", (char *)"rope01.ase");		// +0xe0
	CreateTexture((char *)"rope.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	f_590->scale(1.0f, 1.0f, m_scale);						// shape vtable +0x34
	ev58((char *)"ROPE", 1);								// +0xe8
}

void C3DRope::ev94(CGameObject *o)							// 0x46d4d0
{
	m_dir = v226(0, 0, 130.0f);								// +0x388
	if (!o->NameContains((char *)"C3DJIMMY")) return;		// +0x18
	m_sliding = 1;
	o->v111(0);												// +0x1bc
	float a[4], b[4];
	o->v195(a);												// +0x30c
	v88(b);													// +0x160: our start position
	const float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
	const float dist = sqrtf(dx * dx + dy * dy + dz * dz);
	v88(b);
	v226(0, 0, dist);										// (the result is not used)
	o->v176(0, 0, 0);										// +0x2c0 velocity
	o->v157(0, 0, 0);										// +0x274
	m_rider = o;
	C3DJimmy *j = dynamic_cast<C3DJimmy *>(o);				// (o - 0xc0)
	if (j)													// port: checked
	{
		j->ev116();											// element +0x1d0: footsteps off
		j->f_4d7c = 1;										// no physics
	}
	float r[4], r2[4];
	v201(r);												// +0x324 (twice)
	v201(r2);
	m_rider->v218(0, r2[1], r[2]);							// +0x368
	if (j) j->ev58((char *)"SWING", 1);						// element +0xe8
	if (m_slide_channel == -1)
		m_slide_channel = PlayGameSound(-1, std::string("GRAPLEUP"), true);	// 0x48f420
}

void C3DRope::ev95(CGameObject *o)							// 0x46d6b0
{
	if (!o) return;											// port: the original would crash
	m_sliding = 0;
	o->v111(1);												// +0x1bc
	g_5461cc = 0;
	float v[4];
	o->v174(v);												// +0x2b8 velocity
	o->v175(v[0], 0, v[2], v[3]);							// +0x2bc: no vertical speed
	C3DJimmy *j = dynamic_cast<C3DJimmy *>(o);				// (o - 0xc0)
	if (j)													// port: checked
	{
		j->f_4d7c = 0;
		j->ev58((char *)"FALL", 1);							// element +0xe8
	}
	if (g_jimmy) JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 0, 0.0f);	// Jimmy element +0x228
	if (m_swing_channel != -1)
	{
		StopGameSound(m_swing_channel, false);				// 0x48f0d0
		m_swing_channel = -1;
	}
	if (m_slide_channel != -1)
	{
		StopGameSound(m_slide_channel, false);
		m_slide_channel = -1;
	}
}

// --------------------------------------------------------------------------- CGameObject family

void C3DRope::InitObject(void)								// 0x46d0e0
{
	Log("C3DRope::InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("CanSwing", &m_can_swing, PROP_LONG, 0);
	RegisterProperty("RopeScale", &m_scale, PROP_FLOAT, 0);
}

void C3DRope::v260(void)									// 0x46d140
{
	C3DAnimated::v260();									// 0x41cec0
	if (m_scale == 1.0f)
	{
		f_648 = 0;											// no animations: the OMT shape
		f_6d0 = 2;
		m_db = GetDatabase("objects.omt");					// 0x4a4180
		set_shape(Load3DShape(m_db, 63));					// 0x4b2c80, element +0xac
		MakeShapeUnLit(get_shape());						// 0x4b2520, element +0xb0
		f_648 = 0;
	}
	else
	{
		f_6d0 = 0;											// morph
		InitAnim();											// element +0x110
	}
	if (m_hook || m_can_swing) return;
	// a zip line: its hook, at the far end
	C3DHookSlide *h = new C3DHookSlide;						// 0x94c bytes
	m_hook = h;
	h->m_rope = this;
	h->f_93c = m_rider;
	float p[4];
	v195(p);												// +0x30c
	const OMedia3DVector d = v226(0, 0, m_scale * 600.0f);	// +0x388; 0x4d0ca0
	unsigned int w;
	memcpy(&w, &p[3], 4);									// (the generator typed the w as an int)
	m_hook->v89(d.x + p[0], d.y + p[1], d.z + p[2], w);		// +0x164: its start position
	m_hook_dist = m_scale * 600.0f;
	m_hook_speed = 0;
	v52(0);													// +0xd0
}

void C3DRope::AddTouching(CGameObject *other)				// 0x46d360
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (!other->NameContains((char *)"C3DJIMMY") || !m_can_swing) return;
	C3DJimmy *j = dynamic_cast<C3DJimmy *>(other);			// (other - 0xc0)
	if (!j) return;											// port: checked
	if (j->f_b00 || j->f_aed) return;						// already hanging on / camera frozen
	float p[4];
	other->v195(p);											// +0x30c
	m_grab_pos = OMedia3DVector(p[0], p[1], p[2], p[3]);
	v88(p);													// +0x160: our start position
	m_anchor = OMedia3DPoint(p[0], p[1], p[2], p[3]);
	j->ev143(m_anchor, m_grab_pos, m_can_swing, m_scale * 600.0f);	// element +0x23c
	v136(0);												// +0x220
	m_attached = 1;
	m_swing_channel = PlayGameSound(-1, std::string("ropeswing"), false);	// 0x48f420
}

unsigned char C3DRope::v231(CGameObject *other)				// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DRope::Process(float dt)								// 0x46d880
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	m_hook_moving = 0;
	const float len = m_scale * 600.0f;						// 0x4d0ca0
	if (m_can_swing == 0)
	{
		// ---- the zip line
		if (m_sliding)
		{
			if (PlayerKeyBack()) m_hook_speed = dt * 8.0f + m_hook_speed;	// 0x465980; 0x4f12e4
			else m_hook_speed = m_hook_speed - dt * (PlayerKeyGo() ? 6.0f : 4.0f);	// 0x465910; 0x4d7f90 / 0x4dce64
			if (m_hook_dist < 150.0f || PlayerKeyJump())		// 0x4cb1e4; 0x4658a0
				ev95(m_rider);								// element +0x17c
			int f = (int)((float)m_freq - (m_hook_speed * 350.0f + 4000.0f));	// 0x4d783c, 0x4d2ca8; _ftol
			if (f <= 7000) f = 7000;
			if (m_slide_channel >= 0 && m_slide_channel < kSoundChannels)	// port: the original indexes with -1 after ev95
				g_chan_freq[m_slide_channel] = f;			// 0x6f73d4: the pitch
			C3DPlayer *rp = dynamic_cast<C3DPlayer *>(m_rider);	// (m_rider - 0xc0)
			if (rp)											// port: checked
			{
				const OMedia3DVector look(rp->f_a7c.x, rp->f_a7c.y, rp->f_a7c.z, rp->f_a7c.w);	// 0x411d70
				rp->ev123(dt, OMedia3DVector(0, 300.0f, -250.0f, 1.0f), look, 1.2f);	// element +0x1ec
			}
			if (len + 100.0f < m_hook_dist) ev95(m_rider);	// 0x4d2cb0: past the far end
		}
		else if (len > m_hook_dist)
		{
			m_hook_speed = 10.0f;							// back out to the end
			m_hook_moving = 1;
		}
		else
		{
			m_hook_speed = 0;
			m_hook_moving = 0;
			if (len + 100.0f < m_hook_dist)
			{
				ev95(m_rider);
				m_hook_dist = len;
			}
		}
		m_hook_dist = m_hook_speed + m_hook_dist;			// (per frame, not per second)
		OMedia3DVector q = v226(0, 0, m_hook_dist);			// +0x388
		float p[4];
		v195(p);											// +0x30c
		if (m_hook) m_hook->v196(p[0] + q.x, p[1] + q.y, p[2] + q.z, p[3]);	// +0x310
		if (!m_sliding) return;
		q = v226(0, 64.0f, m_hook_dist);					// the rider hangs below the hook
		v195(p);
		if (m_rider) m_rider->v196(p[0] + q.x, p[1] + q.y, p[2] + q.z, p[3]);
		return;
	}
	// ---- the swinging rope
	if (m_attached)
	{
		if (g_camera)
		{
			v196(m_anchor.x, m_anchor.y, m_anchor.z, m_anchor.w);	// +0x310
			float c[4];
			g_camera->v195(c);
			OMedia3DVector d(c[0] - m_anchor.x, c[1] - m_anchor.y, c[2] - m_anchor.z, 0);
			omt_Angle ax, ay;
			d.angles(ax, ay);								// OMedia3DVector::angles
			v203((float)ax * kAngleToDeg, (float)ay * kAngleToDeg, 0);	// +0x32c: pointing at him
		}
		if (!m_released) m_rider = FindObjectByTag("JIM1");
		if (!PlayerKeyJump())								// 0x4658a0
		{
			m_released = 1;
			return;
		}
		if (m_released != 1) return;
		if (PlayerKeyAction()) return;						// 0x465750
		m_attached = 0;
		m_regrab = 2.0f;
		C3DJimmy *j = dynamic_cast<C3DJimmy *>(m_rider);	// (m_rider - 0xc0)
		if (!j || !j->f_4db8) return;						// port: checked
		j->ev142();											// element +0x238: let go
		j->ActivateMenuItem(this, this, 0, 0.0f);			// element +0x228
		return;
	}
	m_released = 0;
	float r[4], r2[4];
	v201(r);												// +0x324 (not used)
	v91(r2);												// +0x16c: the start rotation
	v202(r2[0], r2[1], r2[2], r2[3]);						// +0x328
	if (m_regrab > 0.0f)
	{
		m_regrab -= dt;
		if (m_regrab > 0.0f) return;
		v136(1);											// +0x220
		m_regrab = 0;
		return;
	}
	if (!g_camera) return;
	C3DPlayer *cp = dynamic_cast<C3DPlayer *>(g_camera);	// (g_camera - 0xc0)
	if (!cp || cp->f_b00) return;							// port: checked
	if (!IsTouching(g_camera)) return;						// +0x3c
	float p[4];
	g_camera->v195(p);
	m_grab_pos = OMedia3DVector(p[0], p[1], p[2], p[3]);
	v88(p);
	m_anchor = OMedia3DPoint(p[0], p[1], p[2], p[3]);
	C3DJimmy *j = dynamic_cast<C3DJimmy *>(g_camera);
	if (j) j->ev143(m_anchor, m_grab_pos, m_can_swing, len);	// element +0x23c; port: checked
	v136(0);
	m_attached = 1;
}

void C3DRope::WriteState(OMediaStreamOperators &s)			// 0x46d800
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	bool attached = m_attached != 0;
	s << attached;
}

void C3DRope::ReadState(OMediaStreamOperators &s)			// 0x46d830
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	bool attached;
	s >> attached;
	m_attached = attached;
	if (!m_attached) return;
	CGameObject *jim = FindObjectByTag("JIM1");
	if (jim) AddTouching(jim);								// +0x40
}

// =========================================================================== C3DHookSlide

C3DHookSlide::C3DHookSlide()								// 0x43f3a0
{
	SetName((char *)"C3DHOOKSLIDE");
	Log("C3DHookSlide()");
	f_649 = 0;												// ev69 loads the animation later
	Log("C3DHookSlide::InitObject()");						// 0x43f600 inlined
	C3DAnimated::InitObject();								// 0x41b0a0
	C3DObject::SetObjectID('3HOS');							// 0x49ab00
	v107(0);
	v52(1);
	v42(0);
	v40(0);
	v34(1);
	v46(0);
	f_924 = 0;
	C3DObject::InitPhysics();								// 0x49acb0
	v67(200.0f);											// radius
	f_928 = 1.0f;
	f_92c[0] = 0;
	f_92c[1] = 0;
	f_92c[2] = 0;
	f_93c = NULL;
	m_rope = NULL;
}

C3DHookSlide::~C3DHookSlide()								// 0x43f550
{
}

void C3DHookSlide::ev69(void)								// 0x43f5a0
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"SLIDE", (char *)"hookslide.ase");	// +0xe0
	CreateTexture((char *)"hookslide.png", 0);				// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"SLIDE", 1);								// +0xe8
}

void C3DHookSlide::InitObject(void)							// 0x43f600
{
	Log("C3DHookSlide::InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
}

void C3DHookSlide::v260(void)								// 0x43f630
{
	C3DAnimated::v260();									// 0x41cec0
	InitAnim();												// element +0x110
	float r[4], r2[4];
	m_rope->v201(r);										// +0x324 (twice)
	m_rope->v201(r2);
	v92(0, r2[1], r[2]);									// +0x170: turned like the rope
}

void C3DHookSlide::AddTouching(CGameObject *other)			// 0x43f6a0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (!other->NameContains((char *)"C3DJIMMY") || !m_rope) return;
	C3DRope *r = dynamic_cast<C3DRope *>(m_rope);			// (m_rope - 0xc0)
	if (r) r->ev94(other);									// element +0x178
}

unsigned char C3DHookSlide::v231(CGameObject *other)		// 0x45ce70 (shared code)
{
	if (g_camera == other) return 1;
	return other->m_object_id == '3JIM';
}
