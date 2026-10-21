/*  C3DAnimated - see C3DAnimated.h. */
#include "C3DAnimated.h"
#include "C3DGrannyObj.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "CGameType.h"
#include "CEditor.h"
#include "gen/globals.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include "OMediaViewPort.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DMorphAnim.h"
#include "OMedia3DShapeConverter.h"
#include "OMediaCanvas.h"
#include "OMediaDataBase.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaStreamOperators.h"
#include "OMediaStringField.h"
#include <string.h>
#include <strings.h>

using namespace neg;

std::list<ModelInfo_t *> neg::g_granny_models;				// 0x545f20

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// The original reads C3DPickupItem's member +0x6d8 (game-object offset; object +0x798), the
// item's index into g_object_states. C3DPickupItem is not reconstructed yet.
static int PickupItemIndex(CGameObject *item)
{
	(void)item;
	NEG_TODO(0x41c203, "C3DPickupItem +0x6d8 (pickup index) for PickupLink");
	return -1;
}

static void SetAllBlend(OMedia3DShape *sh, omt_BlendFunc src, omt_BlendFunc dst)
{
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
		if ((*p).get_material()) (*p).get_material()->set_blend(src, dst);
}

// --------------------------------------------------------------------------- construction

C3DAnimated::C3DAnimated()									// 0x41a690
{
	SetName((char *)"C3DANIMATED");
	Log("C3DAnimated()");
	f_59c = -1;
	f_5a4 = NULL;
	f_640 = NULL;
	f_5a0 = 0;
	f_5a8 = 0;
	f_594 = -1;
	strcpy(f_5fc, g_545f14);
	f_598 = 0;
	for (int i = 0; i < 10; i++) { f_5ac[i] = NULL; f_5d4[i] = NULL; }
	f_63c = 0;
	f_648 = 1;
	f_649 = 1;
	f_64c = -1;												// RequiredLevel
	f_650 = -1;												// ExactLevel
	f_654 = -1;												// RemoveLevel
	f_658 = -1;												// HasCollision
	f_65c = -1;												// InitiallyVisible
	f_660 = 1;												// CanMove
	f_664 = 0;												// SecondPass
	f_668 = 0;
	strcpy(f_669, "none");									// PickupLink
	f_6cd = 0;
	f_6d0 = 0;
	f_884 = 1;												// CanRotate
	f_88c = 0;												// FaceViewport
	f_6d4.clear();
	f_6e0[0] = f_6e0[1] = 0;
	f_6e8[0] = f_6e8[1] = 0;
	memset(f_6f4, 0, sizeof(f_6f4));						// 4 x 25 dwords
	f_6f0 = 1.0f;
	g_545f30 = 0;											// debug line counter of ev80
	f_898 = 0;
	f_8b0 = 1;
	f_894 = 0;
	f_89c = 0;												// Translucent
	f_8a0 = 0;												// Lit
	f_8a4 = 0;
	f_8a5 = 0;
	f_8a6 = 0;
	f_8a7 = 0;
	f_8a8 = 0;
	f_8ac = 1;												// CanClip
	f_891 = 0;
	f_890 = 0;
	strcpy(f_8b1, "none");
	f_918 = NULL;
	f_91c = NULL;
	f_920 = 0;
	f_58c = 0;
	f_590 = NULL;											// port: not set by the original until InitAnim
	f_888 = NULL;											// port: idem
}

