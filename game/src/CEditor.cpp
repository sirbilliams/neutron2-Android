/*  CEditor - level loader/saver and developer editor.  Reconstructed from Neutron2.exe.
    See CEditor.h for the overview. Each function names its original address.

    The original's debug-log calls (CGameObject__v13, compiled empty in the shipped build) are kept
    as Trace(): silent unless NEG_EDITOR_TRACE is set in the environment.                      */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CEditor.h"
#include "CDefaultGame.h"
#include "CViewPort.h"
#include "InterfaceElement.h"
#include "CGameObject.h"
#include "CLocalGameObject.h"
#include "C2DInGameMenu.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "gen/globals.h"

#include "OMediaStringField.h"
#include "OMediaCanvasFont.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaDataBase.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaWorld.h"
#include "OMediaPickRequest.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMediaTrigo.h"
#pragma GCC diagnostic pop

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <string>

using namespace neg;

std::string omf_SDLResolvePath(const std::string &in);		// engine/sdl: DOS path -> real file

namespace neg {
OMediaCanvas			*g_editor_font_canvas = NULL;	// 0x56af00
OMediaCanvasFont		*g_editor_font = NULL;			// 0x56af04
unsigned char			 g_new_game = 0;				// 0x56af08
OMediaCanvasElement		*g_splash = NULL;				// 0x56af10
OMediaDataBase			*g_editor_db = NULL;			// 0x56af14
OMediaFileStream		*g_editor_stream = NULL;		// 0x56af18
OMediaCanvasElement		*g_editor_menu_panel = NULL;	// 0x56af1c
OMediaCanvasElement		*g_editor_list_panel = NULL;	// 0x56af20
OMediaCanvasElement		*g_editor_props_panel = NULL;	// 0x56af24
}

// Windows virtual keys read through g_keys[]
enum { VK_RETURN_ = 0x0d, VK_SHIFT_ = 0x10, VK_CONTROL_ = 0x11, VK_LEFT_ = 0x25, VK_UP_ = 0x26,
	   VK_RIGHT_ = 0x27, VK_DOWN_ = 0x28, VK_DELETE_ = 0x2e, VK_OEM_3_ = 0xc0 };

// Object 4ccs the editor treats specially
enum { ID_TERRAIN = '3TER', ID_SKY = '3SKY', ID_JIMMY = '3JIM' };

// The original's empty debug log.
static void Trace(const char *fmt, ...)
{
	static int on = -1;
	if (on < 0) on = getenv("NEG_EDITOR_TRACE") != NULL;
	if (!on) return;
	char buf[512];
	va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
	SDL_Log("[editor] %s", buf);
}

// --------------------------------------------------------------------------- tables (exe data)

struct NamedID { int id; char name[32]; };		// 0x24 bytes, id -1 ends the table
typedef char MenuName[32];

static MenuName *Menu(void)			{ return (MenuName *)g_editor_menu; }		// 0x53f570
static MenuName *Toggles(void)		{ return (MenuName *)g_editor_toggles; }	// 0x53f810
static NamedID *Catalogue(void)		{ return (NamedID *)g_object_catalogue; }	// 0x53c618
static NamedID *Levels(void)		{ return (NamedID *)g_level_table; }		// 0x53d818
enum { MENU_MAX = 100,								// loops stop at 0x5401f0 / 0x540490
	   LEVELS_MAX = (0x53e62c - 0x53d818) / 0x24 };	// "Load Game" stops at 0x53e62c

// --------------------------------------------------------------------------- small helpers

static void IdString(char out[5], int id)			// the 4cc as text, as the logs print it
{
	out[0] = (char)(id >> 24); out[1] = (char)(id >> 16); out[2] = (char)(id >> 8); out[3] = (char)id; out[4] = 0;
}

// 0x4a38a0: was the key pressed? (clears it)
static bool KeyPressed(int vk)
{
	if (g_keys[vk]) { g_keys[vk] = 0; return true; }
	return false;
}

// 0x4b1ee0: a camera-relative direction (viewport angles), w = 0
static float *CameraDir(float out[4], float x, float y, float z)
{
	const float sx = omd_Sin(g_viewport->get_anglex()), cx = omd_Cos(g_viewport->get_anglex());
	const float sy = omd_Sin(g_viewport->get_angley()), cy = omd_Cos(g_viewport->get_angley());
	const float sz = omd_Sin(g_viewport->get_anglez()), cz = omd_Cos(g_viewport->get_anglez());
	const float t = cz * y - sz * x;
	const float u = sz * y + cz * x;
	const float v = cx * z - t * sx;
	out[3] = 0.0f;
	out[0] = u * cy - v * sy;
	out[1] = sx * z + t * cx;
	out[2] = u * sy + v * cy;
	return out;
}

// 0x4b1de0: a point in front of the camera (viewport position + camera-relative offset), w = 1
static float *CameraPoint(float out[4], float x, float y, float z)
{
	float d[4];
	CameraDir(d, x, y, z);
	out[3] = 1.0f;
	out[0] = g_viewport->getx() + d[0];
	out[2] = g_viewport->getz() + d[2];
	out[1] = d[1] + g_viewport->gety();
	return out;
}

// 0x490890: given an object's 4cc and either its game object or its element, find the other.
// The original switches on the 4cc to know the subobject offsets; RTTI does the same here.
static void ObjectElement(int id, CGameObject **obj, OMediaElement **elem)
{
	(void)id;
	if (*elem == NULL && *obj == NULL) return;
	if (*elem != NULL && *obj != NULL) return;
	if (*elem) *obj = dynamic_cast<CGameObject *>(*elem);
	else *elem = dynamic_cast<OMediaElement *>(*obj);
}

// A game type is an OMediaElement first and a CLocalGameObject at +0x8c; the original subtracts
// 0x8c from the factory's result. CGameType is not reconstructed yet: its element is its start.
static CGameType *GameTypeOf(CGameObject *o)
{
	return (CGameType *)dynamic_cast<OMediaElement *>(o);
}
static OMediaElement *GameTypeElement(void)	{ return (OMediaElement *)g_gametype; }

static void SetDiffuse(OMediaCanvasElement *e, float r, float g, float b, float a)
{
	e->set_diffuse(OMediaFARGBColor(a, r, g, b));		// 2002 +0x90..0x9c = r, g, b, a
}

// 0x4ae550: delete every object flagged f_006 ("Delme")
static void DeleteFlaggedObjects(void)
{
	for (;;)
	{
		std::list<CGameObject *>::iterator i = g_objects.begin();
		while (i != g_objects.end() && !(*i)->f_006) i++;
		if (i == g_objects.end()) return;
		if (*i) delete *i;
	}
}


static void MenuShowLoading(int a)	{ NEG_TODO(0x40c0f0, "C2DInGameMenu loading screen"); (void)a; }

// 0x485360: the game state (0 menu/new game, 1 loading, 3, 5 playing...), told to every object
static void SetGameState(int state)
{
	if (state == g_game_state) return;
	g_prev_game_state = g_game_state;
	g_game_state = state;
	if (state == 1)
	{
		if (g_545d98)
		{
			if (!g_level_requested) { g_54a468 = 1; MenuShowLoading(0); }
			else g_level_requested = 0;
		}
	}
	else
	{
		if (g_54a468 && state == 3) MenuShowLoading(1);
		g_54a468 = 0;
	}
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) static_cast<CLocalGameObject *>(*i)->v270(g_prev_game_state, state);
}

// 0x406a40: start the in-game menu's fade (mode, seconds)
static void StartFade(int mode, float t)
{
	if (g_5324fc == 0) { g_5324ec = 1; return; }
	if (mode == 6) mode = 4;
	if (t < 0.1f) t = 0.1f;				// 0x4caa68
	g_5324e8 = t;
	g_545dcc = t;
	g_545dd0 = mode;
	g_5324ec = 0;
	g_545e5c = 1;
}

// 0x482b40
static void SetLevel1cLoaded(unsigned char v)	{ neg::SetFlag545e54(v != 0); }	// 0x482b40 (InterfaceElement.cpp)


static void DeleteFile(const char *dos_path)	// KERNEL32 DeleteFileA
{
	remove(omf_SDLResolvePath(dos_path).c_str());
}

