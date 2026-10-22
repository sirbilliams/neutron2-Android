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
#include "OMediaViewPort.h"
#include "OMediaRect.h"
#include "neg_version.h"				// NEG_GIT_COMMIT (engine/cmake/neg_version.cmake)
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

// --------------------------------------------------------------------------- 2. the pause screen

// The PDA main menu (C2DInGameMenu page 3, Esc / Start in play) has an empty green grid in its
// middle. Bill's UI puts a scope there: a radar sweep with blips over range rings, and Jimmy's atom
// (three orbits slowly turning, electrons running round them), in the grid's green phosphor (an
// additive canvas redrawn every other frame), and the credit line under it in the game's font.
enum { SCOPE_PX = 320 };
static const float kScopeX = 205.0f, kScopeY = 108.0f, kScopeSize = 230.0f;	// 640x480 units
static Deco s_scope;
static float *s_scope_base, *s_scope_buf, *s_scope_ang, *s_scope_r;
static float s_scope_t, s_scope_fade;

static inline float Line(float dist_px, float width_px) { const float v = 1.0f - fabsf(dist_px) / width_px; return v > 0 ? v : 0; }

static void ScopeInit(void)
{
	const int n = SCOPE_PX;
	s_scope_base = new float[n * n];
	s_scope_buf = new float[n * n];
	s_scope_ang = new float[n * n];
	s_scope_r = new float[n * n];
	const float half = n * 0.5f, pi = 3.14159265f;
	for (int j = 0; j < n; j++)
		for (int i = 0; i < n; i++)
		{
			const float x = (i + 0.5f) / half - 1.0f, y = 1.0f - (j + 0.5f) / half;
			const float r = sqrtf(x * x + y * y);
			float a = atan2f(y, x);
			if (a < 0) a += 2 * pi;
			s_scope_ang[j * n + i] = a;
			s_scope_r[j * n + i] = r;
			float v = 0;
			if (r < 0.95f)
			{
				for (int k = 1; k <= 3; k++) v += 0.20f * Line((r - 0.25f * k) * half, 1.3f);
				v += 0.12f * Line(x * half, 1.1f) + 0.12f * Line(y * half, 1.1f);
				const float deg = a * (180.0f / pi);
				const float d10 = fabsf(fmodf(deg + 5.0f, 10.0f) - 5.0f) * (pi / 180.0f) * r * half;
				const float d30 = fabsf(fmodf(deg + 15.0f, 30.0f) - 15.0f) * (pi / 180.0f) * r * half;
				if (r > 0.89f) v += 0.30f * Line(d10, 1.0f);
				if (r > 0.83f) v += 0.25f * Line(d30, 1.2f);
			}
			v += 0.60f * Line((r - 0.96f) * half, 1.6f);			// the bezel ring and its halo
			if (r > 0.96f) v += 0.18f * expf(-(r - 0.96f) * half / 4.0f);
			s_scope_base[j * n + i] = v;
		}
}

// Adds a soft dot of intensity v at (x, y) in canvas pixels (radius rad).
static void Splat(float *buf, float x, float y, float v, float rad)
{
	const int n = SCOPE_PX;
	const int r = (int)(rad * 2.0f + 1.0f);
	const int cx = (int)x, cy = (int)y;
	const float k = 1.0f / (rad * rad);
	for (int j = cy - r; j <= cy + r; j++)
	{
		if (j < 0 || j >= n) continue;
		for (int i = cx - r; i <= cx + r; i++)
		{
			if (i < 0 || i >= n) continue;
			const float dx = i + 0.5f - x, dy = j + 0.5f - y;
			const float d2 = (dx * dx + dy * dy) * k;
			if (d2 < 4.0f) buf[j * n + i] += v * expf(-d2);
		}
	}
}

static void ScopeDraw(void)
{
	if (!s_scope_base) ScopeInit();
	const int n = SCOPE_PX;
	const float half = n * 0.5f, pi = 3.14159265f, t = s_scope_t;
	float *b = s_scope_buf;
	// the sweep (clockwise, one turn in about 5 s) and its fading trail
	float sweep = fmodf(-t * 1.25f, 2 * pi);
	if (sweep < 0) sweep += 2 * pi;
	for (int i = 0; i < n * n; i++)
	{
		float v = s_scope_base[i];
		const float r = s_scope_r[i];
		if (r < 0.95f)
		{
			float d = s_scope_ang[i] - sweep;
			if (d < 0) d += 2 * pi;
			v += 0.55f * expf(-d * 2.4f);
			const float px = (d > pi ? d - 2 * pi : d) * r * half;
			v += 0.9f * Line(px, 1.4f);
		}
		b[i] = v;
	}
	// blips that light up as the sweep passes them
	static const float kBlips[][2] = { { 0.62f, 0.7f }, { 0.35f, 2.2f }, { 0.80f, 3.1f }, { 0.55f, 4.0f },
									   { 0.72f, 5.3f }, { 0.47f, 5.9f } };
	for (unsigned k = 0; k < sizeof kBlips / sizeof kBlips[0]; k++)
	{
		float d = kBlips[k][1] - sweep;
		if (d < 0) d += 2 * pi;
		const float v = 0.15f + 1.1f * expf(-d * 1.3f);
		Splat(b, half + cosf(kBlips[k][1]) * kBlips[k][0] * half, half - sinf(kBlips[k][1]) * kBlips[k][0] * half, v, 2.2f);
	}
	// the atom: three orbits (ellipses at 60 degrees) turning slowly, an electron on each
	const float A = 0.44f * half, B = 0.15f * half, spin = t * 0.30f;
	for (int k = 0; k < 3; k++)
	{
		const float phi = spin + k * (pi / 3.0f), c = cosf(phi), s = sinf(phi);
		for (int m = 0; m < 240; m++)
		{
			const float u = m * (2 * pi / 240.0f), ex = cosf(u) * A, ey = sinf(u) * B;
			Splat(b, half + ex * c - ey * s, half - (ex * s + ey * c), 0.26f, 0.9f);
		}
		const float u0 = t * (1.9f + 0.25f * k) + k * 2.1f;
		for (int m = 0; m < 12; m++)			// the electron and its tail
		{
			const float u = u0 - m * 0.06f, ex = cosf(u) * A, ey = sinf(u) * B;
			Splat(b, half + ex * c - ey * s, half - (ex * s + ey * c), m == 0 ? 1.6f : 0.35f * (1.0f - m / 12.0f), m == 0 ? 3.0f : 1.6f);
		}
	}
	Splat(b, half, half, 0.35f, 0.13f * half);	// the nucleus: a glow and a bright core
	Splat(b, half, half, 1.3f, 0.045f * half);
	// to pixels: phosphor green, white-hot where bright; alpha = intensity (drawn additively)
	unsigned int *px = DecoBegin(s_scope, g_layer_hud2d, n, n, true);
	if (px)
		for (int i = 0; i < n * n; i++)
		{
			const float v = b[i];
			if (v <= 0.004f) continue;
			const float hot = Clamp01((v - 0.75f) * 0.9f);
			px[i] = Rgba(0.22f + 0.7f * hot, 1.0f, 0.42f + 0.5f * hot, v);
		}
	DecoDone(s_scope);
}

static void ScopeFrame(float dt, bool show)
{
	if (!show)
	{
		DecoHide(s_scope);
		s_scope_fade = 0;
		return;
	}
	s_scope_t += dt;
	s_scope_fade += dt * 3.0f;
	if (s_scope_fade > 1.0f) s_scope_fade = 1.0f;
	if (!s_scope.elt || (s_frame & 1)) ScopeDraw();
	s_scope.elt->set_diffuse(OMediaFARGBColor(s_scope_fade, 1.0f, 1.0f, 1.0f));
	DecoPlace(s_scope, kScopeX, kScopeY, kScopeSize, kScopeSize);

	// the credit line, in the game's small font, under the scope
	static char line2[96];
	if (!line2[0])
	{
		const char *app = getenv("NEG_APP_VERSION");
		// Android: v<versionName> build <commit>; desktop: dev build <commit> (the game's fonts have
		// no parentheses, so "build" stands in for "(<commit>)")
		if (app && *app) snprintf(line2, sizeof line2, "BY SIRBILLIAMS - V%s BUILD %s", app, NEG_GIT_COMMIT);
		else snprintf(line2, sizeof line2, "BY SIRBILLIAMS - DEV BUILD %s", NEG_GIT_COMMIT);
	}
	const char *line1 = "JIMMY NEUTRON VS JIMMY NEGATRON RECOMP";
	const int font = TextWidth('MED', line1) <= 440 ? 'MED' : 'SMAL';
	const int h = FontHeight(font);
	const int keep = g_text_center;
	g_text_center = 1;
	const float y1 = kScopeY + kScopeSize + 10.0f + (float)h;
	const float rgb[3] = { g_text_rgb[0], g_text_rgb[1], g_text_rgb[2] };
	g_text_rgb[0] = 0.62f; g_text_rgb[1] = 1.0f; g_text_rgb[2] = 0.68f;	// phosphor green
	g_text_alpha = s_scope_fade;
	DrawText(320, (short)(int)y1, font, "%s", line1);
	g_text_alpha = s_scope_fade;
	DrawText(320, (short)(int)(y1 + 6.0f + (float)FontHeight('SMAL')), 'SMAL', "%s", line2);
	g_text_rgb[0] = rgb[0]; g_text_rgb[1] = rgb[1]; g_text_rgb[2] = rgb[2];
	g_text_center = keep;
}