C3DAnimated::~C3DAnimated()									// 0x41a9f0
{
	if (f_918) { delete f_918; f_918 = NULL; }				// vtable at +0x24, slot 0 (deleting)
	if (f_91c) { delete f_91c; f_91c = NULL; }
	f_6d4.clear();											// the AnimInfo_t themselves are not freed (as shipped)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DAnimated::InitObject(void)							// 0x41b0a0
{
	C3DObject::InitObject();
	RegisterProperty("RequiredLevel", &f_64c, PROP_LONG, 0);
	RegisterProperty("ExactLevel", &f_650, PROP_LONG, 0);
	RegisterProperty("RemoveLevel", &f_654, PROP_LONG, 0);
	RegisterProperty("HasCollision", &f_658, PROP_LONG, 0);
	RegisterProperty("InitiallyVisible", &f_65c, PROP_LONG, 0);
	RegisterProperty("CanMove", &f_660, PROP_LONG, 0);
	RegisterProperty("SecondPass", &f_664, PROP_LONG, 0);
	RegisterProperty("PickupLink", f_669, PROP_ALPHA, 0);
	RegisterProperty("CanRotate", &f_884, PROP_LONG, 0);
	RegisterProperty("FaceViewport", &f_88c, PROP_LONG, 0);
	RegisterProperty("Translucent", &f_89c, PROP_FLOAT, 0);
	RegisterProperty("Lit", &f_8a0, PROP_FLOAT, 0);
	RegisterProperty("CanClip", &f_8ac, PROP_LONG, 0);
	if (f_6d0 == 1)
	{
		OMedia3DShape *sh = new OMedia3DShape;				// (debug log "OMedia3DShape " + tag, empty)
		set_shape(sh);										// element +0xac
	}
}

void C3DAnimated::UnInitObject(void)						// 0x41ab10
{
	if (f_6d0 == 1)
	{
		OMedia3DShape *sh = get_shape();
		set_shape(NULL);
		if (sh) delete sh;									// vtable at +0x24, slot 0 (deleting)
	}
	if (f_4bc && f_6d0 == 0)
	{
		set_anim_def(NULL);									// element +0xc0
		for (int i = 0; i < 10; i++)
		{
			if (f_5ac[i]) { delete f_5ac[i]; f_5ac[i] = NULL; }
			if (f_5d4[i]) { delete f_5d4[i]; f_5d4[i] = NULL; }
		}
		// Quirk: frees every AnimList but the last one, and f_5a4 keeps pointing at the first.
		AnimList *a = f_5a4;
		if (a && a->next)
		{
			do
			{
				AnimList *dead = a;
				a = a->next;
				delete dead;
			} while (a->next);
		}
	}
	if (f_6d0 == 1)
	{
		if (f_6e8[1]) GrannyCloseSequence(f_6e8[0], f_6e8[1]);
		while (f_6d4.size() != 0)
		{
			AnimInfo_t *ai = f_6d4.front();
			GrannyReleaseControlledAnimation(ai->control[0], ai->control[1], ai->control[2]);
			GrannyCloseAnimation(ai->anim[0], ai->anim[1]);
			f_6d4.pop_front();
			delete ai;
		}
		if (f_6e0[1])
		{
			for (std::list<ModelInfo_t *>::iterator i = g_granny_models.begin(); i != g_granny_models.end(); ++i)
			{
				ModelInfo_t *mi = *i;
				if (mi->model[1] != f_6e0[1]) continue;
				if (--mi->refs == 0)
				{
					GrannyCloseModel(f_6e0[0], f_6e0[1]);
					g_granny_models.remove(mi);
					delete mi;
					return;
				}
			}
		}
	}
}

void C3DAnimated::Process(float dt)							// 0x41c120
{
	float v[4];
	if (g_gametype->IsPaused())								// vtable +0x114
	{
		if (f_660 != 0) return;
		v88(v);	v196(v[0], v[1], v[2], v[3]);				// back to the start position / rotation
		v91(v);	v202(v[0], v[1], v[2], v[3]);
		return;
	}
	if (!f_6cd)
	{
		f_6cd = 1;
		CGameObject *item;
		if (strcasecmp(f_669, "none") != 0 && (item = FindObjectByTag(f_669)) != NULL &&
			item->NameContains((char *)"C3DPICKUPITEM"))
		{
			const int idx = PickupItemIndex(item);
			if (idx >= 0 && g_object_states[idx] != 0)		// port: idx < 0 = not known yet
			{
				HideAgain();
				v232();
				v132(0);
			}
		}
	}
	C3DObject::Process(dt);
	if (f_660 == 0)											// CanMove 0
	{
		v88(v);	v196(v[0], v[1], v[2], v[3]);
		v91(v);	v202(v[0], v[1], v[2], v[3]);
	}
	if (f_884 == 0)											// CanRotate 0
	{
		v91(v);	v202(v[0], v[1], v[2], v[3]);
	}
	if (f_8ac == 1)											// CanClip
	{
		f_8a8 = ev93();
		if (f_8a8) return;
	}
	if (f_890 && f_8b0 && f_898 > 0.0f)						// fading out
	{
		f_898 -= dt;
		if (f_898 <= 0.0f)
		{
			f_890 = 0;
			HideAgain();
		}
		else
		{
			ShowAgain();
			if (f_89c > 0.0f) ev71(f_898 / f_894 * f_89c);
			else if (f_8a0 > 0.0f) ev72(f_898 / f_894 * f_8a0);
			else { ev71(f_898 / f_894); ev91(1); }
		}
	}
	if (f_891)												// fading in
	{
		// (debug log "test" for the object tagged C3DHERMAN, empty)
		if (g_545ef8)
		{
			f_898 = 1.0e-5f;								// 0x3727c5ac
			g_545ef8 = 0;
		}
		if (f_8b0 && f_898 > 0.0f)
		{
			f_898 -= dt;
			if (f_898 <= 0.0f)
			{
				f_891 = 0;
				ShowAgain();
				if (f_89c == 0.0f) { ev71(0.0f); ev91(0); }
			}
			else
			{
				ShowAgain();
				if (f_89c > 0.0f) { ev71((1.0f - f_898 / f_894) * f_89c); ev91(1); }
				else if (f_8a0 > 0.0f) ev72((1.0f - f_898 / f_894) * f_8a0);
				else { ev71(1.0f - f_898 / f_894); ev91(1); }
			}
		}
	}
	if (f_648 && f_649)
	{
		if (f_6d0 == 0)
		{
			f_598 += dt;
			if (!getplay_loop() && f_640 && getcurrentframe_pos() >= f_640->nframes - 1)	// OMediaAnim +0xad, vtable +0x1c
			{
				ev67();
				f_598 = 0;
				return;
			}
		}
		else if (f_6d0 == 1)
		{
			if (f_8a4) { ev80(); return; }
			if (!f_8a5) ev80();
		}
	}
}

void C3DAnimated::v242(float t)								// 0x41b070
{
	(void)t;
	ShowAgain();
	ev80();
}

void C3DAnimated::v260(void)								// 0x41cec0
{
	if (f_65c == 0) HideAgain();
	else if (f_65c == 1) ShowAgain();
	if (f_88c == 1) set_flags(get_flags() | omelf_FaceViewport);	// element +0x7c |= 0x200
	if (f_89c > 0.0f) ev71(f_89c);
	if (f_8a0 > 0.0f) ev72(f_8a0);
	if (f_658 == 1) v234();
	if (f_658 == 0) v136(0);
	if (f_664 == 1) ev91(1);
	if (f_8ac == 0) f_8a8 = 0;
}

void C3DAnimated::v267(int level)							// 0x41c880
{
	CLocalGameObject::v267(level);
	if (f_650 == -1)
	{
		if (f_64c == -1) return;
		if (f_64c <= level)
		{
			if (f_654 == -1) { ev70(1); return; }
			if (level < f_654) { ev70(1); return; }
		}
	}
	else if (level == f_650)
	{
		ev70(1);
		return;
	}
	ev70(0);
}

void C3DAnimated::v268(int state)							// 0x41c710
{
	CLocalGameObject::v268(state);
	// (debug log "Calling NextState for %s at %d", empty)
	if (state == 1)
	{
		if (f_658 != 0) v234();
		ShowAgain();
	}
	else if (state == 0)
	{
		v232();
		HideAgain();
	}
}

void C3DAnimated::v270(int a, int msg)						// 0x41e960
{
	CLocalGameObject::v270(a, msg);
	if (msg != 5 || strcasecmp(m_tag, "C3DHERMAN") != 0) return;
	if (f_891)
	{
		f_898 = 0;
		f_891 = 0;
		ShowAgain();
		if (f_89c <= 0.0f) { ev71(1.0f); ev91(0); }
		else ev71(f_89c);
	}
	if (f_890)
	{
		f_894 = 0;
		f_898 = 0;
		f_890 = 0;
		ShowAgain();
	}
}

void C3DAnimated::v277(void) { ev79(1); }					// 0x41ce80
void C3DAnimated::v278(void) { ev79(0); }					// 0x41cea0

void C3DAnimated::v280(void)								// 0x41e460: the editor lists our animation names
{
	if (!g_editor) return;
	int i = 0;
	if (f_6d0 == 0)
	{
		if (!f_649 || !f_648) return;
		for (AnimList *a = f_5a4; a; a = a->next, i++)
		{
			OMediaStringField *f = g_editor->m_list_fields[i];	// editor +0x28c
			f->set_string(std::string(a->name));			// vtable +0xc8
			f->force_show();
		}
	}
	else
	{
		for (std::list<AnimInfo_t *>::iterator a = f_6d4.begin(); a != f_6d4.end(); ++a, i++)
		{
			OMediaStringField *f = g_editor->m_list_fields[i];
			f->set_string(std::string((*a)->name));
			f->force_show();
		}
	}
}

void C3DAnimated::v282(OMediaMatrix_4x4 &m, float *w, float *h)	// 0x41cfd0
{
	get_layer()->prepare_projection_matrix(0, m, *w, *h);	// element +0x64 its_layer, vtable +0xc
}

// --------------------------------------------------------------------------- morph animations

void C3DAnimated::CreateAnim(char *name, char *file)		// 0x41b360
{
	if (f_6d0 != 0)
	{
		char path[112];
		strcpy(path, file);
		ev87(path, (char *)"grn", (char *)"grn");			// only checks: the result is not used
		AddAnimation(std::string(file), name);
		return;
	}
	AnimList *a;
	bool first = (f_5a4 == NULL);
	if (first)
	{
		// (debug log "CreateAnim1 : %s   ", empty)
		a = f_5a4 = new AnimList;
	}
	else
	{
		AnimList *last = f_5a4;
		while (last->next) last = last->next;
		a = last->next = new AnimList;
	}
	if (!a)
	{
		// (debug log "Unable to allocate AnimList in Anim3D_CreateAnim", empty)
		return;
	}
	strcpy(a->name, g_545f14);
	a->next = NULL;
	a->id = f_5a0;
	char path[112];
	strcpy(path, file);
	ev87(path, (char *)"ase", (char *)"ase");				// path = ase\<file> if it exists
	OMediaFilePath fp(std::string(path), 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	if (!fs.fileexists())
	{
		ErrorLog("ERROR: file %s does not exist for %s", file, m_tag);
		fs.close();
		return;
	}
	fs.open(omcfp_Read, false, false);
	// first animation: a new morph definition, the next ones add a sequence to it
	OMedia3DShapeConverter::set_anim_conv_mode_mode(first ? omc3danconv_AlwaysCreateNewMorphAnimDef : omc3danconv_AddSequence);
	OMedia3DShapeConverter::set_object_convert_mode(omc3doconv_OneShapePerGroup);
	m_db->generate_from_stream(fs, f_590);					// database vtable +0x2c
	if (first) m_db->set_dbsize_dirty();					// +0x4c
	fs.close();
	// The original reads related_morphdefs[0] whether or not this file made a morph definition.
	// A static .ase (no *MESH_ANIMATION, e.g. the intro logo letters) makes none; the converter
	// only erased the vector, and MSVC 6 keeps the old element in place, so the original picks
	// up the id the previous animated file left there (another object's definition).
	// port: keep that id ourselves instead of reading past the end of an empty vector, and do
	// nothing if no animated file was ever converted (the original would crash).
	static omt_FSChunkID last_morph_def = 0;
	if (!OMedia3DShapeConverter::related_morphdefs.empty())
		last_morph_def = OMedia3DShapeConverter::related_morphdefs[0].morph_def;
	OMedia3DMorphAnimDef *def = last_morph_def == 0 ? NULL :
		(OMedia3DMorphAnimDef *)m_db->get_object(OMedia3DMorphAnimDef::db_type,	// +0x14
		last_morph_def);									// port: 2002 kept the chunk ids only
	a->def = def;
	if (def) def->db_unlock();							// port: NULL check
	f_5a0++;
	g_545f2c++;
	if (strlen(name) > 0x3f)
	{
		// (debug log "Truncated Animname %s in Anim3D_CreateAnim", empty)
		name[0x3f] = 0;										// (writes into the caller's string)
	}
	strcpy(a->name, name);
}

AnimList *C3DAnimated::GetSequence(char *name)				// 0x41b8d0
{
	if (f_6d0 == 0)
	{
		if (f_649 && f_648)
			for (AnimList *a = f_5a4; a; a = a->next)
				if (strcasecmp(name, a->name) == 0) return a;
	}
	else
		ErrorLog("ERROR: Anim3D_GetSequence should never be called for granny system %s", m_tag);
	return NULL;
}

void C3DAnimated::ev58(char *name, unsigned char loop)		// 0x41bea0
{
	if (f_8a7 || !f_648 || !f_649) return;
	char buf[64];
	if (f_6d0 == 0)
	{
		f_598 = 0;
		strcpy(buf, g_545f14);
		const float r = v151();								// the shape change below resets the radius
		OMedia3DShapeElement::set_shape(f_590);
		FitToShape();										// element +0xc8
		if (f_124 != 2) v152(r);							// not a box: keep the radius
		strcpy(buf, g_545f14);
		strcpy(f_5fc, name);
		strcat(buf, name);
		AnimList *a = GetSequence(buf);
		if (!a) return;
		f_640 = a;
		strcpy(f_5fc, name);
		SetSequence(a->id, loop);
		set_anim_def(a->def);								// element +0xc0
		f_888 = current_frame;								// OMediaAnim +0x98 (2002)
	}
	else
	{
		strcpy(buf, g_545f14);
		strcpy(f_5fc, name);
		strcat(buf, name);									// (not used)
		ev86(name, NULL, NULL, NULL, loop, 0.2f);
	}
}

void C3DAnimated::SetSequence(int seq, unsigned char loop)	// 0x41b940
{
	if (!f_649) return;
	if (f_6d0 != 0)
	{
		ErrorLog("ERROR: Anim3D_SetSequence(long) should never be called for granny system %s", m_tag);
		return;
	}
	if (seq < 0) return;
	f_59c = seq;
	setcurrentsequence(seq, loop != 0);						// OMediaAnim +0x90 vtable +0x10
	setcurrentframe(current_sequence, 0);					// +0x14
	setplay_loop(loop != 0);								// +0xad
	setplay_timebased(true);								// +0x24
	AnimList *a = f_640;
	OMediaAnimDef *def = a ? a->def : NULL;
	if (a && def && def->getnsequences() != 0)
	{
		omt_FrameList *frames = (*def->getsequence_list())[current_sequence];
		a->nframes = (int)frames->size();					// list +8
		// port: the original stores the value of the frame list's sentinel node here (MSVC6
		// list head +8, never initialised); nothing reads it back
		f_888 = NULL;
	}
}

char *C3DAnimated::GetAnim(void)							// 0x41ba20
{
	if (f_649 && f_648)
	{
		if (f_6d0 == 0)
		{
			for (AnimList *a = f_5a4; a; a = a->next)
				if (a->id == f_59c) return a->name;
			// (debug log "Animation  not found in Anim3D_GetAnim for %s", empty)
			return g_545f14;
		}
		return f_6f4[0].name;
	}
	return g_545f14;
}

int C3DAnimated::ev62(void) { return f_5a0; }				// 0x41ba90

unsigned char C3DAnimated::IsAnim(char *name)				// 0x41c6d0
{
	const char *cur;
	if (f_6d0 == 0) cur = f_5fc;
	else if (f_6d0 == 1) cur = GetAnim();					// element +0xf4
	else return 0;
	return strcasecmp(name, cur) == 0;
}

void C3DAnimated::CreateTexture(char *file, short index)	// 0x41baa0
{
	if (f_6d0 != 0)
	{
		ErrorLog("ERROR: Anim3D_CreateTexture should not be called for granny system %s", m_tag);
		return;
	}
	char path[112];
	strcpy(path, "png\\");
	strcat(path, file);
	OMediaFilePath fp(std::string(path), 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	fs.open(omcfp_Read, false, false);						// (no fileexists test: OMT throws)
	OMediaCanvas *c = new OMediaCanvas;
	f_5ac[index] = c;										// (debug log "OMediaCanvas 2 " + tag, empty)
	fs >> (OMediaClassStreamer *)c;
	fs.close();
	c->set_filtering(omtfc_Nearest, omtfc_Linear);			// canvas +0x60 = 0, +0x64 = 1
	OMedia3DMaterial *m = new OMedia3DMaterial;
	f_5d4[index] = m;										// (debug log "OMedia3DMaterial 2 " + tag, empty)
	m->set_blend(omblendfc_One, omblendfc_Zero);			// +0x30 = 1, +0x34 = 0
	m->set_texture(c);										// vtable +0x20
}

void C3DAnimated::ApplyTexture(OMedia3DShape *shape, short index)	// 0x41be50
{
	if (f_6d0 == 0)
	{
		if (shape) shape->set_material(f_5d4[index]);		// shape vtable +0x70
	}
	else
		ErrorLog("ERROR: Anim3D_ApplyTexture was called using granny system for %s", m_tag);
}

void C3DAnimated::ev65(short index, int src, int dst)		// 0x41bdc0
{
	if (f_6d0 != 0)
	{
		OMedia3DShape *sh = get_shape();
		if (!sh) return;
		omt_PolygonList *polys = sh->get_polygons();
		for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
		{
			OMedia3DMaterial *m = (*p).get_material();
			if (!m) continue;
			if (!g_windowed) m->set_blend((omt_BlendFunc)src, (omt_BlendFunc)dst);
			else m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		}
		return;
	}
	f_5d4[index]->set_blend((omt_BlendFunc)src, (omt_BlendFunc)dst);
}

char *C3DAnimated::ev66(void)								// 0x41c0f0
{
	if (f_6d0 == 0) return f_5fc;
	if (f_6d0 == 1) return f_6f4[0].name;
	return g_545f14;
}

void C3DAnimated::InitAnim(void)							// 0x41c790
{
	if (f_6d0 != 0)
	{
		// (debug log "Error: Invalid InitAnim for granny %s", empty)
		return;
	}
	if (m_db) return;
	OpenObjectDatabase();									// CGameObject vtable +0x448
	OMediaDataBase::register_object(OMediaCanvas::db_type, OMediaCanvas::db_builder);
	OMediaDataBase::register_object(OMedia3DShape::db_type, OMedia3DShape::db_builder);
	OMediaDataBase::register_object(OMedia3DMaterial::db_type, OMedia3DMaterial::db_builder);
	OMediaDataBase::register_object(OMedia3DMorphAnimDef::db_type, OMedia3DMorphAnimDef::db_builder);
	f_594 = 0;
	OMedia3DShape *sh = (OMedia3DShape *)m_db->get_object(OMedia3DShape::db_type, (omt_FSChunkID)0);
	f_590 = sh;
	f_58c = 0;
	set_morph_flags(ommorf_InterpolateVertices | ommorf_InterpolateVNormals);	// element +0xb8 = 3
	sh->set_modified(false);								// +8
	f_590->db_unlock();										// +0x14
	f_63c = 1;
	ev69();
}

void C3DAnimated::ev70(unsigned char show)					// 0x41c920
{
	if (!show)
	{
		HideAgain();
		if (f_658 != -1) { v232(); v132(0); }
		return;
	}
	ev69();
	ShowAgain();
	if (f_658 == -1) return;
	if (f_658 != 0) { v234(); v132(1); }
	else { v232(); v132(0); }
}

void C3DAnimated::ev71(float alpha)							// 0x41cd00
{
	OMedia3DShape *sh = get_shape();
	if (!sh) return;
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		if (alpha <= 0.0f)
		{
			m->set_flags(m->get_flags() & ~ommatf_SecondPass);
			m->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// +0x4c..+0x58
		}
		else
			m->set_diffuse(OMediaFARGBColor(alpha, 1.0f, 1.0f, 1.0f));
		m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	}
}

void C3DAnimated::ev72(float alpha)							// 0x41ca20
{
	OMedia3DShape *sh = get_shape();
	if (!sh) return;
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		if (alpha <= 0.0f)
		{
			m->set_flags(m->get_flags() & ~ommatf_SecondPass);
			m->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
			m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		}
		else
		{
			m->set_diffuse(OMediaFARGBColor(alpha, 1.0f, 1.0f, 1.0f));
			if (!g_windowed) m->set_blend(omblendfc_Src_Color, omblendfc_One);
			else m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
			m->set_flags(m->get_flags() | ommatf_SecondPass);
		}
	}
}

void C3DAnimated::ev73(void)								// 0x41ce30
{
	OMedia3DShape *sh = get_shape();
	if (!sh) return;
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		m->set_flags(m->get_flags() | ommatf_SecondPass);
		m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	}
}