// --- not reconstructed yet
static void PhysicsReset(void)					{ NEG_TODO(0x4b40d0, "CPhysics::Reset"); }
static void CloseAllDatabases(void)				{ NEG_TODO(0x4a4870, "CloseAllDatabases (object database cache 0x56aef4)"); }
static const char *DatabaseName(int i)			{ NEG_TODO(0x4a4a40, "DatabaseName (object database cache 0x56aef4)"); (void)i; return NULL; }
static bool ReadTaskFile(const char *name)		// 0x497b10: reads save\<name>'s header; true when it is usable
{
	NEG_TODO(0x497b10, "ReadTaskFile (savegame header)");
	FILE *f = fopen(omf_SDLResolvePath(std::string("save\\") + name).c_str(), "rb");	// port stand-in: the file exists
	if (f) fclose(f);
	return f != NULL;
}
static void LoadTask(const char *name, int a)	{ NEG_TODO(0x498af0, "LoadTask (savegame)"); (void)name; (void)a; }
static void RestoreTask(const char *name)		{ NEG_TODO(0x498440, "RestoreTask (savegame state after a level load)"); (void)name; }
static void PlaySoundNamed(int channel, const char *name, int a)	{ NEG_TODO(0x4b8380, "PlaySound"); (void)channel; (void)name; (void)a; }
static void MenuDrawCountdown(int a, int b, int c)	{ NEG_TODO(0x409d50, "C2DInGameMenu 0x409d50 (level-change countdown)"); (void)a; (void)b; (void)c; }
static void CutsceneCameraPlay(CGameObject *o)	{ NEG_TODO(0x4a64e2, "C3DCutSceneCamera element slot 59"); (void)o; }
static void MultiCutscenePlay(CGameObject *o)	{ NEG_TODO(0x4a6520, "C3DMultiCutScene element slot 61"); (void)o; }

// Windows cursor (0x4a38c0 / 0x4a3930 / 0x4a3990); the editor flies the camera with the mouse.
static void ShowWinCursor(bool on)				{ NEG_TODO(0x4a38c0, "ShowCursor"); (void)on; }
static void CaptureMouse(bool capture)			// 0x4a3930
{
	if (capture && !g_56aed0)
	{
		NEG_TODO(0x4a3930, "hide the cursor, remember and centre it");
		g_mouse_captured = 1;
		return;
	}
	// (restore the cursor position, OMediaMouseCursor::force_show)
	g_mouse_captured = 0;
}
static void GetMouseDelta(short *dx, short *dy)	// 0x4a3990: cursor offset from the centre, then re-centre
{
	if (!g_56aed0) { NEG_TODO(0x4a3990, "GetMouseDelta"); (void)dx; (void)dy; }
}

// CLocalGameObject slot 283 takes a float (the frame time); the generator typed it as unsigned int,
// so the bits are passed through unchanged.
static unsigned int FloatBits(float f)	{ unsigned int u; memcpy(&u, &f, 4); return u; }

// --------------------------------------------------------------------------- neg::EditorIsEditing

bool neg::EditorIsEditing(void)			// the callers read g_editor +0x8c
{
	return g_editor && g_editor->m_editing;
}

// --------------------------------------------------------------------------- string fields

static OMediaStringField *NewField(float x, float y)
{
	OMediaStringField *f = new OMediaStringField;
	f->link(g_world);									// vtable +0x2c
	f->set_size(0x20, 0xe);								// +0x178 / +0x17a, purge
	f->set_font(g_editor_font);							// +0x1e0, purge
	f->place(x, y);										// +0x44 / +0x48
	return f;
}

// --------------------------------------------------------------------------- 0x4a4aa0

CEditor::CEditor()
{
	// (members built by the compiler: m_cam_save[3] = f_a0[3] = 1, the selection cleared)
	m_cam_save[3] = 1.0f;
	f_a0[3] = 1.0f;
	m_sel_element = NULL; m_sel_object = NULL; m_sel_material = NULL; m_sel_polygon = NULL; m_pick = NULL;
	m_sel_shape = NULL;

	Trace("CEditor()");
	SetGameState(0);
	g_editor = this;
	f_7f9 = 0;

	// InitObject(), inline
	Trace("InitObject()");
	link(g_world);
	m_cam_save[0] = m_cam_save[1] = m_cam_save[2] = 0;
	f_a0[0] = f_a0[1] = f_a0[2] = 0;

	m_editing = 0;
	set_id('EDIT');
	memset(f_c8, 0, sizeof(f_c8));

	// The editor font: png\fontsmall.png, one character per 10-pixel row, proportional.
	{
		OMediaFilePath path(".\\png\\fontsmall.png", 'null', 'null');
		OMediaFileStream fs;
		fs.setpath(&path);
		fs.open(omcfp_Read, false, false);
		OMediaCanvas *canvas = new OMediaCanvas;
		fs >> (OMediaClassStreamer *)canvas;
		g_editor_font_canvas = canvas;
		Trace("Viewport height=%d  fullHeight=%d", g_viewport->getbheight(), g_viewport->getbheight());

		g_editor_font = new OMediaCanvasFont;
		g_editor_font->set_font_canvas(g_editor_font_canvas, g_editor_font_canvas->get_width(), 10, 10);
		g_editor_font->set_proportional(true);
		g_editor_font->create_proportional_tab();
		g_editor_font->set_space_size(3);
		g_editor_font->set_char_space(1);

		m_paste_latch = 0;
		m_copied = NULL;

		const int half_h = g_viewport->getbheight() / 2;

		// list fields: "Empty now", one column
		float x = (float)((short)g_screen_w / 2 - 260);
		int y = half_h - 16;
		for (int i = 0; i < 100; i++)
		{
			char text[76];
			strcpy(text, "Empty now");
			OMediaStringField *f = NewField(x, (float)y);
			m_list_fields[i] = f;
			f->set_string(text);
			f->hide(true);
			f->set_default_filter(omckf_AlphaNumeric);
			y -= 12;
		}

		// menu fields: one per g_editor_menu entry
		int mx = (short)g_screen_w / 2 - 100;
		y = half_h - 16;
		for (short i = 0; i < MENU_MAX; i++)
		{
			if (strcasecmp(Menu()[i], "") == 0) break;
			char text[32];
			strcpy(text, Menu()[i]);
			OMediaStringField *f = NewField((float)mx, (float)y);
			m_menu_fields[i] = f;
			f->set_string(text);
			f->hide(true);
			f->set_default_filter(omckf_AlphaNumeric);
			y -= 12;
		}

		// property name / value fields: columns of 12-pixel rows, wrapping 200 pixels right
		int px = -((short)g_screen_w / 2);
		y = half_h - 48;
		for (int i = 0; i < 100; i++)
		{
			if (y < -(g_viewport->getbheight() / 2)) { y = g_viewport->getbheight() / 2 - 48; px += 200; }
			OMediaStringField *f = NewField((float)px, (float)y);
			m_prop_names[i] = f;
			f->hide(true);
			y -= 12;
		}
		px = 100 - (short)g_screen_w / 2;
		y = half_h - 48;
		for (int i = 0; i < 100; i++)
		{
			if (y < -(g_viewport->getbheight() / 2)) { y = g_viewport->getbheight() / 2 - 48; px += 200; }
			OMediaStringField *f = NewField((float)px, (float)y);
			m_prop_values[i] = f;
			f->set_default_filter(omckf_Integer);
			f->hide(true);
			y -= 12;
		}
	}

	m_edit_name = NULL;
	m_edit_value = NULL;
	CEditor::SetMenuSelection("Select Object");			// direct call
	m_mode = 0;
	m_move_speed = 100.0f;
	m_load_pending = 0;
	strcpy(m_level_name, "");
	strcpy(m_game_name, "");
	f_7a0 = -1;
	m_load_countdown = -1;
	m_load_delay = 0;
	f_7f8 = 0;
	m_list_offset = 0;
	m_key_latch = 0;
	m_start_timer = 3.0f;
	m_start_state = 1;
	f_810 = 0;
	f_811 = 0;
	m_panel_open = 0;
	m_list_filter = 0;
	g_new_game = 0;
	m_label_mode = 1;
	Trace("CEditor() done");
}

// --------------------------------------------------------------------------- 0x4a5460

CEditor::~CEditor()
{
	Trace("~CEditor()");
	Trace("Calling ~Editor");
	for (int i = 0; i < 100; i++)
		if (m_list_fields[i]) { delete m_list_fields[i]; m_list_fields[i] = NULL; }
	for (int i = 0; i < MENU_MAX; i++)
	{
		if (strcasecmp(Menu()[i], "") == 0) break;
		if (m_menu_fields[i]) { delete m_menu_fields[i]; m_menu_fields[i] = NULL; }
	}
	for (int i = 0; i < 100; i++)
		if (m_prop_names[i]) { delete m_prop_names[i]; m_prop_names[i] = NULL; }
	for (int i = 0; i < 100; i++)
		if (m_prop_values[i]) { delete m_prop_values[i]; m_prop_values[i] = NULL; }
	if (g_splash)
	{
		Trace("Deleting splash");
		if (g_splash->get_canvas()) delete g_splash->get_canvas();
		if (g_splash) delete g_splash;
		g_splash = NULL;
	}
	if (g_editor_db) UnInitDatabase(g_editor_db);
	if (g_editor_stream) UnInitFileStream(g_editor_stream);
	if (g_editor_font) { delete g_editor_font; g_editor_font = NULL; }
}

// --------------------------------------------------------------------------- 0x4a5640

void CEditor::update_logic(float millisecs_elapsed)
{
	OMediaElement::update_logic(millisecs_elapsed);
	const float dt = millisecs_elapsed * 0.001f;
	if (!m_editing)
		UpdateGame(dt);
	else
	{
		UpdateEditor(dt);
		if (m_sel_object) m_sel_object->v247();
	}
}

// --------------------------------------------------------------------------- slots 43..48

