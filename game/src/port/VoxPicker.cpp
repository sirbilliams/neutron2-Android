/*  port: the Vox world picker - see VoxPicker.h. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in the OMT headers)
#include "VoxPicker.h"
#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../CanvasWrapper.h"
#include "../Game.h"
#include "../Mouse.h"
#include "../objects/C3DHelpSound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <SDL.h>
#include <list>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace neg;

namespace {

typedef C2DInGameMenu M;
enum { kMax = 8 };

bool		s_on = false;			// the picker runs for this visit of the page
bool		s_pointer = false;		// handed back to the pointer (touch / mouse / right stick moved it)
Uint32		s_seen = 0;				// SDL ticks of the last frame the page was handled
int			s_sel = 0;
int			s_items[kMax];			// the pictures, left to right (main-menu item numbers)
int			s_count = 0;
short		s_px = -1, s_py = -1;	// the chosen picture's centre (pointer space)
short		s_rx = -1, s_ry = -1;	// the real pointer when last seen (the original's d-pad drift followed)
int			s_dir = 0;				// -1 / +1 held
Uint32		s_repeat = 0;			// when a held direction steps again
bool		s_b_armed = false;		// B seen up since the page opened
bool		s_b_down = false;

bool Enabled(void)
{
	const char *e = getenv("NEG_VOX_PICKER");
	return !e || strcmp(e, "0") != 0;
}

// The world pictures of the main page: its items in the grid (the side list sits at x 538, the
// buttons at y 17 / 426), left to right.
int Collect(int *out)
{
	M::MenuPage *p = M::Page(M::MENU_MAIN);
	if (!p) return 0;
	int n = 0;
	for (int i = 0; i < M::MENU_ITEMS && n < kMax; i++)
	{
		const M::MenuItem &it = p->items[i];
		if (it.type == 0 || it.type == 4 || !it.rollover) continue;
		if (it.y < 200.0f || it.y > 400.0f || it.x > 500.0f) continue;
		int k = n++;
		while (k > 0 && p->items[out[k - 1]].x > it.x) { out[k] = out[k - 1]; k--; }
		out[k] = i;
	}
	return n;
}

// The picture's rectangle in pointer space (UpdateRollovers' own test, 0x405030).
void ItemRect(int item, short &l, short &t, short &w, short &h)
{
	OMediaCanvasElementWrapper *r = M::Page(M::MENU_MAIN)->items[item].rollover;
	l = (short)(int)((float)((short)g_screen_w / 2) + r->get_x());
	t = (short)(int)-((r->get_y() + r->get_height()) - (float)((short)g_screen_h / 2));
	w = (short)r->get_width();
	h = (short)r->get_height();
}

// A picture's world name: the side list's HELPMENUITEM for the same warp (C2DInGameMenu::v342
// takes both numbers for one level), else the MENUITEM's name without its "MM".
void ItemName(int item, char *out, size_t size)
{
	static const short kSame[][2] = { { 33, 16 }, { 28, 15 }, { 34, 5 }, { 19, 31 }, { 18, 18 },
									   { 36, 21 }, { 25, 22 }, { 26, 23 }, { 27, 24 }, { 29, 20 } };
	int help = -1;
	for (size_t k = 0; k < sizeof kSame / sizeof kSame[0]; k++)
		if (kSame[k][0] == item) help = kSame[k][1];
	out[0] = 0;
	std::list<M::MenuObject *>::iterator i;
	for (i = M::s_objects.begin(); help >= 0 && i != M::s_objects.end(); ++i)
		if ((*i)->type == M::MOBJ_HELPMENUITEM && (*i)->page == M::MENU_MAIN && (*i)->id == help)
		{
			strncpy(out, (*i)->text, size - 1);
			out[size - 1] = 0;
			return;
		}
	for (i = M::s_objects.begin(); i != M::s_objects.end(); ++i)
		if ((*i)->type == M::MOBJ_MENUITEM && (*i)->page == M::MENU_MAIN && (*i)->id == item)
		{
			const char *s = (*i)->name;
			if (strncasecmp(s, "MM", 2) == 0) s += 2;
			strncpy(out, s, size - 1);
			out[size - 1] = 0;
			return;
		}
}

// Jimmy out of the terminal's radius, away from its centre (backwards when he stands on it).
void StepJimmyBack(void)
{
	C3DJimmy *j = Jimmy();
	if (!j) return;
	CGameObject *jg = j;
	float jp[4];
	jg->v195(jp);
	C3DHelpSound *best = NULL;
	float bd = 1e30f;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		C3DHelpSound *h = *i ? dynamic_cast<C3DHelpSound *>(*i) : NULL;
		if (!h || strcasecmp(h->m_next_trigger, "MAINMENU") != 0) continue;
		float hp[4];
		static_cast<CGameObject *>(h)->v195(hp);
		const float d = (hp[0] - jp[0]) * (hp[0] - jp[0]) + (hp[2] - jp[2]) * (hp[2] - jp[2]);
		if (d < bd) { bd = d; best = h; }
	}
	if (!best) return;
	float hp[4];
	static_cast<CGameObject *>(best)->v195(hp);
	float dx = jp[0] - hp[0], dz = jp[2] - hp[2];
	float len = sqrtf(dx * dx + dz * dz);
	if (len < 1.0f)
	{
		const OMedia3DVector f = jg->v226(0, 0, 1.0f);		// his facing; step back from it
		dx = -f.x; dz = -f.z;
		len = sqrtf(dx * dx + dz * dz);
		if (len < 1e-3f) { dx = 0; dz = -1; len = 1; }
	}
	const float want = static_cast<CGameObject *>(best)->f_034 + 60.0f;
	if (len >= want) return;
	jg->v196(hp[0] + dx / len * want, jp[1], hp[2] + dz / len * want, jp[3]);
}

void Label(short x, short y, int font, bool lit, const char *s)
{
	g_text_center = 1;
	g_text_rgb[0] = g_text_rgb[2] = 0;									// shadow
	g_text_rgb[1] = 0;
	DrawText((short)(x + 1), (short)(y + 1), font, "%s", s);
	g_text_rgb[0] = lit ? 0.8f : 0.3f;
	g_text_rgb[1] = lit ? 1.0f : 0.7f;
	g_text_rgb[2] = lit ? 0.8f : 0.35f;
	DrawText(x, y, font, "%s", s);
	g_text_rgb[0] = g_text_rgb[1] = g_text_rgb[2] = 1.0f;
	g_text_center = 0;
}

} // namespace

void neg::VoxPickerOpened(void)
{
	if (!Enabled()) return;
	s_on = true;
	s_pointer = false;
	s_sel = 0;
	s_seen = SDL_GetTicks();
	s_px = s_py = s_rx = s_ry = -1;
	s_dir = 0;
	s_b_armed = false;
	s_b_down = false;
}

bool neg::VoxPickerFrame(C2DInGameMenu *m)
{
	const Uint32 now = SDL_GetTicks();
	if (s_on && now - s_seen > 300) s_on = false;			// the menu was closed in between
	s_seen = now;
	if (!s_on) return false;
	if (m->v301() != M::MENU_MAIN || !g_menu_active) { s_on = false; return false; }
	s_count = Collect(s_items);
	if (s_count == 0) return false;
	if (s_sel >= s_count) s_sel = s_count - 1;

	// B: back out (on release, so Jimmy does not jump with it)
	const bool b = g_joy_button1 != 0 || g_keys[0x08] != 0;	// pad B, Backspace
	if (!b) s_b_armed = true;
	if (b && s_b_armed) s_b_down = true;
	if (!b && s_b_down)
	{
		s_on = false;
		m->v132(1);											// RESUME's own path (HandleMenuInput)
		m->DeactivateActiveMenu();
		m->MainMenuAction(2);
		StepJimmyBack();
		return true;
	}

	// left / right: d-pad, left stick, arrow keys
	const int dir = (g_joy_x < -0.5f || g_keys[0x25]) ? -1 : (g_joy_x > 0.5f || g_keys[0x27]) ? 1 : 0;
	const bool any = dir != 0 || g_joy_y < -0.5f || g_joy_y > 0.5f || g_keys[0x26] || g_keys[0x28];
	// the real pointer: moved by the player (not by the original's d-pad drift, 0x40a170) = his
	short mx, my;
	GetMousePos(&mx, &my);
	if (any || s_rx < 0) { s_rx = mx; s_ry = my; }
	else if (abs(mx - s_rx) > 3 || abs(my - s_ry) > 3) { s_pointer = true; s_rx = mx; s_ry = my; }
	if (dir != s_dir || (dir != 0 && (Sint32)(now - s_repeat) >= 0))
	{
		if (dir != 0)
		{
			s_repeat = now + (dir != s_dir ? 350 : 180);
			if (s_pointer) s_pointer = false;				// the pad takes over again
			else s_sel = s_sel + dir < 0 ? 0 : s_sel + dir >= s_count ? s_count - 1 : s_sel + dir;
		}
		s_dir = dir;
	}
	short l, t, w, h;
	ItemRect(s_items[s_sel], l, t, w, h);
	s_px = (short)(l + w / 2);
	s_py = (short)(t + h / 2);

	// labels (640 x 480 menu space)
	M::MenuPage *p = M::Page(M::MENU_MAIN);
	for (int k = 0; k < s_count; k++)
	{
		const M::MenuItem &it = p->items[s_items[k]];
		ItemRect(s_items[k], l, t, w, h);
		char name[64];
		ItemName(s_items[k], name, sizeof name);
		const bool lit = !s_pointer && k == s_sel;
		Label((short)(it.x + w / 2), (short)(it.y + h + 26), 'MED', lit, name);
	}
	Label(320, 236, 'SMAL', false, "LEFT RIGHT  CHOOSE    A  GO    B  BACK");	// (below the info box)
	return false;
}

bool neg::VoxPickerPointer(short &x, short &y)
{
	if (!s_on || s_pointer || s_count == 0 || s_px < 0) return false;
	x = s_px;
	y = s_py;
	return true;
}

int neg::VoxPickerCards(VoxCard *cards, int max, int *sel)
{
	if (!s_on || SDL_GetTicks() - s_seen > 300 || !g_menu || !M::Page(M::MENU_MAIN)) return 0;
	M::MenuPage *p = M::Page(M::MENU_MAIN);
	int n = 0;
	for (int k = 0; k < s_count && n < max; k++, n++)
	{
		ItemName(s_items[k], cards[n].name, sizeof cards[n].name);
		cards[n].wire = p->items[s_items[k]].active_id;
		cards[n].colour = p->items[s_items[k]].rollover_id;
	}
	*sel = s_sel;
	if (s_pointer)											// the picture under the pointer
	{
		*sel = -1;
		for (int k = 0; k < n; k++)
			if (p->items[s_items[k]].shown) *sel = k;
	}
	return n;
}