void C3DAnimated::ev74(float alpha, float r, float g, float b)	// 0x41cbc0
{
	OMedia3DShape *sh = get_shape();
	if (!sh) return;
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		if (alpha <= 0.0f)
		{
			m->set_flags(m->get_flags() & ~ommatf_SecondPass);
			m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
			m->set_diffuse(OMediaFARGBColor(1.0f, r, g, b));
		}
		else
		{
			m->set_diffuse(OMediaFARGBColor(alpha, r, g, b));
			m->set_flags(m->get_flags() | ommatf_SecondPass);
			m->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		}
	}
}

void C3DAnimated::ev75(float time)							// 0x41b2e0
{
	if (!f_8b0) return;
	// (debug log "now im fading", empty)
	f_891 = 1;
	f_894 = time;
	f_898 = time;
	f_890 = 0;
}

void C3DAnimated::ev76(float time)							// 0x41b320
{
	if (!f_8b0) return;
	// (debug log "now im fading out", empty)
	f_890 = 1;
	f_891 = 0;
	f_894 = time;
	f_898 = time;
}

unsigned char C3DAnimated::ev77(char *state) { return strcasecmp(f_8b1, state) == 0; }	// 0x41c6b0
void C3DAnimated::ev78(char *state) { strcpy(f_8b1, state); }							// 0x41c680

