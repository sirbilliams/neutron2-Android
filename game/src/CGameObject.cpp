/*  CGameObject - hand-written parts. See CGameObject.h.
    The accessor slots are in gen/CGameObject_gen.cpp.                                        */

#include "CGameObject.h"
#include "Game.h"
#include "Physics.h"
#include "Todo.h"
#include "OMediaStreamOperators.h"
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "Bridge.h"
#include "OMediaTrigo.h"
#include "OMedia3DVector.h"

std::list<CGameObject *> g_objects;					// 0x56af34

namespace neg {
static bool s_end_requested = false;				// 0x56aeb5
}
using namespace neg;

// --------------------------------------------------------------------------- 0x4aa960

CGameObject::CGameObject()
{
	f_048 = 0; f_058 = 0; f_068 = 0; f_078 = 0; f_088 = 0;
	f_098 = 1.0f; f_0a8 = 1.0f; f_0b8 = 1.0f; f_0c8 = 1.0f; f_0d8 = 1.0f; f_0e8 = 1.0f; f_0f8 = 1.0f; f_108 = 1.0f;
	f_170 = 0; f_180 = 0; f_190 = 0; f_1a0 = 0; f_1b0 = 0;
	f_1c0 = 1.0f; f_1d0 = 1.0f; f_1e0 = 1.0f; f_1f0 = 1.0f; f_200 = 1.0f; f_210 = 1.0f; f_220 = 1.0f; f_230 = 1.0f;
	f_280 = 1.0f; f_290 = 1.0f;
	m_name[0] = 0; m_tag[0] = 0; m_object_id = 0;

	strcpy(m_name, "CGAMEOBJECT");
	Log("CGameObject()");

	// Each attribute: the "initial" copy, then the current value through its setter.
	f_00c = 0;  v124(0);
	f_00d = 1;  v126(1);
	f_00e = 1;  v110(1);
	f_00f = 0;  v120(0);
	f_010 = 1;  v122(1);
	f_011 = 1;  v128(1);
	f_012 = 0;  v130(0);
	f_013 = 1;  v114(1);
	f_014 = 1;  v116(1);
	f_015 = 1;  v118(1);
	f_016 = 1;  v132(1);
	f_017 = 0;  v134(0);
	f_018 = 1;  v136(1);
	f_019 = 0;  v138(0);
	f_01c = 0.5f;	v140(0.5f);
	f_020 = 100.0f;	v142(100.0f);
	f_024 = 0.2f;	v144(0.2f);
	f_028 = 0.9f;	v146(0.9f);
	f_02c = 1.0f;	v148(1.0f);
	f_030 = 1.0f;	v150(1.0f);
	f_034 = 0;		v152(0);
	f_038 = 0;		v154(0);
	v157(0, 0, 0);
	v162(0, 0, 0);
	v167(0, 0, 0);
	v172(0, 0, 0);
	v176(0, 0, 0);
	v181(0, 0, 0);
	v188(0, 0, 0);
	v191(0, 0, 0);
	v194(0, 0, 0);
	v197(0, 0, 0);
	v247();
	v203(0, 0, 0);
	v248();
	f_124 = 1;
	f_10c = 0;  v206(0);
	f_110 = 0;  v208(0);
	f_114 = 0;  v210(0);
	f_118 = 0;  v212(0);
	f_11c = 0;  v214(0);
	f_120 = 0;  v216(0);
	f_01a = 1;  v111(1);
	f_00b = 1;
	CGameObject::ResetObject();
	f_294 = 0;
	f_29c = 1.0f;
	f_408 = -1; f_40c = -1; f_410 = -1;
	f_414 = NULL; f_418 = 0;
	g_objects.push_back(this);
	f_41c = 0;
	f_004 = 0;
	m_num_properties = 0;
	m_debug_cube = NULL; m_debug_sphere = NULL;
	f_007 = 1; f_008 = 0; f_009 = 0;
	f_42c = 0;
	f_430 = 1.0f;
	f_438 = 0;
}