void CEditor::InitObject(void)							// 0x4a5690
{
	Trace("InitObject()");
	link(g_world);
	m_cam_save[0] = m_cam_save[1] = m_cam_save[2] = 0;
	f_a0[0] = f_a0[1] = f_a0[2] = 0;
}

void CEditor::UnInitObject(void)	{ Trace("UnInitObject()"); }	// 0x4a56e0
void CEditor::InitGrid(void)		{ Trace("InitGrid()"); }		// 0x4a5c30
void CEditor::UnInitGrid(void)		{ Trace("UnInitGrid()"); }		// 0x4a5c50

// 0x4a5700
void CEditor::InitEditor(void)
{
	Trace("InitEditor()");
	Trace("Entering InitEditor");

	g_editor_menu_panel = new OMediaCanvasElement;
	if (g_editor_menu_panel)
	{
		g_editor_menu_panel->link_layer(g_layer_hud2d);
		PlaceSprite(g_editor_menu_panel, 520.0f, 0.0f, 5, 1.0f, true);
	}
	Trace("EI 1");
	g_editor_menu_panel->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	SetDiffuse(g_editor_menu_panel, 1.0f, 1.0f, 1.0f, 0.6f);
	Trace("EI 2");
	g_editor_menu_panel->force_show();

	g_editor_list_panel = new OMediaCanvasElement;
	if (g_editor_list_panel) g_editor_list_panel->link_layer(g_layer_hud2d);
	Trace("EI 3");
	PlaceSprite(g_editor_list_panel, 370.0f, 0.0f, 6, 1.0f, true);
	g_editor_list_panel->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	Trace("EI 4");
	SetDiffuse(g_editor_list_panel, 1.0f, 1.0f, 1.0f, 0.6f);
	g_editor_list_panel->hide(true);

	g_editor_props_panel = new OMediaCanvasElement;
	if (g_editor_props_panel) g_editor_props_panel->link_layer(g_layer_hud2d);
	PlaceSprite(g_editor_props_panel, 0.0f, 0.0f, 7, 1.0f, true);
	g_editor_props_panel->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	SetDiffuse(g_editor_props_panel, 1.0f, 1.0f, 1.0f, 0.6f);
	Trace("EI 5");
	g_editor_props_panel->hide(true);

	CGfx::UnInitCaptions();									// 0x4b16a0
	ResetLevel();
	m_cam_save[0] = g_viewport->getx();
	m_cam_save[1] = g_viewport->gety();
	m_cam_save[2] = g_viewport->getz();
	g_viewport->set_flags(omcvpf_EnableMouseClick | omcvpf_ClickActivateSupervisor);	// 5
	Trace("EI 6");
	if (g_layer_clear) g_layer_clear->remove_flags(omlayerf_DisablePicking);
	if (g_layer_world) g_layer_world->remove_flags(omlayerf_DisablePicking);	// (0x56afd4: the world layer again)
	Trace("EI 7");
	for (int i = 0; i < MENU_MAX; i++)
	{
		if (strcasecmp(Menu()[i], "") == 0) break;
		m_menu_fields[i]->force_show();
	}
	Trace("EI 8");
	ShowWinCursor(true);
	g_viewport->place(m_cam_save[0], m_cam_save[1], m_cam_save[2]);
	SetMenuSelection("Select Object");
	m_mode = 0;
	CaptureMouse(false);
	Trace("Leaving EI ");
}

// 0x4a5af0
void CEditor::UnInitEditor(void)
{
	Trace("UnInitEditor()");
	Trace("Entering UnInitEditor");
	CaptureMouse(true);
	ResetLevel();
	Trace("UIE 1");
	g_viewport->set_flags(g_viewport->get_flags() & 0xfffa);
	for (int i = 0; i < MENU_MAX; i++)
	{
		if (strcasecmp(Menu()[i], "") == 0) break;
		m_menu_fields[i]->hide(true);
	}
	Trace("UIE 2");
	HideList(true);
	ShowWinCursor(false);
	HighlightSelection(false);
	CGfx::UnInitCaptions();
	Trace("UIE 3");
	ClearSelection();
	g_editor_menu_panel->hide(true);
	g_editor_list_panel->hide(true);
	Trace("UIE 4");
	ResetLevelObjects();
	Trace("UIE 5");
}

void CEditor::ClearSelection(void)
{
	m_sel_element = NULL;
	m_sel_object = NULL;
	m_sel_material = NULL;
	m_sel_polygon = NULL;
	m_pick = NULL;
}

// --------------------------------------------------------------------------- 0x4a5c70

void CEditor::UpdateGame(float dt)
{
	// RequestLevelDelayed: count the delay down, showing the countdown
	if (m_load_delay > 0.0f)
	{
		if (m_load_countdown < 0)
		{
			m_load_delay = m_load_delay - dt;
			if (m_load_delay > 0.0f)
			{
				MenuDrawCountdown(1, 0x197, g_532500);		// g_menu->0x409d50
				DrawText(0x12a, 0x1bd, 'BIG', "%6.0d", neg::g_score);
				goto countdown;
			}
			SetGameState(1);
			m_load_pending = 1;
		}
		m_load_delay = 0;
	}
countdown:
	// RequestLevel / start-up: a few frames, then load
	if (m_load_countdown >= 0)
	{
		m_load_countdown--;
		if ((double)m_load_countdown < 0.1) SetGameState(1);
		if (m_load_countdown < 0) m_load_pending = 1;
	}

	m_start_timer += dt;
	if (m_start_state < 3)
	{
		if (m_start_state == 1)
		{
			// First frame: splash 1, then the level to start with (save\startlevel.txt).
			DrawSplash(1);
			m_start_state = 3;
			m_start_timer = 3.0f;
			FILE *f = fopen(omf_SDLResolvePath("save\\startlevel.txt").c_str(), "r");
			if (!f)
				strcpy(m_level_name, "level1d.gam");
			else
			{
				if (fscanf(f, "%s", m_level_name) < 0) {}		// (an empty file leaves the name as it was)
				if (fclose(f) != 0) SDL_Log("The file 'startlevel.txt' was not closed\n");
			}
			if (strcasecmp(m_level_name, "autosave") == 0)
			{
				if (!ReadTaskFile("autosave.tsk"))
				{
					strcpy(m_level_name, "level1d.gam");
					ErrorLog("ERROR: loading autosave.tsk");
				}
				else
				{
					Trace("MEBTEMP GFN:%s", g_game_file);
					Trace("MEBTEMP TFN:%s", g_task_file);
					LoadTask("autosave.tsk", 0);
				}
			}
			m_load_countdown = 2;
		}
		else if (m_start_state == 2)
			m_start_state = 3;
	}
	if (m_load_pending) CheckLoadLevel();
}

// --------------------------------------------------------------------------- 0x4a8420

void CEditor::CheckLoadLevel(void)
{
	if (!m_load_pending) return;

	Trace("before gcu");
	ListObjects();
	neg::CreateStaticObjects();								// 0x480320
	PhysicsReset();											// 0x4b40d0
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if ((*i)->v127()) { Trace("Uniniting %s", (*i)->m_tag); (*i)->UnInitObject(); }
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if ((*i)->v127()) { Trace("Uniniting %s", (*i)->m_tag); (*i)->UnInitDatabases(); }
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if ((*i)->v127()) { Trace("Delme %s", (*i)->m_tag); (*i)->f_006 = 1; }
	DeleteFlaggedObjects();									// 0x4ae550
	CGfx::UnInitAlpha();									// 0x4a0ef0 ReInitAlpha
	CGfx::InitAlpha();
	Trace("after gcu");
	ListObjects();
	Trace("MEBTEMP Deleted all objects about to load level");
	Trace("Calling CloseAllDatabases()");
	CloseAllDatabases();									// 0x4a4870
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) static_cast<CLocalGameObject *>(*i)->f_4aa = 0;
	LoadGame(m_level_name, -1);
}

// --------------------------------------------------------------------------- 0x4a8590

