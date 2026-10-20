/*  C2DInGameMenu part c: slots 304..331 (not 311).
    HUD bars (health, gadget charge), the menu pages (load / unload, rollovers, clicks), the main
    menu's item info, the save and options menus and the menu's sound channel.

    Constants: 0x4ca4bc = 1/640, 0x4ca4b8 = 1/480 (screen scale). Float to int conversions are
    _ftol (truncation). The menu's own state: f_4ce current menu (v301/v302), f_520 / f_524 /
    f_588 sound index / sound name / channel, f_590 menu to go back to (-1 = none), f_5a4..f_5a9
    "click handled" latches (cleared while the action button is up), f_5ac a timer.

    Menu numbers (MenuIndex): -1 NO_MENU, 0 INV_MENU, 1 PHONE_MENU, 2 PICKUPS_MENU, 3 MAIN_MENU,
    4 SAVE_MENU, 5 OPTIONS_MENU, 6 CREDITS_MENU.                                                */
#include "../DatabaseCache.h"
#include "../C2DInGameMenu.h"
#include "../CanvasWrapper.h"
#include "../Game.h"
#include "../Todo.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaInputEngine.h"
#include "OMediaSDLRetarget.h"				// omf_SDLResolvePath (DOS paths -> files)
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

typedef C2DInGameMenu::MenuPage		MenuPage;
typedef C2DInGameMenu::MenuItem		MenuItem;
typedef C2DInGameMenu::SaveSlotInfo	SaveSlotInfo;


enum { MENU_INV = 0, MENU_PHONE = 1, MENU_PICKUPS = 2, MENU_MAIN = 3, MENU_SAVE = 4,
	   MENU_OPTIONS = 5, MENU_CREDITS = 6 };

// --------------------------------------------------------------------------- helpers

static inline float screen_sx(void) { return (float)(short)g_screen_w * (1.0f / 640.0f); }	// 0x4ca4bc
static inline float screen_sy(void) { return (float)(short)g_screen_h * (1.0f / 480.0f); }	// 0x4ca4b8

// Slot 290 (0x404570, another part): place sprite e at (x, y) in 640x480 space with canvas
// canvas_id of screens.omt (scaled), z = 0; the 1.0 argument is not used by it. Its generated
// signature still has integer types - adapt this one call when that part declares the real one
// (OMediaCanvasElementWrapper *, float x, float y, int id, float, bool scaled, float z).
static void PlaceSprite(C2DInGameMenu *m, OMediaCanvasElementWrapper *e, float x, float y, int canvas_id)
{
	m->SetupSprite(e, x, y, canvas_id, 1.0f, true, 0.0f);
}

// The element's "show, whatever the hide count" idiom: if (hide_count) { hide_count = 1; hide(false); }
static inline void ForceShow(OMediaCanvasElement *e) { e->force_show(); }

// 0x404dd0: set the type of item `item` of menu `menu`. The pickups come in pairs (10/21 and
// 31/38): giving one of a pair also gives the other one, if not owned yet, and counts it.
static void SetItemState(short menu, short item, short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	MenuPage *page = g_menu_pages[menu];
	if (page) page->items[item].type = state;
	if (menu != MENU_PICKUPS) return;
	MenuPage *p = g_menu_pages[MENU_PICKUPS];
	if (item == 10)
	{
		if (p && p->items[21].type) return;
		SetItemState(2, 21, 1);
		g_pickup_counts[21 * 2]++;
	}
	else if (item == 21)
	{
		if (p && p->items[10].type) return;
		SetItemState(2, 10, 1);
		g_pickup_counts[10 * 2]++;
	}
	else if (item == 38)
	{
		if (p && p->items[31].type) return;
		SetItemState(2, 31, 1);
		g_pickup_counts[31 * 2]++;
	}
	else if (item == 31)
	{
		if (p && p->items[38].type) return;
		SetItemState(2, 38, 1);
		g_pickup_counts[38 * 2]++;
	}
}

// Name of a level (save menu), by its 4cc. NULL for an unknown one.
static const char *LevelName(int level)
{
	switch (level)
	{
		case 'L1_A': return "winter";
		case 'L1_B': return "halloween";
		case 'L1_C': return "independence";
		case 'LEV3': return "negatron lab";
		case 'LEV5': return "museum";
		case 'L5B1': return "tomb b";
		case 'LV1C': return "house";
		case 'LV3A': return "underwater";
		case 'LV3C': return "ravine";
		case 'LV3D': return "wharf";
		case 'LV3E': return "negatron lab b";
		case 'LV4B': return "factory";
		case 'LV5A': return "pyramid";
		case 'LV5B': return "tomb a";
		case 'LV5C': return "king carl";
		case 'LV5D': return "museum";
		case 'LV5F': return "dinosaurs";
		case 'VR01': return "VR VOX";
	}
	return NULL;
}

