/*  The 2D interface elements and their effect queue - see InterfaceElement.h.
    Reconstructed from Neutron2.exe 0x480540..0x482700, 0x482b40, 0x482b50.

    Constants: 0x4ca4bc = 1/640, 0x4ca4b8 = 1/480 (screen scale), 0x4caa44 = 0, 0x4caa5c = 0.5,
    0x4caa60 = 1.5, 0x4caa6c = 1. Float to int conversions are _ftol (truncation).
    The original computes with the x87 at extended precision and stores to float only here and
    there; the float arithmetic below can differ from it in the last bit of a canvas size or
    position before truncation, nowhere else.                                                  */

#include "InterfaceElement.h"
#include "CanvasWrapper.h"
#include "CGameObject.h"				// neg::ErrorLog
#include "Game.h"						// neg::LoadCanvasID, layers, screen size
#include "gen/globals.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include <string.h>
#include <strings.h>

using namespace neg;

namespace neg {
// Declaration order = the original's static initialisation order where it matters: the queue
// before the elements (whose constructor writes g_effect_queue_next).
InterfaceEffect				g_effect_queue[EFFECT_QUEUE_SIZE];		// 0x541da0
int							g_effect_queue_next = 0;				// 0x545da4
std::list<InterfaceElt *>	g_interface_elts;						// 0x54a414
InterfaceElement			g_interface;							// 0x545964

InterfaceElt	g_elt_5444d8;			// 0x5444d8
InterfaceElt	g_elt_5445e8;			// 0x5445e8
InterfaceElt	g_elt_544908[14];		// 0x544908
InterfaceElt	g_elt_545740;			// 0x545740
InterfaceElt	g_elt_545858;			// 0x545858
InterfaceElt	g_elt_545a70;			// 0x545a70
InterfaceElt	g_elt_545b78;			// 0x545b78
InterfaceElt	g_elt_545c80;			// 0x545c80

int		g_score = 0;					// 0x54a458
int		g_score_popup = 0;				// 0x54a45c
float	g_score_popup_time = 0.0f;		// 0x54a460
bool	g_flag_545e54 = false;			// 0x545e54
}

// --------------------------------------------------------------------------- helpers

static inline float screen_sx(void) { return (float)(short)g_screen_w * (1.0f / 640.0f); }	// 0x4ca4bc
static inline float screen_sy(void) { return (float)(short)g_screen_h * (1.0f / 480.0f); }	// 0x4ca4b8

// Inlined in Show and the MOVE / MORPH effects: centred canvas element at (fx, fy) in 640x480
// space (y down) on a centre-origin, y-up 2D layer.
static void place_on_screen(OMediaCanvasElement *ce, float fx, float fy)
{
	ce->set_canvas_flags(omcanef_FreeWorldSize);			// +0xc4 = 1
	ce->set_auto_align(omaac_Center, omaac_Center);			// +0xbc, +0xc0 = 3
	ce->place(screen_sx() * fx - (float)((short)g_screen_w / 2),
			  ((float)((short)g_screen_h / 2) - ce->get_height()) - screen_sy() * fy, 0);
}

// "ScaleCanv" (0x482390), also inlined in the SCALE / MORPH effects.
static void scale_canvas(OMediaCanvasElement *ce, float w, float h)
{
	if (!ce) { ErrorLog("ERROR: ScaleCanv CanvasElt = NULL"); return; }
	ce->set_size(screen_sx() * w, screen_sy() * h);
}

// The colour of a fading element: white with alpha a (diffuse at +0x90..+0x9c).
static inline void set_alpha(OMediaCanvasElement *ce, float a)
{
	ce->set_diffuse(OMediaFARGBColor(a, 1.0f, 1.0f, 1.0f));
}

// --------------------------------------------------------------------------- InterfaceEffect

