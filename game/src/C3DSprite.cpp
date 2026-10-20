/*  C3DSprite - see C3DSprite.h. Also the sprite canvas loaders 0x4b2600 / 0x4b2750. */
#include "C3DSprite.h"
#include "Game.h"
#include "Todo.h"
#include "gen/globals.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include "OMediaCanvas.h"
#include "OMediaDataBase.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaStreamOperators.h"
#include <string.h>

using namespace neg;

// --------------------------------------------------------------------------- not reconstructed yet

static void PhysicsAddObject(CGameObject *o)				// 0x4b4ae0
{
	(void)o; NEG_TODO(0x4b4ae0, "CPhysics::AddObject");
}

// 0x4a4180: the level's database cache (list 0x56aef4): returns the open database of
// "omt\<name>" (falls back to "c:\omt\<name>"), opening and caching it on first use;
// "Icons.omt" goes to the permanent cache (0x4a4560) instead.
static OMediaDataBase *GetDatabase(const char *name)
{
	(void)name; NEG_TODO(0x4a4180, "GetDatabase (level database cache 0x56aef4)"); return NULL;
}

// 0x4a4560: the same for the databases kept over level changes (list 0x56aee4).
static OMediaDataBase *GetPermanentDatabase(const char *name)
{
	(void)name; NEG_TODO(0x4a4560, "GetPermanentDatabase (permanent database cache 0x56aee4)"); return NULL;
}

// The game's paths are DOS-style and relative (".\omt\x.omt"), as in Alpha.cpp.
static std::string port_path(const std::string &dos)
{
	std::string s(dos);
	if (s.compare(0, 2, ".\\") == 0) s.erase(0, 2);
	for (size_t i = 0; i < s.size(); i++) if (s[i] == '\\') s[i] = '/';
	return s;
}

// 0x4b2600: a canvas stored in a file of its own. As in the original, errors (no file) are not
// caught here: OMT's exception goes to the caller.
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

// 0x4b2750: a canvas chunk of a database, by name (LoadCanvasID 0x4b2860 is the same by id).
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

C3DSprite::C3DSprite()										// 0x49c6a0
{
	SetName((char *)"C3DSPRITE");
	Log("C3DSprite()");
	m_visible = 1;
	C3DSprite::InitObject();								// inlined in the original: properties
															// are registered here and again by the
															// loader's InitObject (as shipped)
	OMediaElement::set_id('3SPR');							// element +0x78
	m_object_id = '3SPR';									// CGameObject +0x404
	m_sprite_size = 100;
	m_sprite_index = 4;
	strcpy(m_sprite_db, "icons.omt");
}

C3DSprite::~C3DSprite()										// 0x49c890
{
	Log("~C3DSprite()");
}

// --------------------------------------------------------------------------- OMT virtuals

void C3DSprite::update_logic(float millisecs_elapsed)		// 0x49c940
{
	OMediaCanvasElement::update_logic(millisecs_elapsed);
}

void C3DSprite::read_class(OMediaStreamOperators &stream)	// 0x45d880
{
	OMediaCanvasElement::read_class(stream);
}

void C3DSprite::write_class(OMediaStreamOperators &stream)	// 0x45d890
{
	OMediaCanvasElement::write_class(stream);
}

// --------------------------------------------------------------------------- element-family slots

void C3DSprite::SetObjectID(int id)							// 0x49b550 (shared with C3DPolygon)
{
	OMediaElement::set_id(id);
	m_object_id = id;
}

void C3DSprite::InitSpriteName(OMediaDataBase *db, std::string name, float size)	// 0x49ccb0
{
	set_canvas(LoadCanvasName(db, name));					// (the string goes by value: a copy)
	InitSpriteProperties(size);
}

void C3DSprite::InitSpriteID(OMediaDataBase *db, int id, float size)	// 0x49cc50
{
	Log("InitSprite(%d,%f)", id, (double)size);
	set_canvas(LoadCanvasID(db, id));
	// QUIRK: the original passes the raw bits of `id`, not `size`, to InitSpriteProperties
	// (it pushes the second argument again), so sprites set up through here get a tiny
	// (denormal) world size.
	float bits;
	memcpy(&bits, &id, sizeof bits);
	InitSpriteProperties(bits);
	(void)size;
}

void C3DSprite::InitSpriteFile(std::string file, float size)	// 0x49cac0
{
	Log("InitSprite(\"%s\",%f)", file.c_str(), (double)size);
	if (file.length() > 0) set_canvas(LoadCanvasFile(file));
	InitSpriteProperties(size);
}