// Save slots: save-menu items 6..15 are save\JimmyGame1..10.tsk.
static const char *const kSaveFiles[10] = {
	"save\\JimmyGame1.tsk", "save\\JimmyGame2.tsk", "save\\JimmyGame3.tsk", "save\\JimmyGame4.tsk",
	"save\\JimmyGame5.tsk", "save\\JimmyGame6.tsk", "save\\JimmyGame7.tsk", "save\\JimmyGame8.tsk",
	"save\\JimmyGame9.tsk", "save\\JimmyGame10.tsk" };
static const char *const kSaveNames[10] = {
	"JimmyGame1.tsk", "JimmyGame2.tsk", "JimmyGame3.tsk", "JimmyGame4.tsk", "JimmyGame5.tsk",
	"JimmyGame6.tsk", "JimmyGame7.tsk", "JimmyGame8.tsk", "JimmyGame9.tsk", "JimmyGame10.tsk" };

// KERNEL32 DeleteFileA / CopyFileA(src, dst, FALSE) on the game's DOS-style relative paths.
static void DeleteFileDos(const char *path)
{
	remove(omf_SDLResolvePath(path).c_str());
}

static void CopyFileDos(const char *src, const char *dst)
{
	FILE *in = fopen(omf_SDLResolvePath(src).c_str(), "rb");
	if (!in) return;
	FILE *out = fopen(omf_SDLResolvePath(dst).c_str(), "wb");
	if (out)
	{
		char buf[4096];
		size_t n;
		while ((n = fread(buf, 1, sizeof buf, in)) > 0) fwrite(buf, 1, n, out);
		fclose(out);
	}
	fclose(in);
}

// ---- other systems (not reconstructed yet)

static void StopChannel(int channel, bool release)						// 0x4b88e0
{
	NEG_TODO(0x4b88e0, "StopChannel"); (void)channel; (void)release;
}

static int PlaySoundIndex(int channel, int index, int a)				// 0x4b8240
{
	NEG_TODO(0x4b8240, "PlaySound (by index)"); (void)channel; (void)index; (void)a;
	return -1;
}

static int PlaySoundName(int channel, std::string name, int a)			// 0x4b8380
{
	NEG_TODO(0x4b8380, "PlaySound (by name)"); (void)channel; (void)name; (void)a;
	return -1;
}

static void PlaySfx(int channel, std::string name, int a)				// 0x48f420 (plays if 0x54a4b8 and the sfx volume)
{
	NEG_TODO(0x48f420, "PlaySfx"); (void)channel; (void)name; (void)a;
}

static void *LoadMusic(const char *omt_name)							// 0x4a4180 GetDatabase: the music lives in its own .omt
{
	return neg::GetDatabase(omt_name);
}

static void PlayMusic(void *music, int channel, int a, bool b)			// 0x4b8850
{
	NEG_TODO(0x4b8850, "PlayMusic"); (void)music; (void)channel; (void)a; (void)b;
}

static void ShowMouseCursor(bool show)									// 0x4a38c0 (ShowCursor + flags 0x56aec9..)
{
	NEG_TODO(0x4a38c0, "ShowMouseCursor"); (void)show;
}

static void GameTypeSetMenuMode(bool on)								// CGameType vtable +0x110
{
	NEG_TODO(0x110, "CGameType vtable +0x110 (menu on/off)"); (void)on;
}

static bool ReadSaveHeader(const char *file)							// 0x497b10: fills 0x54e6a4.. 0x54e820
{
	NEG_TODO(0x497b10, "ReadSaveHeader"); (void)file;
	return false;
}

static void LoadGame(const char *file, bool b)							// 0x498af0
{
	NEG_TODO(0x498af0, "LoadGame"); (void)file; (void)b;
}

static void SetGameState(int state)										// 0x485360
{
	NEG_TODO(0x485360, "SetGameState"); (void)state;
}

// 0x4b1d10: the mouse position, window pixels (the original asks the object at 0x56afe4 for its
// input engine, vtable +0x38, then +0x20 for the position).
static void GetMousePos(short &x, short &y)
{
	x = y = 0;
	if (g_input_engine) g_input_engine->get_mouse_position(x, y);
}

// The player's "action pressed" test: the object at 0x545fd0 (a CGameObject subobject at +0xc0)
// called through its element vtable +0x1a0 - C3DPlayerObject 0x465650 for every player class.
// That class is not reconstructed yet: its body is copied here.
static bool PlayerActionPressed(void)
{
	NEG_TODO(0x465650, "C3DPlayerObject +0x1a0 (action pressed), copied");
	if (!g_545fd0) return false;		// sic: the original calls through NULL here
	if (g_5461ce) return false;
	return g_56ad9c || g_keys[0x0d] || g_keys[0x11] || g_56b05c;	// mouse, VK_RETURN, VK_CONTROL, joystick
}