void InterfaceEffect::Clear(void)
{
	type = IEFX_NONE;
	duration = 1.0f;
	time_left = 0;
	move_dx = 0;
	move_dy = -1.0f;
	move_dist = 64.0f;
	forward = true;
	active = false;
	f_1e = false;
	forward2 = false;
	scale_x = 0;
	scale_y = 0;
	duration2 = 1.0f;
	time_left2 = 0;
	f_34 = 0;
	alpha_from = 0;
	alpha_range = 1.0f;
	show = false;
	morph_w0 = morph_h0 = morph_w1 = morph_h1 = 0;
	morph_x0 = morph_y0 = morph_x1 = morph_y1 = 0;
}

// --------------------------------------------------------------------------- InterfaceElt

InterfaceElt::InterfaceElt()			// 0x480a10 (inlined in Create)
{
	effect.owner = NULL;				// (InterfaceEffect's constructor)
	db = NULL;
	canvas_elt = NULL;
	canvas = NULL;
	canvas_id = -1;
	x = 0;
	y = 0;
	home_x = 0;
	home_y = 0;
	src_blend = omblendfc_Src_Alpha;
	dest_blend = omblendfc_Inv_Src_Alpha;
	base_w = 0;
	base_h = 0;
	g_effect_queue_next = 0;			// sic: every new element restarts the queue
	f_2c = 0;
	current = 0;
	cur_x = 0;
	cur_y = 0;
	f_34 = 0;
	f_9c = 0;
	// name is left as is
}

// --------------------------------------------------------------------------- 0x480d20

void InterfaceElt::Reset(void)
{
	for (int i = 0; i < EFFECT_QUEUE_SIZE; i++)
		if (g_effect_queue[i].owner == this) g_effect_queue[i].Clear();
	effect.Clear();
	y = home_y;
	x = home_x;
	f_2c = 0;
	current = 0;
	g_effect_queue_next = 0;
	Show();								// creates the canvas element if needed ...
	if (canvas_elt) canvas_elt->hide(true);	// ... hidden
}

// --------------------------------------------------------------------------- effect queue
// 0x480b20 0x480ba0 0x480c60 0x480e70 0x480ec0 0x480f50 0x480fd0. Each writes slot
// g_effect_queue_next if that slot is free and advances it; otherwise the effect is lost.
// The original has no bound check (past slot 99 it would write over the globals that follow);
// here a full queue drops the effect like a busy slot does.

InterfaceEffect *InterfaceElt::NextQueueSlot(int type)
{
	if (g_effect_queue_next < 0 || g_effect_queue_next >= EFFECT_QUEUE_SIZE) return NULL;
	InterfaceEffect *e = &g_effect_queue[g_effect_queue_next];
	if (e->type != IEFX_NONE) return NULL;
	e->type = type;
	g_effect_queue_next++;
	e->owner = this;
	return e;
}

void InterfaceElt::QueueScale(int, float dur, bool fwd, bool active, bool show,		// 0x480b20
							  float amount_x, float amount_y)
{
	InterfaceEffect *e = NextQueueSlot(IEFX_SCALE);
	if (!e) return;
	e->time_left2 = dur;
	e->duration2 = dur;
	e->forward2 = fwd;
	e->active = active;
	e->scale_x = amount_x;
	e->scale_y = amount_y;
	e->show = show;
}

void InterfaceElt::QueueMorph(int, float dur, bool fwd, bool active, bool show,		// 0x480ba0
							  float w0, float h0, float w1, float h1,
							  float x0, float y0, float x1, float y1)
{
	InterfaceEffect *e = NextQueueSlot(IEFX_MORPH);
	if (!e) return;
	e->time_left2 = dur;
	e->duration2 = dur;
	e->forward2 = fwd;
	e->active = active;
	e->morph_w0 = w0;  e->morph_h0 = h0;  e->morph_w1 = w1;  e->morph_h1 = h1;
	e->morph_x0 = x0;  e->morph_y0 = y0;  e->morph_x1 = x1;  e->morph_y1 = y1;
	e->show = show;
}

