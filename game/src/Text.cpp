/*  Bitmap-font text, drawn with the alpha sprite pool.  Reconstructed from Neutron2.exe.
    0x4a0f00 ClearText   0x4a0f60 DrawText (also linked twice more, byte-identical, at 0x4a14f0 and 0x4a1a80)
    0x4a2010 PlaceGlyph

    Coordinates are in the game's 640x480 screen space (y down); -1 centres on that axis.
    The sprites live on the top 2D layer, whose origin is the screen centre with y up.       */

#include "Game.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

using namespace neg;

namespace neg {
float	g_text_rgb[3]	= { 1.0f, 1.0f, 1.0f };	// 0x53ef38..0x53ef40
float	g_text_alpha	= 1.0f;					// 0x53ef34  (reset to 1 after every DrawText)
int		g_text_center	= 0;					// 0x56ad6c  1 = x is the centre of the string
float	g_text_width	= 0.0f;					// 0x56ad68  width of the last string, unscaled
static OMediaCanvasElement *s_loading = 0;		// 0x56ad4c  the "Loading" picture
}

static int font_index(int font)
{
	switch (font)
	{
		case 'SMAL': return 1;
		case 'MED':  return 2;
		case 'LARG': return 3;
		case 'BIG':  return 4;
		case 'TIME': return 5;
		default:     return 0;
	}
}

static OMediaCanvas *glyph(int font, int ch)
{
	int c = toupper(ch);
	if (c < 0x20 || c >= 0x60) return NULL;
	return g_font[font_index(font)][c - 0x20];
}

// --------------------------------------------------------------------------- 0x4a0f00

void neg::ClearText(void)
{
	for (int i = 0; i < (short)g_alpha_used; i++)
		if (g_alpha[i].getvisible()) g_alpha[i].hide(true);
	g_alpha_used = 0;
}

// --------------------------------------------------------------------------- 0x4a2010

OMediaCanvasElement *neg::PlaceGlyph(short x, short y, OMediaCanvas *canv)
{
	if (!canv || g_alpha_used >= ALPHA_SPRITES) return NULL;
	OMediaCanvasElement *s = &g_alpha[g_alpha_used];
	if (!s->islinked())
	{
		s->link(g_world);
		s->link_layer(g_layer_top2d);
	}
	s->set_canvas(canv);
	s->set_canvas_flags(omcanef_FreeWorldSize);
	s->set_size(g_screen_w * (1.0f / 640.0f) * canv->get_width(), canv->get_height() * (g_screen_h * (1.0f / 480.0f)));
	s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	s->set_auto_align(omaac_Left, omaac_Bottom);
	s->set_diffuse(OMediaFARGBColor(g_text_alpha, g_text_rgb[0], g_text_rgb[1], g_text_rgb[2]));
	s->place(x, y, 0);
	if (!s->getvisible()) s->hide(false);
	s->update_logic(0);						// vtable +0x3c(0): applies the auto-align
	g_alpha_used++;
	g_text_width += canv->get_width();
	return s;
}

// --------------------------------------------------------------------------- 0x4a0f60

void neg::DrawText(short x, short y, int font, const char *fmt, ...)
{
	char text[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	const float sx = g_screen_w * (1.0f / 640.0f);
	const float sy = g_screen_h * (1.0f / 480.0f);
	x = (short)(int)(x * sx);
	y = (short)(int)(y * sy);

	// Fullscreen "loading": show the Loading picture instead of text.
	if (!g_windowed && strcmp(text, "loading") == 0)
	{
		if (!s_loading)
		{
			s_loading = new OMediaCanvasElement;
			s_loading->link(g_world);
			s_loading->link_layer(g_layer_top2d);
			s_loading->set_auto_align(omaac_Left, omaac_Bottom);
			s_loading->set_blend(omblendfc_One, omblendfc_Zero);
			// TODO(0x4b28d0): LoadCanvasName(alpha db, "Loading"); TODO(0x4b2fc0): place at (0,0)
			s_loading->set_id('LOAD');
		}
		return;
	}

	// Width of the string in screen pixels.
	int width = 0;
	size_t n = strlen(text);
	for (size_t i = 0; i < n; i++)
	{
		OMediaCanvas *g = glyph(font, text[i]);
		if (g) width = (int)(g->get_width() * sx + width);
	}
	if (g_text_center == 1) x -= width / 2;

	// Line height from the font's '0'.
	int height = 0;
	OMediaCanvas *zero = glyph(font, '0');
	if (zero) height = (int)(zero->get_height() * sy);

	float fx = (float)x - g_screen_w * 0.5f;
	if (y == -1) y = (short)(int)(g_screen_h * 0.5f - height * 0.5f);
	float fy = g_screen_h * 0.5f - y;

	short first = g_alpha_used;
	g_text_width = 0.0f;
	if (n)
	{
		short iy = (short)(int)fy;
		for (size_t i = 0; i < n; i++)
		{
			OMediaCanvas *g = glyph(font, text[i]);
			PlaceGlyph((short)(int)fx, iy, g);
			if (g) fx += g->get_width() * sx;
		}
	}

	// x == -1: centre the whole string horizontally.
	if (x == -1)
	{
		float px = -(g_text_width * 0.5f);
		for (short i = first; i < (short)g_alpha_used; i++)
		{
			g_alpha[i].place(px, fy, 0);
			px += g_alpha[i].get_width() * sx;		// sprite width (already scaled) times sx again, as shipped
		}
	}
	g_text_alpha = 1.0f;
}