// The two sprites of a main-menu item's info: its wire frame fades in, then its picture, then
// the wire fades out again (inlined in every case of 0x405e30).
static void QueueItemInfo(InterfaceElt *wire, InterfaceElt *pic)
{
	wire->QueueWait(0, 1.9f);
	wire->QueueFade(0, 0.1f, true, true, 0.0f, 0.6f);
	pic->QueueWait(0, 4.0f);
	pic->QueueFade(0, 0.5f, true, true, 0.0f, 1.0f);
	wire->QueueWait(0, 2.5f);
	wire->QueueFade(0, 0.5f, false, true, 0.0f, 0.6f);
}

// The selected save slot has a game in it (g_save_slot 6..15).
static inline bool SlotUsed(void) { return g_save_slots[g_save_slot - 6].score != -1; }

// Opens one of the menus' confirmation boxes (inlined in 0x40efa0 and 0x40faf0).
static bool OpenDialog(OMediaCanvasElementWrapper *box, int which)
{
	if (g_menu_dialog_open) return false;
	g_menu_dialog_open = true;
	GameTypeSetMenuMode(true);
	g_5446ec->hide(true);
	ShowMouseCursor(true);
	ForceShow(box);
	g_menu_dialog = which;
	return true;
}

// --------------------------------------------------------------------------- HUD bars

void C2DInGameMenu::ArrangePickups(void)								// 0x408ba0
{
	// The owned pickups of the pickups menu in a grid of 90 pixel cells, each with its count.
	// Quirk kept: the item's x/y get the cell *before* the step, the sprite is placed at the
	// cell after it.
	short x = 55, y = 39;
	MenuPage *page = g_menu_pages[MENU_PICKUPS];
	for (short i = 0; i < MENU_ITEMS; i++)
	{
		if (!page || i < 0 || i >= MENU_ITEMS) continue;
		if (page->items[i].type == 0 || !page->items[i].active) continue;
		page->items[i].x = (float)x;
		x += 90;
		g_menu_pages[MENU_PICKUPS]->items[i].y = (float)y;
		if (x > 505)
		{
			x = 55;
			y += 90;
		}
		MenuItem &it = g_menu_pages[MENU_PICKUPS]->items[i];
		PlaceSprite(this, it.active, (float)x, (float)y, it.active_id);
		if (g_pickup_counts[i * 2]) DrawText(x, y, 'MED', "%d", g_pickup_counts[i * 2]);
		page = g_menu_pages[MENU_PICKUPS];
	}
}

void C2DInGameMenu::ShowGadgetBar(void)									// 0x404bd0
{
	if (g_gadget_bar_frame) ForceShow(g_gadget_bar_frame);
	if (g_gadget_bar)
	{
		ForceShow(g_gadget_bar);
		g_gadget_bar->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 0.1f, 0.1f));	// red
	}
	g_gadget_bar_on = 1;
}

void C2DInGameMenu::HideGadgetBar(void)									// 0x404c70
{
	if (g_gadget_bar_frame) g_gadget_bar_frame->hide(true);
	if (g_gadget_bar) g_gadget_bar->hide(true);
	g_gadget_bar_on = 0;
}

void C2DInGameMenu::GadgetBarEmpty(void)								// 0x404ca0
{
	if (!g_gadget_bar) return;
	g_gadget_bar->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 0.2f, 0.2f));
	ForceShow(g_gadget_bar);
}

void C2DInGameMenu::GadgetBarFull(void)									// 0x404d10
{
	if (g_gadget_bar) g_gadget_bar->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
}

unsigned char C2DInGameMenu::IsGadgetBarShown(void)						// 0x403f40
{
	return g_gadget_bar_on;
}

void C2DInGameMenu::SetHealthBar(float health)							// 0x404f50
{
	// Green from 70 up, yellow from 30, red below; 20 wide, health * 1.044 high (640x480 units).
	if (!g_health_bar) return;
	float r = 1.0f, g = 1.0f;
	float sy = screen_sy();
	if (!(health < 70.0f)) r = 0;
	else if (health < 30.0f) g = 0;
	g_health_bar->set_diffuse(OMediaFARGBColor(1.0f, r, g, 0.0f));
	g_health_bar->set_size(screen_sx() * 20.0f, (float)((double)(sy * health) * 1.044));
}

unsigned char C2DInGameMenu::IsCurrentGadget(int gadget)				// 0x403f50
{
	return g_cur_gadget == gadget;
}

int C2DInGameMenu::GetCurrentGadget(void)								// 0x403f70
{
	return g_cur_gadget;
}

// --------------------------------------------------------------------------- HUD sprites

void C2DInGameMenu::v316(bool show)										// 0x409fb0
{
	// The sprite at 0x5444d0 (canvas 0x67, at 260,1).
	if (show) ForceShow(g_5444d0);
	else g_5444d0->hide(true);
}