// --------------------------------------------------------------------------- 0x4aaf60

CGameObject::~CGameObject()
{
	Log("~CGameObject()");
	StopObjectSounds(this);									// 0x4b8dc0
	if (m_debug_cube)   delete m_debug_cube;				// (+0xc0)->vtable +0x364: delete via the element
	if (m_debug_sphere) delete m_debug_sphere;
	DeleteAllProperties();
	g_objects.remove(this);
	PhysicsRemoveObject(this);								// 0x4b4070
}

// --------------------------------------------------------------------------- properties

void CGameObject::RegisterProperty(const char *name, void *data, int type, int flags)	// 0x4aeee0
{
	CProperty *p = (CProperty *)malloc(sizeof(CProperty));
	if (!p) { ErrorLog("ERROR: Unable to allocate variable %s in AddProperty", name); return; }
	m_num_properties++;
	p->type = type;
	p->flags = flags;
	strcpy(p->name, name);
	p->data = data;
	m_properties.push_back(p);
}

CProperty *CGameObject::FindProperty(const char *name)		// 0x4aedf0: exact, case-sensitive match
{
	for (std::list<CProperty *>::iterator i = m_properties.begin(); i != m_properties.end(); i++)
		if (strlen((*i)->name) == strlen(name) && strcasecmp((*i)->name, name) == 0)	// _stricmp
			return *i;
	return NULL;
}

void CGameObject::DeleteAllProperties(void)					// 0x4af3f0
{
	Log("DeleteAllProperties()");
	while (!m_properties.empty())
	{
		CProperty *p = m_properties.front();
		m_properties.remove(p);
		free(p);
	}
}

void CGameObject::CopyPropertiesTo(CGameObject *dst)		// 0x4ae9a0
{
	if (!dst) return;
	for (std::list<CProperty *>::iterator i = m_properties.begin(); i != m_properties.end(); i++)
	{
		CProperty *src = *i;
		CProperty *d = dst->FindProperty(src->name);
		if (!d) { ErrorLog("ERROR: CopyOfProperty is NULL for %s", m_tag); return; }
		if (src->type == PROP_ALPHA) strcpy((char *)d->data, (char *)src->data);
		else if (src->type == PROP_FLOAT) memcpy(d->data, src->data, 4);
		else if (src->type != PROP_LONG) ErrorLog("ERROR: invalid type writing variable %s", src->name);
	}
}

// Every value property is 4 bytes in the file and is copied as 4 bytes, even into a bool:
// the original wrote/read `int` through the field's address, so a bool property carries the
// three bytes that follow it. Kept as is - the level files depend on it.
static int read4(OMediaStreamOperators &s, void *dst, bool as_float)
{
	if (as_float) { float f; s >> f; memcpy(dst, &f, 4); }
	else { int v; s >> v; memcpy(dst, &v, 4); }
	int bits; memcpy(&bits, dst, 4);
	return bits;
}

