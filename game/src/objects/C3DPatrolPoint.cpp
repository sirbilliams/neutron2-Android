/*  C3DPatrolPoint - see C3DPatrolPoint.h. Reconstructed from Neutron2.exe 0x45c4c0..0x45cc50. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DPatrolPoint.h"
#include "../Physics.h"
#include "../Globals.h"
#include "../Game.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#include "OMedia3DVector.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DPatrolPoint::C3DPatrolPoint()							// 0x45c4c0
{
	SetName((char *)"C3DPATROLPOINT");
	Log("C3DPatrolPoint()");
	C3DPatrolPoint::InitObject();							// (called directly, as in the original)
	SetObjectID('3PAT');
	m_sprite_index = 6;
	m_sprite_size = 50;
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_call_object_tag, "none");
	strcpy(m_activate_anim, "none");
	strcpy(m_wait_anim, "none");
	strcpy(m_sound_name, "none");
	strcpy(m_attach_obj, "none");
	strcpy(m_next_patrol_point, "none");
	m_patrol_speed = -1.0f;
	m_wait_time = 0;
	v67(250.0f);
	v46(0);
	v42(0);
	v40(0);
	v52(0);
	f_008 = 1;												// element +0xd0
	C3DSprite::InitPhysics();
	m_patrol_type = 0;
	f_968 = 1;
	m_group_id = 0;
	for (int i = 0; i < 20; i++) { m_visible_points[i].point = NULL; m_visible_points[i].dist = 0; }
}

C3DPatrolPoint::~C3DPatrolPoint() {}						// 0x45c780

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DPatrolPoint::InitObject(void)						// 0x45c7c0
{
	C3DSprite::InitObject();
	RegisterProperty("CallObjectTag",   m_call_object_tag,   PROP_ALPHA, 0);
	RegisterProperty("ActivateAnim",    m_activate_anim,     PROP_ALPHA, 0);
	RegisterProperty("SoundName",       m_sound_name,        PROP_ALPHA, 0);
	RegisterProperty("NextPatrolPoint", m_next_patrol_point, PROP_ALPHA, 0);
	RegisterProperty("WaitAnim",        m_wait_anim,         PROP_ALPHA, 0);
	RegisterProperty("WaitTime",        &m_wait_time,        PROP_FLOAT, 0);
	RegisterProperty("PatrolType",      &m_patrol_type,      PROP_LONG,  0);
	RegisterProperty("GroupID",         &m_group_id,         PROP_LONG,  0);
	RegisterProperty("PatrolSpeed",     &m_patrol_speed,     PROP_FLOAT, 0);
	RegisterProperty("AttachObj",       m_attach_obj,        PROP_ALPHA, 0);
}

void C3DPatrolPoint::Process(float dt)						// 0x464ee0 (CGameObject's empty slot)
{
	(void)dt;
}

// 0x45c9b0: in the editor, holding A writes the tags of the visible points under this one.
void C3DPatrolPoint::v242(float t)
{
	C3DSpriteType::v242(t);									// 0x473570
	if (!g_keys['A']) return;								// 0x56ade1
	OMedia3DVector pos;
	v195(pos.xyzw());										// +0x30c
	float sx, sy;
	if (!v281(g_viewport, pos.xyzw(), &sx, &sy)) return;	// +0x464: on screen
	for (int i = 0, line = 0; line < 300; i++, line += 15)
	{
		CGameObject *p = m_visible_points[i].point;
		if (!p) continue;
		// QUIRK: the line goes up as the point goes down the screen (line - sy), as shipped.
		const int y = (int)((float)line - sy + 260.0f);	// 0x4ef570
		const int x = (int)(sx + 320.0f);					// 0x4d3420
		DrawText((short)x, (short)y, 'SMAL', "%s", p->m_tag);	// 0x4a14f0 (a copy of DrawText), 0x532e00
	}
}

void C3DPatrolPoint::v260(void)								// 0x45c8d0
{
	C3DSpriteType::v260();									// 0x473650
	if (m_patrol_type != 1) return;
	FindVisiblePoints(m_group_id);
	if (!m_visible_points[0].point)
		set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 0.1f));	// +0x90.. (r, g, b, a) = 1, 1, 0.1, 1: yellow
	else
		set_diffuse(OMediaFARGBColor(1.0f, 0.1f, 1.0f, 0.1f));	// 0.1, 1, 0.1, 1: green
}

void C3DPatrolPoint::v266(void)								// 0x45c990
{
	// (debug log)
	FindVisiblePoints(m_group_id);
}

// 0x45ca90: list the other type-1 patrol points of `group` that can be seen from here (no wall
// on the line between the two), with their distance, at most 20, in object-list order.
void C3DPatrolPoint::FindVisiblePoints(int group)
{
	for (int i = 0; i < 20; i++) { m_visible_points[i].point = NULL; m_visible_points[i].dist = 0; }
	if (m_patrol_type != 1) return;
	int n = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		CGameObject *o = *it;
		if (!o || o == static_cast<CGameObject *>(this)) continue;
		if (o->m_object_id != '3PAT') continue;
		C3DPatrolPoint *p = static_cast<C3DPatrolPoint *>(o);
		if (p->m_patrol_type != 1 || p->m_group_id != group) continue;
		OMedia3DVector a, b;
		o->v195(a.xyzw());
		v195(b.xyzw());
		if (PhysicsLineBlocked(b, a, false) || n >= 20) continue;
		m_visible_points[n].point = o;
		OMedia3DVector c, d;
		o->v195(c.xyzw());
		v195(d.xyzw());
		const float dx = c.x - d.x, dy = c.y - d.y, dz = c.z - d.z;
		m_visible_points[n].dist = sqrtf(dz * dz + dy * dy + dx * dx);	// (x87: one rounding at the end)
		n++;
	}
}

namespace neg {
CGameObject *New_C3DPatrolPoint(void)	{ return new C3DPatrolPoint; }		// factory '3PAT' (0x97c bytes)
}
