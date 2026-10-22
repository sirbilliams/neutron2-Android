/*  port: "Bill's UI" - see BillsUi.h. Everything here is drawn in code into canvases of our own
    (RGBA, alpha blended on the HUD layer); only the game's fonts and pictures are read. */
#include "BillsUi.h"
#include "Widescreen.h"
#include "../Game.h"
#include "../Globals.h"
#include "../C2DInGameMenu.h"
#include "../CanvasWrapper.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include "OMediaLayer.h"
#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

namespace neg {

extern int g_text_center;			// Text.cpp

static volatile int s_bills = -1;	// -1: not read yet

bool BillsUi(void)
{
	if (s_bills < 0)
	{
		const char *e = getenv("NEG_UI");
		s_bills = (e && strcasecmp(e, "vanilla") == 0) ? 0 : 1;	// default: Bill's UI
		SDL_Log("In-game UI: %s", s_bills ? "Bill's UI" : "Vanilla");
	}
	return s_bills != 0;
}

void SetBillsUi(bool on)
{
	s_bills = on ? 1 : 0;
}

// --------------------------------------------------------------------------- text metrics

static OMediaCanvas *Glyph(int font, int ch)
{
	int f = 0;
	switch (font)
	{
		case 'SMAL': f = 1; break;
		case 'MED':  f = 2; break;
		case 'LARG': f = 3; break;
		case 'BIG':  f = 4; break;
		case 'TIME': f = 5; break;
	}
	if (ch >= 'a' && ch <= 'z') ch -= 32;
	if (ch < 0x20 || ch >= 0x60) return NULL;
	return g_font[f][ch - 0x20];
}

int FontHeight(int font)
{
	OMediaCanvas *z = Glyph(font, '0');
	return z ? z->get_height() : 0;
}

int TextWidth(int font, const char *s)
{
	int w = 0;
	for (; *s; s++)
		if (OMediaCanvas *g = Glyph(font, (unsigned char)*s)) w += g->get_width();
	return w;
}

// --------------------------------------------------------------------------- drawn pictures

// A picture of our own: an RGBA canvas (bytes R, G, B, A) on a 2D layer, placed by its top left
// corner in the game's 640x480 space (x may run into the widescreen margins).
struct Deco
{
	OMediaCanvas		*canvas;
	OMediaCanvasElement	*elt;
	int					w, h;			// canvas pixels
	int					key;			// what is drawn in it (the owner's choice)
};

static void DecoFree(Deco &d)
{
	delete d.elt;						// (the element unlinks itself)
	delete d.canvas;
	d.elt = NULL;
	d.canvas = NULL;
	d.w = d.h = 0;
	d.key = 0;
}

// (Re)creates the canvas at w x h; returns its pixels, locked for writing (DecoDone unlocks).
static unsigned int *DecoBegin(Deco &d, OMediaLayer *layer, int w, int h, bool additive)
{
	if (!d.elt)
	{
		d.elt = new OMediaCanvasElementWrapper;
		d.elt->link_layer(layer);
		d.elt->set_canvas_flags(omcanef_FreeWorldSize);
		d.elt->set_auto_align(omaac_Left, omaac_Bottom);
		d.elt->update_logic(0);
		d.elt->hide(true);
	}
	d.elt->set_blend(omblendfc_Src_Alpha, additive ? omblendfc_One : omblendfc_Inv_Src_Alpha);
	if (!d.canvas || d.w != w || d.h != h)
	{
		d.elt->set_canvas(NULL);
		delete d.canvas;
		d.canvas = new OMediaCanvas;
		d.canvas->create(w, h);
		d.w = w;
		d.h = h;
	}
	d.canvas->lock(omlf_Write);
	unsigned int *p = (unsigned int *)d.canvas->get_pixels();
	if (p) memset(p, 0, (size_t)w * h * 4);
	return p;
}

static void DecoDone(Deco &d)
{
	d.canvas->unlock();					// (a write unlock re-uploads the texture)
	d.elt->set_canvas(d.canvas);
}

static void DecoPlace(Deco &d, float x, float y, float w, float h)
{
	if (!d.elt) return;
	const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
	d.elt->set_size(w * sx, h * sy);
	d.elt->place(x * sx - (float)(g_screen_w / 2), (float)(g_screen_h / 2) - (y + h) * sy, 0);
	if (!d.elt->getvisible()) d.elt->hide(false);
}

static void DecoHide(Deco &d)
{
	if (d.elt && d.elt->getvisible()) d.elt->hide(true);
}

static inline unsigned int Rgba(float r, float g, float b, float a)
{
	if (a <= 0) return 0;
	if (a > 1) a = 1;
	r = r > 1 ? 1 : r; g = g > 1 ? 1 : g; b = b > 1 ? 1 : b;
	return (unsigned int)(r * 255.0f + 0.5f) | ((unsigned int)(g * 255.0f + 0.5f) << 8)
		 | ((unsigned int)(b * 255.0f + 0.5f) << 16) | ((unsigned int)(a * 255.0f + 0.5f) << 24);
}

static inline float Clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

// The green hologram plate (the HUD's green: the help pictures' glow, the radar disc): a dark,
// translucent rounded panel with faint scan lines, a bright edge with a soft outer glow, a thin
// inner line and brighter corner brackets. The rectangle is w x h units inside a canvas that has
// `glow` units more on every side; k canvas pixels per unit.
enum { PLATE_K = 2 };
static void DrawPlate(Deco &d, OMediaLayer *layer, float w, float h, float glow)
{
	const int k = PLATE_K;
	const int cw = (int)((w + 2 * glow) * k + 0.5f), ch = (int)((h + 2 * glow) * k + 0.5f);
	unsigned int *px = DecoBegin(d, layer, cw, ch, false);
	if (px)
	{
		const float hw = w * 0.5f, hh = h * 0.5f, r = 6.0f, edge = 1.1f, br = 10.0f;
		for (int j = 0; j < ch; j++)
			for (int i = 0; i < cw; i++)
			{
				const float x = (i + 0.5f) / k - glow - hw, y = (j + 0.5f) / k - glow - hh;
				const float qx = fabsf(x) - (hw - r), qy = fabsf(y) - (hh - r);
				const float ox = qx > 0 ? qx : 0, oy = qy > 0 ? qy : 0;
				const float dist = sqrtf(ox * ox + oy * oy) + (qx > qy ? (qx < 0 ? qx : 0) : (qy < 0 ? qy : 0)) - r;
				float a = 0, cr = 0.35f, cg = 1.0f, cb = 0.55f;
				if (dist <= 0)							// inside: the panel
				{
					a = 0.50f;
					cr = 0.02f; cg = 0.16f; cb = 0.08f;
					if (((j / k) % 3) == 0) { cg = 0.22f; cb = 0.11f; }	// scan lines
					// a faint inner line 3 units in
					const float il = 1.0f - fabsf(dist + 3.5f) / 0.6f;
					if (il > 0) { const float t = il * 0.45f; cr += t * 0.3f; cg += t; cb += t * 0.45f; a += t * 0.4f; }
				}
				// the edge (and the brighter corner brackets)
				const bool corner = fabsf(x) > hw - br && fabsf(y) > hh - br;
				const float e = corner ? edge * 2.0f : edge;
				const float on = Clamp01(1.0f - (fabsf(dist + e * 0.5f) - e * 0.5f) * k * 0.7f);
				if (on > 0)
				{
					const float bright = corner ? 1.0f : 0.8f;
					cr = cr + (0.45f * bright - cr) * on;
					cg = cg + (1.0f * bright - cg) * on;
					cb = cb + (0.6f * bright - cb) * on;
					a = a + (0.95f - a) * on;
				}
				if (dist > 0 && on < 1)					// the glow outside
				{
					const float gl = 0.30f * expf(-dist / (glow * 0.4f));
					if (gl > a) { a = gl; cr = 0.3f; cg = 1.0f; cb = 0.5f; }
				}
				px[j * cw + i] = Rgba(cr, cg, cb, a);
			}
	}
	DecoDone(d);
}

// A plate whose rectangle (not its glow) is at (x, y) w x h in the 640x480 space.
static void ShowPlate(Deco &d, int key, float x, float y, float w, float h)
{
	const float glow = 4.0f;
	if (d.key != key || !d.elt)
	{
		DrawPlate(d, g_layer_hud2d, w, h, glow);
		d.key = key;
	}
	DecoPlace(d, x - glow, y - glow, w + 2 * glow, h + 2 * glow);
}

// --------------------------------------------------------------------------- state

static unsigned s_frame;				// BillsUiFrame calls
static unsigned s_sonar_frame;			// the frame the sonar readout was last drawn
static Deco s_sonar_plate;

// --------------------------------------------------------------------------- 1. the sonar readout

// C3DRadar::UpdateBlips: the range readout ("2X" .. "5X"), centred under the radar disc on a small
// plate. The disc is the menu's sprite f_5b0 (at (490, 10), moved to the right edge in widescreen).
void BillsSonarReadout(int range)
{
	char text[16];
	snprintf(text, sizeof text, "%dX", range);
	const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
	float cx = 550.0f, bottom = 135.0f;
	if (g_menu && g_menu->f_5b0 && g_menu->f_5b0->get_width() > 0)
	{
		cx = 490.0f + g_menu->f_5b0->get_width() / sx * 0.5f;
		bottom = 10.0f + g_menu->f_5b0->get_height() / sy;
	}
	cx += WideMargin();
	const int th = FontHeight('MED'), tw = TextWidth('MED', text);
	const float pw = (float)(tw > 30 ? tw : 30) + 16.0f, ph = (float)th + 8.0f;
	const float top = bottom + 2.0f;
	ShowPlate(s_sonar_plate, 1 + (int)pw * 1000 + (int)ph, cx - pw * 0.5f, top, pw, ph);
	const int keep = g_text_center;
	g_text_center = 1;
	DrawText((short)(int)(cx + 0.5f), (short)(int)(top + 4.0f + (float)th), 'MED', "%s", text);
	g_text_center = keep;
	s_sonar_frame = s_frame;
}

// --------------------------------------------------------------------------- per frame

void BillsUiFrame(float dt)
{
	(void)dt;
	s_frame++;
	const bool on = BillsUi();
	if (!on || s_frame - s_sonar_frame > 2) DecoHide(s_sonar_plate);
}

void BillsUiShutdown(void)
{
	DecoFree(s_sonar_plate);
}

}

#ifdef __ANDROID__
#include <jni.h>

// Companion.setBillsUi (the in-game port menu's "In-game UI" item)
extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetBillsUi(JNIEnv *, jclass, jboolean on)
{
	neg::SetBillsUi(on != JNI_FALSE);
}
#endif
