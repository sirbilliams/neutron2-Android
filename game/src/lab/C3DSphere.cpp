/*  C3DSphere - see C3DSphere.h. Reconstructed from Neutron2.exe 0x49c0b0..0x49c6a0 (and
    0x499920, shared with C3DCube). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSphere.h"
#include "../Game.h"
#include "../Todo.h"
#include "OMedia3DShape.h"
#include "OMedia3DMaterial.h"
#include "OMediaCanvas.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#pragma GCC diagnostic pop

using namespace neg;

// --------------------------------------------------------------------------- helpers

static std::string port_path(const std::string &dos)
{
	std::string s(dos);
	if (s.compare(0, 2, ".\\") == 0) s.erase(0, 2);
	for (size_t i = 0; i < s.size(); i++) if (s[i] == '\\') s[i] = '/';
	return s;
}

// 0x4b2600: a canvas stored in a file of its own (as C3DSprite.cpp's copy). Errors (no file)
// are not caught: OMT's exception goes to the caller, as in the original.
static OMediaCanvas *LoadCanvasFile(std::string name)
{
	// (debug log "LoadCanvasFile(\"%s\")" - empty in the shipped build)
	OMediaFilePath path(port_path(name));
	OMediaFileStream fs;
	fs.setpath(&path);
	fs.open(omcfp_Read, false, false);
	OMediaCanvas *canvas = new OMediaCanvas;
	fs >> (OMediaClassStreamer *)canvas;
	return canvas;
}

// 0x4b2990 "Load3DMaterial": a new material textured with the canvas file <name>.
static OMedia3DMaterial *Load3DMaterial(std::string name)
{
	// (debug log "Load3DMaterial(\"%s\")" - empty in the shipped build)
	OMediaCanvas *canvas = LoadCanvasFile(name);
	OMedia3DMaterial *m = new OMedia3DMaterial;
	m->set_texture(canvas);									// material vtable +0x20
	return m;
}

// --------------------------------------------------------------------------- construction

C3DSphere::C3DSphere()										// 0x49c0b0
{
	SetName((char *)"C3DSPHERE");
	Log("C3DSphere()");
	C3DSphere::InitObject();								// inlined in the original (the loader's
															// InitObject runs it again, as shipped)
	f_58c = NULL;
}

C3DSphere::~C3DSphere()										// 0x49c230
{
	Log("~C3DSphere()");
}

namespace neg {
CGameObject *New_C3DSphere(void) { return new C3DSphere; }				// '3SPH' (0x598 bytes)
}

// --------------------------------------------------------------------------- element slots

void C3DSphere::InitSphere(OMedia3DVector argb, float size)	// 0x49c560
{
	Log("InitSphere(argb,%f)", (double)size);
	get_shape()->make_sphere(size, 8, 12);					// shape vtable +0xac
	OMedia3DMaterial *m = new OMedia3DMaterial;
	f_58c = m;
	// The four values are copied as they are to the material's ambient colour (+0x6c, in memory
	// order red, green, blue, alpha).
	m->set_ambient(OMediaFARGBColor(argb.w, argb.x, argb.y, argb.z));
	f_58c->set_light_mode(ommlmc_Light);					// +0x2c = 2
	get_shape()->set_material(f_58c);						// shape vtable +0x70
	FitToShape();											// element +200
}

void C3DSphere::InitSphere(std::string file, float size)	// 0x49c370
{
	Log("InitSphere(\"%s\",%f)", file.c_str(), (double)size);
	get_shape()->make_sphere(size, 8, 12);					// shape vtable +0xac
	OMedia3DMaterial *m;
	if (file.length() == 0)
	{
		m = new OMedia3DMaterial;
		f_58c = m;
		f_58c->set_shade_mode(omshademc_Gouraud);			// +0x28 = 1
		f_58c->set_light_mode(ommlmc_Light);				// +0x2c = 2
		m = f_58c;
	}
	else m = Load3DMaterial(file);							// (not kept in f_58c: never freed)
	get_shape()->set_material(m);							// shape vtable +0x70
	FitToShape();											// element +200
}

// --------------------------------------------------------------------------- CGameObject family

void C3DSphere::InitObject(void)							// 0x49c2d0
{
	Log("InitObject()");
	C3DObject::InitObject();
	set_shape(new OMedia3DShape);							// element +0xac
	SetObjectID('3SPH');									// element +0xc4
}

void C3DSphere::UnInitObject(void)							// 0x499920 (C3DCube's too)
{
	Log("UnInitObject()");
	OMedia3DShape *sh = get_shape();						// element +0xb0
	set_shape(NULL);										// element +0xac
	if (sh) delete sh;										// vtable at +0x24, slot 0 (deleting)
	if (f_58c)
	{
		delete f_58c;										// through the virtual-base streamer
		f_58c = NULL;
	}
	C3DObject::UnInitObject();
}
