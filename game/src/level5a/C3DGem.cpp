/*  C3DGem - see C3DGem.h. Reconstructed from Neutron2.exe 0x436fe0..0x437ac0, 0x473010 (shared). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DGem.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
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

namespace neg {
CGameObject *New_C3DGem(void)	{ return new C3DGem; }		// factory '3GEM' (0xddc bytes)
}
