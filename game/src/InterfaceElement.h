/*  The 2D interface elements: named sprites of the menus and the HUD, each owning one canvas
    element on the HUD 2D layer, animated by a queue of effects (fade, move, scale, morph...).
    Reconstructed from Neutron2.exe, free functions 0x480540..0x482700 (+ 0x482b40, 0x482b50).

    Three types:
    * InterfaceElement - the class the RTTI names (vtable 0x506904: only a virtual destructor).
      It has no data; its one instance g_interface (0x545964) is the manager whose methods
      create / find / update / reset / destroy the elements of the list g_interface_elts.
    * InterfaceElt     - one element (0x104 bytes, not polymorphic: its canvas element pointer is
      at +0). Created by InterfaceElement::Create, or a static/member object (the menu and HUD
      globals below, and members of several C3D* enemy classes - their constructors call 0x480a10).
    * InterfaceEffect  - one queued animation step (100 bytes). Queue functions write a record
      into g_effect_queue for an element; InterfaceElt::Update moves the first record of that
      element into InterfaceElt::effect and runs it until it ends, then takes the next one.

    Coordinates: element positions (x, y, home_x, ...) are in the game's 640x480 screen space,
    y down. The canvas element is placed on the 2D layer (origin at the screen centre, y up)
    with centred auto-align at
        ( x * w/640 - w/2 ,  (h/2 - canvas height) - y * h/480 )        w, h = g_screen_w/h
    Sizes passed to Scale / the morph effects are in 640x480 units too.

    Times (effect durations, Update's dt) are in whatever unit the caller's frame time is in;
    the menu passes seconds (durations like 0.1, 0.5).                                           */
#pragma once
#include <list>
#include "OMediaRenderConstants.h"

class OMediaCanvas;
class OMediaCanvasElement;
class OMediaCanvasElementWrapper;
class OMediaDataBase;
struct InterfaceElt;

// Effect kinds (InterfaceEffect::type). 3, 4 and 9 are handled by Update but nothing in the game
// queues them.
enum
{
	IEFX_NONE			= 0,	// free record / no effect running
	IEFX_SCALE			= 1,	// grow the canvas from its base size			QueueScale
	IEFX_MOVE			= 2,	// slide along a direction						QueueMove
	IEFX_COMMIT_POS		= 3,	// position := drawn position (cur_x, cur_y)	(never queued)
	IEFX_GO_HOME		= 4,	// position := home position					(never queued)
	IEFX_WAIT			= 5,	// do nothing for a while						QueueWait
	IEFX_FADE			= 6,	// alpha ramp									QueueFade
	IEFX_MORPH			= 7,	// size and position from one rect to another	QueueMorph
	IEFX_HIDE			= 8,	// hide the canvas element (and stop the queue)	QueueHide
	IEFX_SHOW			= 9,	// Show() (and stop the queue)					(never queued)
	IEFX_MORPH_FLASH	= 10	// morph with an alpha 0 -> 0.75 -> 0 flash, hidden at the end	QueueMorphFlash
};

// --------------------------------------------------------------------------- InterfaceEffect
// One animation step (100 bytes). Two timers: duration/time_left for FADE, MOVE and WAIT,
// duration2/time_left2 for SCALE and the MORPHs. Each timer counts down from its duration; the
// effect's progress is a = time_left / duration, or 1 - a when `forward` is set.
struct InterfaceEffect
{
	InterfaceElt	*owner;			// 0x00  element it belongs to (kept by Clear)
	int				 type;			// 0x04  IEFX_*
	float			 duration;		// 0x08  FADE / MOVE / WAIT
	float			 time_left;		// 0x0c
	float			 move_dx;		// 0x10  MOVE: direction (default 0, -1) ...
	float			 move_dy;		// 0x14
	float			 move_dist;		// 0x18  ... times distance (default 64)
	bool			 forward;		// 0x1c  FADE / MOVE: true = play 0 -> 1 and stay visible,
									//       false = play 1 -> 0 and hide at the end
	bool			 active;		// 0x1d  the effect runs; a taken effect with this false
									//       stalls the element's queue (Update returns at once)
	bool			 f_1e;			// 0x1e  set when a forward MOVE ends, cleared again right
									//       after by Clear (never read)
	float			 duration2;		// 0x20  SCALE / MORPH
	float			 time_left2;	// 0x24
	bool			 forward2;		// 0x28  like `forward`, for SCALE / MORPH
	float			 scale_x;		// 0x2c  SCALE amounts (see QueueScale)
	float			 scale_y;		// 0x30
	int				 f_34;			// 0x34  never set
	float			 alpha_from;	// 0x38  FADE
	float			 alpha_range;	// 0x3c  FADE: to - from
	float			 morph_w0;		// 0x40  MORPH: size from (w0, h0) to (w1, h1) ...
	float			 morph_h0;		// 0x44
	float			 morph_w1;		// 0x48
	float			 morph_h1;		// 0x4c
	float			 morph_x0;		// 0x50  ... and position from (x0, y0) to (x1, y1)
	float			 morph_y0;		// 0x54
	float			 morph_x1;		// 0x58
	float			 morph_y1;		// 0x5c
	bool			 show;			// 0x60  while running, Show() the element every frame;
									//       false: hide its canvas element every frame instead