void CGameObject::ReadProperties(OMediaStreamOperators &s)	// 0x4aea70
{
	int count; s >> count;
	if (count != m_num_properties)
		;	// "WARNING: Num properties in file does not match current NumProperties" (debug log)
	int sum = 0;
	for (int n = 0; n < count; n++)
	{
		char name[64]; s >> name;
		CProperty *p = FindProperty(name);
		int type, size;
		if (p)
		{
			s >> type; p->type = type;
			switch (type)
			{
				case PROP_NONE: s >> size; break;
				case PROP_ALPHA:
					s >> size; s >> (char *)p->data;
					if (size != (int)strlen((char *)p->data))
						ErrorLog("ERROR: Invalid EDIT_ALPHA writing variable %s in object %s", name, m_name);
					break;
				case PROP_BOOL:  s >> size; sum += read4(s, p->data, false); break;
				case PROP_FLOAT: s >> size; sum += read4(s, p->data, true); break;
				case PROP_SPECIFIC: s >> size; sum += read4(s, p->data, false); break;
				case PROP_5: s >> size; break;
				case PROP_LONG: s >> size; sum += read4(s, p->data, false); break;
				default: s >> size; ErrorLog("ERROR: invalid type reading variable %s in object %s", name, m_name);
			}
		}
		else
		{
			// "WARNING: Could not find variable %s in object %s" (debug log); skip the value
			s >> type; s >> size;
			char tmp[128]; int v; float f;
			switch (type)
			{
				case PROP_NONE: if (size) ErrorLog("ERROR: bad EDIT_NONE for %s during load of %s", name, m_name); break;
				case PROP_ALPHA:
					s >> tmp;
					if (size != (int)strlen(tmp)) ErrorLog("ERROR: Invalid EDIT_ALPHA for %s during load of %s", name, m_name);
					break;
				case PROP_BOOL:
					if (size != 4) ErrorLog("ERROR: bad EDIT_BOOL for %s during load of %s", name, m_name);
					s >> v; sum += v; break;
				case PROP_FLOAT:
					if (size != 4) ErrorLog("ERROR: bad EDIT_FLOAT for %s during load of %s", name, m_name);
					s >> f; memcpy(&v, &f, 4); sum += v; break;
				case PROP_SPECIFIC:
					if (size != 4) ErrorLog("ERROR: bad EDIT_SPECIFIC for %s during load of %s", name, m_name);
					s >> v; sum += v; break;
				case PROP_5: break;
				case PROP_LONG:
					if (size != 4) ErrorLog("ERROR: bad EDIT_LONG for %s during load of %s", name, m_name);
					s >> v; sum += v; break;
				default: ErrorLog("ERROR: invalid type reading variable %s in object %s", name, m_name);
			}
		}
	}
	int file_sum; s >> file_sum;
	if (file_sum != sum) ErrorLog("ERROR: Cheat warning: Invalid FILE checksum");
}

void CGameObject::WriteProperties(OMediaStreamOperators &s)	// 0x4ae7d0
{
	int sum = 0;
	for (std::list<CProperty *>::iterator i = m_properties.begin(); i != m_properties.end(); i++)
	{
		CProperty *p = *i;
		s << p->name;
		s << p->type;
		s.getsize();							// (result unused in the original)
		int size, v;
		switch (p->type)
		{
			case PROP_NONE: case PROP_5: size = 0; s << size; break;
			case PROP_ALPHA: size = strlen((char *)p->data); s << size; s << (char *)p->data; break;
			case PROP_BOOL: case PROP_SPECIFIC: case PROP_LONG:
				size = 4; s << size; memcpy(&v, p->data, 4); s << v; sum += v; break;
			case PROP_FLOAT:
			{
				size = 4; s << size; float f; memcpy(&f, p->data, 4); s << f; memcpy(&v, p->data, 4); sum += v; break;
			}
			default: size = 0; s << size; ErrorLog("ERROR: invalid type writing variable %s", p->name);
		}
	}
	s << sum;
}

// --------------------------------------------------------------------------- identity

// The original upper-cases its argument in place (_strupr). Callers pass string literals, which
// MSVC 6 placed in writable memory; here they are read-only, so the upper-casing works on a copy.
// The only difference: the caller's string is not modified.
void CGameObject::SetName(char *tag_in)						// 0x4ab1b0
{
	char tag[100];
	strncpy(tag, tag_in, sizeof tag - 1); tag[sizeof tag - 1] = 0;
	for (char *c = tag; *c; c++) *c = (char)toupper(*c);	// _strupr
	strcat(m_name, "_");								// 0x540140
	strcat(m_name, tag);
	strcpy(m_tag, tag);
}

bool CGameObject::NameContains(char *s_in)					// 0x4ab260 (same note as SetName)
{
	char s[256];
	strncpy(s, s_in, sizeof s - 1); s[sizeof s - 1] = 0;
	for (char *c = s; *c; c++) *c = (char)toupper(*c);
	return strstr(m_name, s) != NULL;
}

