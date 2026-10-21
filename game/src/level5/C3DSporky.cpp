/*  C3DSporky - see C3DSporky.h. Reconstructed from Neutron2.exe 0x4719e0..0x4721f0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSporky.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaStreamOperators.h"
#include <list>
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

C3DSporky::C3DSporky()										// 0x4719e0
{
	SetName((char *)"C3DSPORKY");
	Log("C3DSporky()");
	f_649 = 0;												// animations not loaded
	Log("InitObject()");									// 0x471d10 inlined
	C3DAI::InitObject();									// 0x4110e0
	RegisterProperty("DeliverFood", &m_deliver_food, PROP_LONG, 0);
	RegisterProperty("RespawnTime", &m_respawn_time, PROP_FLOAT, 0);
	InitAnim();												// +0x110
	C3DObject::SetObjectID('3SPK');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 200.0f;											// speed
	f_96c = 1;												// state: wait
	f_c08 = 1;												// AIState
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"RUN");						// 0x411210
	C3DAI::ev106(f_a9c, (char *)"STOP");
	C3DAI::ev106(f_b14, (char *)"STOP");
	m_respawn_time = 30.0f;
	m_bag_time = 0;
	m_deliver_food = 1;
}

C3DSporky::~C3DSporky()										// 0x471bd0
{
}

namespace neg {
CGameObject *New_C3DSporky(void) { return new C3DSporky; }	// '3SPK' (0x1b64 bytes)
}

void C3DSporky::InitObject(void)							// 0x471d10
{
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0
	RegisterProperty("DeliverFood", &m_deliver_food, PROP_LONG, 0);	// +0x3fc
	RegisterProperty("RespawnTime", &m_respawn_time, PROP_FLOAT, 0);
	InitAnim();												// +0x110
}

void C3DSporky::ev69(void)									// 0x471c20
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"TALK", (char *)"sporkytalk.ase");	// +0xe0
	CreateAnim((char *)"BAGIN", (char *)"sporkybagin.ase");
	CreateAnim((char *)"BAGOUT", (char *)"sporkybagout.ase");
	CreateAnim((char *)"BAGLEAVE", (char *)"sporkybagleave.ase");
	CreateAnim((char *)"RUN", (char *)"sporkyrun.ase");
	CreateAnim((char *)"STOP", (char *)"sporkystop.ase");
	CreateTexture((char *)"sporky.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"STOP", 1);								// +0xe8 looped
	v67(150.0f);											// radius
	v64(0.5f);
}

void C3DSporky::ev67(void)									// 0x4720a0
{
	if (strcasecmp(ev66(), "BAGIN") == 0)					// +0x108, 0x4c4520
	{
		HideAgain();										// +0xdc
		return;
	}
	if (strcasecmp(ev66(), "BAGOUT") == 0)
		ev58((char *)"BAGLEAVE", 1);
}

void C3DSporky::v260(void)									// 0x471d80
{
	C3DAI::v260();											// 0x4143a0
	if (m_deliver_food == 1)
	{
		ev58((char *)"BAGLEAVE", 1);						// element +0xe8
		v46(0);												// +0xb8
		v34(1);												// +0x88
		v42(0);												// +0xa8
		v40(0);												// +0xa0
		switch (g_difficulty)								// 0x53b5b0
		{
			case 0: v67(700.0f); break;						// radius
			case 1: v67(500.0f); break;
			case 2: v67(300.0f); break;
		}
		return;
	}
	if (m_deliver_food == 2)
	{
		v52(1);												// +0xd0
		f_96c = 3;											// state: patrol only
		f_c08 = 3;
		return;
	}
	v52(0);
}

void C3DSporky::ResetObject(void)							// 0x471e60 (does not call the base's)
{
	float s[4];
	v88(s);													// +0x160 start point
	v196(s[0], s[1], s[2], s[3]);							// +0x310 position
	float p[4];
	v195(p);												// +0x30c
	const float z = p[2];
	v195(p);
	const float y = p[1];
	v195(p);
	place(p[0], y, z);										// element +0x44..
	f_b8c = 0;												// no destination
}

void C3DSporky::AddTouching(CGameObject *other)				// 0x471f00
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (m_deliver_food == 0 || other != g_camera || m_deliver_food != 1 || m_bag_time != 0.0f)
		return;
	CGameObject *o = FindObjectByTag("JIM1");				// 0x4aefb0
	if (!o) return;
	PlayGameSound(-1, std::string("jimmystore09"), false);	// 0x410850, 0x48f420
	C3DJimmy *j = dynamic_cast<C3DJimmy *>(o);				// (o - 0xc0)
	if (j)													// port: checked (JIM1 is Jimmy)
	{
		j->ev156(10.0f, 0, 1.0f, 0);						// element +0x270: green sparkle
		j->f_4d94 += 20;
		if (j->f_4d94 > 100) j->f_4d94 = 100;
	}
	m_bag_time = m_respawn_time;
	ev58((char *)"BAGIN", 0);								// element +0xe8
	v132(0);												// +0x210
}

void C3DSporky::Process(float dt)							// 0x472000
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (m_deliver_food != 1)
	{
		C3DAI::Process(dt);									// 0x411240
		return;
	}
	C3DAnimated::Process(dt);								// 0x41c120 (no AI)
	if (m_bag_time > 0.0f)
	{
		m_bag_time = m_bag_time - dt;
		if (m_bag_time <= 0.0f)
		{
			m_bag_time = 0;
			ev58((char *)"BAGOUT", 0);						// element +0xe8
			ShowAgain();									// element +0xd8
		}
	}
}

void C3DSporky::WriteState(OMediaStreamOperators &s)		// 0x472150
{
	C3DAI::WriteState(s);									// 0x415760
	bool visible = f_589 != 0;
	s << visible;
	if (visible) ShowAgain();								// (sic: also when saving)
	else HideAgain();
}

void C3DSporky::ReadState(OMediaStreamOperators &s)			// 0x472100
{
	C3DAI::ReadState(s);									// 0x4155e0
	bool visible;
	s >> visible;
	if (visible) ShowAgain();
	else HideAgain();
}