// The picture sprite at 0x545d84 is deleted with its canvas (inlined twice in 0x409e20).
static void DeleteHudPicture(void)
{
	if (OMediaCanvas *c = g_hud_picture->get_canvas())
	{
		delete c;						// through its OMediaEngineImpMaster base (+0x24), as shipped
		g_hud_picture->reset();
	}
	delete g_hud_picture;
	g_hud_picture = NULL;
}

void C2DInGameMenu::ShowHudPicture(int canvas_id, bool show, int x, int y)	// 0x409e20
{
	if (show)
	{
		if (g_hud_picture) DeleteHudPicture();
		g_hud_picture_id = canvas_id;
		g_hud_picture = new OMediaCanvasElementWrapper;
		g_hud_picture->link_layer(g_layer_hud2d);
		PlaceSprite(this, g_hud_picture, (float)x, (float)y, g_hud_picture_id);
		ForceShow(g_hud_picture);
	}
	else
	{
		if (g_hud_picture)
		{
			g_hud_picture->hide(true);
			DeleteHudPicture();
		}
		g_hud_picture_id = -1;
	}
}

void C2DInGameMenu::v327(bool show)										// 0x404ba0
{
	// The interface element 0x545b78: shown in fullscreen only.
	if (!show) g_elt_545b78.Hide();
	else if (!g_windowed) g_elt_545b78.Show();
}

// --------------------------------------------------------------------------- menu pages

int C2DInGameMenu::MenuIndex(const char *name)							// 0x40e600
{
	if (!strcasecmp(name, "NO_MENU")) return -1;
	if (!strcasecmp(name, "INV_MENU")) return MENU_INV;
	if (!strcasecmp(name, "PHONE_MENU")) return MENU_PHONE;
	if (!strcasecmp(name, "PICKUPS_MENU")) return MENU_PICKUPS;
	if (!strcasecmp(name, "MAIN_MENU")) return MENU_MAIN;
	if (!strcasecmp(name, "SAVE_MENU")) return MENU_SAVE;
	if (!strcasecmp(name, "OPTIONS_MENU")) return MENU_OPTIONS;
	if (!strcasecmp(name, "CREDITS_MENU")) return MENU_CREDITS;
	return -1;
}

void C2DInGameMenu::LoadMyMenu(int menu)								// 0x4047a0
{
	// Creates the sprites of a page that are not there yet: background (fullscreen: one pixel
	// higher), then each item's picture and rollover. An item of type 0 or 4 starts hidden.
	MenuPage *page = g_menu_pages[menu];
	// ("LoadMyMenu: CurMenu = %d": debug text, empty in the shipped build)
	if (!page) return;
	if (!page->background)
	{
		page->background = new OMediaCanvasElementWrapper;
		if (page->background) page->background->link_layer(g_layer_hud2d);
		PlaceSprite(this, page->background, 0.0f, g_windowed ? 0.0f : -1.0f, page->background_id);
	}
	for (int i = 0; i < MENU_ITEMS; i++)
	{
		MenuItem &it = page->items[i];
		if (!it.active && it.active_id != -1)
		{
			it.active = new OMediaCanvasElementWrapper;
			if (it.active)
			{
				it.active->link_layer(g_layer_hud2d);
				PlaceSprite(this, it.active, (float)(short)(int)it.x, (float)(short)(int)it.y, it.active_id);
				it.w = (short)(int)it.active->get_width();
				it.h = (short)(int)it.active->get_height();
				switch (it.type)
				{
					case 0: case 4: it.active->hide(true); break;
					case 1: ForceShow(it.active); break;
				}
			}
		}
		if (!it.rollover && it.rollover_id != -1)
		{
			it.rollover = new OMediaCanvasElementWrapper;
			if (it.rollover)
			{
				it.rollover->link_layer(g_layer_hud2d);
				PlaceSprite(this, it.rollover, (float)(short)(int)it.x, (float)(short)(int)it.y, it.rollover_id);
				it.w = (short)(int)it.rollover->get_width();
				it.h = (short)(int)it.rollover->get_height();
				switch (it.type)
				{
					case 0: case 4: it.rollover->hide(true); it.shown = 0; break;
					case 1: ForceShow(it.rollover); it.shown = 1; break;
				}
			}
		}
	}
}

void C2DInGameMenu::UnloadMyMenu(int menu)								// 0x404aa0
{
	MenuPage *page = g_menu_pages[menu];
	// (debug texts "Enter UnloadMyMenu:%d", "Not a pickups_menu, deallocating all",
	//  "Deallocaing ActiveItem %d", "Deallocaing Rollover %d", "Leave UnloadMyMenu": empty)
	if (!page) return;
	if (page->background)
	{
		page->background->hide(true);
		delete page->background;
		page->background = NULL;
	}
	for (int i = 0; i < MENU_ITEMS; i++)
	{
		MenuItem &it = page->items[i];
		if (it.active)
		{
			delete it.active;
			it.active = NULL;
		}
		if (it.rollover)
		{
			delete it.rollover;
			it.rollover = NULL;
		}
	}
	delete page;
	// sic: g_menu_pages[menu] keeps pointing at the deleted page.
}