	InterfaceEffect() : owner(0) { Clear(); }	// static initialiser 0x4011a0 (the queue)
	void Clear(void);				// inline everywhere: back to defaults, `owner` kept
};

// --------------------------------------------------------------------------- InterfaceElt
struct InterfaceElt
{
	OMediaCanvasElementWrapper	*canvas_elt;	// 0x00  created by Show(), on g_layer_hud2d
	OMediaCanvas				*canvas;		// 0x04  canvas of canvas_id (from db)
	short				 canvas_id;		// 0x08  -1 = none
	short				 x;				// 0x0a  position (640x480 space; see SetPos for a quirk)
	short				 y;				// 0x0c
	short				 home_x;		// 0x0e  position given to Create; Reset goes back there
	short				 home_y;		// 0x10
	short				 cur_x;			// 0x12  position drawn by the last MOVE / MORPH frame
	short				 cur_y;			// 0x14
	omt_BlendFunc		 src_blend;		// 0x18  default omblendfc_Src_Alpha
	omt_BlendFunc		 dest_blend;	// 0x1c  default omblendfc_Inv_Src_Alpha
	OMediaDataBase		*db;			// 0x20  database canvas_id is loaded from
	int					 base_w;		// 0x24  canvas element size when Show() created it
	int					 base_h;		// 0x28  (screen pixels) - SCALE scales from it
	int					 f_2c;			// 0x2c  only ever set to 0
	int					 current;		// 0x30  type of the effect taken from the queue; 0 =
										//       take the next one. HIDE and SHOW leave it set,
										//       so the queue stops until Reset().
	int					 f_34;			// 0x34  last argument of Create (1 for every menu item)
	char				 name[100];		// 0x38  for InterfaceElement::Find
	int					 f_9c;			// 0x9c  only ever set to 0
	InterfaceEffect		 effect;		// 0xa0  the running effect

	InterfaceElt();									// 0x480a10  (also resets g_effect_queue_next)
	~InterfaceElt() {}								// 0x463890  (empty)

	// Stop: drop this element's queued and running effects, back to the home position, the
	// canvas element created (if canvas_id >= 0) and hidden, the shared queue restarted.
	void Reset(void);								// 0x480d20

	// One frame: take the next queued effect if none is running, run the current one.
	void Update(float dt);							// 0x481010

	// Make the canvas element visible, creating it on first use (needs canvas_id >= 0):
	// linked to g_layer_hud2d, canvas loaded and scaled to the screen, placed at (x, y),
	// blend set, base_w/base_h taken. Visible again whatever its hide count was.
	void Show(void);								// 0x482520
	// canvas_elt->hide(true) - hides nest: each Hide needs a Show (Show forces visible).
	void Hide(void);								// 0x4826f0
	// Set the canvas element's size to (w, h) in 640x480 units. Logs an error if not created.
	void Scale(float w, float h);					// 0x482390
	// Move the (already created) canvas element to (px, py), 640x480 space. Quirks of the
	// original kept: the canvas is re-set *unscaled* (native pixel size), and x/y and cur_x/cur_y
	// end up holding the layer (centre-origin) coordinates, not 640x480 ones.
	void SetPos(float px, float py);				// 0x4823f0
	// Load canvas `id` from db into `canvas`/canvas_id (unless id == -1), size `ce` to it -
	// scaled to the screen when `scaled` - and give `ce` that canvas.
	void SetCanvas(OMediaCanvasElement *ce, int id, bool scaled);	// 0x482700

	// --- effect queue. Each call appends one record for this element to g_effect_queue (dropped
	// silently if the next slot is still in use). `unused` is ignored (the game passes 0).
	// `active` and `show`: see InterfaceEffect. `forward`: true = transition in (0 -> 1, stays
	// visible), false = transition out (1 -> 0, the element is hidden at the end for FADE/MOVE).

