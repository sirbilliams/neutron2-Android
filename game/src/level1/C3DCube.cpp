/*  C3DCube - see C3DCube.h. Reconstructed from Neutron2.exe 0x499650..0x49a280. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCube.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DShape.h"
#include "OMedia3DMaterial.h"
#include "OMediaDataBase.h"

using namespace neg;

// 0x4b2c20 "Load3DMaterial(%d)": material <id> of db ('3DMa'), unlocked.
static OMedia3DMaterial *Load3DMaterial(OMediaDataBase *db, int id)
{
	OMedia3DMaterial *m = NULL;
	if (db)
	{
		m = (OMedia3DMaterial *)db->get_object('3DMa', id);
		if (g_56afaa) NEG_TODO(0x4b2c20, "debug database call (db vtable +0x4c)");
		if (m) m->db_unlock();
	}
	return m;
}

// --------------------------------------------------------------------------- construction

C3DCube::C3DCube()											// 0x499650
{
	SetName((char *)"C3DCUBE");
	Log("C3DCube()");
	C3DCube::InitObject();									// (inlined)
	f_58c = NULL;
	v107(0);
}

C3DCube::~C3DCube() {}										// 0x4997e0

namespace neg {
CGameObject *New_C3DCube(void) { return new C3DCube; }		// '3CUB' (0x598 bytes)
}

// --------------------------------------------------------------------------- element slots

void C3DCube::ev56(OMediaDataBase *db, int id, float size)	// 0x49a0a0
{
	get_shape()->make_cube(size, false);					// shape vtable +0xa8
	get_shape()->set_material(Load3DMaterial(db, id));		// shape vtable +0x70
	FitToShape();											// element +200
}

void C3DCube::InitCube(OMedia3DVector argb, float size)		// 0x499fb0
{
	Log("InitCube(argb,%f)", (double)size);
	get_shape()->make_cube(size, false);					// shape vtable +0xa8
	OMedia3DMaterial *m = new OMedia3DMaterial;
	f_58c = m;
	// The four values are copied as they are to the material's ambient colour (+0x6c, in memory
	// order red, green, blue, alpha), as C3DSphere does.
	m->set_ambient(OMediaFARGBColor(argb.w, argb.x, argb.y, argb.z));
	f_58c->set_light_mode(ommlmc_Light);					// +0x2c = 2
	get_shape()->set_material(f_58c);						// shape vtable +0x70
	FitToShape();											// element +200
}

void C3DCube::InitCube(std::string file, float size)		// 0x4999a0
{
	Log("InitCube(\"%s\",%f)", file.c_str(), (double)size);
	// The original builds the 8 corners (+-size/2) and the 6 textured quads itself, with a
	// material textured from <file> ("" when none is given). Nothing in the shipped game calls it.
	NEG_TODO(0x4999a0, "C3DCube::InitCube(file, size): the textured cube");
}

void C3DCube::ev59(float alpha)								// 0x49a0f0
{
	OMedia3DShape *sh = get_shape();						// element +0xb0
	if (!sh) return;
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		if (alpha <= 0)
		{
			m->set_flags(m->get_flags() & ~ommatf_SecondPass);
			m->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
			m->set_blend(omblendfc_Src_Alpha, omblendfc_One);
		}
		else
		{
			m->set_diffuse(OMediaFARGBColor(alpha, 1.0f, 1.0f, 1.0f));
			m->set_flags(m->get_flags() | ommatf_SecondPass);
			m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		}
	}
}

// --------------------------------------------------------------------------- CGameObject family

void C3DCube::InitObject(void)								// 0x499880
{
	Log("InitObject()");
	C3DObject::InitObject();								// 0x49ac20
	set_shape(new OMedia3DShape);							// element +0xac
	SetObjectID('3CUB');									// element +0xc4
}

void C3DCube::UnInitObject(void)							// 0x499920
{
	Log("UnInitObject()");
	OMedia3DShape *sh = get_shape();						// element +0xb0
	set_shape(NULL);										// element +0xac
	if (sh) delete sh;
	if (f_58c)
	{
		delete f_58c;
		f_58c = NULL;
	}
	C3DObject::UnInitObject();
}