void C3DAnimated::ev79(unsigned char pause)					// 0x41b010
{
	if (f_6d0 != 0) return;
	if (!pause)
	{
		if (f_668) { OMediaAnim::pause(false); f_668 = 0; }
	}
	else if (!f_668)
	{
		OMediaAnim::pause(true);
		f_668 = 1;
	}
}

void C3DAnimated::ev91(unsigned char on)					// 0x41cb60
{
	OMedia3DShape *sh = get_shape();
	if (!sh) return;
	omt_PolygonList *polys = sh->get_polygons();
	for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); ++p)
	{
		OMedia3DMaterial *m = (*p).get_material();
		if (!m) continue;
		if (on) m->set_flags(m->get_flags() | ommatf_SecondPass);
		else m->set_flags(m->get_flags() & ~ommatf_SecondPass);
	}
}

void C3DAnimated::ev92(int src, int dst)					// 0x41c9d0
{
	OMedia3DShape *sh = get_shape();
	if (sh) SetAllBlend(sh, (omt_BlendFunc)src, (omt_BlendFunc)dst);
}

unsigned char C3DAnimated::ev93(void)						// 0x41e840
{
	if (f_8a4) return 0;
	float p[4];
	v195(p);
	OMedia3DVector d(g_viewport->getx() - p[0], g_viewport->gety() - p[1], g_viewport->getz() - p[2]);
	d.w = 0;
	if (d.quick_magnitude() > g_layer_world->get_far_clip()) return 1;	// 0x56afd4 layer +0xc0 (2002)
	float f[4];
	{
		// 0x4b1ee0: the viewport's forward direction (0, 0, 1)
		const float sx = omd_Sin(g_viewport->get_anglex()), cx = omd_Cos(g_viewport->get_anglex());
		const float sy = omd_Sin(g_viewport->get_angley()), cy = omd_Cos(g_viewport->get_angley());
		const float sz = omd_Sin(g_viewport->get_anglez()), cz = omd_Cos(g_viewport->get_anglez());
		const float t = cz * 0.0f - sz * 0.0f;
		const float u = sz * 0.0f + cz * 0.0f;
		const float v = cx * 1.0f - t * sx;
		f[0] = u * cy - v * sy;
		f[1] = sx * 1.0f + t * cx;
		f[2] = u * sy + v * cy;
	}
	const float inv = 1.0f / d.quick_magnitude();
	d.x *= inv;
	d.y *= inv;
	const double dot = (double)f[2] * ((double)inv * d.z) + (double)f[1] * d.y + (double)d.x * f[0];
	return dot > -0.75 ? 1 : 0;								// 0x4cdb24: not in the 41-degree cone ahead
}