// --------------------------------------------------------------------------- 2b. the score

// DrawScoreHud: the score counts up to its new value (fast at first, about half a second) and,
// as points come in, pops (the digits grow for a moment) with a green glow behind it.
static double s_score_shown = -1.0;
static int s_score_seen;
static float s_score_pop, s_score_glow;
static Deco s_score_halo;

void BillsScore(float dt, short x, short y)
{
	const int target = g_score;
	if (s_score_shown < 0 || (double)target < s_score_shown - 0.5)	// a load, a new game: no count
	{
		s_score_shown = target;
		s_score_seen = target;
	}
	if (target > s_score_seen)
	{
		s_score_pop = 1.0f;
		s_score_glow = 1.0f;
		s_score_seen = target;
	}
	const bool counting = s_score_shown < (double)target;
	if (counting)
	{
		s_score_shown += ((double)target - s_score_shown) * (1.0 - exp(-(double)dt * 7.0)) + 25.0 * dt;
		if (s_score_shown > (double)target) s_score_shown = target;
	}
	s_score_pop -= dt * 4.0f;
	if (s_score_pop < 0) s_score_pop = 0;
	s_score_glow -= dt * 1.6f;
	if (s_score_glow < (counting ? 0.55f : 0.0f)) s_score_glow = counting ? 0.55f : 0.0f;

	char text[32];
	snprintf(text, sizeof text, "%6.0d", (int)s_score_shown);
	int lead = 0;
	while (text[lead] == ' ') lead++;
	if (!text[lead])									// nothing to show (score 0, as the original)
	{
		DecoHide(s_score_halo);
		return;
	}
	char spaces[32];
	memcpy(spaces, text, lead);
	spaces[lead] = 0;
	const float dx0 = (float)x + (float)TextWidth('LARG', spaces);	// the digits, in 640x480 units
	const float dw = (float)TextWidth('LARG', text + lead), th = (float)FontHeight('LARG');
	const float cx = dx0 + dw * 0.5f, cy = (float)y - th * 0.5f;

	if (s_score_glow > 0.01f)
	{
		if (!s_score_halo.elt)
		{
			const int w = 128, h = 64;
			unsigned int *px = DecoBegin(s_score_halo, g_layer_hud2d, w, h, true);
			if (px)
				for (int j = 0; j < h; j++)
					for (int i = 0; i < w; i++)
					{
						const float u = (i + 0.5f) / w * 2 - 1, v = (j + 0.5f) / h * 2 - 1;
						const float d2 = u * u + v * v;
						px[j * w + i] = Rgba(0.35f, 1.0f, 0.5f, d2 < 1 ? 0.85f * (expf(-d2 * 3.2f) - 0.0408f) / 0.9592f : 0.0f);
					}
			DecoDone(s_score_halo);
		}
		const float gw = dw + 50.0f + 20.0f * s_score_pop, gh = th + 26.0f + 10.0f * s_score_pop;
		s_score_halo.elt->set_diffuse(OMediaFARGBColor(s_score_glow, 1.0f, 1.0f, 1.0f));
		DecoPlace(s_score_halo, cx - gw * 0.5f, cy - gh * 0.5f, gw, gh);
	}
	else DecoHide(s_score_halo);

	const unsigned short first = g_alpha_used;
	DrawText(x, y, 'LARG', "%s", text);
	const float k = 1.0f + 0.22f * s_score_pop * s_score_pop;
	if (k > 1.001f)
	{
		// grow the glyphs about the digits' centre (layer coordinates: centred, y up)
		const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
		const float lx = cx * sx - (float)(g_screen_w / 2), ly = (float)(g_screen_h / 2) - cy * sy;
		for (unsigned short i = first; i < g_alpha_used; i++)
		{
			OMediaCanvasElement &e = g_alpha[i];
			e.set_size(e.get_width() * k, e.get_height() * k);
			e.place(lx + (e.get_x() - lx) * k, ly + (e.get_y() - ly) * k, 0);
		}
	}
}

// --------------------------------------------------------------------------- 2c. the energy bar

// C2DInGameMenu::SetHealthBar's work (the fill sprite g_health_bar, 20 wide, health * 1.044 high,
// green from 70 up, yellow from 30, red below - the original's thresholds), with life: the fill
// drains / fills smoothly, the colour eases between the thresholds' colours, a white flash when
// energy is gained, a red flicker when hurt, a warning pulse below 30, and a soft glow around it
// (an additive canvas of our own, in the bar's colour, breathing gently).
static float s_hb = -1.0f, s_hb_last, s_hb_flash, s_hb_hurt, s_hb_t;
static float s_hb_col[3] = { 0.0f, 1.0f, 0.0f };
static unsigned s_hb_frame, s_energy_frame;
static Deco s_hb_halo;

// --- the energy meter designs (a setting: NEG_UI_ENERGY=a|b|c - TUBE / ORBIT / DIAL, default a;
// the launcher's Settings and the port menu set it; "bar" = the original bar with the animation
// above, desktop only). They replace the bar, its frame, the atom picture
// and the number (those sprites are hidden while a widget shows, shown again otherwise).
static volatile int s_energy_design = -1;
void SetEnergyDesign(int design) { s_energy_design = design; }

static int EnergyDesign(void)
{
	int d = s_energy_design;
	if (d < 0)
	{
		const char *e = getenv("NEG_UI_ENERGY");
		d = 1;
		if (e && (e[0] == 'b' || e[0] == 'B')) d = (e[1] == 'a' || e[1] == 'A') ? 0 : 2;	// "bar" / "b"
		else if (e && (e[0] == 'c' || e[0] == 'C')) d = 3;
		s_energy_design = d;
	}
	return d;
}

static bool s_orig_hidden;
static void ShowOriginalBar(bool show)
{
	OMediaCanvasElementWrapper *const kPieces[3] = { g_545854, g_545968, g_5448e4 };	// atom picture, frame, fill
	if (!show)						// (every frame: the game shows them again on entering play)
	{
		for (int i = 0; i < 3; i++)
			if (kPieces[i] && kPieces[i]->getvisible()) kPieces[i]->hide(true);
		s_orig_hidden = true;
		return;
	}
	if (!s_orig_hidden) return;
	for (int i = 0; i < 3; i++)
		if (kPieces[i]) kPieces[i]->force_show();
	s_orig_hidden = false;
}

// A float RGBA painter for the widgets: premultiplied "over" compositing, shapes by signed
// distance (in widget units; k pixels per unit), anti-aliased over one pixel.
struct Paint
{
	float	*p;			// r, g, b, a (premultiplied)
	int		w, h;
	float	k;
	float	u, v;		// the current pixel's centre, in units
	int		i;			// its index
	void Over(float r, float g, float b, float a)
	{
		if (a <= 0) return;
		if (a > 1) a = 1;
		float *q = p + i * 4;
		const float ia = 1.0f - a;
		q[0] = r * a + q[0] * ia; q[1] = g * a + q[1] * ia; q[2] = b * a + q[2] * ia; q[3] = a + q[3] * ia;
	}
	void Add(float r, float g, float b, float a)	// a glow: brightens, adds coverage
	{
		if (a <= 0) return;
		float *q = p + i * 4;
		q[0] += r * a; q[1] += g * a; q[2] += b * a; q[3] += a * 0.6f;
		if (q[3] > 1) q[3] = 1;
	}
	float Cov(float sd) const { return Clamp01(0.5f - sd * k); }
};