void InterfaceElt::QueueMorphFlash(int, float dur, bool fwd, bool active, bool show,	// 0x480c60
								   float w0, float h0, float w1, float h1,
								   float x0, float y0, float x1, float y1)
{
	InterfaceEffect *e = NextQueueSlot(IEFX_MORPH_FLASH);
	if (!e) return;
	e->time_left2 = dur;
	e->duration2 = dur;
	e->forward2 = fwd;
	e->active = active;
	e->morph_w0 = w0;  e->morph_h0 = h0;  e->morph_w1 = w1;  e->morph_h1 = h1;
	e->morph_x0 = x0;  e->morph_y0 = y0;  e->morph_x1 = x1;  e->morph_y1 = y1;
	e->show = show;
}

void InterfaceElt::QueueWait(int, float dur)										// 0x480e70
{
	InterfaceEffect *e = NextQueueSlot(IEFX_WAIT);
	if (!e) return;
	e->time_left = dur;
	e->duration = dur;
	e->active = true;
}

void InterfaceElt::QueueMove(int, float dur, bool fwd, bool active, bool show,		// 0x480ec0
							 float dir_x, float dir_y, float distance)
{
	InterfaceEffect *e = NextQueueSlot(IEFX_MOVE);
	if (!e) return;
	e->time_left = dur;
	e->duration = dur;
	e->forward = fwd;
	e->active = active;
	e->move_dx = dir_x;
	e->move_dy = dir_y;
	e->move_dist = distance;
	e->show = show;
}

void InterfaceElt::QueueFade(int, float dur, bool fwd, bool show,					// 0x480f50
							 float alpha_from, float alpha_to)
{
	InterfaceEffect *e = NextQueueSlot(IEFX_FADE);
	if (!e) return;
	e->time_left = dur;
	e->duration = dur;
	e->forward = fwd;
	e->active = true;
	e->alpha_from = alpha_from;
	e->alpha_range = alpha_to - alpha_from;
	e->show = show;
}

void InterfaceElt::QueueHide(void)													// 0x480fd0
{
	InterfaceEffect *e = NextQueueSlot(IEFX_HIDE);
	if (!e) return;
	e->active = true;
}

// --------------------------------------------------------------------------- 0x481010