short C2DInGameMenu::UpdateRollovers(short mx, short my)				// 0x405030
{
	// Shows / hides the rollovers of the current menu for the mouse at (mx, my) (window pixels)
	// and returns the last item under it, -1 for none.
	short over = -1;
	short cur = v301();
	if (cur == -1) return cur;
	for (short i = 0; i < MENU_ITEMS; i++)
	{
		MenuItem &it = g_menu_pages[f_4ce]->items[i];
		if (it.type == 0 || it.type == 4 || !it.rollover) continue;
		OMediaCanvasElementWrapper *r = it.rollover;
		short left = (short)(int)((float)((short)g_screen_w / 2) + r->get_x());
		short top = (short)(int)-((r->get_y() + r->get_height()) - (float)((short)g_screen_h / 2));
		bool inside = mx > left && (double)r->get_width() + left > (double)mx
				   && my > top && (double)r->get_height() + top > (double)my;
		if (inside)
		{
			if (it.type == 3)
			{
				r->hide(true);
				it.shown = 0;
			}
			else
			{
				ForceShow(r);
				it.shown = 1;
			}
			over = i;
		}
		else if (it.type == 2)
		{
			ForceShow(r);
			it.shown = 1;
		}
		else
		{
			r->hide(true);
			it.shown = 0;
		}
	}
	return over;
}

void C2DInGameMenu::DeleteAllMenuObject(void)							// 0x404400
{
	Log("DeleteAllMenuObject()");
	while (s_objects.size() != 0)
	{
		MenuObject *e = s_objects.front();
		s_objects.remove(e);
		delete e;
	}
}

// --------------------------------------------------------------------------- main menu

void C2DInGameMenu::ShowMainMenuInfo(int item)							// 0x405e30
{
	// Main menu, mouse over item `item`: shows its text (the TRIGTEXTSTRING lines of that name)
	// and its picture with a wire frame; VR TANK also sweeps the line glow across.
	InterfaceElt *cindy_wire = g_interface.Find("CindyWire");
	InterfaceElt *cindy = g_interface.Find("Cindy");
	InterfaceElt *carl_wire = g_interface.Find("CarlWire");
	InterfaceElt *carl = g_interface.Find("Carl");
	InterfaceElt *resume_wire = g_interface.Find("ResumeWire");
	InterfaceElt *resume = g_interface.Find("Resume");
	InterfaceElt *exit = g_interface.Find("Exit");
	InterfaceElt *exit_wire = g_interface.Find("ExitWire");
	InterfaceElt *options = g_interface.Find("Options");
	InterfaceElt *options_wire = g_interface.Find("OptionsWire");
	InterfaceElt *pickups = g_interface.Find("Pickups");
	InterfaceElt *pickups_wire = g_interface.Find("PickupsWire");
	InterfaceElt *credits = g_interface.Find("Credits");
	InterfaceElt *credits_wire = g_interface.Find("CreditsWire");
	InterfaceElt *line_glow = g_interface.Find("LineGlow");
	InterfaceElt *ls = g_interface.Find("LS");
	InterfaceElt *ls_wire = g_interface.Find("LSWire");
	InterfaceElt *johnny_wire = g_interface.Find("JohnnyWire");
	InterfaceElt *johnny = g_interface.Find("Johnny");
	InterfaceElt *fowl_wire = g_interface.Find("FowlWire");
	InterfaceElt *fowl = g_interface.Find("Fowl");
	InterfaceElt *momdad_wire = g_interface.Find("MomDadWire");
	InterfaceElt *momdad = g_interface.Find("MomDad");
	InterfaceElt *dino_wire = g_interface.Find("DinoWire");
	InterfaceElt *dino = g_interface.Find("Dino");

	if (f_4ce != MENU_MAIN) return;
	if (g_main_menu_over == item) return;
	g_main_menu_over = item;
	ResetInterface();

	const char *text;
	InterfaceElt *wire, *pic;
	switch (item)
	{
		case 1:					text = "EXIT2";		wire = exit_wire;		pic = exit;		break;
		case 2:					text = "RETURN2";	wire = resume_wire;		pic = resume;	break;
		case 3:					text = "LOADSAV2";	wire = ls_wire;			pic = ls;		break;
		case 4:					text = "OPTION2";	wire = options_wire;	pic = options;	break;
		case 5: case 34:		text = "CARL2";		wire = carl_wire;		pic = carl;		break;
		case 6:					text = "CREDIT2";	wire = credits_wire;	pic = credits;	break;
		case 7:					text = "MUSEUM2";	wire = cindy_wire;		pic = cindy;	break;
		case 13:				text = "PICKUP2";	wire = pickups_wire;	pic = pickups;	break;
		case 15: case 28:		text = "PYRAMID";	wire = fowl_wire;		pic = fowl;		break;
		case 16: case 33:		text = "VRTANK2";	wire = cindy_wire;		pic = cindy;	break;
		case 18: case 35:		text = "TOMB";		wire = momdad_wire;		pic = momdad;	break;
		case 19: case 31:		text = "DINOS";		wire = dino_wire;		pic = dino;		break;
		case 22: case 25:		text = "HALLOWEEN";	wire = johnny_wire;		pic = johnny;	break;
		case 23: case 26:		text = "WINTER";	wire = johnny_wire;		pic = johnny;	break;
		case 24: case 27:		text = "ID4";		wire = johnny_wire;		pic = johnny;	break;
		case 29:				text = "JIMLAB";	wire = johnny_wire;		pic = johnny;	break;
		default:				return;
	}
	strcpy((char *)g_text_trigger, text);
	g_5324f4 = 40.0f;
	g_545e4c = 0;
	g_545e48 = 0;
	PlayPdaIntro();
	QueueItemInfo(wire, pic);
	if (item == 16 || item == 33)
	{
		line_glow->QueueWait(0, 1.0f);
		line_glow->QueueMove(0, 1.0f, true, true, true, 1.0f, 0.0f, 300.0f);
		line_glow->QueueMove(0, 1.0f, true, true, true, 1.0f, 0.0f, 300.0f);
		line_glow->QueueMove(0, 1.0f, true, true, true, 1.0f, 0.0f, 300.0f);
		line_glow->QueueMove(0, 1.0f, true, true, false, 1.0f, 0.0f, 300.0f);
	}
}