static float SdCircle(float u, float v, float cx, float cy, float r) { return sqrtf((u - cx) * (u - cx) + (v - cy) * (v - cy)) - r; }
static float SdBox(float u, float v, float cx, float cy, float hw, float hh, float rad)
{
	const float qx = fabsf(u - cx) - (hw - rad), qy = fabsf(v - cy) - (hh - rad);
	const float ox = qx > 0 ? qx : 0, oy = qy > 0 ? qy : 0;
	return sqrtf(ox * ox + oy * oy) + (qx > qy ? (qx < 0 ? qx : 0) : (qy < 0 ? qy : 0)) - rad;
}
static float SdSeg(float u, float v, float ax, float ay, float bx, float by)
{
	const float pax = u - ax, pay = v - ay, bax = bx - ax, bay = by - ay;
	float h = (pax * bax + pay * bay) / (bax * bax + bay * bay);
	h = Clamp01(h);
	const float dx = pax - bax * h, dy = pay - bay * h;
	return sqrtf(dx * dx + dy * dy);
}
// brushed steel: light at the top, a dark band, fine horizontal grain
static void Steel(float v01, int row, float *c)
{
	const float base = 0.62f + 0.30f * cosf(v01 * 3.14159f * 1.15f) - 0.18f * expf(-(v01 - 0.62f) * (v01 - 0.62f) * 40.0f);
	const float grain = (float)(((unsigned)row * 2654435761u >> 24) & 15) / 15.0f * 0.06f - 0.03f;
	c[0] = c[1] = c[2] = Clamp01(base + grain);
	c[2] = Clamp01(c[2] + 0.03f);
}
// the original's energy colour at value e (green from 70, yellow from 30, red)
static void EnergyColour(float e, float *c)
{
	c[0] = e < 70.0f ? 1.0f : 0.15f; c[1] = e < 30.0f ? 0.18f : 1.0f; c[2] = 0.12f;
}
// a 7-segment digit d (0..9, -1 blank) with its top left at (x, y), w x h, segment thickness t
static float SevenSeg(float u, float v, float x, float y, float w, float h, float t, int d, bool *lit)
{
	static const unsigned char kSeg[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
	const float m = y + h * 0.5f, x1 = x + w, y1 = y + h, in = t * 0.6f;
	const float segs[7][4] = {
		{ x + in, y, x1 - in, y }, { x1, y + in, x1, m - in }, { x1, m + in, x1, y1 - in }, { x + in, y1, x1 - in, y1 },
		{ x, m + in, x, y1 - in }, { x, y + in, x, m - in }, { x + in, m, x1 - in, m } };
	float best = 1e9f;
	*lit = false;
	for (int s = 0; s < 7; s++)
	{
		const float dd = SdSeg(u, v, segs[s][0], segs[s][1], segs[s][2], segs[s][3]) - t * 0.5f;
		if (dd < best) { best = dd; *lit = d >= 0 && ((kSeg[d] >> s) & 1); }
	}
	return best;
}

static Deco s_energy;
static float s_energy_right = 130.0f, s_energy_bottom = 122.0f;	// its extent (640x480 units, with the number)
static float *s_energy_buf;
static int s_energy_cap;

static void EnergyWidget(int design, float health, float fr, float fg, float fb, float pulse)
{
	ShowOriginalBar(false);
	DecoHide(s_hb_halo);
	const float e = s_hb < 0 ? 0 : (s_hb > 100 ? 100 : s_hb);
	const float W = design == 1 ? 232.0f : 116.0f, H = design == 1 ? 48.0f : 116.0f;
	const float k = 2.0f;
	const int pw = (int)(W * k), ph = (int)(H * k);
	if (s_energy_cap < pw * ph)
	{
		delete[] s_energy_buf;
		s_energy_buf = new float[pw * ph * 4];
		s_energy_cap = pw * ph;
	}
	memset(s_energy_buf, 0, sizeof(float) * 4 * pw * ph);
	Paint P;
	P.p = s_energy_buf; P.w = pw; P.h = ph; P.k = k;
	const float t = s_hb_t, pi = 3.14159265f;
	float col[3];
	col[0] = fr; col[1] = fg; col[2] = fb;			// the animated colour (flash, flicker, pulse)
	const bool low = e < 30.0f;

	// electrons for the atoms (design 1's cap, design 2's gauge): faster and flickering red when low
	const float espeed = low ? 4.5f : 1.8f;
	for (int j = 0; j < ph; j++)
		for (int i = 0; i < pw; i++)
		{
			P.i = j * pw + i;
			const float u = (i + 0.5f) / k, v = (j + 0.5f) / k;
			if (design == 1)
			{
				// A "Steel LED tube": a chrome capsule with 10 LED cells, a glowing atom cap on the left
				const float cx = 24.0f, cy = 24.0f;
				// capsule body
				const float sd = SdBox(u, v, 132.0f, 24.0f, 96.0f, 14.0f, 14.0f);
				if (sd < 1.0f)
				{
					float c[3];
					Steel((v - 10.0f) / 28.0f, j, c);
					P.Over(c[0] * 0.55f, c[1] * 0.55f, c[2] * 0.55f, P.Cov(sd));					// rim (darker)
					const float sd2 = SdBox(u, v, 132.0f, 24.0f, 95.0f, 13.0f, 13.0f);
					P.Over(c[0], c[1], c[2], P.Cov(sd2));
					const float win = SdBox(u, v, 136.0f, 24.0f, 82.0f, 8.0f, 4.0f);				// the window
					P.Over(0.02f, 0.06f, 0.04f, P.Cov(win));
					// cells
					const int N = 10;
					const float x0 = 56.0f, x1 = 216.0f, cw = (x1 - x0) / N;
					const int ci = (int)((u - x0) / cw);
					if (ci >= 0 && ci < N && win < 0)
					{
						const float ccx = x0 + cw * (ci + 0.5f);
						const float cs = SdBox(u, v, ccx, 24.0f, cw * 0.5f - 1.2f, 6.0f, 1.2f);
						float on = Clamp01(e / 100.0f * N - ci);			// the last cell partly lit
						float cc[3];
						EnergyColour(ccx < x0 ? 0 : (ci + 1) * 100.0f / N, cc);
						if (on > 0) { cc[0] = col[0]; cc[1] = col[1]; cc[2] = col[2]; }
						const float lit = on * pulse;
						P.Over(0.04f + cc[0] * 0.9f * lit, 0.10f + cc[1] * 0.9f * lit, 0.06f + cc[2] * 0.9f * lit, P.Cov(cs));
						if (lit > 0) P.Add(cc[0], cc[1], cc[2], 0.35f * lit * expf(-fabsf(v - 24.0f) / 3.0f));
					}
					// a highlight along the top of the glass
					const float hl = SdBox(u, v, 136.0f, 19.0f, 80.0f, 1.2f, 1.2f);
					P.Add(1, 1, 1, 0.18f * P.Cov(hl));
				}
				// the cap: a steel ring round a glowing atom
				const float sc = SdCircle(u, v, cx, cy, 22.0f);
				if (sc < 6.0f)
				{
					float c[3];
					Steel((v - 2.0f) / 44.0f, j, c);
					P.Add(col[0], col[1], col[2], 0.35f * expf(-(sc > 0 ? sc : 0) / 2.0f) * (sc > 0 ? 1.0f : 0.0f));
					P.Over(c[0], c[1], c[2], P.Cov(sc));
					const float inner = SdCircle(u, v, cx, cy, 17.5f);
					const float gl = 0.35f + 0.65f * Clamp01(1.0f - sqrtf((u - cx) * (u - cx) + (v - cy) * (v - cy)) / 17.5f);
					P.Over(0.02f + col[0] * 0.25f * gl, 0.05f + col[1] * 0.35f * gl, 0.03f + col[2] * 0.25f * gl, P.Cov(inner));
					if (inner < 0)
					{
						for (int o = 0; o < 3; o++)
						{
							const float phi = t * 0.4f + o * pi / 3.0f, c0 = cosf(phi), s0 = sinf(phi);
							const float lx = (u - cx) * c0 + (v - cy) * s0, ly = -(u - cx) * s0 + (v - cy) * c0;
							const float ell = sqrtf(lx * lx / (13.0f * 13.0f) + ly * ly / (4.5f * 4.5f));
							P.Add(0.7f, 1.0f, 0.8f, 0.55f * Clamp01(1.0f - fabsf(ell - 1.0f) * 9.0f));
							const float ua = t * espeed + o * 2.1f;
							const float ex = cx + cosf(ua) * 13.0f * c0 - sinf(ua) * 4.5f * s0, ey = cy + cosf(ua) * 13.0f * s0 + sinf(ua) * 4.5f * c0;
							const bool red = low && ((int)(t * 12.0f + o) & 1);
							P.Add(red ? 1.0f : 0.8f, red ? 0.2f : 1.0f, red ? 0.15f : 0.85f, Clamp01(1.0f - SdCircle(u, v, ex, ey, 1.6f) * 0.8f));
						}
						P.Add(1, 1, 1, Clamp01(1.0f - SdCircle(u, v, cx, cy, 2.6f) * 0.7f));
					}
				}
			}
			else if (design == 2)
			{
				// B "Atom orbit gauge": a 270 degree ring that fills / drains round Jimmy's atom
				const float cx = 58.0f, cy = 58.0f;
				const float dx = u - cx, dy = cy - v, rr = sqrtf(dx * dx + dy * dy);
				P.Over(0.01f, 0.05f, 0.03f, 0.55f * P.Cov(rr - 54.0f));					// hologram backing
				P.Add(0.3f, 1.0f, 0.5f, 0.5f * Clamp01(1.0f - fabsf(rr - 54.0f) * k * 0.5f));	// its rim
				float ang = atan2f(dy, dx) * (180.0f / pi);
				if (ang < -135.0f) ang += 360.0f;									// -45 .. 225 is the arc
				const float pos = (225.0f - ang) / 270.0f * 100.0f;				// 0 at the lower left, 100 at the lower right
				if (pos >= 0 && pos <= 100)
				{
					const float ring = fabsf(rr - 47.0f) - 4.0f;
					const float cv = P.Cov(ring);
					if (cv > 0)
					{
						if (pos <= e) P.Over(col[0] * pulse, col[1] * pulse, col[2] * pulse, cv);
						else P.Over(0.05f, 0.14f, 0.08f, cv);
						if (pos <= e) P.Add(col[0], col[1], col[2], 0.25f * pulse);
					}
					// the bright leading edge
					if (fabsf(pos - e) < 1.2f && ring < 1.5f) P.Add(1, 1, 1, 0.6f);
					// ticks at 30 and 70
					if ((fabsf(pos - 30.0f) < 0.5f || fabsf(pos - 70.0f) < 0.5f) && fabsf(rr - 47.0f) < 6.0f) P.Over(0.9f, 0.95f, 0.9f, 0.8f);
				}
				// the atom: three orbits, an electron on each, a glowing nucleus
				for (int o = 0; o < 3; o++)
				{
					const float phi = t * 0.3f + o * pi / 3.0f, c0 = cosf(phi), s0 = sinf(phi);
					const float lx = (u - cx) * c0 + (v - cy) * s0, ly = -(u - cx) * s0 + (v - cy) * c0;
					const float A = 34.0f, B = 11.0f;
					const float ell = sqrtf(lx * lx / (A * A) + ly * ly / (B * B));
					P.Add(0.6f, 1.0f, 0.7f, 0.6f * Clamp01(1.0f - fabsf(ell - 1.0f) * 16.0f));
					const float ua = t * espeed + o * 2.1f;
					const float ex = cx + cosf(ua) * A * c0 - sinf(ua) * B * s0, ey = cy + cosf(ua) * A * s0 + sinf(ua) * B * c0;
					const bool red = low && ((int)(t * 12.0f + o) & 1);
					P.Add(red ? 1.0f : 0.85f, red ? 0.2f : 1.0f, red ? 0.15f : 0.9f, Clamp01(1.0f - SdCircle(u, v, ex, ey, 2.6f) * 0.7f));
				}
				P.Add(col[0], col[1], col[2], 0.5f * expf(-rr / 6.0f));
				P.Add(1, 1, 1, Clamp01(1.0f - SdCircle(u, v, cx, cy, 4.0f) * 0.7f));
			}
			else
			{
				// C "Dial": the bottom-screen panel's energy dial - steel bezel, green-yellow-red arc,
				// needle, the number in LED digits
				const float cx = 58.0f, cy = 58.0f;
				const float dx = u - cx, dy = cy - v, rr = sqrtf(dx * dx + dy * dy);
				const float bz = rr - 56.0f;
				if (bz < 1.0f)
				{
					float c[3];
					Steel(Clamp01((v - 2.0f) / 112.0f), j, c);
					P.Over(c[0], c[1], c[2], P.Cov(bz));
					P.Over(0.03f, 0.05f, 0.04f, 0.95f * P.Cov(rr - 49.0f));			// the face
				}
				float ang = atan2f(dy, dx) * (180.0f / pi);
				if (ang < -135.0f) ang += 360.0f;
				const float pos = (225.0f - ang) / 270.0f * 100.0f;
				if (pos >= 0 && pos <= 100 && rr < 49.0f)
				{
					float bc[3];
					EnergyColour(pos, bc);
					const float band = P.Cov(fabsf(rr - 42.0f) - 3.5f);
					const float lit = pos <= e ? 1.0f * (pos > e - 100 ? 1.0f : 1.0f) : 0.28f;
					P.Over(bc[0] * lit, bc[1] * lit, bc[2] * lit, band);
					if (pos <= e) P.Add(bc[0], bc[1], bc[2], 0.2f * band);
					const float tick = fabsf(fmodf(pos + 5.0f, 10.0f) - 5.0f) * 270.0f / 100.0f * (pi / 180.0f) * rr;
					if (rr > 33.0f && rr < 37.5f) P.Over(0.85f, 0.9f, 0.85f, Clamp01(1.0f - tick * k * 0.8f) * 0.9f);
				}
				// LED digits (the number), under the hub
				const int n = (int)(health + 0.5f) < 0 ? 0 : ((int)(health + 0.5f) > 999 ? 999 : (int)(health + 0.5f));
				const int dg[3] = { n >= 100 ? n / 100 : -1, n >= 10 ? (n / 10) % 10 : -1, n % 10 };
				for (int q = 0; q < 3; q++)
				{
					bool on;
					const float sd = SevenSeg(u, v, 41.0f + q * 12.5f, 71.0f, 8.0f, 15.0f, 2.2f, dg[q], &on);
					const float cv = P.Cov(sd);
					if (cv > 0)
					{
						if (on) P.Over(col[0] * pulse, col[1] * pulse, col[2] * pulse + 0.1f, cv);
						else P.Over(0.06f, 0.12f, 0.08f, cv * 0.8f);
					}
					if (on && sd < 3.0f) P.Add(col[0], col[1], col[2], 0.25f * Clamp01(1.0f - sd / 3.0f) * pulse);
				}
				// the needle (a pointer by the arc, r 33..45, and a short stub at the hub: it never
				// crosses the digits, which lie within r 32 below the hub) and the hub
				const float na = (225.0f - 270.0f * e / 100.0f) * (pi / 180.0f), ca = cosf(na), sa = -sinf(na);
				const float sn = SdSeg(u, v, cx + ca * 33.0f, cy + sa * 33.0f, cx + ca * 45.0f, cy + sa * 45.0f) - 1.6f;
				const float st = SdSeg(u, v, cx, cy, cx + ca * 10.0f, cy + sa * 10.0f) - 1.6f;
				P.Over(1.0f, 0.35f, 0.15f, P.Cov(sn < st ? sn : st));
				const float sh = SdCircle(u, v, cx, cy, 6.0f);
				if (sh < 1.0f)
				{
					float c[3];
					Steel(Clamp01((v - 52.0f) / 12.0f), j, c);
					P.Over(c[0], c[1], c[2], P.Cov(sh));
				}
			}
		}
	unsigned int *px = DecoBegin(s_energy, g_layer_hud2d, pw, ph, false);
	if (px)
		for (int i = 0; i < pw * ph; i++)
		{
			const float *q = s_energy_buf + i * 4;
			if (q[3] <= 0.002f) continue;
			px[i] = Rgba(q[0] / q[3], q[1] / q[3], q[2] / q[3], q[3]);
		}
	DecoDone(s_energy);
	// a shake when hurt
	const float shake = s_hb_hurt > 0 ? sinf(t * 70.0f) * 2.5f * s_hb_hurt : 0.0f;
	const float x = -WideMargin() + 6.0f + shake, y = 6.0f;
	DecoPlace(s_energy, x, y, W, H);
	s_energy_frame = s_frame;
	char num[16];
	snprintf(num, sizeof num, "%d", (int)(short)(int)health);
	s_energy_right = x - shake + W;
	s_energy_bottom = y + H;
	if (design == 1)		// A: the number in the game's font, after the tube
	{
		DrawText((short)(int)(x + W + 6.0f), (short)(int)(y + 24.0f + FontHeight('MED') * 0.5f), 'MED', "%s", num);
		s_energy_right += 6.0f + (float)TextWidth('MED', "100");
	}
	else if (design == 2)	// B: the game's big digits, beside the gauge
	{
		DrawText((short)(int)(x + W + 4.0f), (short)(int)(y + 58.0f + FontHeight('BIG') * 0.5f), 'BIG', "%s", num);
		s_energy_right += 4.0f + (float)TextWidth('BIG', "100");
	}
}

void BillsHealthBar(float dt, float health)
{
	OMediaCanvasElement *bar = g_health_bar;
	if (!bar) return;
	if (s_hb < 0) s_hb = s_hb_last = health;
	if (health > s_hb_last + 0.5f) s_hb_flash = 1.0f;
	else if (health < s_hb_last - 0.5f) s_hb_hurt = 1.0f;
	s_hb_last = health;
	s_hb += (health - s_hb) * (1.0f - expf(-dt * 7.0f));
	if (fabsf(health - s_hb) < 0.05f) s_hb = health;
	s_hb_t += dt;
	s_hb_flash -= dt * 2.5f; if (s_hb_flash < 0) s_hb_flash = 0;
	s_hb_hurt -= dt * 2.0f;  if (s_hb_hurt < 0) s_hb_hurt = 0;

	// the original's colour for the shown value, eased
	const float tr = s_hb < 70.0f ? 1.0f : 0.0f, tg = s_hb < 30.0f ? 0.0f : 1.0f;
	const float ease = 1.0f - expf(-dt * 6.0f);
	s_hb_col[0] += (tr - s_hb_col[0]) * ease;
	s_hb_col[1] += (tg - s_hb_col[1]) * ease;
	float r = s_hb_col[0], g = s_hb_col[1], b = 0.0f, k = 1.0f;
	if (s_hb < 30.0f) k = 0.5f + 0.5f * (0.5f + 0.5f * sinf(s_hb_t * 9.0f));	// low: warning pulse
	r = r * k + (1.0f - r * k) * s_hb_flash;										// gained: white flash
	g = g * k + (1.0f - g * k) * s_hb_flash;
	b = s_hb_flash * 0.8f;
	if (s_hb_hurt > 0 && ((int)(s_hb_t * 24.0f) & 1))							// hurt: red flicker
	{
		r = 1.0f; g = 0.12f; b = 0.08f;
	}
	const int design = EnergyDesign();
	if (design != 0)
	{
		EnergyWidget(design, health, r, g, b, k);
		return;
	}
	ShowOriginalBar(true);
	DrawText(WideX(40, WIDE_LEFT), 67, 'BIG', "%d", (int)(short)(int)health);
	s_energy_right = -WideMargin() + 40.0f + (float)TextWidth('BIG', "100");
	s_energy_bottom = 122.0f;
	const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
	bar->set_diffuse(OMediaFARGBColor(1.0f, r, g, b));
	bar->set_size(sx * 20.0f, (float)((double)(sy * s_hb) * 1.044));

	// the glow, around the fill (the sprite's place is its bottom left corner)
	if (!s_hb_halo.elt)
	{
		const int w = 32, h = 128;
		unsigned int *px = DecoBegin(s_hb_halo, g_layer_hud2d, w, h, true);
		if (px)
			for (int j = 0; j < h; j++)
				for (int i = 0; i < w; i++)
				{
					// signed distance (pixels) to the inner rectangle, 8 px in from each side: a soft
					// glow outside it, a faint fill inside rising to the edge
					const float x = fabsf(i + 0.5f - w * 0.5f) - (w * 0.5f - 8.0f);
					const float y = fabsf(j + 0.5f - h * 0.5f) - (h * 0.5f - 8.0f);
					const float sd = (x > 0 || y > 0) ? sqrtf((x > 0 ? x * x : 0) + (y > 0 ? y * y : 0)) : (x > y ? x : y);
					const float a = sd > 0 ? 0.9f * expf(-(sd / 4.0f) * (sd / 4.0f)) : 0.2f + 0.7f * expf(-(sd / 3.0f) * (sd / 3.0f));
					px[j * w + i] = Rgba(1.0f, 1.0f, 1.0f, a);
				}
		DecoDone(s_hb_halo);
	}
	const float fh = s_hb * 1.044f;
	if (fh > 0.5f)
	{
		const float x0 = (bar->get_x() + (float)(g_screen_w / 2)) / sx, bottom = ((float)(g_screen_h / 2) - bar->get_y()) / sy;
		float glow = 0.30f + 0.12f * sinf(s_hb_t * 2.2f) + 0.6f * s_hb_flash + 0.6f * s_hb_hurt;
		if (s_hb < 30.0f) glow += 0.35f * k;
		s_hb_halo.elt->set_diffuse(OMediaFARGBColor(glow > 1 ? 1.0f : glow, r, g, b + 0.2f * g));
		DecoPlace(s_hb_halo, x0 - 8.0f, bottom - fh - 8.0f, 36.0f, fh + 16.0f);
		s_hb_frame = s_frame;
	}
}

// --------------------------------------------------------------------------- 3. the help pictures

// The help picture of the current gadget (g_elt_544908[g_help_index], bottom left: the mouse and
// keys, H / F1) and the help keys panel (g_help_menu_elt, bottom right). Bill's UI frames each on a
// green-hologram plate, the picture's drawn part (its opaque pixels, found once per canvas) centred
// in it, 10 units in from the window's edges. Vanilla: the original places (the pieces are put
// back if they were moved).
struct HelpPiece
{
	Deco		plate;
	InterfaceElt *moved;			// the piece we placed (NULL: at its original place)
	OMediaCanvasElement *linked;	// the element relinked after the plate (drawn over it)
	OMediaCanvas *box_of;			// the canvas `box` was measured on
	int			box[4];				// its opaque pixels: x0, y0, x1, y1 (exclusive); x1 <= x0: none
};
static HelpPiece s_help[2];			// 0 the picture (left), 1 the keys (right)

static void MeasureCanvas(HelpPiece &h, OMediaCanvas *c)
{
	h.box_of = c;
	h.box[0] = h.box[1] = 0;
	h.box[2] = h.box[3] = 0;
	if (!c || c->get_width() <= 0) return;
	const int w = c->get_width(), ht = c->get_height();
	c->lock(omlf_Read);
	const unsigned char *p = (const unsigned char *)c->get_pixels();
	if (p)
	{
		int x0 = w, y0 = ht, x1 = 0, y1 = 0;
		for (int y = 0; y < ht; y++)
			for (int x = 0; x < w; x++)
				if (p[(y * w + x) * 4 + 3] > 24)
				{
					if (x < x0) x0 = x;
					if (x >= x1) x1 = x + 1;
					if (y < y0) y0 = y;
					if (y >= y1) y1 = y + 1;
				}
		if (x1 > x0)
		{
			h.box[0] = x0; h.box[1] = y0; h.box[2] = x1; h.box[3] = y1;
		}
	}
	c->unlock();
}

// The original's placement (InterfaceElement.cpp place_on_screen) of a piece's top left (x, y).
static void PlaceAt(OMediaCanvasElement *ce, float x, float y)
{
	const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
	ce->place(sx * x - (float)((short)g_screen_w / 2), ((float)((short)g_screen_h / 2) - ce->get_height()) - sy * y, 0);
}

static void HelpPieceFrame(HelpPiece &h, InterfaceElt *e, int side, bool on)
{
	OMediaCanvasElement *ce = e ? (OMediaCanvasElement *)e->canvas_elt : NULL;
	const bool shown = ce && ce->getvisible();
	if (h.moved && (!on || h.moved != e))		// put back where the original has it
	{
		if (h.moved->canvas_elt) PlaceAt(h.moved->canvas_elt, (float)h.moved->x + WideShift(h.moved), (float)h.moved->y);
		h.moved = NULL;
	}
	if (!on || !shown)
	{
		DecoHide(h.plate);
		return;
	}
	OMediaCanvas *c = ce->get_canvas();
	if (c != h.box_of) MeasureCanvas(h, c);
	if (h.box[2] <= h.box[0] || !c)
	{
		DecoHide(h.plate);
		return;
	}
	const float sx = g_screen_w * (1.0f / 640.0f);
	const float f = ce->get_width() / sx / (float)c->get_width();	// 640x480 units per canvas pixel
	const float cw = (h.box[2] - h.box[0]) * f, chh = (h.box[3] - h.box[1]) * f;
	const float pad = 8.0f, edge = 10.0f;
	const float pw = cw + 2 * pad, ph = chh + 2 * pad;
	const float px = side < 0 ? -WideMargin() + edge : 640.0f + WideMargin() - edge - pw;
	const float py = 480.0f - edge - ph;
	ShowPlate(h.plate, 2 + (int)pw * 1000 + (int)ph, px, py, pw, ph);
	if (h.linked != ce)						// draw the picture after (over) the plate
	{
		ce->link_layer(g_layer_hud2d);
		h.linked = ce;
	}
	PlaceAt(ce, px + pad - h.box[0] * f, py + pad - h.box[1] * f);
	h.moved = e;
}

// --------------------------------------------------------------------------- 4. the rear-view mirror

// The scooter's backup camera (C3DVehicle::ev139: a viewport 0.3 x 0.2 of the screen, top centre,
// while reversing). Bill's UI frames it on a plate a little larger than it: the viewport draws over
// the plate's middle, so only the bezel shows round it.
static OMediaViewPort *s_mirror;
static Deco s_mirror_plate;

void BillsMirror(void *viewport)		{ s_mirror = (OMediaViewPort *)viewport; }
void BillsMirrorGone(void *viewport)	{ if (s_mirror == viewport) s_mirror = NULL; }

static void MirrorFrame(bool on)
{
	if (s_mirror && !on)					// Vanilla: the original's place, 4 from the top
	{
		OMediaRect b;
		s_mirror->getbounds(&b);
		const int top = (int)(g_screen_h * (4.0f / 480.0f));
		if (b.top != top) { b.bottom += top - b.top; b.top = top; s_mirror->setbounds(&b); }
	}
	if (!on || !s_mirror || g_game_mode != 5)
	{
		DecoHide(s_mirror_plate);
		return;
	}
	OMediaRect b;
	s_mirror->getbounds(&b);				// window units: 480 lines, the 4:3 area from WideMargin()
	// 12 down (the original: 4) so the bezel shows, or just below the energy meter when they would
	// overlap (4:3: the meter's number reaches under the mirror)
	float top640 = 12.0f;
	{
		const float sx0 = g_screen_w * (1.0f / 640.0f);
		const float mx = (float)b.left / sx0 - WideMargin();
		if (s_energy_right + 6.0f > mx) top640 = s_energy_bottom + 8.0f;
	}
	const int top = (int)(g_screen_h * (top640 / 480.0f));
	if (b.top != top)
	{
		b.bottom += top - b.top;
		b.top = top;
		s_mirror->setbounds(&b);
	}
	const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
	const float x = (float)b.left / sx - WideMargin(), y = (float)b.top / sy;
	const float w = (float)(b.right - b.left) / sx, h = (float)(b.bottom - b.top) / sy;
	const float m = 5.0f;
	ShowPlate(s_mirror_plate, 3 + (int)(w + 2 * m) * 1000 + (int)(h + 2 * m), x - m, y - m, w + 2 * m, h + 2 * m);
}

// --------------------------------------------------------------------------- 5. the gadget hologram

// The current gadget / vehicle picture (g_help_icons[g_cur_gadget], screens.omt 80x70, bottom left
// at (0, 410) while the help is closed; drawn additively in full screen) is the game's "hologram".
// Bill's UI builds on it (a setting: NEG_UI_HOLO=a|b|c|off, default a; the launcher's "Gadget
// hologram" and the port menu set it):
//   a BAY: a framed bay bottom left, the hologram over a projector (scan lines, slow bob and turn,
//     flicker on change), the energy in LEDs along its top, the gadget's name;
//   b CONSOLE: a curved steel console along the bottom, a slot per gadget Jimmy has (the current
//     one lit), the hologram over a projector at its left end, energy LEDs beside it;
//   c WHEEL: a ring of the gadgets Jimmy has, bottom left, turning the current one to the top,
//     its hologram in the middle, energy LEDs above;
//   off: the game's own icon (as Vanilla).
// The holograms are the game's pictures recoloured at run time (green-cyan, scan lines, a bright
// rim), never stored. "Has": the vehicles in this level, the water gun / burp gun once collected
// (pickups page items 1 / 19), the other gadgets once used in this level, and the current one.
static const char *const kGadgetName[14] = { "ON FOOT", "ROCKET", "INVISIBLE", "JETPACK", "SCOOTER", "VR TANK",
	"WATER GUN", "GRAPPLE", "SWIM", "MINI SUB", "BURP GUN", "CRANE", "HOVER CYCLE", "LANDER" };

static volatile int s_holo_design = -1;
void SetHoloDesign(int design) { s_holo_design = design; }

static int HoloDesign(void)
{
	int d = s_holo_design;
	if (d < 0)
	{
		const char *e = getenv("NEG_UI_HOLO");
		d = 1;
		if (e && (e[0] == 'b' || e[0] == 'B')) d = 2;
		else if (e && (e[0] == 'c' || e[0] == 'C')) d = 3;
		else if (e && (e[0] == 'o' || e[0] == 'O' || e[0] == '0')) d = 0;
		s_holo_design = d;
	}
	return d;
}

// The gadgets Jimmy has (bits 1..13).
static unsigned s_holo_level_mask, s_holo_seen, s_holo_scan_frame;
static char s_holo_level[64];

static unsigned OwnedGadgets(int g)
{
	if (strncmp(s_holo_level, g_game_file, sizeof s_holo_level - 1) != 0)	// a new level
	{
		strncpy(s_holo_level, g_game_file, sizeof s_holo_level - 1);
		s_holo_seen = 0;
		s_holo_scan_frame = 0;
	}
	if (s_holo_scan_frame == 0 || s_frame - s_holo_scan_frame > 120)	// the vehicles here (every 2 s)
	{
		static const struct { const char *tag; int g; } kVehicles[] = {
			{ "C3DROCKETSHIP", 1 }, { "C3DJEEP", 4 }, { "C3DVRTANK", 5 }, { "C3DMINISUB", 9 },
			{ "C3DCRANEBOOM", 11 }, { "C3DHOVERCYCLE", 12 }, { "C3DLUNARLANDER", 13 } };
		unsigned m = 0;
		for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
			if (*it)
				for (unsigned k = 0; k < sizeof kVehicles / sizeof kVehicles[0]; k++)
					if (strcasecmp((*it)->m_tag, kVehicles[k].tag) == 0) m |= 1u << kVehicles[k].g;
		s_holo_level_mask = m;
		s_holo_scan_frame = s_frame ? s_frame : 1;
	}
	unsigned m = s_holo_level_mask;
	C2DInGameMenu::MenuPage *p2 = C2DInGameMenu::GetPage(2);
	if (p2 && p2->items[1].type) m |= 1u << 6;				// WATERGUN collected
	if (p2 && p2->items[19].type) m |= 1u << 10;			// MEGABURP collected
	if (g > 0 && g < 14) s_holo_seen |= 1u << g;
	return (m | s_holo_seen) & 0x3ffeu;
}

static OMediaCanvas *s_holo_can[14], *s_holo_src[14];

// Gadget i's picture as a hologram (cached per source canvas).
static OMediaCanvas *HoloCanvas(int i)
{
	if (i < 0 || i >= 14 || !g_help_icons[i]) return NULL;
	OMediaCanvas *src = g_help_icons[i]->get_canvas();
	if (!src || src->get_width() < 8) return NULL;
	if (s_holo_can[i] && s_holo_src[i] == src) return s_holo_can[i];
	const int w = src->get_width(), h = src->get_height();
	OMediaCanvas *c = new OMediaCanvas;
	c->create(w, h);
	src->lock(omlf_Read);
	c->lock(omlf_Write);
	const unsigned char *sp = (const unsigned char *)src->get_pixels();
	unsigned int *dp = (unsigned int *)c->get_pixels();
	if (sp && dp)
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
			{
				const unsigned char *q = sp + (y * w + x) * 4;
				const float a = q[3] / 255.0f;
				const float L = (0.3f * q[0] + 0.59f * q[1] + 0.11f * q[2]) / 255.0f;
				// a rim where the alpha falls off (the outline glows)
				float na = 0;
				int n = 0;
				for (int dy = -1; dy <= 1; dy++)
					for (int dx = -1; dx <= 1; dx++)
					{
						const int xx = x + dx, yy = y + dy;
						if (xx < 0 || yy < 0 || xx >= w || yy >= h) { n++; continue; }
						na += sp[(yy * w + xx) * 4 + 3] / 255.0f;
						n++;
					}
				const float rim = Clamp01(a - na / n) * 2.5f;
				const float scan = (y % 3 == 0) ? 0.55f : 1.0f;
				const float v = (0.35f + 0.85f * L) * scan;
				const float al = Clamp01(a * (0.35f + 0.65f * L) * scan + rim);
				dp[y * w + x] = Rgba(0.30f * v + rim, 0.95f * v + rim, 0.75f * v + rim, al);
			}
	c->unlock();
	src->unlock();
	delete s_holo_can[i];
	s_holo_can[i] = c;
	s_holo_src[i] = src;
	return c;
}

// A pool of sprite elements for the holograms (placed each frame, the rest hidden).
enum { HOLO_POOL = 20 };
static OMediaCanvasElementWrapper *s_pool[HOLO_POOL];
static int s_pool_used;

static void ElemPlace(OMediaCanvasElement *e, float x, float y, float w, float h)
{
	const float sx = g_screen_w * (1.0f / 640.0f), sy = g_screen_h * (1.0f / 480.0f);
	e->set_size(w * sx, h * sy);
	e->place(x * sx - (float)(g_screen_w / 2), (float)(g_screen_h / 2) - (y + h) * sy, 0);
	if (!e->getvisible()) e->hide(false);
}

static void PoolPut(OMediaCanvas *c, float x, float y, float w, float h, float a)
{
	if (!c || s_pool_used >= HOLO_POOL) return;
	OMediaCanvasElementWrapper *&e = s_pool[s_pool_used++];
	if (!e)
	{
		e = new OMediaCanvasElementWrapper;
		e->link_layer(g_layer_hud2d);
		e->set_canvas_flags(omcanef_FreeWorldSize);
		e->set_auto_align(omaac_Left, omaac_Bottom);
		e->update_logic(0);
		e->set_blend(omblendfc_Src_Alpha, omblendfc_One);
	}
	if (e->get_canvas() != c) e->set_canvas(c);
	e->set_diffuse(OMediaFARGBColor(Clamp01(a), 1.0f, 1.0f, 1.0f));
	ElemPlace(e, x, y, w, h);
}

static void PoolEnd(void)
{
	for (int i = s_pool_used; i < HOLO_POOL; i++)
		if (s_pool[i] && s_pool[i]->getvisible()) s_pool[i]->hide(true);
}

static Deco s_holo_bg, s_holo_fx, s_holo_led;
static float s_holo_t, s_holo_change, s_wheel_angle = -1000.0f;
static int s_holo_last = -1;
static bool s_icon_hidden;

// LEDs for the energy (n cells in a row, or a column), redrawn into s_holo_led.
static void HoloLeds(float x, float y, float w, float h, int n, bool vertical)
{
	const int k = 2, pw = (int)(w * k), ph = (int)(h * k);
	unsigned int *px = DecoBegin(s_holo_led, g_layer_hud2d, pw, ph, false);
	float col[3];
	EnergyColour(s_hb < 0 ? 100.0f : s_hb, col);
	const float e = s_hb < 0 ? 100.0f : s_hb;
	if (px)
		for (int j = 0; j < ph; j++)
			for (int i = 0; i < pw; i++)
			{
				const float u = (i + 0.5f) / k, v = (j + 0.5f) / k;
				const float len = vertical ? h : w, along = vertical ? h - v : u, across = vertical ? u : v;
				const float cell = len / n;
				const int ci = (int)(along / cell);
				const float cu = along - ci * cell;
				const float th = vertical ? w : h;
				if (cu < 0.8f || cu > cell - 0.8f || across < 0.8f || across > th - 0.8f) { px[j * pw + i] = Rgba(0.02f, 0.06f, 0.04f, 0.8f); continue; }
				const float on = Clamp01(e / 100.0f * n - ci);
				px[j * pw + i] = Rgba(0.05f + col[0] * on, 0.12f + col[1] * 0.9f * on, 0.07f + col[2] * on, 0.95f);
			}
	DecoDone(s_holo_led);
	DecoPlace(s_holo_led, x, y, w, h);
}

// The projector's light: a soft cone from an emitter ellipse (additive), drawn once.
static void HoloCone(Deco &d, float x, float y, float w, float h)
{
	if (!d.elt || d.key != 77)
	{
		const int pw = 64, ph = 128;
		unsigned int *px = DecoBegin(d, g_layer_hud2d, pw, ph, true);
		if (px)
			for (int j = 0; j < ph; j++)
				for (int i = 0; i < pw; i++)
				{
					const float u = (i + 0.5f) / pw * 2.0f - 1.0f, v = (j + 0.5f) / ph;	// v: 0 top, 1 bottom
					const float half = 0.18f + 0.82f * (1.0f - v);					// wider at the top
					float a = Clamp01(1.0f - fabsf(u) / half) * (0.10f + 0.25f * v);
					const float ey = (v - 0.93f) / 0.05f;								// the emitter
					const float er = u * u / 0.25f + ey * ey;
					if (er < 1.0f) a += (1.0f - er) * 0.9f;
					px[j * pw + i] = Rgba(0.5f, 1.0f, 0.8f, a);
				}
		DecoDone(d);
		d.key = 77;
	}
	DecoPlace(d, x, y, w, h);
}

// The hologram of gadget g with its centre bottom at (cx, by), size about w x h: a slow bob and
// turn, and a flicker for a moment after a change.
static void HoloFigure(int g, float cx, float by, float w, float h)
{
	OMediaCanvas *c = HoloCanvas(g);
	if (!c) return;
	const float t = s_holo_t;
	const float turn = 0.80f + 0.20f * cosf(t * 0.8f);
	const float bob = sinf(t * 1.7f) * 2.0f;
	float a = 0.85f + 0.10f * sinf(t * 23.0f) * sinf(t * 7.0f);
	float grow = 1.0f;
	if (s_holo_change > 0)
	{
		grow = 1.0f - s_holo_change * 0.6f;
		a *= ((int)(t * 30.0f) & 1) ? 0.35f : 1.0f;
	}
	const float ww = w * turn * grow, hh = h * grow;
	PoolPut(c, cx - ww * 0.5f, by - hh + bob, ww, hh, a);
}

static void HoloText(float cx, float y, const char *text)
{
	const int keep = g_text_center;
	g_text_center = 1;
	const float rgb[3] = { g_text_rgb[0], g_text_rgb[1], g_text_rgb[2] };
	g_text_rgb[0] = 0.62f; g_text_rgb[1] = 1.0f; g_text_rgb[2] = 0.68f;
	DrawText((short)(int)cx, (short)(int)y, 'SMAL', "%s", text);
	g_text_rgb[0] = rgb[0]; g_text_rgb[1] = rgb[1]; g_text_rgb[2] = rgb[2];
	g_text_center = keep;
}

static void HoloHideAll(void)
{
	DecoHide(s_holo_bg); DecoHide(s_holo_fx); DecoHide(s_holo_led);
	s_pool_used = 0;
	PoolEnd();
}

static void HoloFrame(float dt, bool on)
{
	const int design = on ? HoloDesign() : 0;
	OMediaCanvasElementWrapper *icon = (g_help_index >= 0 && g_help_index < 14) ? g_help_icons[g_help_index] : NULL;
	const bool playing = g_game_mode == 5 && g_menu && g_menu->f_594 == 0;
	if (design == 0 || !playing)
	{
		HoloHideAll();
		if (design == 0 && s_icon_hidden)						// the game's icon back
		{
			if (icon && playing && g_help_index != 0) icon->force_show();
			s_icon_hidden = false;
		}
		return;
	}
	if (icon && icon->getvisible()) icon->hide(true);		// ours instead
	s_icon_hidden = true;
	s_holo_t += dt;
	const int g = g_help_index;
	if (g != s_holo_last) { s_holo_change = 1.0f; s_holo_last = g; }
	s_holo_change -= dt * 2.5f;
	if (s_holo_change < 0) s_holo_change = 0;
	const bool any = g > 0 && g < 14 && HoloCanvas(g);
	const unsigned owned = OwnedGadgets(g);
	int list[14], n = 0;
	for (int i = 1; i < 14; i++)
		if ((owned >> i) & 1) list[n++] = i;
	s_pool_used = 0;
	const float L = -WideMargin();

	if (design == 1)											// a: the bay
	{
		if (!any) { HoloHideAll(); return; }
		const float bw = 140.0f, bh = 128.0f, x0 = L + 8.0f, y0 = 480.0f - 8.0f - bh;
		ShowPlate(s_holo_bg, 11, x0, y0, bw, bh);
		HoloLeds(x0 + 10.0f, y0 + 6.0f, bw - 20.0f, 7.0f, 10, false);
		HoloCone(s_holo_fx, x0 + 25.0f, y0 + 20.0f, bw - 50.0f, 92.0f);
		HoloFigure(g, x0 + bw * 0.5f, y0 + 100.0f, 92.0f, 80.0f);
		HoloText(x0 + bw * 0.5f, y0 + bh - 5.0f, kGadgetName[g]);
	}
	else if (design == 2)										// b: the console
	{
		if (n == 0) { HoloHideAll(); return; }
		// [LEDs][projector][slot][slot]... centred on the 4:3 area, its top curved
		const float sp = 30.0f, pj = 56.0f;
		const float cw = 14.0f + pj + n * sp + 10.0f, ch = 40.0f, x0 = 320.0f - cw * 0.5f, y0 = 480.0f - ch - 2.0f;
		const int key = 2200 + n;
		if (!s_holo_bg.elt || s_holo_bg.key != key)
		{
			const int k = 2, pw = (int)(cw * k), ph = (int)(ch * k);
			unsigned int *px = DecoBegin(s_holo_bg, g_layer_hud2d, pw, ph, false);
			if (px)
				for (int j = 0; j < ph; j++)
					for (int i = 0; i < pw; i++)
					{
						const float u = (i + 0.5f) / k, v = (j + 0.5f) / k;
						const float cu = (u - cw * 0.5f) / (cw * 0.5f);
						const float top = 2.0f + 10.0f * cu * cu;				// the curved top edge
						const float d = top - v;
						const float side = SdBox(u, v, cw * 0.5f, ch * 0.5f + 6.0f, cw * 0.5f, ch * 0.5f + 6.0f, 10.0f);
						if (d > 1.0f || side > 1.0f) continue;
						float c[3];
						Steel(Clamp01((v - top) / (ch - top)), j, c);
						const float a = Clamp01(1.0f - d * k) * Clamp01(0.5f - side * k);
						const float edge = Clamp01(1.0f - fabsf(v - top - 1.0f) * 1.2f);	// a green rim
						const float sy0 = top + 7.0f;
						const float su = u - 14.0f - pj;
						const int si = su >= 0 ? (int)(su / sp) : -1;
						const float in = su - si * sp;
						if (si >= 0 && si < n && in > 2.0f && in < sp - 2.0f && v > sy0 && v < sy0 + 25.0f)
						{ c[0] = 0.02f; c[1] = 0.07f; c[2] = 0.05f; }
						const float pd = SdCircle(u, v, 14.0f + pj * 0.5f, top + 18.0f, 16.0f);	// the projector
						if (pd < 0) { const float q = Clamp01(-pd / 16.0f); c[0] = 0.05f + 0.2f * q; c[1] = 0.15f + 0.6f * q; c[2] = 0.1f + 0.35f * q; }
						px[j * pw + i] = Rgba(c[0] + edge * 0.3f, c[1] + edge * 0.8f, c[2] + edge * 0.45f, a);
					}
			DecoDone(s_holo_bg);
			s_holo_bg.key = key;
		}
		DecoPlace(s_holo_bg, x0, y0, cw, ch);
		for (int q = 0; q < n; q++)
		{
			const int i = list[q];
			const float sx0 = x0 + 14.0f + pj + q * sp;
			const float cu = (sx0 + sp * 0.5f - 320.0f) / (cw * 0.5f);
			const float sy0 = y0 + 2.0f + 10.0f * cu * cu + 7.0f;
			PoolPut(HoloCanvas(i), sx0 + 2.0f, sy0 + 1.0f, sp - 4.0f, 23.0f, i == g ? 1.0f : 0.28f);
		}
		HoloLeds(x0 + 3.0f, y0 + 12.0f, 8.0f, 24.0f, 5, true);
		const float pcx = x0 + 14.0f + pj * 0.5f, pcu = (pcx - 320.0f) / (cw * 0.5f), ptop = y0 + 2.0f + 10.0f * pcu * pcu;
		if (any)
		{
			HoloCone(s_holo_fx, pcx - 28.0f, ptop - 62.0f, 56.0f, 80.0f);
			HoloFigure(g, pcx, ptop + 6.0f, 72.0f, 63.0f);
		}
		else DecoHide(s_holo_fx);
	}
	else														// c: the wheel
	{
		if (n == 0) { HoloHideAll(); return; }
		const float R = 70.0f, cx = L + 10.0f + R, cy = 480.0f - 8.0f - R;
		if (!s_holo_bg.elt || s_holo_bg.key != 33)
		{
			const int k = 2, nn = (int)(2 * R * k);
			unsigned int *px = DecoBegin(s_holo_bg, g_layer_hud2d, nn, nn, false);
			if (px)
				for (int j = 0; j < nn; j++)
					for (int i = 0; i < nn; i++)
					{
						const float u = (i + 0.5f) / k - R, v = (j + 0.5f) / k - R, r = sqrtf(u * u + v * v);
						if (r > R) continue;
						float c[3];
						if (r > R - 7.0f) Steel(Clamp01((v + R) / (2 * R)), j, c);		// steel rim
						else if (r > R - 30.0f) { c[0] = 0.03f; c[1] = 0.10f; c[2] = 0.06f; }	// the slot ring
						else { c[0] = 0.01f; c[1] = 0.05f; c[2] = 0.03f; }					// the projector well
						const float a = Clamp01((R - r) * k) * (r > R - 30.0f ? 0.92f : 0.6f);
						const float line = Clamp01(1.0f - fabsf(r - (R - 30.0f)) * 1.5f) + Clamp01(1.0f - fabsf(r - (R - 7.0f)) * 1.5f);
						// the pointer notch at the top
						const float notch = (v < -(R - 34.0f) && fabsf(u) < (v + R - 26.0f) * -0.6f) ? 1.0f : 0.0f;
						px[j * nn + i] = Rgba(c[0] + line * 0.3f + notch * 0.4f, c[1] + line * 0.9f + notch, c[2] + line * 0.5f + notch * 0.6f, a + line * 0.3f);
					}
			DecoDone(s_holo_bg);
			s_holo_bg.key = 33;
		}
		DecoPlace(s_holo_bg, cx - R, cy - R, 2 * R, 2 * R);
		// the ring turns so that the current gadget is at the top
		const float pi = 3.14159265f, step = 2.0f * pi / (float)(n < 3 ? 3 : n);
		int cur = -1;
		for (int q = 0; q < n; q++) if (list[q] == g) cur = q;
		const float want = pi * 0.5f + (cur >= 0 ? cur : 0) * step;
		if (s_wheel_angle < -100.0f) s_wheel_angle = want;
		float diff = want - s_wheel_angle;
		while (diff > pi) diff -= 2 * pi;
		while (diff < -pi) diff += 2 * pi;
		s_wheel_angle += diff * (1.0f - expf(-dt * 6.0f));
		for (int q = 0; q < n; q++)
		{
			const float an = s_wheel_angle - q * step;
			const float px = cx + cosf(an) * (R - 18.5f), py = cy - sinf(an) * (R - 18.5f);
			const bool lit = list[q] == g;
			const float w = lit ? 28.0f : 22.0f, h = w * 0.875f;
			PoolPut(HoloCanvas(list[q]), px - w * 0.5f, py - h * 0.5f, w, h, lit ? 1.0f : 0.3f);
		}
		HoloLeds(cx - 34.0f, cy - R - 10.0f, 68.0f, 6.0f, 10, false);	// the energy, above the wheel
		DecoHide(s_holo_fx);
		if (any) HoloFigure(g, cx, cy + 20.0f, 50.0f, 44.0f);
	}
	PoolEnd();
}

// --------------------------------------------------------------------------- per frame

void BillsUiFrame(float dt)
{
	s_frame++;
	const bool on = BillsUi();
	if (!on || s_frame - s_sonar_frame > 2) DecoHide(s_sonar_plate);
	if (!on || s_frame - s_hb_frame > 2) DecoHide(s_hb_halo);
	if (!on) s_hb = -1.0f;
	if (!on || s_frame - s_energy_frame > 2) DecoHide(s_energy);
	if (!on && s_orig_hidden && (g_game_mode == 5 || g_game_mode == 6)) ShowOriginalBar(true);
	ScopeFrame(dt, on && g_game_mode == 4 && g_menu && g_menu->f_4ce == 3);
	MirrorFrame(on);
	HoloFrame(dt, on);
	if (g_help_index >= 0 && g_help_index < 14) HelpPieceFrame(s_help[0], &g_elt_544908[g_help_index], WIDE_LEFT, on);
	HelpPieceFrame(s_help[1], g_help_menu_elt, WIDE_RIGHT, on);
	if (!on || (g_game_mode != 5 && g_game_mode != 6))
	{
		DecoHide(s_score_halo);
		if (!on) s_score_shown = -1.0;
	}
}

void BillsUiShutdown(void)
{
	DecoFree(s_sonar_plate);
	DecoFree(s_scope);
	DecoFree(s_score_halo);
	DecoFree(s_hb_halo);
	DecoFree(s_energy);
	DecoFree(s_mirror_plate);
	DecoFree(s_holo_bg); DecoFree(s_holo_fx); DecoFree(s_holo_led);
	for (int i = 0; i < HOLO_POOL; i++) { delete s_pool[i]; s_pool[i] = NULL; }
	for (int i = 0; i < 14; i++) { delete s_holo_can[i]; s_holo_can[i] = NULL; s_holo_src[i] = NULL; }
	s_icon_hidden = false;
	s_holo_last = -1;
	s_orig_hidden = false;
	s_hb = -1.0f;
	for (int i = 0; i < 2; i++)
	{
		DecoFree(s_help[i].plate);
		s_help[i].moved = NULL;
		s_help[i].linked = NULL;
		s_help[i].box_of = NULL;
	}
}

}

#ifdef __ANDROID__
#include <jni.h>

// Companion.setGadgetHologram (the port menu's "Gadget hologram" item): 0 off, 1 BAY, 2 CONSOLE, 3 WHEEL
extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetGadgetHologram(JNIEnv *, jclass, jint design)
{
	if (design >= 0 && design <= 3) neg::SetHoloDesign(design);
}

// Companion.setEnergyMeter (the port menu's "Energy meter" item): 1 TUBE, 2 ORBIT, 3 DIAL
extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetEnergyMeter(JNIEnv *, jclass, jint design)
{
	if (design >= 1 && design <= 3) neg::SetEnergyDesign(design);
}

// Companion.setBillsUi (the in-game port menu's "In-game UI" item)
extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetBillsUi(JNIEnv *, jclass, jboolean on)
{
	neg::SetBillsUi(on != JNI_FALSE);
}
#endif
