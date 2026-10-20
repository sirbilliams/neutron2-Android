/*  Negatron - reconstructed classes of Jimmy Neutron vs. Jimmy Negatron (Neutron2.exe).
    Each method names the address it was reconstructed from. Unfinished parts say TODO(address). */
#pragma once
#include "OMediaApplication.h"
#include "OMediaListener.h"
#include "Globals.h"

// --------------------------------------------------------------------------- CAweReal (0x38 bytes)
// The application. vtables 0x51affc (OMediaApplication) and 0x51afe8 (OMediaListener at +0x24).
class CAweReal : public OMediaApplication, public OMediaListener
{
	public:
	CAweReal();												// inline in omt_main 0x4a3c00
	virtual ~CAweReal();									// 0x4a32f0
	virtual void listen_to_message(omt_Message msg, void *param = NULL);	// 0x4a3380
};

// --------------------------------------------------------------------------- CGfx (4 bytes)
class CGfx
{
	public:
	CGfx();													// 0x4b0e80
	virtual ~CGfx();

	void InitDisplay(void);									// 0x4b1270
	void InitWorld(void);									// 0x4b1270 (second half)
	static void InitLayers(void);							// 0x4b1710
	static void InitViewPort(void);							// 0x4b1560
	static void SetDisplayMode(int w, int h, int depth, bool windowed);	// 0x4b18a0
	static void InitFont(void), UnInitFont(void);
	static void InitCaptions(void), UnInitCaptions(void);	// 0x4b16a0
	static void InitAlpha(void), UnInitAlpha(void);			// 0x4a0ce0 / 0x4a0e10
};

// --------------------------------------------------------------------------- CInput (4 bytes)
class CInput
{
	public:
	CInput();												// 0x4b3260
	virtual ~CInput();
};

// --------------------------------------------------------------------------- free functions
class OMediaFileStream;
class OMediaDataBase;
class OMediaCanvas;
class OMediaCanvasElement;

namespace neg {
enum { ALPHA_SPRITES = 0x226, FONT_COUNT = 6, FONT_GLYPHS = 64 };
extern OMediaCanvasElement	 g_alpha[ALPHA_SPRITES];			// 0x54e868  text sprite pool
extern OMediaCanvas			*g_font[FONT_COUNT][FONT_GLYPHS];	// 0x56a748  glyph canvases, [font][ch - ' ']
extern unsigned short		 g_alpha_used;						// 0x56ad48

OMediaFileStream *InitFileStream(const char *dos_path);		// 0x4b1130
OMediaDataBase *InitDatabase(OMediaFileStream *fs);			// 0x4b0fc0
void UnInitDatabase(OMediaDataBase *db);					// 0x4b1040
void UnInitFileStream(OMediaFileStream *fs);				// 0x4b1250
OMediaCanvas *LoadCanvasID(OMediaDataBase *db, int id);		// 0x4b2860

// text (Text.cpp)
extern float g_text_rgb[3], g_text_alpha, g_text_width;		// 0x53ef38 / 0x53ef34 / 0x56ad68
extern int   g_text_center;									// 0x56ad6c
void ClearText(void);										// 0x4a0f00
void DrawText(short x, short y, int font, const char *fmt, ...);	// 0x4a0f60  font: 'SMAL' 'MED' 'LARG' 'BIG' 'TIME'
OMediaCanvasElement *PlaceGlyph(short x, short y, OMediaCanvas *c);	// 0x4a2010

void DeleteAllObjects(void);								// 0x4ae4a0
void OpenDebugLog(void);									// 0x4b3ac0
void SetWindowTitle(void);									// 0x496110
bool CheckCD(void);											// 0x496200
void InitSoundEngine(void);									// 0x4b7db0
void LoadNewLevel(unsigned int which);						// omt_main / AWEMSG_LOAD
void SDLKeyToVK(int sdl_scancode, bool down);				// SDL replacement for WM_KEYDOWN/UP
}