void CEditor::LoadGame(char *name, int level_id)
{
	g_new_game = 0;
	SetLevel1cLoaded(0);
	if (strcasecmp(m_level_name, "level1d.gam") == 0)
	{
		// A new game: forget the autosaves.
		g_new_game = 1;
		SetGameState(0);
		DeleteFile("save\\autosave.tsk");
		static const char *const autosaves[] = {
			"save\\autosave01.tsk", "save\\autosave02.tsk", "save\\autosave03.tsk", "save\\autosave04.tsk",
			"save\\autosave05.tsk", "save\\autosave06.tsk", "save\\autosave07.tsk", "save\\autosave08.tsk",
			"save\\autosave09.tsk", "save\\autosave10.tsk", "save\\autosave11.tsk", "save\\autosave12.tsk",
			"save\\autosave13.tsk", "save\\autosave14.tsk", "save\\autosave15.tsk", "save\\autosave16.tsk",
			"save\\autosave17.tsk" };
		for (unsigned i = 0; i < sizeof(autosaves) / sizeof(autosaves[0]); i++) DeleteFile(autosaves[i]);
	}
	else if (strcasecmp(m_level_name, "level1_c.gam") == 0)
		SetLevel1cLoaded(1);

	strcpy(m_game_name, name);
	strcpy(g_game_file, name);
	char path[128];
	strcpy(path, "gam\\");
	strcat(path, name);
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	Trace("LOADING NEW GAME");
	CGfx::UnInitCaptions();									// 0x4b16a0
	PhysicsReset();											// 0x4b40d0
	fs.setpath(&fp);
	if (!fs.fileexists())
	{
		Trace("WARNING file %s does not exist for load", name);
		CGameObject *o = neg::CreateObject(level_id);
		if (!o)
		{
			g_gametype = NULL;
			ErrorLog("ERROR unable to create LevelObjectID in LoadGame for %s", name);
		}
		else
		{
			g_gametype = GameTypeOf(o);
			if (!g_gametype) ErrorLog("ERROR unable to create LevelObjectID in LoadGame for %s", name);
		}
		return;
	}

	fs.open(omcfp_Read, false, false);						// (throws OMediaError when it fails, as OMT 2002 did)
	if (!fs.isopen())
	{
		ErrorLog("ERROR opening file %s for load", name);
		return;
	}

	fs >> level_id;											// the game type's 4cc (the parameter is reused)
	CGameObject *gt = neg::CreateObject(level_id);
	if (!gt || !GameTypeOf(gt))
	{
		ErrorLog("ERROR: CurrentGame=NULL in LoadGame, probably invalid .GAM file");
		return;
	}
	g_gametype = GameTypeOf(gt);

	int num;
	fs >> num;
	if (num > 0)
	{
		int id;
		fs >> id;
		CGameObject *o = IsSingletonLoaded(id) ? FindObject(id) : neg::CreateObject(id);
		if (o)
		{
			for (;;)
			{
				num--;
				Trace("Created %s", o->m_name);
				o->m_object_id = id;
				o->ReadProperties(fs);						// stream >> (OMediaClassStreamer *)o -> read_class
				Trace("NumObjects=%d in LoadGame", num);
				if (num < 1) break;
				fs >> id;
				char ids[5];
				IdString(ids, id);
				if (IsSingletonLoaded(id))
				{
					o = FindObject(id);
					if (!o)
					{
						ErrorLog("ERROR: loading Only one, but could not find it %s", ids);
						ErrorLog("ERROR: NULL object spawned in LoadGame for %s", ids);
						break;
					}
					Trace("Item was already loaded %s %s", ids, o->m_tag);
				}
				else
				{
					o = neg::CreateObject(id);
					if (!o) { ErrorLog("ERROR: NULL object spawned in LoadGame for %s", ids); break; }
				}
				Trace("Object %d Spawned in LoadGame", id);
			}
		}
	}
	fs.close();
	Trace("File closed, Calling ResetLevel 1");
	ResetLevel();
	if (g_task_file[0]) RestoreTask(g_task_file);			// 0x498440
	Trace("File closed, Calling ResetLevel 2");
	g_splash->hide(true);
}

// --------------------------------------------------------------------------- 0x4a8100

void CEditor::SaveGame(char *name)
{
	char path[128];
	strcpy(path, "gam\\");
	strcat(path, name);
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	PlaySoundNamed(-1, "neutron", 0);						// 0x4b8380
	fs.setpath(&fp);
	fs.open(omcfp_Write, true, true);

	int num = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if ((*i)->f_00b == 1) num++;
	int type_id = GameTypeElement()->get_id();
	fs << type_id;
	fs << num;
	Trace("Saving game %d objects for %s", num, name);
	int n = 1;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o || o->f_00b != 1) continue;
		Trace("%3d) %20s    %s %d", n, o->m_tag, o->m_name, o->m_num_properties);
		fs << o->m_object_id;
		fs << o->m_num_properties;
		o->WriteProperties(fs);								// stream << (OMediaClassStreamer *)o -> write_class
		n++;
	}
	fs.close();
}

// --------------------------------------------------------------------------- 0x4a8dc0

// name -> "<name>_bak_<n>" for the first n that does not exist yet (n = 0 when name itself is free)
void CEditor::MakeBackupName(char *name)
{
	char path[128], tmp[128];
	strcpy(path, "gam\\");
	strcat(path, name);
	int n = 0;
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	while (fs.fileexists())
	{
		n++;
		sprintf(tmp, "gam\\%s_bak_%.3d", name, n);
		OMediaFilePath p2(tmp, 'null', 'null');
		fs.setpath(&p2);
	}
	strcpy(tmp, name);
	sprintf(name, "%s_bak_%.3d", tmp, n);
}

// --------------------------------------------------------------------------- 0x4a8340 / 0x4a83b0

void CEditor::RequestLevel(char *name)
{
	if (g_menu)
	{
		g_menu->v288();
		StartFade(0, 1.0f);									// 0x406a40
	}
	m_load_countdown = 2;
	strcpy(m_level_name, name);
	g_level_requested = 1;
}

void CEditor::RequestLevelDelayed(char *name, float delay)
{
	if (g_menu)
	{
		g_menu->v288();
		StartFade(0, 1.0f);
	}
	m_load_delay = delay;
	strcpy(m_level_name, name);
	g_level_requested = 1;
}

// --------------------------------------------------------------------------- 0x4aa0b0 (vtable +0x118)

void CEditor::OpenEditorDatabase(void)
{
	std::string name("omt\\Editor.omt");
	g_editor_stream = InitFileStream(name.c_str());
	g_editor_db = InitDatabase(g_editor_stream);
}

// --------------------------------------------------------------------------- object list queries

bool CEditor::IsSingletonLoaded(int id)					// 0x4a8c00: an object with this 4cc and f_009 set
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && id == (*i)->m_object_id && (*i)->f_009) return true;
	return false;
}

CGameObject *CEditor::FindObject(int id)				// 0x4a8c40
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && (*i)->m_object_id == id) return *i;
	return NULL;
}

int CEditor::CountObjects(void)							// 0x4a9c80
{
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) n++;
	return n;
}

int CEditor::CountSavedObjects(void)					// 0x4a9ca0: objects written by SaveGame (v108 = f_00b)
{
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && (*i)->v108()) n++;
	return n;
}

int CEditor::CountCollisionObjects(void)				// 0x4a9ce0: v135 = f_140
{
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && (*i)->v135()) n++;
	return n;
}

// 0x4a8c70: writes the object list to the (empty) debug log
void CEditor::ListObjects(void)
{
	Trace("             *********************************");
	Trace("             ******** LIST OF OBJECTS ********");
	Trace("             *********************************");
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o) continue;
		n++;
		char ids[5];
		IdString(ids, o->m_object_id);
		char coll[80];
		strcpy(coll, "");
		if (o->v135())
		{
			strcpy(coll, "YES");
			if (o->f_124 == 2) strcpy(coll, "CUBE COLLISION");
		}
		Trace("%3d) %18s  ID=%s   %s  %s", n, o->m_tag, ids, o->m_name, coll);
	}
}

// --------------------------------------------------------------------------- 0x4a8ad0

void CEditor::ResetLevel(void)
{
	Trace("RESETLEVEL 1");
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o) continue;
		Trace("%s RESETLEVEL 2", o->m_tag);
		o->ResetObject();
		if (o->f_008)
		{
			CGameObject *obj = o; OMediaElement *elem = NULL;
			ObjectElement(o->m_object_id, &obj, &elem);
			if (elem)
			{
				if (!m_editing) elem->hide(true);
				else elem->force_show();
			}
		}
		o->v260();
		if (o->f_008)
		{
			CGameObject *obj = o; OMediaElement *elem = NULL;
			ObjectElement(o->m_object_id, &obj, &elem);
			if (elem && m_editing) elem->force_show();
		}
	}
	ListObjects();
	Trace("EXITING LOADING NEW GAME");
	ResetFrameTimer();										// 0x4b9600
}

// 0x4a8a90
void CEditor::ResetLevelObjects(void)
{
	Trace("RESETLEVEL 1");
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) (*i)->v264();
}

// --------------------------------------------------------------------------- splash / sprites

// 0x4aa180 (vtable +0x12c): replace the full-screen splash with canvas id of omt\Editor.omt
void CEditor::DrawSplash(int canvas_id)
{
	Trace("EDS");
	Trace("1 DrawSplash ALL:%d", (int)OMediaDataBase::get_memory_used_all());
	if (g_splash)
	{
		Trace("Deleting splash");
		if (g_splash->get_canvas()) delete g_splash->get_canvas();
		if (g_splash) delete g_splash;
		g_splash = NULL;
	}
	Trace("after Deleting splash");
	Trace("2 DrawSplash after delete ALL:%d", (int)OMediaDataBase::get_memory_used_all());
	if (!g_splash)
	{
		Trace("Allocating aplash");
		g_splash = new OMediaCanvasElement;
		if (g_splash)
		{
			g_splash->link_layer(g_layer_hud2d);
			if (g_splash)
			{
				const float sx = (short)g_screen_w * (1.0f / 640.0f);
				const float sy = (short)g_screen_h * (1.0f / 480.0f);
				OMediaCanvas *c = LoadCanvasID(g_editor_db, canvas_id);
				if (c)
				{
					c->db_unlock();
					c->set_filtering(omtfc_Linear, omtfc_Linear);
					g_splash->set_size(c->get_width() * sx, c->get_height() * sy);
				}
				g_splash->set_canvas(c);
			}
		}
		g_splash->set_canvas_flags(omcanef_FreeWorldSize);
		g_splash->set_auto_align(omaac_Center, omaac_Center);	// (3, 3; never applied: not linked to the world)
		g_splash->setz(0);
		g_splash->setx((float)-((short)g_screen_w / 2));
		g_splash->sety((float)((short)g_screen_h / 2) - g_splash->get_height());
		g_splash->force_show();
	}
	Trace("3 DrawSplash after allocation ALL:%d", (int)OMediaDataBase::get_memory_used_all());
}