void C2DInGameMenu::MainMenuAction(short item)							// 0x40faf0
{
	switch (item)
	{
		case 1:			// EXIT: quit confirmation
			ResetInterface();
			DeactivateActiveMenu();
			OpenDialog(g_dlg_quit, 7);
			break;
		case 2:			// RETURN
			DeactivateActiveMenu();
			GameTypeSetMenuMode(false);
			break;
		case 3:			// LOAD / SAVE: read the header of each save slot
		{
			ActivateActiveMenu(4);
			char keep1[120], keep2[120];
			strcpy(keep1, (char *)g_54e6a4);
			strcpy(keep2, (char *)g_54e71c);
			SaveSlotInfo *s = g_save_slots;
			for (int k = 1; k <= 10; k++, s++)
			{
				char name[80];
				sprintf(name, "JimmyGame%d.tsk", k);
				if (ReadSaveHeader(name))
				{
					s->score = (int)g_54e81c;
					s->f_10 = (int)g_54e820;
					s->level = g_54e810;
				}
				else s->score = -1;
			}
			strcpy((char *)g_54e6a4, keep1);
			strcpy((char *)g_54e71c, keep2);
			break;
		}
		case 4:			// OPTIONS
			ActivateActiveMenu(5);
			break;
		case 6:			// CREDITS
			ActivateActiveMenu(6);
			break;
		case 12:
			ActivateActiveMenu(0);
			break;
		case 13:		// PICKUPS
			ActivateActiveMenu(2);
			break;
		default:		// a level
			v342(item);
			GameTypeSetMenuMode(false);
			break;
	}
	ResetInterface();
}

// --------------------------------------------------------------------------- clicks

