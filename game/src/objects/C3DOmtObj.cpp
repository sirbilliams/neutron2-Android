/*  C3DOmtObj - see C3DOmtObj.h. Reconstructed from Neutron2.exe 0x45b270..0x45c4c0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DOmtObj.h"
#include "../C3DFlyingObject.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"
#include "../C3DTriggerType.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DShape.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x4af000: the object whose save index (f_438) is <index>.
static CGameObject *FindObjectByIndex(int index)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && (int)(*i)->f_438 == index) return *i;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DOmtObj::C3DOmtObj()										// 0x45b270
{
	SetName((char *)"C3DOMTOBJ");							// 0x4ab1b0
	Log("C3DOMTOBJ()");
	v94(1);
	C3DAnimated::InitObject();								// 0x41b0a0 (called directly)
	strcpy(f_9b4, "none");
	RegisterProperty("OmtDatabase", f_924, PROP_ALPHA, 0);
	RegisterProperty("OmtIndex", &f_988, PROP_LONG, 0);
	RegisterProperty("SpecialCase", &f_98c, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("RotateSpeed", &f_9ac, PROP_FLOAT, 0);
	RegisterProperty("ResetTrigger", f_9b4, PROP_ALPHA, 0);
	RegisterProperty("DistToView", &f_a1c, PROP_FLOAT, 0);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DObject::SetObjectID('3OMT');							// 0x49ab00
	f_6d0 = 2;												// neither morph (0) nor Granny (1)
	v67(5.0f);
	v152(5.0f);
	v52(1);
	strcpy(f_924, "objects.omt");
	f_988 = 4;
	f_98c = 0;
	f_9ac = 20.0f;
	f_648 = 0;
	f_6d0 = 2;												// (again)
	v34(1);
	for (int i = 0; i < 6; i++) f_990[i] = 0;
	f_9a8 = 0;
	f_9b0 = 0;
	f_a18 = 3.0f;
	f_a1c = 500.0f;
	// (then a call to the empty 0x463890)
}

C3DOmtObj::~C3DOmtObj()										// 0x45b520
{
}

namespace neg {
CGameObject *New_C3DOmtObj(void) { return new C3DOmtObj; }				// '3OMT' (0xa28 bytes)
}

// --------------------------------------------------------------------------- element family

void C3DOmtObj::ev94(CGameObject *o)						// 0x45b950
{
	for (int i = 0; i < 6; i++)
	{
		if (f_990[i]) continue;
		// "MEBTEMP locking %s to %s" (o's element +0x92c, our tag) to the empty 0x463890
		f_990[i] = o;
		o->v136(0);
		C3DAnimated *a = dynamic_cast<C3DAnimated *>(f_990[i]);	// (f_990[i] - 0xc0)
		if (a) a->f_660 = 1;								// port: the original does not check; "CanMove"
		break;
	}
	f_9b0++;
	if (f_9b0 < 4) return;
	CGameObject *done = FindObjectByTag("CRANEDONE");
	CGameObject *boom = FindObjectByTag("C3DCRANEBOOM");
	JimmyActivateMenuItem(g_jimmy, this, g_jimmy, 0, 0.0f);	// Jimmy element +0x228
	if (!done || !g_camera) return;
	float p[4];
	done->v195(p);
	g_camera->v196(p[0], p[1], p[2], p[3]);
	g_5461cc = 0;
	g_camera->v261();
	if (g_camera->NameContains((char *)"C3DJIMMY"))
	{
		C3DJimmy *j = dynamic_cast<C3DJimmy *>(g_camera);	// (g_camera - 0xc0)
		if (j) j->f_4d7c = 0;								// port: the original does not check; off the crane
	}
	if (boom)
	{
		C3DAnimated *b = dynamic_cast<C3DAnimated *>(boom);
		// the crane boom's sound channel (its element +0x8cc)
		if (b) StopGameSound(*(int *)((char *)b + 0x8cc), false);	// port: C3DCraneBoom is not reconstructed yet
	}
	C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(done);	// (done - 0xc8), not checked
	if (tt) tt->CallNextTrigger("CRANEDONE", 0);			// element +0xdc
	SetTaskState("SCENE", 150);								// 0x497980
}

// --------------------------------------------------------------------------- CGameObject family

void C3DOmtObj::AddTouching(CGameObject *other)				// 0x45bb30
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (f_98c == 1) return;
	if (f_98c == 7)
	{
		if (other != g_camera) return;
		PlayGameSound(-1, std::string("tilefall"), false);
		v111(1);
		v132(1);
		return;
	}
	if (f_98c != 9 && strcasecmp(f_9b4, "none") == 0) return;
	v120(1);
	if (other != g_camera) return;
	if (other->NameContains((char *)"C3DROCKETSHIP") || other->NameContains((char *)"C3DMINISUB"))
	{
		if (strcasecmp(f_9b4, "TURNAROUND") != 0)
		{
			v120(1);
			return;
		}
		v120(0);
		C3DFlyingObject *f = dynamic_cast<C3DFlyingObject *>(other);	// (other - 0xc0)
		if (f) f->f_9de = 1;								// turn around next frame
		return;
	}
	if (!other->NameContains((char *)"C3DJIMMY")) return;
	CGameObject *o = FindObjectByTag(f_9b4);
	if (!o)
	{
		ErrorLog("ERROR: ResetTrigger %s not found", f_9b4);
		return;
	}
	if (!o->NameContains((char *)"C3DTRIGGERTYPE")) return;
	C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(o);	// (o - 0xc8)
	if (tt) tt->CallNextTrigger(f_9b4, 0);					// element +0xdc
}

unsigned char C3DOmtObj::v231(CGameObject *other)			// 0x45c0a0
{
	if (f_98c == 12) return 0;
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	if (f_98c == 9 || f_98c == 10)
		return id == '3SUB' || id == '3GOD' || id == '3JIM' || id == '3VRT';
	return id == '3JIM' || id == '3CLW' || id == '3GOD' || id == '3OMT' || id == '3WBL';
}

void C3DOmtObj::Process(float dt)							// 0x45bcf0
{
	C3DAnimated::Process(dt);								// 0x41c120
	if (!f_589) return;
	switch (f_98c)
	{
		case 1:
			if (strcasecmp(m_tag, "PLATFORM") != 0) break;
			for (int i = 0; i < 6; i++)						// carry the locked objects along
			{
				if (!f_990[i]) continue;
				float p[4], a[4];
				v195(p);
				f_990[i]->v196(p[0], p[1], p[2], p[3]);
				v201(a);
				f_990[i]->v202(a[0], a[1], a[2], a[3]);
			}
			return;
		case 2: AddRotation(dt * f_9ac, 0, 0); return;
		case 3: AddRotation(0, dt * f_9ac, 0); return;
		case 4: AddRotation(0, 0, dt * f_9ac); return;
		case 5:												// 2002 element +0x8c: the shape
			get_shape()->offset_textures(0, (float)(dt * -1.5), false);	// 0x4eefe0 (double)
			return;
		case 7:
			if (f_a18 > 0)
			{
				f_a18 -= dt;
				if (f_a18 <= 1.5f)							// shake about the start position
				{
					float s[4];
					v91(s);
					const float k = g_545ddc * 0.65f;		// 0x40b270: the pulse; 0x4eefdc
					v203(k + s[0], s[1], k + s[2]);
				}
				if (f_a18 <= 0)
				{
					unsigned int *seed = g_random_seed;
					*seed = *seed * 0x19660d + 0x7fff;
					f_a18 = (float)((int)((*seed >> 16) * 1001) >> 16) * 0.001f * 10.0f;	// 0..10 s
					return;
				}
			}
			break;
		case 8: AddRotation(0, dt * 500.0f, dt * 1000.0f); return;
		case 12:
			if (!g_camera) break;
			if (g_54a434 != 0)
			{
				const float k = (float)((double)(g_545ddc * 0.5f) + 0.5);	// 0x4cb210 (double)
				ev74(0.8f, 1.0f, k, k);						// element +0x128
				return;
			}
			{
				float c[4], p[4];
				g_camera->v195(c);
				v195(p);
				const float d = sqrtf((c[0] - p[0]) * (c[0] - p[0]) + (c[1] - p[1]) * (c[1] - p[1]) +
									  (c[2] - p[2]) * (c[2] - p[2]));
				if (d < f_a1c)
				{
					ev71(d / f_a1c);						// element +0x11c
					ev91(1);								// element +0x16c
				}
			}
			return;
		default: break;
	}
}

void C3DOmtObj::v260(void)									// 0x45b570
{
	if (strcasecmp(f_9b4, "none") != 0)
	{
		v94(2);
		f_008 = 1;
	}
	if (!m_db)												// the shape, from the OMT database
	{
		m_db = GetDatabase(f_924);							// 0x4a4180
		set_shape(Load3DShape(m_db, f_988));				// element +0xac
		MakeShapeUnLit(get_shape());						// element +0xb0
	}
	if (f_664 == 1) ev73();									// "SecondPass"
	if (f_98c == 7) v94(1);
	if (f_658 == 0)											// "HasCollision" 0
	{
		v136(0);
		v42(0);
		v40(0);
		v132(0);
	}
	C3DAnimated::v260();									// 0x41cec0
	if (f_98c == 1)
	{
		if (strcasecmp(m_tag, "PLATFORM") == 0) v67(170.0f);
		else
		{
			v46(0);
			v67(70.0f);
		}
	}
	f_9b0 = 0;
	if (strcasecmp(f_9b4, "none") != 0 || f_98c == 9)
	{
		v94(2);
		v136(1);
		v34(1);
		v46(0);
		v42(0);
		v40(0);
		f_008 = 1;
		HideAgain();										// element +0xdc
	}
	if (f_98c == 5)
	{
		v94(2);
		v46(0);
		v136(1);
		v34(0);
		v42(0);
		v40(0);
		ev51();												// element +0xcc
	}
	if (f_98c == 6) ev71(0.6f);								// element +0x11c
	if (f_98c == 7)
	{
		v94(1);
		set_shape(get_shape());								// element +0xac / +0xb0
		v42(0);
		v40(0);
		v34(1);
		v136(1);
		v46(0);
		v111(0);
		ev51();
		ShowAgain();										// element +0xd8
		v136(1);
	}
	if (f_98c == 8)
	{
		v42(0);
		v40(0);
		v34(1);
		v136(0);
		v46(0);
		v111(1);
	}
	if (f_98c == 10)
	{
		v94(2);
		v46(0);
		v136(1);
		v34(0);
		v42(0);
		v40(0);
		f_008 = 1;
		HideAgain();
	}
	if (f_98c == 12)
	{
		v94(1);
		v46(0);
		v136(0);
		f_008 = 1;
		ShowAgain();
		ev51();
	}
}

void C3DOmtObj::v271(void)									// 0x45c130
{
	if (g_54a434 == 0 && f_98c == 12)
	{
		ev71(0);											// element +0x11c
		ev91(1);											// element +0x16c
	}
}

void C3DOmtObj::WriteState(OMediaStreamOperators &s)		// 0x45c2d0
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	float p[4], a[4];
	v195(p);
	float x = p[0], y = p[1], z = p[2];
	s << x;
	s << y;
	s << z;
	v201(a);
	x = a[0], y = a[1], z = a[2];
	s << x;
	s << y;
	s << z;
	s << f_9b0;
	bool b = v135() != 0;
	s << b;
	for (int i = 0; i < 6; i++)
	{
		int index = f_990[i] ? (int)f_990[i]->f_438 : -1;
		s << index;
	}
	bool b1 = v112() != 0;
	s << b1;
	bool b2 = v131() != 0;
	s << b2;
}

void C3DOmtObj::ReadState(OMediaStreamOperators &s)			// 0x45c180
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	float x, y, z;
	s >> x;
	s >> y;
	s >> z;
	v197(x, y, z);
	s >> x;
	s >> y;
	s >> z;
	v203(x, y, z);
	v218(x, y, z);
	s >> f_9b0;
	bool b;
	s >> b;
	v136(b);
	for (int i = 0; i < 6; i++)
	{
		int index;
		s >> index;
		f_990[i] = index == -1 ? 0 : FindObjectByIndex(index);
	}
	s >> b;
	v111(b);
	s >> b;
	v132(b);
}