// --------------------------------------------------------------------------- granny

void C3DAnimated::ev80(void)								// 0x41e120
{
	if (g_gametype->IsPaused() || !f_589) return;
	// the original prints the playing animations' names with the debug text routine (empty in
	// the shipped build); its line counter g_545f30 still moves on
	g_545f30++;
	if (f_6f4[0].control[2])
	{
		granny_uint32 expired = 0;
		GrannyHasControlExpired(f_6f4[0].control[0], f_6f4[0].control[1], f_6f4[0].control[2], &expired);
		if (expired) { f_8a5 = 1; f_8a6 = 1; }
		g_545f30++;
	}
	if (f_6f4[1].control[2]) g_545f30++;
	if (f_6f4[2].control[2]) g_545f30++;
	if (f_6f4[3].control[2]) g_545f30++;
	granny_uint32 lock[2];
	if (g_granny && GrannyLockSequenceForRendering(f_6e8[0], f_6e8[1], GrannyStreamPositions | GrannyStreamNormals, lock) == GrannyOK)
	{
		granny_uint32 n = 0;
		if (GrannyGetRenderingStatesLeft(lock[0], lock[1], &n) == GrannyOK)
		{
			omt_VertexList::iterator vx = get_shape()->get_vertices()->begin();	// shape +0x68
			omt_NormalList::iterator nx = get_shape()->get_normals()->begin();	// shape +0x78
			for (granny_uint32 i = 0; i < n; i++)
			{
				granny_rendering_state st;
				if (GrannyLockNextRenderingState(lock[0], lock[1], &st) != GrannyOK) continue;
				if (st.positions.present)
				{
					const float *s = (const float *)st.positions.data;
					for (granny_uint32 k = st.positions.count; k; k--, ++vx, s += 3)
					{
						(*vx).z = s[0];
						(*vx).x = -s[1];
						(*vx).y = s[2];
					}
				}
				if (st.normals.present)
				{
					const float *s = (const float *)st.normals.data;
					for (granny_uint32 k = st.normals.count; k; k--, ++nx, s += 3)
					{
						(*nx).z = s[0];
						(*nx).x = -s[1];
						(*nx).y = s[2];
					}
				}
				GrannyUnlockRenderingState(&st);
			}
			if (f_6f0 != 1.0f) get_shape()->scale(f_6f0, f_6f0, f_6f0);	// shape vtable +0x34
			get_shape()->prepare(ompfc_ComputeShapeSphere);		// +0x28 (1)
			shape->set_modified(true);							// element +0x8c -> +8
		}
		GrannyUnlockRendering(lock[0], lock[1]);
	}
	if (strcasecmp(f_6f4[0].name, "STOP") == 0) f_8a5 = 1;
	if (f_8a6)
	{
		f_8a6 = 0;
		ev67();
	}
}

