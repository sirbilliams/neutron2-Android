/*  C3DSky - see C3DSky.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSky.h"
#include "Game.h"
#include "CGameType.h"				// Load3DShape
#include "Todo.h"
#include "gen/globals.h"
#include "OMediaLayer.h"
#include "OMediaDataBase.h"
#include "OMedia3DShape.h"
#include "OMedia3DMaterial.h"
#include "OMediaCanvas.h"
#pragma GCC diagnostic pop

using namespace neg;


// 0x4b2750: a canvas chunk of a database, by name (the same code as C3DSprite.cpp's copy).
static OMediaCanvas *LoadCanvasName(OMediaDataBase *db, std::string name)
{
	OMediaCanvas *c = NULL;
	if (db && db->object_exists(OMediaCanvas::db_type, name))
	{
		c = (OMediaCanvas *)db->get_object(OMediaCanvas::db_type, name);
		if (g_56afaa) NEG_TODO(0x4b2750, "debug database call (db vtable +0x4c)");
		if (c) c->db_unlock();								// +0x14 lock count, if > 0
	}
	return c;
}

// --------------------------------------------------------------------------- construction

C3DSky::C3DSky()											// 0x49b8a0
{
	SetName((char *)"C3DSKY");
	Log("C3DSky()");
	C3DSky::InitObject();									// inlined in the original: linked to the
															// world, then to the clear layer
	SetObjectID('3SKY');									// 0x49ab00 (direct call)
	CGameObject::v46(0);									// 0x4abf10 (direct calls)
	CGameObject::v107(0);									// 0x4ad4d0: not written to the .gam (f_00b)
	// (debug log, empty in the shipped build)
}

C3DSky::~C3DSky()											// 0x49ba00
{
	Log("~C3DSky()");
	// (debug log)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DSky::InitObject(void)								// 0x49baa0
{
	Log("InitObject()");
	C3DObject::InitObject();
	link_layer(g_layer_clear);								// element +0x34 (0x56afd0): drawn behind the world
	v232();													// vtable +0x3a0
	// (debug log)
}

void C3DSky::UnInitObject(void)								// 0x49baf0
{
	Log("UnInitObject()");
	C3DObject::UnInitObject();
	ev57();													// element +0xe4: close the sky database
	// (debug log)
}

// --------------------------------------------------------------------------- element family

// Opens `file` as the sky database, unless one is open already. `this` is not used.
void C3DSky::ev56(std::string file)							// 0x49bb30
{
	if (!g_sky_stream)
	{
		g_sky_stream = InitFileStream(file.c_str());		// 0x4b1130
		if (!g_sky_stream) return;
	}
	if (!g_sky_db) g_sky_db = InitDatabase(g_sky_stream);	// 0x4b0fc0
}

void C3DSky::ev57(void)										// 0x49bc90
{
	UnInitDatabase(g_sky_db);								// 0x4b1040
	UnInitFileStream(g_sky_stream);							// 0x4b1250
	g_sky_db = NULL;
	g_sky_stream = NULL;
}

// Opens the sky database and puts its first shape on the sky. Every material is made fully
// emissive white (it is drawn at its texture's colours, whatever the lights); with `alt` the
// materials named "sky1".."sky3" get the textures "sky1alt".."sky3alt" of the same database.
void C3DSky::ev58(std::string file, bool alt)				// 0x49bcc0
{
	g_sky_alt = alt;										// also picks CGameType's "fogcoloralt"
	ev56(file);												// element +0xe0
	if (!g_sky_db) return;
	set_shape(Load3DShape(g_sky_db, 0));					// element +0xac (0x4b2c80)
	if (!get_shape()) return;								// element +0xb0
	OMedia3DShape *shape = get_shape();
	omt_PolygonList *polys = shape->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		m->set_color(OMediaFRGBColor(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.0f, 0.0f);	// emission 1, the rest 0
		if (!g_sky_alt) continue;
		std::string name;
		m->get_chunk_name(name);							// "" without a chunk (0x545f14)
		const char *alt_name;
		if (name.compare("sky1") == 0) alt_name = "sky1alt";
		else if (name.compare("sky2") == 0) alt_name = "sky2alt";
		else if (name.compare("sky3") == 0) alt_name = "sky3alt";
		else continue;
		OMediaCanvas *c = LoadCanvasName(g_sky_db, alt_name);	// 0x4b2750
		if (c) m->set_texture(c);							// material vtable +0x20
	}
}

namespace neg {
CGameObject *New_C3DSky(void)	{ return new C3DSky; }						// factory '3SKY' (0x594 bytes)
}