void InterfaceElt::Update(float dt)
{
	OMediaCanvasElement *ce;

	// Nothing running: take this element's first queued effect.
	if (current == 0)
	{
		for (short i = 0; i < EFFECT_QUEUE_SIZE; i++)
		{
			InterfaceEffect &q = g_effect_queue[i];
			if (q.type == IEFX_NONE || q.owner != this) continue;
			effect = q;
			q.Clear();
			current = effect.type;
			break;
		}
	}
	if (!effect.active) return;

	// Each kind below is tested in turn (as in the original); a finished effect is cleared to
	// type 0, so at most one runs per frame.

	// --- FADE
	if (effect.type == IEFX_FADE && effect.owner == this)
	{
		if (effect.show)
		{
			Show();
			canvas_elt->force_show();			// (again; Show already did)
		}
		else
		{
			if (canvas_elt) canvas_elt->hide(true);
			canvas_elt->hide(true);				// sic: hidden twice per frame, the second unchecked
		}
		if (effect.time_left > 0)
		{
			float t = effect.time_left - dt;
			effect.time_left = t;
			if (t > 0)
			{
				float a = t / effect.duration;
				if (effect.forward) a = 1.0f - a;
				set_alpha(canvas_elt, a * effect.alpha_range + effect.alpha_from);
			}
			else
			{
				effect.time_left = 0;
				current = 0;
				effect.active = false;
				if (effect.forward)
				{
					set_alpha(canvas_elt, effect.alpha_from + effect.alpha_range);
					if (effect.alpha_range == 0) canvas_elt->hide(true);
				}
				else
				{
					set_alpha(canvas_elt, 0);
					canvas_elt->hide(true);
				}
				effect.Clear();
			}
		}
	}

	// --- WAIT
	if (effect.type == IEFX_WAIT && effect.owner == this && effect.time_left > 0)
	{
		float t = effect.time_left - dt;
		effect.time_left = t;
		if (!(t > 0))
		{
			effect.time_left = 0;
			current = 0;
			effect.active = false;
			effect.Clear();
		}
	}

	// --- COMMIT_POS / GO_HOME (never queued by the game)
	if (effect.type == IEFX_COMMIT_POS && effect.owner == this)
	{
		current = 0;
		x = cur_x;
		y = cur_y;
		effect.active = false;
		effect.Clear();
	}
	if (effect.type == IEFX_GO_HOME && effect.owner == this)
	{
		current = 0;
		x = home_x;
		y = home_y;
		effect.active = false;
		effect.Clear();
	}

	// --- HIDE / SHOW: `current` is left set, so the queue stops here
	if (effect.type == IEFX_HIDE && effect.owner == this)
	{
		effect.active = false;
		effect.Clear();
		if (canvas_elt) canvas_elt->hide(true);
	}
	if (effect.type == IEFX_SHOW && effect.owner == this)
	{
		effect.active = false;
		effect.Clear();
		Show();
	}

	// --- MOVE
	if (effect.type == IEFX_MOVE && effect.owner == this)
	{
		if (effect.show) Show();
		else if (canvas_elt) canvas_elt->hide(true);

		if (effect.time_left > 0)
		{
			float t = effect.time_left - dt;
			effect.time_left = t;
			if (t > 0)
			{
				float a = t / effect.duration;
				if (effect.forward) a = 1.0f - a;
				cur_x = (short)(int)(effect.move_dx * effect.move_dist * a + (float)x);
				cur_y = (short)(int)(effect.move_dy * effect.move_dist * a + (float)y);
				if ((ce = canvas_elt) != NULL)
				{
					SetCanvas(ce, canvas_id, true);
					place_on_screen(ce, (float)cur_x, (float)cur_y);
				}
			}
			else
			{
				effect.time_left = 0;
				current = 0;
				if (effect.forward)
				{
					cur_x = (short)(int)(effect.move_dx * effect.move_dist + (float)x);
					cur_y = (short)(int)(effect.move_dy * effect.move_dist + (float)y);
					if ((ce = canvas_elt) != NULL)
					{
						SetCanvas(ce, canvas_id, true);
						place_on_screen(ce, (float)cur_x, (float)cur_y);
					}
					effect.active = false;
					effect.f_1e = true;			// (cleared again just below)
				}
				else
				{
					canvas_elt->hide(true);		// unchecked in the original
					effect.active = false;
					effect.f_1e = false;
				}
				effect.Clear();
			}
		}
	}

	// --- MORPH / MORPH_FLASH
	if ((effect.type == IEFX_MORPH || effect.type == IEFX_MORPH_FLASH) && effect.owner == this)
	{
		if (effect.show) Show();
		else if (canvas_elt) canvas_elt->hide(true);

		if (effect.time_left2 > 0)
		{
			float t = effect.time_left2 - dt;
			effect.time_left2 = t;
			if (t > 0)
			{
				float a = t / effect.duration2;
				if (effect.forward2) a = 1.0f - a;
				float dw = effect.morph_w1 - effect.morph_w0;
				float dh = effect.morph_h1 - effect.morph_h0;
				if (effect.type == IEFX_MORPH_FLASH)
				{
					if (a < 0) a = 0;			// (clamps a for the size and position too)
					set_alpha(canvas_elt, (a < 0.5f ? a : 1.0f - a) * 1.5f);	// unchecked
				}
				scale_canvas(canvas_elt, dw * a + effect.morph_w0, dh * a + effect.morph_h0);
				cur_x = (short)(int)((effect.morph_x1 - effect.morph_x0) * a + effect.morph_x0);
				cur_y = (short)(int)((effect.morph_y1 - effect.morph_y0) * a + effect.morph_y0);
				if ((ce = canvas_elt) != NULL) place_on_screen(ce, (float)cur_x, (float)cur_y);
			}
			else
			{
				effect.time_left2 = 0;
				current = 0;
				if (effect.forward2)
				{
					scale_canvas(canvas_elt, (effect.morph_w1 - effect.morph_w0) + effect.morph_w0,
								 (effect.morph_h1 - effect.morph_h0) + effect.morph_h0);
					cur_x = (short)(int)effect.morph_x1;
					cur_y = (short)(int)effect.morph_y1;
					if ((ce = canvas_elt) != NULL) place_on_screen(ce, (float)cur_x, (float)cur_y);
					if (effect.type == IEFX_MORPH_FLASH && canvas_elt) canvas_elt->hide(true);
				}
				effect.Clear();
			}
		}
	}

	// --- SCALE
	if (effect.type != IEFX_SCALE || effect.owner != this) return;
	if (effect.show) Show();
	else if (canvas_elt) canvas_elt->hide(true);

	if (!(effect.time_left2 > 0)) return;
	float t = effect.time_left2 - dt;
	effect.time_left2 = t;
	// Two modes, chosen by scale_x only: an amount in (0, 1) is a factor of the base size,
	// anything else is added to 1.
	bool factor = effect.scale_x < 1.0f && effect.scale_x > 0;
	if (t > 0)
	{
		float a = t / effect.duration2;
		if (effect.forward2) a = 1.0f - a;
		if (factor)
			scale_canvas(canvas_elt, (float)base_w * a * effect.scale_x, (float)base_h * effect.scale_y * a);
		else
			scale_canvas(canvas_elt, (a * effect.scale_x + 1.0f) * (float)base_w,
						 (a * effect.scale_y + 1.0f) * (float)base_h);
		return;
	}
	effect.time_left2 = 0;
	current = 0;
	if (effect.forward2)
	{
		if (factor)
			scale_canvas(canvas_elt, (float)base_w * effect.scale_x, (float)base_h * effect.scale_y);
		else
			scale_canvas(canvas_elt, (float)base_w * (effect.scale_x + 1.0f),
						 (effect.scale_y + 1.0f) * (float)base_h);
	}
	effect.Clear();
}