	// SCALE: canvas size = base size * (1 + a * amount) - or, when 0 < amount_x < 1, base size *
	// a * amount (both axes use the test on amount_x). QueueScale(0, t, 1,1,1, 0,0) restores
	// the base size.
	void QueueScale(int unused, float duration, bool forward, bool active, bool show,
					float amount_x, float amount_y);							// 0x480b20
	// MOVE: draw at (x, y) + a * distance * (dir_x, dir_y); x, y themselves are not changed.
	void QueueMove(int unused, float duration, bool forward, bool active, bool show,
				   float dir_x, float dir_y, float distance);					// 0x480ec0
	// WAIT: hold the queue for `duration`.
	void QueueWait(int unused, float duration);									// 0x480e70
	// FADE: alpha = from + a * (to - from), colour white; at the end of a backward fade the
	// alpha is 0 and the element hidden (also hidden after a forward fade when to == from).
	void QueueFade(int unused, float duration, bool forward, bool show,
				   float alpha_from, float alpha_to);							// 0x480f50
	// MORPH: size (w0,h0) -> (w1,h1), position (x0,y0) -> (x1,y1), 640x480 units. The end
	// values are applied exactly only for a forward morph.
	void QueueMorph(int unused, float duration, bool forward, bool active, bool show,
					float w0, float h0, float w1, float h1,
					float x0, float y0, float x1, float y1);					// 0x480ba0
	// MORPH_FLASH: as QueueMorph, plus alpha = 1.5 * a up to a = 0.5, then 1.5 * (1 - a);
	// a forward one hides the element at the end.
	void QueueMorphFlash(int unused, float duration, bool forward, bool active, bool show,
						 float w0, float h0, float w1, float h1,
						 float x0, float y0, float x1, float y1);				// 0x480c60
	// HIDE: hide the canvas element. The element's queue stops after it (until Reset).
	void QueueHide(void);														// 0x480fd0

	private:
	InterfaceEffect *NextQueueSlot(int type);	// common head of the Queue* functions
};

// --------------------------------------------------------------------------- InterfaceElement
// The manager (RTTI "InterfaceElement", 4 bytes: just the vtable). One instance, g_interface.
class InterfaceElement
{
	public:
	InterfaceElement() {}								// 0x480540
	virtual ~InterfaceElement() {}						// 0x480570 (deleting: 0x480550)

	// New element `name` (copied, < 100 chars) showing canvas `canvas_id` of `db` at (x, y) =
	// its home position, appended to g_interface_elts. Not shown yet - call Show() / queue
	// effects. Returns nothing: the menu fetches it with Find(name) right after.
	// f_34 is stored as is (the menu passes 1).
	void Create(const char *name, OMediaDataBase *db, short canvas_id, short x, short y,
				omt_BlendFunc src_blend, omt_BlendFunc dest_blend, int f_34);	// 0x4807a0
	// The element called `name` (case-insensitive), or NULL.
	InterfaceElt *Find(const char *name);				// 0x480970
	// Update(dt) on every element.
	void UpdateAll(float dt);							// 0x4809e0
	// Reset() on every element (the original has Reset inlined).
	void ResetAll(void);								// 0x480610
	// Delete every element and its canvas element; empties g_interface_elts.
	void DestroyAll(void);								// 0x480580
};

namespace neg {
enum { EFFECT_QUEUE_SIZE = 100 };

extern InterfaceElement				g_interface;						// 0x545964
extern std::list<InterfaceElt *>	g_interface_elts;					// 0x54a414  (count 0x54a418)
extern InterfaceEffect				g_effect_queue[EFFECT_QUEUE_SIZE];	// 0x541da0
extern int							g_effect_queue_next;				// 0x545da4  slot the next Queue* call writes

// Static element objects (constructed by the static initialisers 0x401000..0x401230). Used by
// the menu constructor 0x401380 (it sets their fields directly rather than through Create) and
// by the functions noted.
extern InterfaceElt		g_elt_5444d8;			// 0x5444d8
extern InterfaceElt		g_elt_5445e8;			// 0x5445e8
extern InterfaceElt		g_elt_544908[14];		// 0x544908  set up like Create by 0x401380: each its own name and canvas, x 0, one shared y
extern InterfaceElt		g_elt_545740;			// 0x545740
extern InterfaceElt		g_elt_545858;			// 0x545858
extern InterfaceElt		g_elt_545a70;			// 0x545a70  shown / hidden by 0x40b280 / 0x40b2a0
extern InterfaceElt		g_elt_545b78;			// 0x545b78  (no other use found)
extern InterfaceElt		g_elt_545c80;			// 0x545c80

// Score (shown with "%6.0d"; CJimmyGame 0x4854e0 ranks it in steps of 3000..35000).
extern int		g_score;				// 0x54a458
extern int		g_score_popup;			// 0x54a45c  points of the "+N" pop-up (0x4859b0 draws it)
extern float	g_score_popup_time;		// 0x54a460  pop-up timer, 1.2 when points are added

// Flag set by CEditor's level loader (0 for every level, 1 for level1_c.gam), C2DInGameMenu
// and C3DJimmy; read by C3DJetPack, C3DPlayer and 0x447xxx. Meaning not established.
extern bool		g_flag_545e54;			// 0x545e54

void SetFlag545e54(bool on);			// 0x482b40
// Add (or, negative, remove) points. Positive amounts are scaled by the skill setting
// 0x53b5b0 (0: x0.5, 2: x1.5, else x1) and start the pop-up; the score never goes below 0.
void AddScore(int points);				// 0x482b50
}
