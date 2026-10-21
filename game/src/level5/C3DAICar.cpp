/*  C3DAICar - see C3DAICar.h. Reconstructed from Neutron2.exe 0x4159b0..0x415de0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DAICar.h"
#include "../C3DPlayer.h"
#include "../Sound.h"
#pragma GCC diagnostic pop
#include <string>

using namespace neg;

C3DAICar::C3DAICar()										// 0x4159b0
{
	f_968 = 400.0f;											// speed
	f_96c = 3;												// state: patrol only
	f_c08 = 3;												// AIState
	CGameObject::v46(1);									// 0x4abf10 (direct call)
	f_b9c.y = 2.0f;											// turn rates
	f_b9c.z = 2.0f;
	f_b9c.x = 1.0f;
	m_stop_time = 0;
	m_hold_pos.x = 0;
	m_hold_pos.y = 0;
	m_hold_pos.z = 0;
	m_hold_pos.w = 1.0f;
	m_held = 0;
	f_c48 = 0;
	f_1b68 = -1;
}

C3DAICar::~C3DAICar()										// 0x415b10
{
}

void C3DAICar::Process(float dt)							// 0x415b60
{
	C3DAI::Process(dt);										// 0x411240
	if (m_held)
		v196(m_hold_pos.x, m_hold_pos.y, m_hold_pos.z, m_hold_pos.w);	// +0x310 position
	m_stop_time = m_stop_time - dt;
	if (m_stop_time < 0.0f)									// 0x4caa44
		f_968 = 400.0f;										// (every frame once the time is up)
}

void C3DAICar::AddTouching(CGameObject *other)				// 0x415be0
{
	C3DAI::AddTouching(other);								// 0x415150
	if (!other->NameContains((char *)"C3DJIMMY") &&			// +0x18
		!other->NameContains((char *)"C3DJEEP") &&
		!other->NameContains((char *)"C3DGODDARD") &&
		!other->NameContains((char *)"C3DROCKETSHIP"))
		return;
	float o[4], p[4];
	other->v195(o);											// +0x30c
	v195(p);
	if (o[1] > p[1] + 480.0f)								// 0x4cb960: on top of us
	{
		other->v175(700.0f, 700.0f, 200.0f, 0);				// +0x2bc thrown off
		if (other->NameContains((char *)"C3DJIMMY"))
		{
			C3DPlayer *j = dynamic_cast<C3DPlayer *>(other);	// (other - 0xc0)
			if (j) j->f_a8c = 0;							// port: checked (not in jump any more)
		}
	}
	PlayGameSound(-1, std::string("horn"), false);			// 0x410850, 0x48f420
	f_968 = 0.0001f;										// stop (0x38d1b717)
	m_stop_time = 8.0f;
	m_held = 1;
	v195(p);
	m_hold_pos.x = p[0];
	m_hold_pos.y = p[1];
	m_hold_pos.z = p[2];
	m_hold_pos.w = p[3];
}

void C3DAICar::RemoveTouching(CGameObject *other)			// 0x415d30
{
	C3DAI::RemoveTouching(other);							// 0x415110
	m_held = 0;
}

void C3DAICar::v277(void)									// 0x415d50
{
	ev79(1);												// +0x13c
}

void C3DAICar::v278(void)									// 0x415d60
{
	ev79(0);
}