void CGameObject::Log(const char *fmt, ...)					// 0x4ae780 (writes "+%s->%s" to the debug log)
{
	(void)fmt;
	// The shipped build's debug log is empty; uncomment to trace object calls.
	// char buf[256]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
	// SDL_Log("+%s->%s", m_name, buf);
}

// --------------------------------------------------------------------------- lifecycle

void CGameObject::InitObject(void)							// 0x4ab290
{
	Log("InitObject()");
	while (!m_properties.empty()) m_properties.pop_front();	// (the CProperty blocks are not freed here)
	m_num_properties = 0;
	RegisterProperty("ObjectTag",    m_tag, PROP_ALPHA, 0);
	RegisterProperty("RotateToDest", &f_00a, PROP_BOOL, 0);
	RegisterProperty("ObjectID",     &m_object_id, PROP_LONG, 0);
	RegisterProperty("PositionX",    &f_0ec, PROP_FLOAT, 0);
	RegisterProperty("PositionY",    &f_0f0, PROP_FLOAT, 0);
	RegisterProperty("PositionZ",    &f_0f4, PROP_FLOAT, 0);
	RegisterProperty("RotationX",    &f_0fc, PROP_FLOAT, 0);
	RegisterProperty("RotationY",    &f_100, PROP_FLOAT, 0);
	RegisterProperty("RotationZ",    &f_104, PROP_FLOAT, 0);
}

void CGameObject::ResetObject(void)							// 0x4ab420: current values = initial values
{
	Log("ResetObject()");
	v124(v25());   v126(v29());   v110(v31());   v120(v33());   v122(v49());
	v128(v35());   v130(v37());   v116(v41());   v118(v43());   v114(v39());
	v132(v45());   v136(v51());   v138(v53());
	v140(v55());   v144(v59());   v146(v61());   v148(v63());   v150(v65());	// (no v142: as shipped)
	v152(f_034);   v154(v68());
	float t[4];
	v70(t);  v156(t[0], t[1], t[2], t[3]);
	v72(t);  v161(t[0], t[1], t[2], t[3]);
	v74(t);  v166(t[0], t[1], t[2], t[3]);
	v76(t);  v171(t[0], t[1], t[2], t[3]);
	v78(t);  v175(t[0], t[1], t[2], t[3]);
	v80(t);  v180(t[0], t[1], t[2], t[3]);
	v82(t);  v187(t[0], t[1], t[2], t[3]);
	v84(t);  v190(t[0], t[1], t[2], t[3]);
	v86(t);  v193(t[0], t[1], t[2], t[3]);
	v88(t);  v196(t[0], t[1], t[2], t[3]);
	v91(t);  v202(t[0], t[1], t[2], t[3]);
	v206(v95());  v208(v97());  v210(v99());  v212(v101());  v214(v103());  v216(v105());
	m_touching.clear();
	f_274 = 0; f_278 = 0; f_27c = 0;
	f_298 = 0;
	f_005 = 1;
	f_00a = 0;
	f_006 = 0;
}

// --------------------------------------------------------------------------- collisions

// The touch list holds the other objects (PhysicsCollideObjects passes them, as do C3DConveyor,
// C3DMovingBlocks and C3DRope with the player object 0x56af7c).
void CGameObject::v3(CGameObject *other)					// 0x4ab1a0
{
	AddTouching(other);									// vtable +0x40
}

bool CGameObject::IsTouching(CGameObject *other)			// 0x4ab940
{
	for (std::list<CGameObject *>::iterator i = m_touching.begin(); i != m_touching.end(); i++)
		if (*i == other) return true;
	return false;
}

void CGameObject::AddTouching(CGameObject *other)			// 0x4ab970
{
	if (IsTouching(other) || f_006) return;
	m_touching.push_back(other);
	if (f_408 == -1)
		f_408 = GameTypeTouchStart(this, other);		// g_gametype vtable +0x14c
}

void CGameObject::RemoveTouching(CGameObject *other)		// 0x4aba00
{
	if (!v131() && v45()) v132(1);
	m_touching.remove(other);
	if (f_410 == -1)
		f_410 = GameTypeTouchEnd(this, other);			// g_gametype vtable +0x150
}