void C3DSprite::InitSpriteProperties(float size)			// 0x49ce10
{
	Log("InitSpriteProperties(%f)", (double)size);
	// +0x7c = 0x248 (assumes OMT 2002 used the 2.5 flag values)
	set_flags(omelf_SecondPass | omelf_DisableRotate | omelf_FaceViewport);
	set_canvas_flags(omcanef_FreeWorldSize);				// +0xc4 = 1
	set_size(size, size);									// +0xac, +0xb0
	set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);	// +0xb4 = 6, +0xb8 = 7
	set_auto_align(omaac_Center, omaac_Center);				// +0xbc, +0xc0 = 3
	if (g_windowed) set_diffuse(OMediaFARGBColor(1.0f, 0.9f, 0.9f, 0.9f));	// 0x56afab: sic, dimmer in a window
}

void C3DSprite::ShowAgain(void)								// 0x49d060
{
	m_visible = 1;
	force_show();											// hide_count = 1; hide(false)
}

void C3DSprite::HideAgain(void)								// 0x49d080
{
	if (m_visible)
	{
		m_visible = 0;
		OMediaElement::hide(true);
	}
}

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DSprite::InitObject(void)							// 0x49c950
{
	Log("InitObject()");
	CLocalGameObject::InitObject();
	link(g_world);											// element +0x2c
	link_layer(g_layer_world);								// element +0x34 (0x56afc8)
	RegisterProperty("SpriteSize",     &m_sprite_size,  PROP_LONG,  0);
	RegisterProperty("SpriteDatabase", m_sprite_db,     PROP_ALPHA, 0);
	RegisterProperty("SpriteIndex",    &m_sprite_index, PROP_LONG,  0);
}

void C3DSprite::UnInitObject(void)							// 0x49c9f0
{
	Log("UnInitObject()");
	CLocalGameObject::UnInitObject();
	set_canvas(NULL);										// element +0xac
}

void C3DSprite::InitPhysics(void)							// 0x49ca30
{
	CLocalGameObject::InitPhysics();
	v90(x, y, z);											// position from the element (+0x44..)
	const float k = 360.0f / 16384.0f;						// 0x4cb1f8: OMT angle units -> degrees
	v92((float)anglex * k, (float)angley * k, (float)anglez * k);
	PhysicsAddObject(this);
}

void C3DSprite::Process(float dt)							// 0x49cfa0
{
	CLocalGameObject::Process(dt);
}

void C3DSprite::v247(void)									// 0x49cee0
{
	float p[4];
	v195(p);												// CGameObject +0x30c: current position
	x = p[0]; y = p[1]; z = p[2];
}

void C3DSprite::v248(void)									// 0x49cf20
{
	CGameObject::v248();
	float a[4];
	v201(a);												// CGameObject +0x324: current angles (degrees)
	const float k = 16384.0f / 360.0f;						// 0x4cb228: degrees -> OMT angle units
	anglex = (short)((int)(a[0] * k) & 0x3fff);
	angley = (short)((int)(a[1] * k) & 0x3fff);
	anglez = (short)((int)(a[2] * k) & 0x3fff);
}

void C3DSprite::v258(void)									// 0x4735a0
{
	CLocalGameObject::v258();
	LoadSprite(false);										// (inlined)
}

void C3DSprite::v260(void)									// 0x49cfd0
{
	LoadSprite(false);										// (inlined)
}

// Inlined in v258/v260 here and in C3DSpriteType / C3DPermanentSprite (which take their
// databases from the permanent cache): the canvas SpriteIndex of SpriteDatabase at SpriteSize,
// else canvas 3 of the default icons.
void C3DSprite::LoadSprite(bool permanent)
{
	OMediaDataBase *db = permanent ? GetPermanentDatabase(m_sprite_db) : GetDatabase(m_sprite_db);
	if (!db)
	{
		db = permanent ? GetPermanentDatabase("permanenticons.omt") : GetDatabase("icons.omt");
		if (db) InitSpriteID(db, 3, (float)m_sprite_size);	// element +200
	}
	else
	{
		InitSpriteID(db, m_sprite_index, (float)m_sprite_size);
		OMediaCanvas *c = get_canvas();						// element +0x8c
		if (c) c->set_filtering(omtfc_Linear, omtfc_Linear);	// canvas +0x60, +0x64 = 1, +0x24 vtable +0x10
	}
}