// 0x4aa3e0: same as DrawSplash's set-up, on an element of the caller's
void CEditor::PlaceSplash(OMediaCanvasElement *e, int unused1, int unused2, int canvas_id)
{
	(void)unused1; (void)unused2;
	if (!e) return;
	const float sx = (short)g_screen_w * (1.0f / 640.0f);
	const float sy = (short)g_screen_h * (1.0f / 480.0f);
	OMediaCanvas *c = LoadCanvasID(g_editor_db, canvas_id);
	if (c)
	{
		c->db_unlock();
		c->set_filtering(omtfc_Linear, omtfc_Linear);
		e->set_size(c->get_width() * sx, c->get_height() * sy);
	}
	e->set_canvas(c);
	e->set_canvas_flags(omcanef_FreeWorldSize);
	e->set_auto_align(omaac_Center, omaac_Center);
	e->setz(0);
	e->setx((float)-((short)g_screen_w / 2));
	e->sety((float)((short)g_screen_h / 2) - g_splash->get_height());	// g_splash's height, as shipped
	e->force_show();
}

// 0x4aa500: a sprite at (x, y) in 640x480 screen space, top-left origin
void CEditor::PlaceSprite(OMediaCanvasElement *e, float x, float y, int canvas_id, float unused, bool scaled)
{
	(void)unused;
	const float sx = (short)g_screen_w * (1.0f / 640.0f);
	const float sy = (short)g_screen_h * (1.0f / 480.0f);
	if (!e) return;
	SetSpriteCanvas(e, canvas_id, scaled);
	e->set_canvas_flags(omcanef_FreeWorldSize);
	e->set_auto_align(omaac_Center, omaac_Center);
	e->setz(0);
	e->setx(sx * x - (float)((short)g_screen_w / 2));
	e->sety(((float)((short)g_screen_h / 2) - e->get_height()) - sy * y);
}

// 0x4aa5c0
void CEditor::SetSpriteCanvas(OMediaCanvasElement *e, int canvas_id, bool scaled)
{
	const float sx = (short)g_screen_w * (1.0f / 640.0f);
	const float sy = (short)g_screen_h * (1.0f / 480.0f);
	if (canvas_id == -1)
	{
		// As shipped this passes the flag itself as the canvas; no caller uses -1.
		e->set_canvas((OMediaCanvas *)(intptr_t)scaled);
		return;
	}
	OMediaCanvas *c = GetCanvas(canvas_id);
	if (!c) { e->set_canvas(NULL); return; }
	if (scaled) e->set_size(c->get_width() * sx, c->get_height() * sy);
	else e->set_size((float)c->get_width(), (float)c->get_height());
	e->set_canvas(c);
}

// 0x4aa690
OMediaCanvas *CEditor::GetCanvas(int canvas_id)
{
	OMediaCanvas *c = LoadCanvasID(g_editor_db, canvas_id);
	if (c) c->db_unlock();
	return c;
}

// 0x4aa7e0: the list panel (and the property panel when closing)
void CEditor::ShowPanel(bool on)
{
	if (on)
	{
		g_editor_list_panel->force_show();
		m_panel_open = 1;
		return;
	}
	g_editor_list_panel->hide(true);
	m_panel_open = 0;
	g_editor_props_panel->hide(true);
}

// ===========================================================================================
// The interactive editor (only while m_editing is set; nothing in the shipped game sets it).
// Screen-space fields: x from the screen centre, y from the viewport's top.
// ===========================================================================================

static bool HitField(OMediaStringField *f, float mx, float my, int half_w)
{
	const int half_h = g_viewport->getbheight() / 2;
	return f->getx() + (float)half_w < mx && mx < f->get_width() + f->getx() + (float)half_w &&
		   ((float)half_h - f->gety()) - f->get_height() < my && my < (float)half_h - f->gety();
}

// Flag the selection's element moved: give the object the element's position.
static void MoveSelectedObject(CEditor *ed)
{
	OMediaElement *e = ed->m_sel_element;
	ed->m_sel_object->v197(e->getx(), e->gety(), e->getz());
	ed->m_sel_object->v90(e->getx(), e->gety(), e->getz());
}

// --------------------------------------------------------------------------- 0x4a5ec0