void C3DAnimated::InitModel(std::string file)				// 0x41cff0
{
	// (debug log "Entered InitModel", empty)
	for (std::list<ModelInfo_t *>::iterator i = g_granny_models.begin(); i != g_granny_models.end(); ++i)
	{
		ModelInfo_t *mi = *i;
		if (mi->name == file)								// (no break: as shipped)
		{
			f_6e0[0] = mi->model[0];
			f_6e0[1] = mi->model[1];
			mi->refs++;
		}
	}
	char path[120];
	strcpy(path, "grn\\");
	strcat(path, file.c_str());
	// (debug log "%s" path, empty)
	if (f_6e0[1] == 0 && GrannyOpenModel(g_granny, path, f_6e0) == GrannyOK)
	{
		ModelInfo_t *mi = new ModelInfo_t;					// (debug log "ModelInfo_t " + tag, empty)
		mi->model[0] = mi->model[1] = 0;
		mi->refs = 0;
		mi->name = file;
		mi->model[0] = f_6e0[0];
		mi->model[1] = f_6e0[1];
		mi->refs++;
		g_granny_models.push_back(mi);
	}
	if (f_6e0[1] != 0 && GrannyOpenSequence(f_6e0[0], f_6e0[1], f_6e8) == GrannyOK)
	{
		f_918 = new OMediaCanvas;							// (debug log "OMediaCanvas " + tag, empty)
		f_91c = new OMedia3DMaterial;						// (debug log "OMedia3DMaterial " + tag, empty)
		// (debug log "Calling BindTex", empty)
		BindTex();
		// (debug log "Calling BuildTex", empty)
		BuildTex();
		// (debug log "Calling MakeShapeUnlit", empty)
		MakeShapeUnLit(shape);								// 0x4b2520 on element +0x8c
	}
	// (debug log "Exiting InitModel", empty)
}