void CGameObject::v18(CGameObject *other)					// 0x4abaa0
{
	if (f_40c == -1)
		f_40c = GameTypeTouch154(this, other);			// g_gametype vtable +0x154
}

void CGameObject::Collide(CGameObject *other, float dist)	// 0x4abad0
{
	if (other == this || other->v119()) return;
	if (v131()) v262(other, dist);						// vtable +0x418
	if (v125()) v24(other);								// vtable +0x60
}

void CGameObject::v21(CGameObject *other)					// 0x4abd40
{
	other->v23(this);
}

void CGameObject::AddRotation(float x, float y, float z)	// 0x4ad310: keep each angle in 0..360
{
	float r[3] = { x + f_224, y + f_228, z + f_22c };
	for (int i = 0; i < 3; i++)
	{
		if (r[i] < 0.0f) r[i] = (float)(360 - abs((int)r[i]));
		else if (r[i] > 360.0f) r[i] -= 360.0f;
	}
	f_224 = r[0]; f_228 = r[1]; f_22c = r[2];
}

void CGameObject::RequestEnd(void)							// 0x4ad4c0
{
	f_006 = 1;
	if (!s_end_requested)
	{
		PostGameMessage(2);								// PostMessage(NULL, WM_USER, 2, 0)
		s_end_requested = true;
	}
}

void CGameObject::v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly)	// 0x4ad520: hit a ceiling
{
	(void)nx; (void)ny; (void)nz; (void)nw; (void)poly;
	float v[4];
	v174(v);											// velocity: stop going up
	if (v[1] > 0.0f) v175(v[0], 0, v[2], v[3]);			// vtable +0x2bc
	v155(v);											// constant acceleration: the same
	if (v[1] > 0.0f) v156(v[0], 0, v[2], v[3]);			// vtable +0x270
}

void CGameObject::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x48ef40
{
	(void)poly;
	if (v117()) v221(a, b, c, d);
}

// 0x4ad800: rotate (x, y, z) by the object's angles v201 (degrees; OMT's 14-bit sin/cos tables)
// and add the position. Rotation order as compiled: z, then x, then y.
OMedia3DVector CGameObject::v224(float x, float y, float z)
{
	float rot[4];
	v201(rot);											// vtable +0x324
	const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360
	omt_Angle ax = (omt_Angle)((int)(rot[0] * kDegToAngle) & omc_MaxAngleMask);
	omt_Angle ay = (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask);
	omt_Angle az = (omt_Angle)((int)(rot[2] * kDegToAngle) & omc_MaxAngleMask);
	OMediaCosSinTable &t = OMediaCosSinTable::global;
	float sx = t.sin(ax), cx = t.cos(ax), sy = t.sin(ay), cy = t.cos(ay), sz = t.sin(az), cz = t.cos(az);
	float a = cz * y - sz * x;							// about z
	float b = sz * y + cz * x;
	float c = cx * z - a * sx;							// about x
	float rx = b * cy - c * sy;							// about y
	float ry = a * cx + sx * z;
	float rz = c * cy + b * sy;
	float p[4];
	v195(p);											// position, vtable +0x30c
	return OMedia3DVector(p[0] + rx, p[1] + ry, p[2] + rz, 1.0f);
}

unsigned char CGameObject::v231(CGameObject *other)		// 0x410490: PhysicsCollideObjects asks both objects
{
	(void)other;
	return 1;
}

// floor polygon and shape (PhysicsSnapToGround): initial values f_10c / f_114, current f_234 / f_23c
OMedia3DPolygon *CGameObject::v95(void)			{ return f_10c; }						// 0x4ac620
void CGameObject::v96(OMedia3DPolygon *poly)	{ f_10c = poly; v206(poly); }			// 0x4ac630
OMedia3DShape *CGameObject::v99(void)			{ return f_114; }						// 0x4ac680
void CGameObject::v100(OMedia3DShape *shape)	{ f_114 = shape; v210(shape); }			// 0x4ac690
OMedia3DPolygon *CGameObject::v205(void)		{ return f_234; }						// 0x4ad400
void CGameObject::v206(OMedia3DPolygon *poly)	{ f_234 = poly; }						// 0x4ad410
OMedia3DShape *CGameObject::v209(void)			{ return f_23c; }						// 0x4ad440
void CGameObject::v210(OMedia3DShape *shape)	{ f_23c = shape; }						// 0x4ad450

