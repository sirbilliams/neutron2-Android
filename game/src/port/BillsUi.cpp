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

// --------------------------------------------------------------------------- per frame

void BillsUiFrame(float dt)
{
	s_frame++;
	const bool on = BillsUi();
	if (!on || s_frame - s_sonar_frame > 2) DecoHide(s_sonar_plate);
	ScopeFrame(dt, on && g_game_mode == 4 && g_menu && g_menu->f_4ce == 3);
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

// Companion.setBillsUi (the in-game port menu's "In-game UI" item)
extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetBillsUi(JNIEnv *, jclass, jboolean on)
{
	neg::SetBillsUi(on != JNI_FALSE);
}
#endif