void C3DAnimated::BindTex(void)								// 0x41ae10
{
	granny_uint32 n = 0;
	GrannyGetNewTexturesLeft(g_granny, &n);
	for (granny_uint32 i = 0; i < n; i++)
	{
		granny_new_texture tex;
		if (GrannyLockNextNewTexture(g_granny, &tex) != GrannyOK) continue;
		f_918->create(tex.width, tex.height, NULL);			// canvas vtable +0x1c
		f_918->lock(omlf_Write);							// +0x24 vtable +4 (2)
		unsigned char *dst = (unsigned char *)f_918->get_pixels();	// canvas +0x58
		const unsigned char *src = tex.pixels;
		for (short y = (short)f_918->get_height(); y > 0; y--)		// +0x44 / +0x40 (shorts)
			for (short x = (short)f_918->get_width(); x > 0; x--)
			{
				dst[0] = src[0];							// RGB -> RGBA (memory order)
				dst[1] = src[1];
				dst[2] = src[2];
				src += 3;
				dst[3] = 0xff;
				dst += 4;
			}
		f_918->unlock();									// +0x24 vtable +8
		f_91c->set_texture(f_918);							// vtable +0x20
		f_91c->set_shade_mode(omshademc_Gouraud);			// +0x28 = 1
		f_91c->set_light_mode(ommlmc_Light);				// +0x2c = 2
		f_91c->set_flags(f_91c->get_flags() | 0x81c);		// +0x38 |= 0x81c (2002 flag bits, kept as is)
		GrannyUnlockNewTexture(&tex, (granny_cookie)f_91c);
	}
}

void C3DAnimated::BuildTex(void)							// 0x41d7a0
{
	granny_uint32 lock[2];
	if (GrannyLockSequenceForRendering(f_6e8[0], f_6e8[1], 0x3b, lock) != GrannyOK) return;
	granny_uint32 n = 0;
	if (GrannyGetRenderingStatesLeft(lock[0], lock[1], &n) == GrannyOK)
	{
		unsigned int base = 0;								// first vertex of the current state
		get_shape();
		get_shape();
		for (granny_uint32 i = 0; i < n; i++)
		{
			granny_rendering_state st;
			if (GrannyLockNextRenderingState(lock[0], lock[1], &st) != GrannyOK) continue;
			if (st.positions.present)
			{
				const float *s = (const float *)st.positions.data;
				for (granny_uint32 k = 0; k < st.positions.count; k++, s += 3)
				{
					OMedia3DPoint v(-s[1], s[2], s[0], 1.0f);
					omt_VertexList *vl = get_shape()->get_vertices();	// shape +0x64
					vl->insert(vl->end(), 1, v);
				}
			}
			if (st.normals.present)
			{
				const float *s = (const float *)st.normals.data;
				for (granny_uint32 k = 0; k < st.normals.count; k++, s += 3)
				{
					OMedia3DVector nv(-s[1], s[2], s[0]);
					nv.w = 0.0f;
					omt_NormalList *nl = get_shape()->get_normals();	// +0x74
					nl->insert(nl->end(), 1, nv);
					OMediaFARGBColor c(1.0f, 1.0f, 1.0f, 1.0f);
					omt_ColorList *cl = get_shape()->get_colors();		// +0xa4
					cl->insert(cl->end(), 1, c);
				}
			}
			std::vector<OMedia3DPoint> uv;					// texture coordinates of this state
			if (st.texcoords.present)
			{
				const float *s = (const float *)st.texcoords.data;
				for (short k = 0; (granny_uint32)k < st.texcoords.count; k++, s += 3)
					uv.insert(uv.end(), 1, OMedia3DPoint(s[0], s[1], s[2], 1.0f));
			}
			if (st.triangles.present)
			{
				const unsigned short *t = (const unsigned short *)st.triangles.data;
				for (granny_uint32 k = 0; k < st.triangles.count; k++, t += 3)
				{
					OMedia3DPolygon poly;
					poly.set_triangle();
					for (short c = 0; c < 3; c++)
					{
						poly.set_point(c, t[c] + base, true);
						poly.set_text_coord_u(c, uv[t[c]].x);
						poly.set_text_coord_v(c, uv[t[c]].y);
					}
					poly.set_material((OMedia3DMaterial *)st.texture_cookie);	// state +0x18
					omt_PolygonList *pl = get_shape()->get_polygons();			// +0x84
					pl->insert(pl->end(), 1, poly);
				}
			}
			base += st.positions.count;
			GrannyUnlockRenderingState(&st);
		}
		get_shape()->prepare(0x33);							// shape vtable +0x28
		// (debug log "SHAPEINFO: VERTS=%ld POLYS=%ld", empty)
	}
	GrannyUnlockRendering(lock[0], lock[1]);
}

