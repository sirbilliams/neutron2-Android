/*  C3DGem - see C3DGem.h. Reconstructed from Neutron2.exe 0x436fe0..0x437ac0, 0x473010 (shared). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DGem.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
#include "../InterfaceElement.h"				// AddScore
#include "../level1/C3DSpriteObject.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>

using namespace neg;

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DGem::C3DGem()											// 0x436fe0
{
	SetName((char *)"C3DGEM");								// 0x4ab1b0
	C3DGem::InitObject();									// 0x4371f0 (called directly)
	C3DObject::SetObjectID('3GEM');							// 0x49ab00
	v64(1.265f);
	v67(10.0f);												// radius
	strcpy(f_924, "gembase.grn");							// the model
	f_008 = 1;												// element +0xc8
	f_00b = 1;
	f_65c = 1;												// InitiallyVisible
	g_545f5c = 0;
	g_545f60 = 0;
	g_545f58 = 0;
	f_dc8 = 0;
	f_dcc = 0;
	g_545f68 = 0;
	f_d9c = 0;
	f_d98 = 0;
	f_660 = -1;
	m_red = 1.0f;
	m_green = 1.0f;
	m_blue = 1.0f;
	m_opacity = 0;											// port: not set by the original
	f_dd0 = 0;
	C3DGrannyObj::InitPhysics();							// 0x43ba20
}

C3DGem::~C3DGem() {}										// 0x4371c0

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DGem::InitObject(void)								// 0x4371f0
{
	C3DGrannyObj::InitObject();								// 0x43ba70
	RegisterProperty("Red",              &m_red,     PROP_FLOAT, 0);
	RegisterProperty("Green",            &m_green,   PROP_FLOAT, 0);
	RegisterProperty("Blue",             &m_blue,    PROP_FLOAT, 0);
	RegisterProperty("Opacity",          &m_opacity, PROP_FLOAT, 0);
	RegisterProperty("InitiallyVisible", &f_65c,     PROP_LONG,  0);
	RegisterProperty("Radius",           &f_034,     PROP_FLOAT, 0);
}

void C3DGem::ResetObject(void)								// 0x473010
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	v176(0, 0, 0);											// +0x2c0
	v157(0, 0, 0);											// +0x274
}

// 0x437650: Jimmy takes the gem when he holds nothing (and no gem is placed).
void C3DGem::AddTouching(CGameObject *other)
{
	if (g_545f68) return;
	C3DJimmy *jim = g_jimmy ? dynamic_cast<C3DJimmy *>(g_jimmy) : NULL;	// (g_545fd0 - 0xc0)
	if (!jim || jim->f_d38) return;							// port: NULL checked
	C3DGrannyObj::AddTouching(other);						// 0x43c430
	if (!other->NameContains((char *)"C3DJIMMY")) return;	// +0x18
	if (strcasecmp(m_tag, "REDGEM") == 0 && g_545f58 == 0) g_545f58 = 1;
	if (strcasecmp(m_tag, "YELLOWGEM") == 0 && g_545f58 == 0) g_545f58 = 2;
	if (strcasecmp(m_tag, "BLUEGEM") == 0 && g_545f58 == 0) g_545f58 = 4;
	jim->f_d38 = this;										// he holds it (+0xc78 game object view)
}

void C3DGem::v260(void)										// 0x4372a0
{
	C3DGrannyObj::v260();									// 0x43bcd0
	if (f_65c == 0) HideAgain();							// element +0xdc
	else ShowAgain();										// element +0xd8
	f_dc8 = 0;
	f_dcc = 0;
	// the first polygon's material (2002: shape +0x88 first polygon, +4 material) takes the colour
	OMedia3DShape *sh = get_shape();
	if (sh && !sh->get_polygons()->empty())				// port: the original tests the material only
	{
		OMedia3DMaterial *m = sh->get_polygons()->front().get_material();
		if (m) m->set_diffuse(OMediaFARGBColor(m_opacity, m_red, m_green, m_blue));	// +0x4c.. (r, g, b, a)
	}
}

void C3DGem::WriteState(OMediaStreamOperators &s)			// 0x437a40
{
	C3DGrannyObj::WriteState(s);							// 0x43cbd0
	int v = g_545f64;						// (a long in 2002)
	s << v;
}

void C3DGem::ReadState(OMediaStreamOperators &s)			// 0x437960
{
	C3DGrannyObj::ReadState(s);								// 0x43c9f0
	int v;
	s >> v;
	g_545f64 = v;
	g_545f64 = 3;
	static const char *const doors[3] = { "GREENDOOR", "ORANGEDOOR", "PURPLEDOOR" };
	for (int i = 0; i < 3; ++i)
		if (CGameObject *d = FindObjectByTag(doors[i])) d->ResetObject();	// +0x28
	ResetObject();
	g_545f64 = 0;
	g_545f5c = 0;
	g_545f60 = 0;
	g_545f58 = 0;
	g_545f68 = 0;
	CGameObject *ray = FindObjectByTag("SPHYNXRAY");
	if (C3DAnimated *a = ray ? dynamic_cast<C3DAnimated *>(ray) : NULL)	// (ray - 0xc0)
		a->ev74(0.6f, 1.0f, 1.0f, 1.0f);					// element +0x128 tint
}

// --------------------------------------------------------------------------- the sphinx puzzle

static void SetFirstMaterial(C3DGem *g, const OMedia3DVector &c)	// shape +0x88 first polygon, +4 material, +0x4c..
{
	OMedia3DShape *sh = g->get_shape();
	if (!sh || sh->get_polygons()->empty()) return;			// port: the original tests the material only
	OMedia3DMaterial *m = sh->get_polygons()->front().get_material();
	if (m) m->set_diffuse(OMediaFARGBColor(c.w, c.x, c.y, c.z));
}

static void TintRay(const OMedia3DVector &c)				// SPHYNXRAY element +0x128
{
	CGameObject *ray = FindObjectByTag("SPHYNXRAY");
	if (C3DAnimated *a = ray ? dynamic_cast<C3DAnimated *>(ray) : NULL)
		a->ev74(0.6f, c.x, c.y, c.z);
}

static C3DJimmy *JimmyElement(void)	{ return g_jimmy ? dynamic_cast<C3DJimmy *>(g_jimmy) : NULL; }	// (g_545fd0 - 0xc0)

// A door of the puzzle opens (its NextState(-1): toggled open), for 100 points.
static bool OpenDoor(const char *tag)
{
	CGameObject *d = FindObjectByTag(tag);
	if (!d) return false;
	if (CLocalGameObject *l = dynamic_cast<CLocalGameObject *>(d)) l->v268(-1);	// +0x430 NextState: toggle
	AddScore(100);											// g_gametype, 0x482b50
	return true;
}

// 0x437750: the colour of a gem sum (1 red, 2 yellow, 4 blue), alpha 0.6. A mixed colour opens the
// next door of the order purple, orange, green.
OMedia3DVector C3DGem::Colour(int gems)
{
	f_dd0 = gems;
	float r = 1.0f, g = 1.0f, b = 1.0f;
	switch (gems)
	{
		case 1: g = 0; b = 0; break;						// red
		case 2: g = 1.0f; b = 0; break;						// yellow
		case 3:												// orange
			g = 0.6f; b = 0.2f;
			if (g_545f64 == 1 && OpenDoor("ORANGEDOOR")) g_545f64 = 2;
			break;
		case 4: r = 0; g = 0.4f; b = 1.0f; break;			// blue
		case 5:												// purple
			r = 0.57f; g = 0.15f; b = 0.56f;				// 0x4de8d0
			if (g_545f64 == 0 && OpenDoor("PURPLEDOOR")) g_545f64 = 1;
			break;
		case 6:												// green
			r = 0; g = 1.0f; b = 0;
			if (g_545f64 == 2 && FindObjectByTag("GREENDOOR"))
			{
				g_545f68 = 1;								// the gems stay where they are
				OpenDoor("GREENDOOR");
				g_545f64 = 3;
			}
			break;
	}
	return OMedia3DVector(r, g, b, 0.6f);
}

// 0x437410 (thiscall on the gem Jimmy holds): Jimmy puts it in `holder`.
void C3DGem::PutDown(CGameObject *jimmy, CGameObject *holder)
{
	C3DJimmy *jim = JimmyElement();
	if (!jim || !jim->f_d38) return;						// port: NULL checked
	float p[4];
	holder->v195(p);
	float x = p[0] + 10.0f, y = p[1] + 10.0f, z = p[2] - 30.0f;	// 0x4cb1cc, 0x4ca4b4
	if (strcasecmp(holder->m_tag, "LEFTGEM") == 0)
	{
		g_545f5c = g_545f58;
		x += 15.0f;											// 0x4dce54
		y += 10.0f;
		z -= 5.0f;											// 0x4caa90
	}
	if (strcasecmp(holder->m_tag, "RIGHTGEM") == 0)
	{
		g_545f60 = g_545f58;
		x += 20.0f;											// 0x4caa40
	}
	SetFirstMaterial(this, Colour(g_545f58));
	jimmy->v261();											// +0x414
	if (C3DAnimated *j = dynamic_cast<C3DAnimated *>(jimmy)) j->ev58((char *)"PICKUP", 0);	// element +0xe8
	g_545f58 = 0;
	f_dcc = 0.75f;
	ev98(this, 0);											// element +0x188: let go
	if (C3DSpriteObject *h = dynamic_cast<C3DSpriteObject *>(holder))	// (holder - 0xc8)
	{
		h->f_694[0] = x;									// the holder keeps it there
		h->f_694[1] = y;
		h->f_694[2] = z;
	}
	jim->f_d38 = NULL;
	TintRay(Colour(g_545f60 + g_545f5c));
}

// 0x437340 (thiscall on the gem): taken out of `holder`.
void C3DGem::TakenFrom(CGameObject *holder)
{
	if (g_545f68) return;
	C3DJimmy *jim = JimmyElement();
	if (jim && !jim->f_d38)									// port: NULL checked
	{
		if (strcasecmp(holder->m_tag, "LEFTGEM") == 0) g_545f5c = 0;
		if (strcasecmp(holder->m_tag, "RIGHTGEM") == 0) g_545f60 = 0;
	}
	TintRay(Colour(g_545f60 + g_545f5c));
}

namespace neg {
CGameObject *New_C3DGem(void)	{ return new C3DGem; }		// factory '3GEM' (0xddc bytes)
}