void CGameObject::Link(CGameObject *other)					// 0x4ade00
{
	if (!other) return;
	other->m_linked.push_back(this);
	m_linked.push_back(other);
}

void CGameObject::Unlink(CGameObject *other)				// 0x4adeb0
{
	if (!other) return;
	other->m_linked.remove(this);
	m_linked.remove(other);
}

void CGameObject::DisableCollisionID(int id)				// 0x4adf60
{
	Log("DisableCollisionID(%ld)", id);
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o || o->m_object_id != id) continue;
		bool linked = false;
		for (std::list<CGameObject *>::iterator j = m_linked.begin(); j != m_linked.end(); j++)
			if (*j == o) { linked = true; break; }
		if (!linked) Link(o);							// vtable +0x3a4 (slot 233)
	}
}

void CGameObject::EnableCollisionID(int id)					// 0x4adfe0
{
	Log("EnableCollisionID(%ld)", id);
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && (*i)->m_object_id == id) Unlink(*i);	// vtable +0x3ac (slot 235)
}

// --------------------------------------------------------------------------- per frame

void CGameObject::v4(float t)								// 0x48f880
{
	f_294 = t;
	if (t != 0.0f)
	{
		if (EditorIsEditing()) v242(t);					// vtable +0x3c8
		else Process(t);								// vtable +0x3c0
	}
}

void CGameObject::Process(float dt)							// 0x4ae0b0
{
	CheckSounds(dt);									// vtable +0x3d4
	if (f_298 > 0.0f)
	{
		f_298 -= dt;
		if (f_298 <= 0.0f) ResetObject();				// vtable +0x28
	}
	RotateToDest(dt);									// vtable +0x3d8
}

void CGameObject::CheckSounds(float dt)						// 0x4ae110: forget channels that stopped
{
	(void)dt;
	if (f_408 != -1 && !IsChannelPlaying(f_408)) f_408 = -1;
	if (f_40c != -1 && !IsChannelPlaying(f_40c)) f_40c = -1;
	if (f_410 != -1 && !IsChannelPlaying(f_410)) f_410 = -1;
}

static float wrap180(float d)
{
	if (d > 180.0f) return d - 360.0f;
	if (d < -180.0f) return d + 360.0f;
	return d;
}

void CGameObject::RotateToDest(float dt)					// 0x4ae180: turn towards f_274.. at rate f_29c
{
	if (!GameIsRunning() || !f_005 || !f_00a) return;	// 0x4b0ca0
	float dx = f_274, dy = f_278, dz = f_27c;
	float ry = 0, rx = 0, rz = 0;
	f_00a = 0;
	if (dx != f_224) { ry = wrap180(dx - f_224); f_00a = 1; }
	if (dy != f_228) { rx = wrap180(dy - f_228); f_00a = 1; }
	if (dz != f_22c) { rz = wrap180(dz - f_22c); f_00a = 1; }
	float k = dt * f_29c;
	if (k <= fabsf(k * ry)) f_224 = k * ry + f_224; else f_224 = dx;
	if (k <= fabsf(k * rx)) f_228 = k * rx + f_228; else f_228 = dy;
	if (fabsf(rz * k) < k) f_22c = dz; else f_22c = rz * k + f_22c;
}

