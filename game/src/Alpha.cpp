/*  Database helpers and the "alpha" (bitmap font) sprite pool.  Reconstructed from Neutron2.exe.
    0x4b1130 InitFileStream   0x4b0fc0 InitDatabase   0x4b2860 LoadCanvasID
    0x4b1040 / 0x4b1250 UnInitDatabase / UnInitFileStream
    0x4a0ce0 InitAlpha        0x4a0e10 UnInitAlpha     0x4a0ef0 ReInitAlpha               */

#include "Game.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaDataBase.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include "OMediaError.h"
#include <SDL.h>
#include <string>
#include <string.h>

using namespace neg;

namespace neg {
OMediaCanvasElement	 g_alpha[ALPHA_SPRITES];	// 0x54e868  (0xd0 bytes each in the 2002 OMT)
OMediaCanvas		*g_font[FONT_COUNT][FONT_GLYPHS];	// 0x56a748
unsigned short		 g_alpha_used = 0;			// 0x56ad48
static OMediaFileStream	*s_alpha_stream = 0;	// 0x56ad64
static OMediaDataBase	*s_alpha_db = 0;		// 0x56ad60

// Canvas IDs in alpha.omt for each font, indexed by (character - ' '); -1 = no glyph.
// Read from Neutron2.exe at 0x53ef44 (6 x 64 int16).
static const short kFontGlyphs[FONT_COUNT][FONT_GLYPHS] = {
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 82,83,84,85,86,87,88,89,90,91,-1,-1,-1,-1,-1,-1,
	  -1,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,-1,-1,-1,-1,-1 },
	{ 130,-1,-1,-1,-1,-1,-1,133,-1,-1,-1,134,134,138,140,-1, 82,83,84,85,86,87,88,89,90,91,139,135,136,137,-1,-1,
	  -1,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,-1,-1,-1,-1,-1 },
	{ 129,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,141,-1, 0,8,7,6,5,4,3,2,1,9,-1,-1,-1,-1,-1,-1,
	  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1 },
	{ 129,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 45,36,37,38,44,43,42,41,40,39,-1,-1,-1,-1,-1,-1,
	  -1,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,-1,-1,-1,-1,-1 },
	{ 128,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 45,36,37,38,44,43,42,41,40,39,-1,-1,-1,-1,-1,-1,
	  -1,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 118,127,120,121,122,123,124,125,126,119,-1,-1,-1,-1,-1,-1,
	  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1 },
};
}

// --------------------------------------------------------------------------- database helpers

// The game's paths are DOS-style and relative (".\omt\alpha.omt").
static std::string port_path(const char *dos)
{
	std::string s(dos);
	if (s.compare(0, 2, ".\\") == 0) s.erase(0, 2);
	for (size_t i = 0; i < s.size(); i++) if (s[i] == '\\') s[i] = '/';
	return s;
}

OMediaFileStream *neg::InitFileStream(const char *name)		// 0x4b1130
{
	SDL_Log("InitFileStream(%s)", name ? name : "");
	if (!name) return NULL;
	OMediaFileStream *fs = new OMediaFileStream;
	try
	{
		OMediaFilePath path(port_path(name));
		fs->setpath(&path);
		fs->open(omcfp_Read);
	}
	catch (OMediaError &e)
	{
		SDL_Log("InitFileStream: cannot open %s (error %d)", name, (int)e.errorcode);
		delete fs;
		return NULL;
	}
	return fs;
}

OMediaDataBase *neg::InitDatabase(OMediaFileStream *fs)		// 0x4b0fc0
{
	SDL_Log("InitDatabase()");
	if (!fs) return NULL;
	return new OMediaDataBase(fs);
}

void neg::UnInitDatabase(OMediaDataBase *db)	{ delete db; }				// 0x4b1040
void neg::UnInitFileStream(OMediaFileStream *fs) { delete fs; }			// 0x4b1250

OMediaCanvas *neg::LoadCanvasID(OMediaDataBase *db, int id)				// 0x4b2860
{
	if (!db || !db->object_exists(OMediaCanvas::db_type, id)) return NULL;
	OMediaCanvas *c = (OMediaCanvas*)db->get_object(OMediaCanvas::db_type, id);
	// (0x56afaa set: an extra database call - not hit in the shipped config)
	if (c) c->db_unlock();
	return c;
}

// --------------------------------------------------------------------------- 0x4a0ce0

void CGfx::InitAlpha(void)
{
	SDL_Log("InitAlpha()");
	s_alpha_stream = InitFileStream(".\\omt\\alpha.omt");
	if (s_alpha_stream)
	{
		s_alpha_db = InitDatabase(s_alpha_stream);
		if (s_alpha_db)
		{
			int loaded = 0;
			for (int f = 0; f < FONT_COUNT; f++)
				for (int g = 0; g < FONT_GLYPHS; g++)
				{
					g_font[f][g] = kFontGlyphs[f][g] == -1 ? NULL : LoadCanvasID(s_alpha_db, kFontGlyphs[f][g]);
					if (g_font[f][g]) loaded++;
				}
			SDL_Log("InitAlpha: %d glyphs", loaded);
		}
	}

	for (int i = 0; i < ALPHA_SPRITES; i++)
	{
		OMediaCanvasElement &s = g_alpha[i];
		if (s.islinked()) { s.unlink_layer(); s.unlink(); }
		s.set_auto_align(omaac_Left, omaac_Bottom);		// +0xbc = 1, +0xc0 = 2
		s.set_canvas_flags(omcanef_FreeWorldSize);		// +0xc4 = 1
		// vtable +0xac (2002 OMT; compute_auto_align in 2.5)
		s.set_id('ALPH');								// +0x78
	}
	g_alpha_used = 0;
	SDL_Log("InitAlpha() done");
}

// --------------------------------------------------------------------------- 0x4a0e10

void CGfx::UnInitAlpha(void)
{
	SDL_Log("UnInitAlpha()");
	for (int i = 0; i < ALPHA_SPRITES; i++)
	{
		OMediaCanvasElement &s = g_alpha[i];
		s.set_canvas(NULL);
		if (s.getvisible()) s.hide(true);
		if (s.islinked()) { s.unlink_layer(); s.unlink(); }
	}
	// 0x56ad4c: an owned object released here (a caption/text helper; TODO)
	memset(g_font, 0, sizeof(g_font));
	UnInitDatabase(s_alpha_db);   s_alpha_db = NULL;
	UnInitFileStream(s_alpha_stream); s_alpha_stream = NULL;
}
