/*  CLocalGameObject - hand-written parts. See CLocalGameObject.h. */
#include "CLocalGameObject.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "OMediaMemStream.h"
#include "OMediaDataBase.h"
#include "OMediaStreamOperators.h"
#include "OMediaViewPort.h"
#include "OMediaMatrix.h"
#include "OMediaRect.h"
#include <string.h>

using namespace neg;

static bool s_all_paused = false;							// 0x54a4b9

CLocalGameObject::CLocalGameObject()						// 0x48ed70
{
	f_4a9 = 1;
	strcpy(m_task_name, "scene");
	f_4a0 = 0;
	f_4a4 = 0;
	f_4a8 = 1;
	f_4aa = 0;
	f_4ab = 0;
	f_4ac = 0;
	m_debug = 0;
	m_db = NULL;
	m_db_stream = NULL;
	CGameObject::v26(1);
	f_4bc = 0;
	m_show_on_radar = 0;
	f_4c4 = 0;
}

CLocalGameObject::~CLocalGameObject() {}					// 0x48ee70

void CLocalGameObject::InitObject(void)						// 0x48eec0
{
	Log("InitObject()");
	CGameObject::InitObject();
	RegisterProperty("TaskName",    m_task_name,      PROP_ALPHA, 0);
	RegisterProperty("Debug",       &m_debug,         PROP_LONG, 0);
	RegisterProperty("ShowOnRadar", &m_show_on_radar, PROP_LONG, 0);
}

void CLocalGameObject::ResetObject(void)					// 0x48ee80
{
	CGameObject::ResetObject();
	v26(1);													// vtable +0x68
}

void CLocalGameObject::InitPhysics(void)					// 0x48efc0: only once
{
	if (!f_4aa)
	{
		CGameObject::InitPhysics();
		f_4aa = 1;
	}
}

void CLocalGameObject::UnInitDatabases(void)				// 0x48f540
{
	if (f_4bc) CloseObjectDatabase();						// vtable +0x444 (tail call)
}

unsigned char CLocalGameObject::v133(void)					// 0x48f010
{
	if (f_4ac) return 1;
	return f_13f;
}

void CLocalGameObject::AddRotation(float x, float y, float z)	// 0x48f8d0
{
	if (GameIsRunning()) CGameObject::AddRotation(x, y, z);
}

void CLocalGameObject::Process(float dt)					// 0x48efe0
{
	CGameObject::Process(dt);
}

void CLocalGameObject::v265(float a, float b, float c, float d)	// 0x48ef80
{
	if (v117()) v221(a, b, c, d);							// vtable +0x1d4 / +0x374
}

void CLocalGameObject::DebugLog(const char *fmt, ...)		// 0x48f520: only when the "Debug" property is set
{
	(void)fmt;
	if (m_debug) { /* writes to the (empty in release) debug log */ }
}

void CLocalGameObject::CloseObjectDatabase(void)			// 0x48f560
{
	// "Calling CloseObjectDatabase for %s"
	if (m_db)
	{
		UnInitDatabase(m_db);								// 0x4b1040
		m_db = NULL;
	}
	if (m_db_stream)
	{
		delete m_db_stream;
		m_db_stream = NULL;
	}
}

void CLocalGameObject::OpenObjectDatabase(void)				// 0x48f5f0: an in-memory database
{
	if (!m_db_stream) m_db_stream = new OMediaMemStream;
	if (!m_db) m_db = new OMediaDataBase(m_db_stream);
}

int CLocalGameObject::CountObjectsWithID(int id)			// 0x48f700
{
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && (*i)->m_object_id == id) n++;
	return n;
}

void CLocalGameObject::PauseAll(bool pause)					// 0x48f6a0
{
	if (pause == s_all_paused) return;
	s_all_paused = pause;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		if (!*i) continue;
		CLocalGameObject *o = (CLocalGameObject *)*i;		// every listed object has these slots
		if (pause) o->v277(); else o->v278();				// vtable +0x454 / +0x458
	}
}

// 0x48f900: project the world point pos (x, y, z) through the viewport's camera and our layer's
// projection (v282) to the screen: *sx, *sy relative to the viewport centre (y up), in pixels.
// Returns 0 when the point is outside the frustum (|x|, |y| or |z| beyond w * (1 + 640 / width)
// in clip space); *sx / *sy are written either way. The original has OMT 2002's matrix code
// inlined (0x48fd70 identity, 0x48fdb0 / 0x48fe30 / 0x48fea0 rotate x / y / z of the negated
// viewport angles, multiplied z * x * y, 0x48ff20 multiply by the translation to -position,
// hint rotate-translate; 0x48ffd0 transform): that is OMediaMatrix_4x4::set_world_transform.
unsigned char CLocalGameObject::v281(OMediaViewPort *vp, float *pos, float *sx, float *sy)	// 0x48f900
{
	OMediaRect b;
	vp->getbounds(&b);										// (2002 +0xd4)
	float w = (float)(b.right - b.left);
	float h = (float)(b.bottom - b.top);
	const float half_w = w * 0.5f;
	const float half_h = h * 0.5f;
	unsigned char visible = 1;
	OMediaMatrix_4x4 view, proj;
	// the camera's inverse rotation (z, x, y of the negated angles) after its inverse position,
	// a rotate-translate matrix: OMT's world transform
	view.set_world_transform(vp->get_x(), vp->get_y(), vp->get_z(), vp->get_anglex(), vp->get_angley(), vp->get_anglez());
	proj.set_zero();										// port: the original leaves it uninitialised (v282 fills it)
	v282(proj, &w, &h);										// vtable +0x468
	float v[4] = { pos[0], pos[1], pos[2], 1.0f };
	view.multiply(v);
	proj.multiply(v);
	const float lim = v[3] / w * 640.0f + v[3];				// 0x515f28
	if (v[0] < -lim || v[0] > lim) visible = 0;
	if (v[1] < -lim || v[1] > lim) visible = 0;
	if (v[2] < -lim || v[2] > lim) visible = 0;
	*sx = v[0] / v[3] * half_w;
	*sy = v[1] / v[3] * half_h;
	return visible;
}

void CLocalGameObject::WriteState(OMediaStreamOperators &s)	// 0x48fbb0
{
	s << m_object_id;
	s << f_438;
	s << f_430;
}

void CLocalGameObject::ReadState(OMediaStreamOperators &s)	// 0x48fbf0
{
	int id = 0;
	s >> id;
	if (id != m_object_id)
	{
		char a[5] = { (char)(id >> 24), (char)(id >> 16), (char)(id >> 8), (char)id, 0 };
		char b[5] = { (char)(m_object_id >> 24), (char)(m_object_id >> 16), (char)(m_object_id >> 8), (char)m_object_id, 0 };
		ErrorLog("ERROR1: CheckID %s does not equal ObjectID %s for %s", a, b, m_tag);
	}
	s >> f_438;
	s >> f_430;
}