void C3DAnimated::ev83(OMedia3DShape *sh)					// 0x41dc90
{
	granny_uint32 lock[2];
	if (GrannyLockSequenceForRendering(f_6e8[0], f_6e8[1], 0x3b, lock) != GrannyOK) return;
	granny_uint32 n = 0;
	if (GrannyGetRenderingStatesLeft(lock[0], lock[1], &n) == GrannyOK)
	{
		for (granny_uint32 i = 0; i < n; i++)
		{
			granny_rendering_state st;
			if (GrannyLockNextRenderingState(lock[0], lock[1], &st) != GrannyOK) continue;
			if (st.positions.present) sh->get_vertices()->clear();
			if (st.normals.present)
			{
				sh->get_normals()->clear();
				sh->get_colors()->clear();
			}
			if (st.triangles.present) sh->get_polygons()->clear();
			GrannyUnlockRenderingState(&st);
		}
		// (debug log "SHAPEINFO: VERTS=%ld POLYS=%ld" of our own shape, empty)
	}
	GrannyUnlockRendering(lock[0], lock[1]);
}

void C3DAnimated::AddAnimation(std::string file, char *name)	// 0x41df00
{
	AnimInfo_t info;
	memset(&info, 0, sizeof info);
	char path[120];
	strcpy(path, "grn\\");
	strcat(path, file.c_str());
	if (GrannyOpenAnimation(g_granny, path, info.anim) != GrannyOK)
	{
		ErrorLog("ERROR: opening file %s in AddAnimation", name);
		return;
	}
	strcpy(info.name, name);
	AnimInfo_t *ai = new AnimInfo_t;						// (debug log "AnimInfo_t " + tag, empty)
	*ai = info;
	f_6d4.push_back(ai);
	GrannyAddControlledAnimation(f_6e8[0], f_6e8[1], ai->anim[0], ai->anim[1], ai->control);
}

void C3DAnimated::ev86(char *a, char *b, char *c, char *d, unsigned char loop, float blend)	// 0x41d470
{
	bool none = true;
	for (int s = 0; s < 4; s++)
		if (f_6f4[s].control[2])
			GrannyRemoveControlledAnimation(f_6f4[s].control[0], f_6f4[s].control[1], f_6f4[s].control[2]);
	f_8a5 = 0;
	for (std::list<AnimInfo_t *>::iterator i = f_6d4.begin(); i != f_6d4.end(); ++i)
	{
		AnimInfo_t *ai = *i;
		int which = 0;
		if (a && strcasecmp(ai->name, a) == 0) which = 1;
		if (b && strcasecmp(ai->name, b) == 0) which = 2;
		if (c && strcasecmp(ai->name, c) == 0) which = 3;
		if (d && strcasecmp(ai->name, d) == 0) which = 4;
		if (!which) continue;
		granny_uint32 *k = ai->control;
		GrannyAddControlledAnimation(f_6e8[0], f_6e8[1], ai->anim[0], ai->anim[1], k);
		GrannySetAnimationParameter(k[0], k[1], k[2], GrannyParamSpeed, (double)f_430);	// time scale
		GrannySetAnimationParameter(k[0], k[1], k[2], GrannyParamLoopCount, loop ? 9999.0 : 1.0);
		if (f_920)
		{
			GrannySetAnimationParameter(k[0], k[1], k[2], GrannyParamBlendInTime, 0.0);
			GrannySetAnimationParameter(k[0], k[1], k[2], GrannyParamBlendOutTime, 0.0);
		}
		else
		{
			GrannySetAnimationParameter(k[0], k[1], k[2], GrannyParamBlendInTime, (double)blend);
			GrannySetAnimationParameter(k[0], k[1], k[2], GrannyParamBlendOutTime, (double)blend);
		}
		f_6f4[which - 1] = *ai;
		none = false;
	}
	if (none) ev60();
}

unsigned char C3DAnimated::ev87(char *file, char *dir, char *ext)	// 0x41e6a0
{
	(void)ext;
	char path[120];
	strcpy(path, dir);
	strcat(path, "\\");									// 0x534a64
	strcat(path, file);
	OMediaFilePath fp(std::string(path), 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	if (!fs.fileexists())
	{
		fs.close();
		ErrorLog("ERROR: File does not exist %s looking in directory %s", file, path);
	}
	else
	{
		fs.close();
		strcpy(file, path);
	}
	return 1;
}

void C3DAnimated::ev88(char *bone, CGameObject *other)		// 0x41af40
{
	if (!other->NameContains((char *)"C3DANIMATED")) return;
	granny_uint32 index = 0;
	GrannyGetBoneIndexFromName(f_6e8[0], f_6e8[1], bone, &index);
	C3DAnimated *o = other ? dynamic_cast<C3DAnimated *>(other) : NULL;		// other - 0xc0
	GrannyAttachSequence(o->f_6e8[0], o->f_6e8[1], 0, f_6e8[0], f_6e8[1], index, 0x1ff, 0x1ff);
}

void C3DAnimated::ev89(CGameObject *other)					// 0x41afd0
{
	if (!other->NameContains((char *)"C3DANIMATED")) return;
	C3DAnimated *o = other ? dynamic_cast<C3DAnimated *>(other) : NULL;
	GrannyDetachSequence(o->f_6e8[0], o->f_6e8[1]);
}

void C3DAnimated::ev90(unsigned char detach)				// 0x439ba0
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *o = *i;
		if (!o || o->m_object_id != '3GRO') continue;
		C3DGrannyObj *g = dynamic_cast<C3DGrannyObj *>(o);
		if (g && g->f_d88 && g->f_d70 == (CGameObject *)this && detach)
		{
			g->ev98((CGameObject *)this, 0);				// element vtable +0x188
			g->HideAgain();									// +0xdc
		}
	}
}
