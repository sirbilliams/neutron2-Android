/*  Texture animations - see TextureAnims.h. Reconstructed from Neutron2.exe 0x4a23e0 (scan a
    shape's textures), 0x4a25e0 (add one), 0x4a28c0 (play), 0x4a2270 (free), the list at 0x56ad78;
    and 0x490300 / 0x4904d0 with the list at 0x54a4c8 for the "m-" materials.                 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "TextureAnims.h"
#include "Todo.h"
#include "gen/globals.h"
#include "OMediaDataBase.h"
#include "OMediaCanvas.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#pragma GCC diagnostic pop
#include <list>
#include <string>

using namespace neg;

namespace {

// 0x2c bytes in the original: the texture's name (+0x00), the material (+0x10), the frames
// (+0x14, the material's own texture first), the time in the frame (+0x20), the frame time
// (+0x24) and the frame shown (+0x28).
struct TextureAnim
{
	std::string							name;
	OMedia3DMaterial					*material;
	std::list<OMediaCanvas *>			frames;
	float								time;
	float								frame_time;
	std::list<OMediaCanvas *>::iterator	current;
};
std::list<TextureAnim *> g_texture_anims;				// 0x56ad78

// The "m-" list (0x54a4c8): the same record with materials as frames; filled, never read.
struct MaterialAnim
{
	std::string							name;
	OMedia3DMaterial					*material;
	std::list<OMedia3DMaterial *>		frames;
	float								time;
	float								frame_time;
	std::list<OMedia3DMaterial *>::iterator	current;
};
std::list<MaterialAnim *> g_material_anims;				// 0x54a4c8

// The name of a database object's chunk ("" without one; 0x545f14 is an empty string).
std::string ChunkName(OMediaDBObject *o)
{
	std::string n;
	o->get_chunk_name(n);
	return n;
}

// The digit of frame `i` goes in the third character. (The original writes it at ptr + 2 even
// in a two-character name, over the terminator, where the length keeps it out of the copies.)
void SetFrameDigit(std::string &name, int i)
{
	if (name.size() > 2) name[2] = (char)('0' + i);
}

// 0x4b2750: a canvas chunk of a database, by name (the same code as C3DSprite.cpp's copy).
OMediaCanvas *LoadCanvasName(OMediaDataBase *db, std::string name)
{
	OMediaCanvas *c = NULL;
	if (db && db->object_exists(OMediaCanvas::db_type, name))
	{
		c = (OMediaCanvas *)db->get_object(OMediaCanvas::db_type, name);
		if (g_56afaa) NEG_TODO(0x4b2750, "debug database call (db vtable +0x4c)");
		if (c) c->db_unlock();
	}
	return c;
}

// 0x4b2b10: a material chunk by name. Quirk: the test is made for a canvas of that name.
OMedia3DMaterial *LoadMaterialName(OMediaDataBase *db, std::string name)
{
	OMedia3DMaterial *m = NULL;
	if (db && db->object_exists(OMediaCanvas::db_type, name))
	{
		m = (OMedia3DMaterial *)db->get_object(OMedia3DMaterial::db_type, name);
		if (g_56afaa) NEG_TODO(0x4b2b10, "debug database call (db vtable +0x4c)");
		if (m) m->db_unlock();
	}
	return m;
}

// 0x4a25e0: animate `material`'s texture, unless its name already has an animation.
void AddTextureAnim(OMediaDataBase *db, OMedia3DMaterial *material, float frame_time)
{
	std::string name = ChunkName(material->get_texture());
	for (std::list<TextureAnim *>::iterator i = g_texture_anims.begin(); i != g_texture_anims.end(); ++i)
		if ((*i)->name.compare(0, (*i)->name.size(), name) == 0) return;

	TextureAnim *a = new TextureAnim;
	a->name = name;
	a->frames.push_back(material->get_texture());
	a->time = 0;
	a->frame_time = frame_time;
	a->material = material;
	a->current = a->frames.begin();
	for (int i = 1; i < 10; i++)
	{
		SetFrameDigit(name, i);
		if (!db->object_exists(OMediaCanvas::db_type, name)) break;		// db vtable +0x1c
		OMediaCanvas *c = LoadCanvasName(db, name);
		c->set_filtering(omtfc_Linear, omtfc_Linear);	// +0x60 / +0x64 = 1, then delete_imp_slaves
		a->frames.push_back(c);
	}
	g_texture_anims.push_back(a);
}

// 0x4904d0: the same for a "m-" material, its frames materials (0x4b2b10).
void AddMaterialAnim(OMediaDataBase *db, OMedia3DMaterial *material, float frame_time)
{
	std::string name = ChunkName(material);
	for (std::list<MaterialAnim *>::iterator i = g_material_anims.begin(); i != g_material_anims.end(); ++i)
		if ((*i)->name.compare(0, (*i)->name.size(), name) == 0) return;

	MaterialAnim *a = new MaterialAnim;
	a->name = name;
	a->frames.push_back(material);
	a->time = 0;
	a->frame_time = frame_time;
	a->material = material;
	a->current = a->frames.begin();
	for (int i = 1; i < 10; i++)
	{
		SetFrameDigit(name, i);
		if (!db->object_exists(OMediaCanvas::db_type, name)) break;
		a->frames.push_back(LoadMaterialName(db, name));
	}
	g_material_anims.push_back(a);
}

} // namespace

// 0x4a23e0: called by Load3DShape for every shape it loads.
void neg::AddShapeTextureAnims(OMediaDataBase *db, OMedia3DShape *shape)
{
	omt_PolygonList *polys = shape->get_polygons();			// (no NULL test in the original)
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m || !m->get_texture()) continue;
		std::string name = ChunkName(m->get_texture());
		if (name.size() < 2 || name[0] != 'a' || name[1] != '-') continue;
		float t = m->get_ambient_r();						// +0x6c (grey ambient: 0.03..0.23 s in Level3/5)
		if (t == 0.0f) t = 0.5f;							// 0x4caa44
		AddTextureAnim(db, m, t);							// 0x4a25e0
	}
}

// 0x490300: called by Load3DShape after 0x4a23e0.
void neg::AddShapeMaterialAnims(OMediaDataBase *db, OMedia3DShape *shape)
{
	omt_PolygonList *polys = shape->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		std::string name = ChunkName(m);
		if (name.size() < 2 || name[0] != 'm' || name[1] != '-') continue;
		AddMaterialAnim(db, m, 0.5f);						// 0x4904d0
	}
}

// 0x4a28c0: every frame while the game runs (CViewPort::render_viewport).
void neg::UpdateTextureAnims(float dt)
{
	for (std::list<TextureAnim *>::iterator i = g_texture_anims.begin(); i != g_texture_anims.end(); ++i)
	{
		TextureAnim *a = *i;
		a->time += g_enemy_time_scale * dt;					// 0x540594
		if (a->frame_time <= a->time)
		{
			a->time -= a->frame_time;
			if (++a->current == a->frames.end()) a->current = a->frames.begin();
			a->material->set_texture(*a->current);			// material vtable +0x20
		}
	}
}

// 0x4a2270: CGameType::InitObject / UnInitGame. The records go, the canvases stay in their
// databases. (The "m-" list is not touched.)
void neg::ResetTextureAnims(void)
{
	// (debug log "Reset()")
	while (!g_texture_anims.empty())
	{
		TextureAnim *a = g_texture_anims.front();
		g_texture_anims.pop_front();
		delete a;											// 0x4a2340
	}
}