// --------------------------------------------------------------------------- 0x482390

void InterfaceElt::Scale(float w, float h)
{
	scale_canvas(canvas_elt, w, h);
}

// --------------------------------------------------------------------------- 0x4823f0

void InterfaceElt::SetPos(float px, float py)
{
	float sx = screen_sx(), sy = screen_sy();
	OMediaCanvasElement *ce = canvas_elt;
	if (!ce) { ErrorLog("ERROR: Setpos CanvasElt = NULL"); return; }
	cur_x = (short)(int)px;
	cur_y = (short)(int)py;
	SetCanvas(ce, canvas_id, false);					// sic: unscaled
	ce->set_canvas_flags(omcanef_FreeWorldSize);
	ce->set_auto_align(omaac_Center, omaac_Center);
	// sic: x/y (and cur_x/cur_y) get the layer coordinates, truncated
	x = (short)(int)(sx * px - (float)((short)g_screen_w / 2));
	y = (short)(int)(((float)((short)g_screen_h / 2) - ce->get_height()) - sy * py);
	cur_y = y;
	cur_x = x;
	ce->place((float)x, (float)y, 0);
}

// --------------------------------------------------------------------------- 0x482520

void InterfaceElt::Show(void)
{
	if (canvas_elt)
	{
		canvas_elt->force_show();				// hide count 1, then hide(false)
		return;
	}
	if (canvas_id < 0) return;
	canvas_elt = new OMediaCanvasElementWrapper;
	if (!canvas_elt) return;
	canvas_elt->link_layer(g_layer_hud2d);
	SetCanvas(canvas_elt, canvas_id, true);
	place_on_screen(canvas_elt, (float)x, (float)y);
	canvas_elt->force_show();
	canvas_elt->set_blend(src_blend, dest_blend);
	base_w = (int)canvas_elt->get_width();
	base_h = (int)canvas_elt->get_height();
}