unsigned char C2DInGameMenu::HandleMenuInput(float dt)					// 0x40efa0
{
	// Per frame while a menu is up: draws the save slots / option values, updates the rollovers
	// under the mouse, and acts on a click. Each menu acts once per press (the f_5a4.. latches).
	(void)dt;
	int difficulty = g_difficulty;

	if (v301() == MENU_SAVE)
	{
		SetItemState(MENU_SAVE, (short)g_save_slot, 2);
		char name[80] = "";				// sic: an unknown level shows the previous slot's name
		short y = 121;
		for (int k = 0; k < 10; k++, y += 23)
		{
			SaveSlotInfo &s = g_save_slots[k];
			if (s.score == -1) continue;
			if (const char *n = LevelName(s.level)) strcpy(name, n);
			g_text_rgb[0] = g_text_rgb[1] = g_text_rgb[2] = 0;	// shadow
			DrawText(239, (short)(y + 1), 'SMAL', "%s", name);
			DrawText(470, (short)(y + 1), 'SMAL', "%6d", s.score);
			g_text_rgb[0] = g_text_rgb[1] = g_text_rgb[2] = 1.0f;
			DrawText(240, y, 'SMAL', "%s", name);
			DrawText(470, y, 'SMAL', "%6d", s.score);
		}
	}
	if (v301() == MENU_OPTIONS)
	{
		DrawText(g_music_volume < 10 ? 160 : 145, 200, 'BIG', "%d", g_music_volume);
		DrawText(g_sfx_volume < 10 ? 429 : 414, 200, 'BIG', "%d", g_sfx_volume);
		switch (g_difficulty)
		{
			case 0: DrawText(270, 200, 'MED', "EASY"); break;
			case 1: DrawText(270, 200, 'MED', "NORMAL"); break;
			case 2: DrawText(270, 200, 'MED', "HARD"); break;
		}
	}

	short mx, my;
	GetMousePos(mx, my);
	short over = UpdateRollovers(mx, my);
	// (0x463890 "OverItemIndex: %d" at 70,400 - empty in the shipped build)
	if (over == -1)
	{
		if (v301() == MENU_MAIN || v301() == MENU_PHONE) PlayMenuSoundIndex(-1);
		return 1;
	}
	if (v301() == MENU_MAIN)
	{
		ShowMainMenuInfo(over);
		switch (over)
		{
			case 1: PlayMenuSound("jimmyself22"); break;
			case 2: case 3: case 4: case 6: case 11: case 12: case 13: break;
			default: PlayMenuSoundIndex(-1); break;
		}
	}
	else v301();						// (result unused)

	if (!PlayerActionPressed())
	{
		f_5a4 = 0;
		f_5a6 = 0;
		f_5a5 = 0;
		return 1;
	}
	if (over == 0) return 1;
	f_5a9 = 0;
	switch (v301())
	{
		case MENU_INV:
			if (f_5a5) break;
			f_5a5 = 1;
			v132(1);
			if (f_590 == -1) break;
			ActivateActiveMenu(3);
			f_590 = 3;
			break;

		case MENU_PHONE:
			if (f_5a5) break;
			f_5a5 = 1;
			v132(1);
			DeactivateActiveMenu();
			v342(over);
			GameTypeSetMenuMode(false);
			break;

		case MENU_PICKUPS:
		case MENU_CREDITS:
			if (f_5a5) break;
			f_5a5 = 1;
			v132(1);
			DeactivateActiveMenu();
			ActivateActiveMenu(3);
			f_590 = 3;
			break;

		case MENU_MAIN:
			if (f_5a9) break;			// (always clear: reset just above)
			f_5a9 = 1;
			v132(1);
			DeactivateActiveMenu();
			MainMenuAction(over);
			break;

		case MENU_SAVE:
		{
			if (f_5a6) break;
			v132(1);
			switch (over)
			{
				case 1:					// back
					f_5a6 = 1;
					DeactivateActiveMenu();
					if (f_590 != -1) ActivateActiveMenu(3);
					else GameTypeSetMenuMode(false);
					break;
				case 2:
					f_5a6 = 1;
					DeactivateActiveMenu();
					OpenDialog(g_dlg_11, 11);
					break;
				case 3:
					f_5a6 = 1;
					if (!SlotUsed()) break;
					DeactivateActiveMenu();
					OpenDialog(g_dlg_9, 9);
					break;
				case 4:					// save: over a used slot asks first
					f_5a6 = 1;
					if (SlotUsed())
					{
						DeactivateActiveMenu();
						OpenDialog(g_dlg_8, 8);
						break;
					}
					DeactivateActiveMenu();
					SaveToSlot();
					PlayMenuSound("pickup2");
					GameTypeSetMenuMode(false);
					break;
				case 5:
					f_5a6 = 1;
					if (!SlotUsed()) break;
					DeactivateActiveMenu();
					OpenDialog(g_dlg_13, 13);
					PlayMenuSound("toilet1");
					break;
				default:				// a slot: select it
					f_5a6 = 1;
					SetItemState(MENU_SAVE, (short)g_save_slot, 1);
					g_save_slot = over;
					SetItemState(MENU_SAVE, over, 2);
					break;
			}
			break;
		}

		case MENU_OPTIONS:
			if (f_5a4) break;
			switch (over)
			{
				case 1:					// music louder
					f_5a4 = 1;
					if (++g_music_volume >= 10) g_music_volume = 10;
					g_6f7518 = g_music_volume;
					g_6f759c = LoadMusic("musiclogo.omt");
					if (g_6f759c) PlayMusic(g_6f759c, -1, 0, true);
					break;
				case 2:					// music softer
					f_5a4 = 1;
					if (--g_music_volume <= 0) g_music_volume = 0;
					g_6f7518 = g_music_volume;
					g_6f759c = LoadMusic("musiclogo.omt");
					if (g_6f759c) PlayMusic(g_6f759c, -1, 0, true);
					break;
				case 3:					// back
					f_5a4 = 1;
					v132(1);
					DeactivateActiveMenu();
					ActivateActiveMenu(3);
					f_590 = 3;
					break;
				case 4:					// sound louder
					f_5a4 = 1;
					if (++g_sfx_volume >= 10) g_sfx_volume = 10;
					g_54a4b8 = true;
					PlaySfx(-1, "bubble boing", 0);
					g_54a4b8 = false;
					break;
				case 5:					// sound softer
					f_5a4 = 1;
					if (--g_sfx_volume <= 0) g_sfx_volume = 0;
					g_54a4b8 = true;
					PlaySfx(-1, "bubble boing", 0);
					g_54a4b8 = false;
					break;
				case 6:					// harder
					f_5a4 = 1;
					g_difficulty = difficulty + 1 > 2 ? 2 : difficulty + 1;
					break;
				case 7:					// easier
					f_5a4 = 1;
					g_difficulty = difficulty - 1 < 0 ? 0 : difficulty - 1;
					break;
			}
			SaveOptions("dat\\options.opt");
			break;
	}
	return 1;
}