// 0x4ae3a0: the element's angles (OMT units, 0..0x3fff) from the rotation in degrees (slot 249,
// SetElementAngles). Each angle: _CIfmod(deg, 360.0) * (16384 / 360.0), _ftol, & 0x3fff.
void CGameObject::v248(void)
{
	float r[4];
	v201(r);												// vtable +0x324 (called once per angle)
	const short az = (short)((int)(fmod((double)r[2], 360.0) * 45.51111111111111) & 0x3fff);	// 0x51b5d8, 0x51b5d0
	v201(r);
	const short ay = (short)((int)(fmod((double)r[1], 360.0) * 45.51111111111111) & 0x3fff);
	v201(r);
	const short ax = (short)((int)(fmod((double)r[0], 360.0) * 45.51111111111111) & 0x3fff);
	v249(ax, ay, az);										// vtable +0x3e4
}

// --------------------------------------------------------------------------- not reconstructed yet

// 0x4af030: touching `other` (centre distance `dist`): pushed away by it, unless it is a C3DWHEEL.
// (The original pushes `dist` for slot 263 before it calls slot 151, which takes no argument.)
void CGameObject::v262(CGameObject *other, float dist)
{
	if (other->NameContains((char *)"C3DWHEEL")) return;	// vtable +0x18
	float p[4];
	other->v195(p);											// +0x30c
	const float r = other->v151();							// +0x25c radius
	v263(other, OMedia3DVector(p[0], p[1], p[2], p[3]), r, dist);	// +0x41c
}

// 0x4af0b0: the push between two touching spheres. Unless this object is fixed (slot 93 = 2) or
// has no physics, it is pushed out along the line of centres by the overlap (half of it unless
// the other one is fixed; never downwards): the velocity gets overlap * normal * overlap (sic),
// the position is moved against the terrain by overlap * normal. Unless slot 129, a moving other
// also spins it round Y: (its direction turned 90 degrees round Y . normal) * its speed * 0.05.
void CGameObject::v263(CGameObject *other, OMedia3DVector pos, float r_other, float dist)
{
	if (v93() == 2) return;									// +0x174
	const float s = v151() + r_other;						// +0x25c
	if (!(s > dist)) return;
	if (!v131()) return;									// +0x20c
	float me[4];
	v195(me);
	OMedia3DVector n(me[0] - pos.x, me[1] - pos.y, me[2] - pos.z, 0.0f);
	if (n.quick_magnitude() == 0.0f) return;				// 0x4caa44 (an unordered result returns too)
	float inv = 1.0f / n.quick_magnitude();					// 0x4caa6c
	n.x = inv * n.x; n.y = inv * n.y; n.z = inv * n.z;
	const OMedia3DVector normal = n;
	float k = s - dist;
	if (other->v93() != 2) k = k * 0.5f;					// 0x4caa5c
	n.x = n.x * k;
	if (n.y > 0.0f) n.y = n.y * k;
	else n.y = 0;
	n.z = n.z * k;
	v178(n.x * k, n.y * k, n.z * k);						// +0x2c8 velocity +=
	float p[4];
	v195(p);
	OMedia3DVector at(p[0], p[1], p[2], p[3]);
	PhysicsMoveAgainstTerrain(this, &at, n, true);			// 0x4b6500
	v196(at.x, at.y, at.z, at.w);							// +0x310
	if (v129()) return;										// +0x204
	float ov[4];
	other->v174(ov);										// +0x2b8 velocity
	OMedia3DVector u(ov[0], ov[1], ov[2], ov[3]);
	const float m = u.quick_magnitude();
	if (!(m > 0.0f)) return;
	inv = 1.0f / u.quick_magnitude();
	u.x = inv * u.x; u.y = inv * u.y; u.z = inv * u.z;
	u.rotate(0, 0x1000, 0);									// (inline, OMediaCosSinTable)
	const float spin = (u.z * normal.z + u.y * normal.y + u.x * normal.x) * m * 0.05f;	// 0x4d2cbc
	v173(0, spin, 0);										// +0x2b4 spin +=
}

void CGameObject::v221(float a, float b, float c, float d)	// 0x4ad5e0: velocity / heading physics
{
	(void)a; (void)b; (void)c; (void)d;
	NEG_TODO(0x4ad5e0, "CGameObject slot 221");
}