void CEditor::UpdateEditor(float dt)
{
	ev52();
	if (m_editing)
	{
		int coll = CountCollisionObjects();
		int saved = CountSavedObjects();
		int all = CountObjects();
		DrawText(0xe1, 3, 'SMAL', "NUMOBJS  %d  SAVE %d  COLLOBJS  %d", all, saved, coll);
		if (m_copied)
		{
			DrawText(0x1e0, 0xf0, 'SMAL', "COPIED");
			DrawText(0x1e0, 0xfc, 'SMAL', "%s", m_copied->m_tag);
		}

		// Ctrl-C copies the selection, Ctrl-V pastes a copy in front of the camera
		if (g_keys[VK_CONTROL_])
		{
			if (g_keys['C'] && m_sel_element && m_sel_object) m_copied = m_sel_object;
			if (g_keys[VK_CONTROL_] && g_keys['V'])
			{
				if (!m_paste_latch)
				{
					m_paste_latch = 1;
					if (m_copied) PasteObject(m_copied);
				}
			}
			else m_paste_latch = 0;
		}
		else m_paste_latch = 0;

		if (g_keys[VK_DELETE_] && m_sel_element && m_sel_object)
		{
			m_sel_object->f_006 = 1;
			HighlightSelection(false);
			ClearSelection();
			DeleteFlaggedObjects();
		}
		if (g_keys[VK_OEM_3_])
		{
			HighlightSelection(false);
			ClearSelection();
		}

		// arrows scroll the lists by 20; Shift-N cycles the object labels
		if (g_keys[VK_UP_])
		{
			if (!m_key_latch)
			{
				m_key_latch = 1;
				if (m_mode == 5)
				{
					m_list_offset -= 20;
					if (m_list_offset < 0) m_list_offset = 0;
					ShowObjectList();
				}
				else if (m_mode == 1)
				{
					m_list_offset -= 20;
					if (m_list_offset < 0) m_list_offset = 0;
					ShowCatalogue();
				}
			}
		}
		else if (g_keys[VK_DOWN_])
		{
			if (!m_key_latch)
			{
				m_key_latch = 1;
				if (m_mode == 5) { m_list_offset += 20; ShowObjectList(); }
				else if (m_mode == 1) { m_list_offset += 20; ShowCatalogue(); }
			}
		}
		else if (g_keys[VK_SHIFT_] && g_keys['N'])
		{
			if (!m_key_latch)
			{
				m_key_latch = 1;
				if (m_label_mode == 0) m_label_mode = 1;
				else if (m_label_mode == 1) m_label_mode = 2;
				else if (m_label_mode == 2) m_label_mode = 0;
			}
		}
		else m_key_latch = 0;
	}

	// Camera angles shown in degrees; past 360 they are pulled back (by 16384, the full turn
	// in OMT units, subtracted from the degree value - as shipped).
	const float k = 360.0f / 16384.0f;		// 0x4cb1f8
	OMediaViewPort *vp = g_viewport;
	if (vp->get_anglex() * k > 360.0f) vp->set_anglex((short)(int)(vp->get_anglex() * k - 16384.0f));
	if (vp->get_angley() * k > 360.0f) vp->set_angley((short)(int)(vp->get_angley() * k - 16384.0f));
	if (vp->get_anglez() * k > 360.0f) vp->set_anglez((short)(int)(vp->get_anglez() * k - 16384.0f));
	DrawText(0x1e, 10, 'SMAL', "ROT %6.0f   %6.0f   %6.0f", (double)(vp->get_anglex() * k),
			 (double)(vp->get_angley() * k), (double)(vp->get_anglez() * k));
	DrawText(0x1e, 20, 'SMAL', "POS %6.0f   %6.0f   %6.0f", (double)g_viewport->getx(),
			 (double)g_viewport->gety(), (double)g_viewport->getz());

	// object labels
	if (m_label_mode != 0)
	{
		for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		{
			CGameObject *o = *i;
			if (!o || !o->f_00b) continue;
			const float cx = g_viewport->getx(), cy = g_viewport->gety(), cz = g_viewport->getz();
			float p[4];
			o->v195(p);
			const float dx = cx - p[0], dy = cy - p[1], dz = cz - p[2];
			if (m_label_mode == 1 && !(sqrtf(dz * dz + dy * dy + dx * dx) <= 1000.0f)) continue;
			float q[4];
			o->v195(q);
			float sx, sy;
			if (static_cast<CLocalGameObject *>(o)->v281(g_viewport, q, &sx, &sy))
				DrawText((short)(int)(sx + 320.0f), (short)(int)(240.0f - sy), 'SMAL', "%s", o->m_tag);
		}
	}

	// the selection: its label; Ctrl-L previews a cutscene
	if (m_sel_object)
	{
		static_cast<CLocalGameObject *>(m_sel_object)->v283(FloatBits(dt));
		float q[4];
		m_sel_object->v195(q);
		float sx, sy;
		if (static_cast<CLocalGameObject *>(m_sel_object)->v281(g_viewport, q, &sx, &sy))
			DrawText((short)(int)(sx + 320.0f), (short)(int)(240.0f - sy), 'SMAL', "%s", m_sel_object->m_tag);
		if (g_keys['L'] && g_keys[VK_CONTROL_])
		{
			if (m_sel_object->NameContains("C3DCUTSCENECAMERA")) CutsceneCameraPlay(m_sel_object);
			else if (m_sel_object->NameContains("C3DMULTICUTSCENE")) MultiCutscenePlay(m_sel_object);
		}
	}

	// Enter applies the property being edited; otherwise the arrows fly the camera
	if (KeyPressed(VK_RETURN_))
	{
		if (m_edit_name && m_edit_value && m_sel_object) ApplyPropertyEdit();
	}
	else if (!(g_keys[VK_CONTROL_] && KeyPressed('E')))
	{
		float d[4];
		if (g_keys[VK_DOWN_])
		{
			if (!m_panel_open)
			{
				CameraDir(d, 0, 0, -10.0f);
				g_viewport->setx(d[0] * dt * 300.0f + g_viewport->getx());
				g_viewport->sety(d[1] * dt * 300.0f + g_viewport->gety());
				g_viewport->setz(d[2] * dt * 300.0f + g_viewport->getz());
			}
		}
		else if (g_keys[VK_UP_])
		{
			if (!m_panel_open)
			{
				CameraDir(d, 0, 0, 10.0f);
				g_viewport->setx(d[0] * dt * 500.0f + g_viewport->getx());
				g_viewport->sety(d[1] * dt * 500.0f + g_viewport->gety());
				g_viewport->setz(d[2] * dt * 500.0f + g_viewport->getz());
			}
		}
	}
	if (g_keys[VK_LEFT_])
	{
		float d[4];
		CameraDir(d, -10.0f, 0, 0);
		g_viewport->setx(d[0] * dt * 100.0f + g_viewport->getx());
		g_viewport->sety(d[1] * dt * 100.0f + g_viewport->gety());
		g_viewport->setz(d[2] * dt * 100.0f + g_viewport->getz());
	}
	if (g_keys[VK_RIGHT_])
	{
		float d[4];
		CameraDir(d, 10.0f, 0, 0);
		g_viewport->setx(d[0] * dt * 100.0f + g_viewport->getx());
		g_viewport->sety(d[1] * dt * 100.0f + g_viewport->gety());
		g_viewport->setz(d[2] * dt * 100.0f + g_viewport->getz());
	}

	// Mouse: both buttons move up/down, left moves in the ground plane, right turns. With Shift
	// the selection moves instead of the camera; with Ctrl both move (Ctrl-right turns the selection).
	const float speed = m_move_speed;
	if (g_mouse_down)
	{
		if (g_mouse_captured)
		{
			short mdx = 0, mdy = 0;
			GetMouseDelta(&mdx, &mdy);
			float d[4];
			if (g_mouse_rdown)
			{
				CameraDir(d, (float)-mdx, (float)mdy, 0);
				if (m_sel_element && g_keys[VK_SHIFT_])
				{
					m_sel_element->sety(d[1] * speed * dt + m_sel_element->gety());
					MoveSelectedObject(this);
				}
				else
				{
					g_viewport->sety(d[1] * speed * dt + g_viewport->gety());
					if (g_keys[VK_CONTROL_] && m_sel_object)
					{
						m_sel_element->sety(d[1] * speed * dt + m_sel_element->gety());
						MoveSelectedObject(this);
					}
				}
			}
			else
			{
				CameraDir(d, (float)-mdx, 0, (float)mdy);
				if (m_sel_element && g_keys[VK_SHIFT_])
				{
					m_sel_element->setx(d[0] * speed * dt + m_sel_element->getx());
					m_sel_element->setz(d[2] * speed * dt + m_sel_element->getz());
					MoveSelectedObject(this);
				}
				else
				{
					g_viewport->setx(d[0] * speed * dt + g_viewport->getx());
					g_viewport->setz(d[2] * speed * dt + g_viewport->getz());
					if (g_keys[VK_CONTROL_] && m_sel_object)
					{
						m_sel_element->setx(d[0] * speed * dt + m_sel_element->getx());
						m_sel_element->setz(d[2] * speed * dt + m_sel_element->getz());
						MoveSelectedObject(this);
					}
				}
			}
		}
	}
	else if (g_mouse_rdown)
	{
		if (g_mouse_captured)
		{
			short mdx = 0, mdy = 0;
			GetMouseDelta(&mdx, &mdy);
			OMediaElement *e = (m_sel_element && g_keys[VK_CONTROL_]) ? m_sel_element : (OMediaElement *)g_viewport;
			const short ry = (short)(int)(mdx * speed * dt);
			const short rx = (short)(int)(mdy * speed * dt);
			e->set_anglex((short)(e->get_anglex() + rx));
			e->set_angley((short)(e->get_angley() + ry));
		}
	}
	if (g_mouse_down || g_mouse_rdown)
	{
		if (!g_mouse_captured) CaptureMouse(true);
	}
	else if (g_mouse_captured == 1)
		CaptureMouse(false);
}

// 0x4a7e30: computes the point at the camera and drops it
void CEditor::ev52(void)
{
	float p[4];
	CameraPoint(p, 0, 0, 0);
}

// --------------------------------------------------------------------------- 0x4a74f0

void CEditor::ViewportClicked(OMediaPickRequest *req)
{
	if (m_sel_object && m_sel_element) ApplyPropertyEdit();
	const int x = (int)req->pickx, y = (int)req->picky;
	switch (m_mode)
	{
		case 0: case 2: case 3:
			ClickMenu(x, y);
			if (m_sel_object && ClickPropertyField(x, y)) return;
			break;
		case 1: case 5:
			ClickMenu(x, y);
			ClickList(x, y);
			break;
		case 4: case 6:
			ClickMenu(x, y);
			ClickList(x, y);
			return;
		case 7:
			ClickMenu(x, y);
			return;
	}
	if (req->closer_hit.type == omptc_Null) return;
	if (m_mode == 0)
	{
		// pick an object
		m_pick = req;
		HighlightSelection(false);
		m_sel_object = NULL;
		m_sel_element = req->closer_hit.element;
		ObjectElement(m_sel_element->get_id(), &m_sel_object, &m_sel_element);
		if (!m_sel_object) return;
		m_sel_shape = req->closer_hit.shape;
		const int id = m_sel_object->m_object_id;
		if (id == ID_TERRAIN || id == ID_SKY) { ClearSelection(); return; }
		ShowPanel(false);
		HighlightSelection(true);
	}
	else if (m_mode == 2)
	{
		// pick a polygon: take its material off (it is drawn as "no material" while selected)
		if (req->closer_hit.sub_info.size() == 0) return;
		const int poly = req->closer_hit.sub_info[0].polygon;
		if (poly == -1) return;
		m_sel_polygon = &(*req->closer_hit.shape->get_polygons())[poly];
		m_sel_material = m_sel_polygon->get_material();
		m_sel_polygon->set_material(NULL);
		m_sel_shape = req->closer_hit.shape;
	}
}

// --------------------------------------------------------------------------- 0x4a6b20

