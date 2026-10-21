/*  C3DFan - see C3DFan.h. Reconstructed from Neutron2.exe 0x4314d0..0x431b60. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFan.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include <math.h>

using namespace neg;

C3DFan::C3DFan()											// 0x4314d0
{
	SetName((char *)"C3DFAN");
	C3DFan::InitObject();									// 0x431680 (called directly)
	C3DObject::SetObjectID('3FAN');							// 0x49ab00
	m_fan_speed = 400.0f;
	m_fan_range = 1500.0f;
	v64(0.05f);
	v66(1000.0f);
	v46(0);
	v52(0);
	m_speed = 0;
	m_target = 0;
	m_fan_on = 1;
	C3DObject::InitPhysics();								// 0x49acb0
}

C3DFan::~C3DFan()											// 0x431630
{
}

namespace neg {
CGameObject *New_C3DFan(void) { return new C3DFan; }					// '3FAN' (0x940 bytes)
}

void C3DFan::InitObject(void)								// 0x431680
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("FanSpeed", &m_fan_speed, PROP_FLOAT, 0);
	RegisterProperty("FanRange", &m_fan_range, PROP_FLOAT, 0);
	RegisterProperty("FanOn", &m_fan_on, PROP_LONG, 0);
	InitAnim();												// element +0x110
	CreateAnim((char *)"DEFAULT", (char *)"fan.ase");		// +0xe0
	CreateTexture((char *)"fan.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	ev58((char *)"DEFAULT", 1);								// +0xe8
}

void C3DFan::ResetObject(void)								// 0x431a80
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	if (m_fan_on)
	{
		m_target = m_fan_speed;
		return;
	}
	m_target = 0;
}

void C3DFan::Process(float dt)								// 0x431750
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	AddRotation(0, 0, (float)((double)(m_speed * dt) * 0.35));	// +0x330; 0x4da470 (double)
	if (m_speed < m_target)
	{
		m_speed += dt * 130.0f;								// 0x4da46c
		if (m_speed > m_target) m_speed = m_target;
	}
	if (m_speed > m_target)
	{
		m_speed -= dt * 200.0f;								// 0x4cb21c
		if (m_speed < m_target) m_speed = m_target;
	}
	if (!g_camera) return;
	float p[4], c[4];
	v195(p);												// +0x30c
	g_camera->v195(c);
	OMedia3DVector d(p[0] - c[0], p[1] - c[1], p[2] - c[2], 0);
	const float dist = sqrtf(d.y * d.y + d.z * d.z + d.x * d.x);
	if (!(dist < m_fan_range)) return;
	if (!(m_fan_range > 0.0f)) return;						// 0x4caa44
	OMedia3DVector n = v226(0, 0, 1.0f);					// +0x384 (0x4ada90: v226 of the vector (0, 0, 1, 0))
	const float id = 1.0f / d.quick_magnitude();			// 0x4caa6c
	d.x *= id;
	d.y *= id;
	d.z *= id;
	const float in = 1.0f / n.quick_magnitude();
	n.x *= in;
	n.y *= in;
	n.z *= in;
	float f = dist / m_fan_range + (1.0f - (d.z * n.z + d.y * n.y + n.x * d.x));
	if (f < -1.0f) f = -1.0f;								// 0x4cb1dc
	else if (f > 1.0f) f = 1.0f;
	OMedia3DVector q = v224(0, 0, m_speed);					// +0x380: the point `speed` ahead
	v195(p);
	const float k = 1.0f - f;
	g_camera->v178(k * (p[0] - q.x), k * (p[1] - q.y), k * (p[2] - q.z));	// +0x2c8 velocity +=
}

void C3DFan::v268(int state)								// 0x431ab0
{
	if (state == 0)
	{
		m_fan_on = 0;
		m_target = 0;
		return;
	}
	m_fan_on = 1;
	m_target = m_fan_speed;
}