// --------------------------------------------------------------------------- save menu

void C2DInGameMenu::LoadSaveSlot(void)									// 0x40e900
{
	if (g_save_slot >= 6 && g_save_slot <= 15) LoadGame(kSaveNames[g_save_slot - 6], false);
}

void C2DInGameMenu::SaveToSlot(void)									// 0x40e7a0
{
	// The game is saved by copying the autosave into the selected slot.
	char src[52];
	strcpy(src, "save\\");
	strcat(src, "autosave.tsk");
	if (g_save_slot >= 6 && g_save_slot <= 15) CopyFileDos(src, kSaveFiles[g_save_slot - 6]);
	f_5ac = 1.0f;
	ForceShow(g_game_saved_box);
	SetGameState(5);
}

void C2DInGameMenu::DeleteSaveSlot(void)								// 0x40e6e0
{
	if (g_save_slot >= 6 && g_save_slot <= 15) DeleteFileDos(kSaveFiles[g_save_slot - 6]);
}

// --------------------------------------------------------------------------- options file

void C2DInGameMenu::LoadOptions(const char *path)						// 0x40eb70
{
	// dat\options.opt: 0xababab, difficulty, music volume, sound volume (OMT stream longs).
	char full[120];
	strcpy(full, (char *)g_empty_string);
	strcat(full, path);
	OMediaFilePath fp((std::string(full)));
	OMediaFileStream fs;
	fs.setpath(&fp);
	if (!fs.fileexists())
	{
		// ("WARNING: no options file": debug text, empty in the shipped build)
		fs.close();
		return;
	}
	fs.open(omcfp_Read, false, false);
	if (!fs.isopen())
	{
		ErrorLog("ERROR opening options file ");
		return;
	}
	int magic = 0, v;
	fs >> magic;
	if (magic == 0xababab)
	{
		fs >> v;
		g_difficulty = v;
		fs >> v;
		g_music_volume = v;
		g_6f7518 = v;
		fs >> v;
		g_sfx_volume = v;
	}
	fs.close();
}

void C2DInGameMenu::SaveOptions(const char *path)						// 0x40e9e0
{
	char full[120];
	strcpy(full, (char *)g_empty_string);
	strcat(full, path);
	OMediaFilePath fp((std::string(full)));
	OMediaFileStream fs;
	fs.setpath(&fp);
	fs.open(omcfp_Write, true, true);
	int magic = 0xababab, difficulty = g_difficulty, v = g_6f7518;
	fs << magic;
	fs << difficulty;
	// ("MEBTEMP SaveOptions %d": debug text, empty in the shipped build)
	fs << v;
	v = g_sfx_volume;
	fs << v;
	fs.close();
}

// --------------------------------------------------------------------------- menu sound channel

void C2DInGameMenu::PlayMenuSound(const char *name)					// 0x40b370
{
	// One sound at a time on the menu's channel f_588; "" stops it. Playing the same name again
	// while it is the current one does nothing.
	if (!strcasecmp(name, (char *)g_empty_string))
	{
		strcpy(f_524, (char *)g_empty_string);
		StopChannel(f_588, false);
		f_588 = -1;
	}
	if (strcasecmp(f_524, name))
	{
		if (IsChannelPlaying(f_588)) StopChannel(f_588, false);
		f_588 = PlaySoundName(-1, name, 0);
		strcpy(f_524, name);
	}
}

void C2DInGameMenu::PlayMenuSoundIndex(int index)						// 0x40b2c0
{
	// As PlayMenuSound, by sound index (f_520); -1 stops it.
	if (index == -1)
	{
		f_520 = -1;
		StopChannel(f_588, false);
		f_588 = -1;
		strcpy(f_524, (char *)g_empty_string);
	}
	if (f_520 != index)
	{
		if (IsChannelPlaying(f_588)) StopChannel(f_588, false);
		f_588 = PlaySoundIndex(-1, index, 0);
		f_520 = index;
	}
}