void CEditor::ClickList(int x, int y)
{
	const float mx = (float)x;
	for (short i = 0; i < 100; i++)
	{
		OMediaStringField *f = m_list_fields[i];
		if (!HitField(f, mx, (float)y, (short)g_screen_w / 2)) continue;
		switch (m_mode)
		{
			case 1:		// create the clicked catalogue entry in front of the camera
			{
				f->select_all();
				m_list_fields[i]->set_focus(true);
				const char *name = Catalogue()[m_list_offset + i].name;
				if (Catalogue()[0].id == -1) break;
				int j = 0;
				while (strcasecmp(Catalogue()[j].name, name) != 0)
				{
					j++;
					if (Catalogue()[j].id == -1) { j = -1; break; }
				}
				if (j < 0) break;
				const int id = Catalogue()[j].id;
				if (id == -1) break;
				OMediaElement *elem = NULL;
				CGameObject *o = neg::CreateObject(id);
				o->v258();
				ObjectElement(id, &o, &elem);
				float p[4];
				CameraPoint(p, 0, -90.0f, 300.0f);
				if (elem) elem->place(p[0], p[1], p[2]);
				o->v197(p[0], p[1], p[2]);
				o->v90(p[0], p[1], p[2]);
				HideList(true);
				SetMenuSelection("Select Object");
				m_mode = 0;
				break;
			}
			case 4:		// a toggle
				if (ToggleOption(i)) return;
				break;
			case 5:		// select the clicked object of the list and go there
			{
				HighlightSelection(false);
				int n = 0, shown = 0;
				for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); it++)
				{
					CGameObject *o = *it;
					if (m_list_filter == 1 && !o->NameContains("C3DANIMATED")) continue;
					if (m_list_filter == 3 && !o->NameContains("C3DPICKUPITEM")) continue;
					if (m_list_filter == 2 && !o->NameContains("C3DTRIGGERTYPE") && !o->NameContains("C3DFLYINGCAMERA")) continue;
					if (shown >= 100) break;
					if (!o || o->m_object_id == ID_TERRAIN || !o->f_00b) continue;
					if (n > m_list_offset)
					{
						if (shown == i)
						{
							m_sel_object = o;
							float p[4];
							o->v195(p);
							g_viewport->place(p[0], p[1], p[2]);
						}
						shown++;
					}
					n++;
				}
				if (m_sel_object)
				{
					m_sel_element = NULL;
					ObjectElement(m_sel_object->m_object_id, &m_sel_object, &m_sel_element);
					ShowProperties(true);
					if (m_sel_element)
					{
						SetMenuSelection("Select Object");
						m_mode = 0;
						HideList(true);
						m_pick = NULL;
						m_sel_shape = NULL;
						m_sel_material = NULL;
					}
				}
				break;
			}
			case 6:		// load the clicked level
				HighlightSelection(false);
				ClearSelection();
				DeleteAllObjects();
				HideList(true);
				m_mode = 0;
				LoadGame(Levels()[i].name, Levels()[i].id);
				break;
		}
	}
}

// --------------------------------------------------------------------------- 0x4a6f90 (no this)