// --------------------------------------------------------------------------- 0x4826f0

void InterfaceElt::Hide(void)
{
	if (canvas_elt) canvas_elt->hide(true);
}

// --------------------------------------------------------------------------- 0x482700

void InterfaceElt::SetCanvas(OMediaCanvasElement *ce, int id, bool scaled)
{
	float sx = screen_sx(), sy = screen_sy();
	if (id != -1)
	{
		OMediaCanvas *c = LoadCanvasID(db, id);
		if (c) c->db_unlock();					// inline: lock count (+0x14) down if > 0
		canvas = c;
		canvas_id = (short)id;
		if (c)
		{
			if (scaled) ce->set_size((float)c->get_width() * sx, (float)c->get_height() * sy);
			else ce->set_size((float)c->get_width(), (float)c->get_height());
		}
	}
	if (canvas) ce->set_canvas(canvas);			// vtable +0xac
}

// --------------------------------------------------------------------------- InterfaceElement

void InterfaceElement::DestroyAll(void)			// 0x480580
{
	while (g_interface_elts.size() != 0)
	{
		InterfaceElt *e = g_interface_elts.front();
		if (e && e->canvas_elt)
		{
			delete e->canvas_elt;
			e->canvas_elt = NULL;
		}
		g_interface_elts.remove(e);
		delete e;
	}
}

void InterfaceElement::ResetAll(void)			// 0x480610
{
	for (std::list<InterfaceElt *>::iterator i = g_interface_elts.begin(); i != g_interface_elts.end(); ++i)
		if (*i) (*i)->Reset();
}

void InterfaceElement::Create(const char *name, OMediaDataBase *db, short canvas_id, short x, short y,	// 0x4807a0
							  omt_BlendFunc src_blend, omt_BlendFunc dest_blend, int f_34)
{
	InterfaceElt *e = new InterfaceElt;
	g_interface_elts.push_back(e);
	strcpy(e->name, name);
	e->canvas_id = canvas_id;
	e->db = db;
	e->x = x;
	e->src_blend = src_blend;
	e->home_x = x;
	e->y = y;
	e->dest_blend = dest_blend;
	e->home_y = y;
	e->f_34 = f_34;
}

InterfaceElt *InterfaceElement::Find(const char *name)		// 0x480970
{
	for (std::list<InterfaceElt *>::iterator i = g_interface_elts.begin(); i != g_interface_elts.end(); ++i)
	{
		InterfaceElt *e = *i;
		if (e && strlen(e->name) == strlen(name) && strcasecmp(e->name, name) == 0)	// _stricmp
			return e;
	}
	return NULL;
}

void InterfaceElement::UpdateAll(float dt)		// 0x4809e0
{
	for (std::list<InterfaceElt *>::iterator i = g_interface_elts.begin(); i != g_interface_elts.end(); ++i)
		if (*i) (*i)->Update(dt);
}

// --------------------------------------------------------------------------- 0x482b40 / 0x482b50

void neg::SetFlag545e54(bool on)
{
	g_flag_545e54 = on;
}

void neg::AddScore(int points)
{
	if (points > 0)
	{
		if (g_53b5b0 == 0) points = (int)(points * 0.5);			// 0x4cb210
		else if (g_53b5b0 == 2) points = (int)(points * 1.5);		// 0x4cb1d0
		g_score_popup_time = 1.2f;
		g_score_popup += points;
	}
	g_score += points;
	if (g_score < 0) g_score = 0;
}