bool CEditor::ToggleOption(int index)
{
	const char *opt = Toggles()[index];
	if (strcasecmp("Sky", opt) == 0) { Trace("Sky"); return true; }
	if (strcasecmp("Info", opt) == 0) { Trace("Info"); return true; }
	if (strcasecmp("Terrain", opt) == 0) { Trace("Terrain"); return true; }
	if (strcasecmp("Far Plane", opt) == 0)
	{
		// world layers' far clip: 5000 -> 10000 -> 35000 -> 1000 (0x56afc8 and 0x56afd4: both the world layer)
		const float fc = g_layer_world->get_far_clip();
		float v;
		if (fc < 5000.0f) v = 5000.0f;
		else if (fc < 10000.0f) v = 10000.0f;
		else if (fc < 35000.0f) v = 35000.0f;
		else v = 1000.0f;
		g_layer_world->set_far_clip(v);
		g_layer_world->set_far_clip(v);
		return true;
	}
	if (strcasecmp("Fill Mode", opt) == 0)
	{
		// point -> line -> solid, on the world and clear layers
		switch (g_layer_world->get_override_fill_mode())
		{
			case omfillmc_Point:
				g_layer_world->enable_override_fill_mode(omfillmc_Line);
				g_layer_clear->enable_override_fill_mode(omfillmc_Line);
				g_layer_world->enable_override_fill_mode(omfillmc_Line);
				if (g_layer_clear) g_layer_clear->add_flags(omlayerf_ClearColor);		// 0x4b1880
				return true;
			case omfillmc_Line:
				g_layer_world->enable_override_fill_mode(omfillmc_Solid);
				g_layer_clear->enable_override_fill_mode(omfillmc_Solid);
				g_layer_world->enable_override_fill_mode(omfillmc_Solid);
				if (g_layer_clear) g_layer_clear->remove_flags(omlayerf_ClearColor);	// 0x4b1860
				return true;
			case omfillmc_Solid:
				g_layer_world->enable_override_fill_mode(omfillmc_Point);
				g_layer_clear->enable_override_fill_mode(omfillmc_Point);
				g_layer_world->enable_override_fill_mode(omfillmc_Point);
				if (g_layer_clear) g_layer_clear->add_flags(omlayerf_ClearColor);
				return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------------- 0x4a71b0

bool CEditor::ClickPropertyField(int x, int y)
{
	const float mx = (float)x;
	for (int i = 0; i < 100; i++)
	{
		OMediaStringField *f = m_prop_values[i];
		if (!HitField(f, mx, (float)y, g_viewport->getbwidth() / 2)) continue;

		m_edit_value = m_prop_values[i];
		m_edit_name = m_prop_names[i];
		std::string s;
		m_edit_value->get_string(s);
		char text[100];
		strcpy(text, s.c_str());
		if (strcasecmp(text, "TRUE") == 0)
		{
			m_edit_value->set_string("FALSE");
			ApplyPropertyEdit();
		}
		else if (strcasecmp(text, "FALSE") == 0)
		{
			m_edit_value->set_string("TRUE");
			ApplyPropertyEdit();
		}
		else
		{
			m_prop_values[i]->set_focus(true);
			m_prop_values[i]->select_all();
		}
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------- 0x4a78c0

void CEditor::HighlightSelection(bool on)
{
	const OMediaFARGBColor picked(0.5f, 0.0f, 1.0f, 0.0f);		// green, half transparent
	const OMediaFARGBColor normal(1.0f, 0.8f, 0.8f, 0.8f);		// 0x4aa830(0.8, 0.8, 0.8), a = 1
	if (on) g_editor_props_panel->force_show();
	else g_editor_props_panel->hide(true);
	if (!m_sel_object) return;
	if (on)
	{
		if (m_sel_shape && m_pick)
		{
			m_sel_shape->lock(omlf_Write);
			omt_PolygonList *polys = m_pick->closer_hit.shape->get_polygons();
			for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
				if (p->get_material())
				{
					p->get_material()->set_diffuse(picked);
					p->get_material()->set_light_mode(ommlmc_Color);
				}
			m_sel_shape->unlock();
		}
		ShowProperties(true);
		for (int i = 0; i < 100; i++)
		{
			g_editor->m_list_fields[i]->set_string(" ");
			g_editor->m_list_fields[i]->force_show();
		}
		static_cast<CLocalGameObject *>(m_sel_object)->v280();
	}
	else
	{
		if (m_sel_shape)
		{
			m_sel_shape->lock(omlf_Write);
			omt_PolygonList *polys = m_sel_shape->get_polygons();
			for (omt_PolygonList::iterator p = polys->begin(); p != polys->end(); p++)
				if (p->get_material())
				{
					p->get_material()->set_diffuse(normal);
					p->get_material()->set_light_mode(ommlmc_Color);
				}
			m_sel_shape->unlock();
		}
		ShowProperties(false);
	}
}

// --------------------------------------------------------------------------- 0x4a7780

void CEditor::SetMenuSelection(const char *entry)
{
	short i = 0;
	for (;;)
	{
		if (strcasecmp(Menu()[i], "") == 0) return;
		if (strcasecmp(entry, Menu()[i]) == 0) break;
		if (++i > 99) return;
	}
	for (i = 0; i < 100; i++)
	{
		if (strcasecmp(Menu()[i], "") == 0) return;
		if (strcasecmp(entry, Menu()[i]) == 0) SetDiffuse(m_menu_fields[i], 1.0f, 1.0f, 0.0f, 1.0f);
		else SetDiffuse(m_menu_fields[i], 1.0f, 0.0f, 1.0f, 0.3f);
	}
}

// --------------------------------------------------------------------------- 0x4a6ac0

void CEditor::HideList(bool hide)
{
	g_editor_list_panel->force_show();
	for (int i = 0; i < 100; i++)
	{
		if (!hide) m_list_fields[i]->force_show();
		else m_list_fields[i]->hide(true);
	}
}

// --------------------------------------------------------------------------- 0x4a7e50

void CEditor::ApplyPropertyEdit(void)
{
	if (!m_sel_object || !m_sel_element) return;
	if (m_edit_name && m_edit_value)
	{
		std::string name, value;
		m_edit_name->get_string(name);
		char value_text[100];
		m_edit_value->get_string(value);
		strcpy(value_text, value.c_str());
		(void)atof(value_text);									// (computed and dropped, as shipped)
		for (std::list<CProperty *>::iterator i = m_sel_object->m_properties.begin(); i != m_sel_object->m_properties.end(); i++)
		{
			CProperty *p = *i;
			if (name.compare(0, name.size(), p->name, strlen(p->name)) != 0) continue;
			switch (p->type)
			{
				case PROP_ALPHA:	strcpy((char *)p->data, value_text); break;
				case PROP_BOOL:		*(int *)p->data = strcasecmp(value_text, "TRUE") == 0; break;
				case PROP_FLOAT:	*(float *)p->data = (float)atof(value_text); break;
				case PROP_SPECIFIC:
				case PROP_LONG:		*(int *)p->data = atoi(value_text); break;
			}
		}
	}
	m_sel_object->v258();
}

// --------------------------------------------------------------------------- 0x4a7b30

void CEditor::ShowProperties(bool on)
{
	std::list<CProperty *> &props = m_sel_object->m_properties;
	int i = 0;
	if (on)
	{
		for (std::list<CProperty *>::iterator it = props.begin(); it != props.end(); it++, i++)
		{
			CProperty *p = *it;
			m_prop_names[i]->set_string(p->name);
			SetDiffuse(m_prop_names[i], 1.0f, 1.0f, 1.0f, 0.7f);
			m_prop_names[i]->hide(false);
			char text[100];
			strcpy(text, "");
			switch (p->type)
			{
				case PROP_ALPHA:
					sprintf(text, "%s", (char *)p->data);
					m_prop_values[i]->set_default_filter(omckf_AlphaNumericExtended);
					break;
				case PROP_BOOL:
					sprintf(text, "%s", *(int *)p->data ? "TRUE" : "FALSE");
					m_prop_values[i]->set_default_filter(omckf_AlphaNumeric);
					break;
				case PROP_FLOAT:
					sprintf(text, "%f", (double)*(float *)p->data);
					m_prop_values[i]->set_default_filter(omckf_Double);
					break;
				case PROP_SPECIFIC:
				case PROP_LONG:
					sprintf(text, "%d", *(int *)p->data);
					m_prop_values[i]->set_default_filter(omckf_Integer);
					break;
			}
			m_prop_values[i]->set_string(text);
			m_prop_values[i]->hide(false);
		}
	}
	else
	{
		for (std::list<CProperty *>::iterator it = props.begin(); it != props.end(); it++, i++)
		{
			m_prop_names[i]->hide(true);
			m_prop_values[i]->hide(true);
		}
	}
}

// --------------------------------------------------------------------------- 0x4a8f90

void CEditor::ClickMenu(int x, int y)
{
	const float mx = (float)x, my = (float)y;
	for (int i = 0; i < MENU_MAX; i++)
	{
		const char *entry = Menu()[i];
		if (strcasecmp(entry, "") == 0) return;
		if (!HitField(m_menu_fields[i], mx, my, (short)g_screen_w / 2)) continue;

		if (strcasecmp(entry, "Select Object") == 0)
		{
			HideList(true);
			ShowPanel(false);
			m_mode = 0;
			Trace("Caption %s", entry);
			SetMenuSelection(entry);
			if (m_sel_element && m_sel_object) HighlightSelection(false);
			Trace("MEBTEMP selected object");
			return;
		}
		if (strcasecmp(entry, "(C)reate Object") == 0)
		{
			Trace("MEBTEMP CreateObj");
			HideList(true);
			ShowPanel(true);
			SetMenuSelection(entry);
			ShowCatalogue();
			m_mode = 1;
			return;
		}
		if (strcasecmp(entry, "Select Poly") == 0)
		{
			HideList(true);
			m_mode = 2;
			SetMenuSelection(entry);
			if (m_sel_element && m_sel_object) HighlightSelection(false);
			ClearSelection();
			return;
		}
		if (strcasecmp(entry, "Toggles") == 0)
		{
			HideList(true);
			SetMenuSelection(entry);
			m_mode = 4;
			for (int j = 0; j < 100; j++)
			{
				if (strcasecmp(Toggles()[j], "") == 0) return;
				char text[32];
				strcpy(text, Toggles()[j]);
				m_list_fields[j]->set_string(text);
				m_list_fields[j]->force_show();
			}
			return;
		}
		if (strcasecmp(entry, "Save Game") == 0)
		{
			if (!m_game_name[0]) { SaveGame("SavedGame.gam"); return; }
			Trace("SAVING...%s", m_game_name);
			SaveGame(m_game_name);
			char backup[128];
			strcpy(backup, m_game_name);
			MakeBackupName(backup);
			Trace("SAVING backup...%s", backup);
			SaveGame(backup);
			return;
		}
		if (strcasecmp(entry, "Load Game") == 0)
		{
			Trace("MEBTEMP Clicked Load Game 1");
			SetMenuSelection(entry);
			m_mode = 6;
			HideList(true);
			ShowPanel(true);
			Trace("MEBTEMP Clicked Load Game 2");
			for (int j = 0; j < LEVELS_MAX; j++)
			{
				char text[32];
				strcpy(text, Levels()[j].name);
				if (strcasecmp(text, "") == 0) break;
				m_list_fields[j]->set_string(text);
				m_list_fields[j]->force_show();
			}
			Trace("MEBTEMP Clicked Load Game 3");
			return;
		}
		if (strcasecmp(entry, "Delete Object") == 0)
		{
			if (m_sel_element && m_sel_object)
			{
				m_sel_object->f_006 = 1;
				HighlightSelection(false);
				ClearSelection();
				DeleteFlaggedObjects();
			}
			return;
		}
		if (strcasecmp(entry, "Databases") == 0)
		{
			HideList(true);
			ShowPanel(true);
			m_mode = 7;
			for (int j = 0; j < 100; j++)
			{
				const char *db = DatabaseName(j);			// 0x4a4a40
				if (!db) continue;
				char text[128];
				strcpy(text, db);
				if (strcasecmp(text, "") == 0) return;
				m_list_fields[j]->set_string(text);
				m_list_fields[j]->force_show();
			}
			return;
		}
		int filter = -1;
		if (strcasecmp(entry, "List All") == 0) filter = 0;
		else if (strcasecmp(entry, "List 3D") == 0) filter = 1;
		else if (strcasecmp(entry, "List CutScene") == 0) filter = 2;
		else if (strcasecmp(entry, "List Pickups") == 0) filter = 3;
		if (filter >= 0)
		{
			HideList(true);
			ShowPanel(true);
			SetMenuSelection(entry);
			if (m_sel_element && m_sel_object)
			{
				HighlightSelection(false);
				ClearSelection();
			}
			m_mode = 5;
			m_list_filter = filter;
			ShowObjectList();
			return;
		}
	}
}

// --------------------------------------------------------------------------- 0x4a9a50

void CEditor::ShowCatalogue(void)
{
	char text[128];
	strcpy(text, "");			// (the original leaves it unset when the list is empty)
	int i = 0;
	for (const NamedID *e = &Catalogue()[m_list_offset]; strcasecmp(e->name, "") != 0; e++)
	{
		strcpy(text, e->name);
		m_list_fields[i]->set_string(text);
		m_list_fields[i]->force_show();
		if (++i > 99) return;
	}
	for (; i < 100; i++)			// the rest: hidden, holding the last name
	{
		m_list_fields[i]->set_string(text);
		m_list_fields[i]->hide(true);
	}
}

// --------------------------------------------------------------------------- 0x4a9d20

void CEditor::ShowObjectList(void)
{
	char text[128];
	int n = 0, shown = 0;
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); it++)
	{
		CGameObject *o = *it;
		if (m_list_filter == 1 && !o->NameContains("C3DANIMATED")) continue;
		if (m_list_filter == 3 && !o->NameContains("C3DPICKUPITEM")) continue;
		if (m_list_filter == 2 && !o->NameContains("C3DTRIGGERTYPE") && !o->NameContains("C3DFLYINGCAMERA")) continue;
		if (shown > 99) return;
		if (!o || o->m_object_id == ID_TERRAIN || !o->f_00b) continue;

		// "<class>:<tag>" from the catalogue, else the bare 4cc
		const NamedID *c = NULL;
		for (const NamedID *e = Catalogue(); e->id != -1; e++)
			if (e->id == o->m_object_id) { c = e; break; }
		if (c)
		{
			sprintf(text, "%s:", c->name);
			strcat(text, o->m_tag);
		}
		else
		{
			char ids[5];
			IdString(ids, o->m_object_id);
			sprintf(text, "%s", ids);
		}
		if (m_list_offset < n)
		{
			m_list_fields[shown]->set_string(text);
			m_list_fields[shown]->force_show();
			shown++;
		}
		n++;
	}
	if (shown > 99) return;
	strcpy(text, "");
	for (int i = shown; i < 100; i++)
	{
		m_list_fields[i]->set_string(text);
		m_list_fields[i]->hide(true);
	}
}

// --------------------------------------------------------------------------- 0x4aa6c0

// A copy of src (same class, same properties) in front of the camera.
void CEditor::PasteObject(CGameObject *src)
{
	if (!src) return;
	OMediaElement *elem = NULL;
	CGameObject *o = neg::CreateObject(src->m_object_id);
	src->CopyPropertiesTo(o);
	o->v197(g_viewport->getx(), g_viewport->gety(), g_viewport->getz());
	o->v258();
	ObjectElement(src->m_object_id, &o, &elem);
	float p[4];
	CameraPoint(p, 0, 0, 500.0f);
	if (elem) elem->place(p[0], p[1], p[2]);
	o->v197(p[0], p[1], p[2]);
	o->v90(p[0], p[1], p[2]);
	HideList(true);
	SetMenuSelection("Select Object");
	m_mode = 0;
}
