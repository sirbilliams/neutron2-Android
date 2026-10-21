/*  C2DInGameMenu part b: the per-frame update (Process) and its helpers, game-mode changes (v270),
    the menu pages and their items, the sprite helpers, help / yes-no dialog / pickup pop-up / score
    screen handling.  Reconstructed from Neutron2.exe; each function names its original address.

    Sprites are OMediaCanvasElementWrapper (0xd8 bytes in 2002) on the HUD 2D layer, positioned in
    the game's 640x480 space (y down) by PositionSprite / SetupSprite. "force_show" is the original's
    inline `if (hide_count) { hide_count = 1; hide(false); }`.

    Game modes (g_game_mode 0x53b5b4, changed by SetGameMode 0x485360, which calls slot 270 of
    every game object): 0 Starting, 1 Loading, 2 Saving, 3 In CutScene, 4 In Menu, 5 Playing,
    6 Paused, 9 None.                                                                           */
#include "../C2DInGameMenu.h"
#include "../Mouse.h"
#include "../CanvasWrapper.h"
#include "../Game.h"
#include "../SaveGame.h"
#include "../Todo.h"
#include "../CGameType.h"
#include "../Sound.h"
#include "../CEditor.h"
#include "../C3DJimmy.h"
#include "../C3DRadar.h"
#include "../port/Widescreen.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasElement.h"
#include <SDL.h>
#include <math.h>
#include <string.h>

using namespace neg;

typedef C2DInGameMenu::MenuPage MenuPage;
typedef C2DInGameMenu::MenuItem MenuItem;

// Windows virtual-key codes read from g_keys
enum { VK_ESC = 0x1b, VK_F1_ = 0x70 };

// --------------------------------------------------------------------------- small helpers

static float ScreenSX(void) { return (float)(short)g_screen_w * 0.0015625f; }		// 1/640
static float ScreenSY(void) { return (float)(short)g_screen_h * 0.0020833334f; }	// 1/480

// Float argument passed to a slot whose generated signature still says `unsigned int` (the
// original pushes the float's bits).
static unsigned int FloatBits(float f) { unsigned int u; memcpy(&u, &f, sizeof(u)); return u; }

// --------------------------------------------------------------------------- other systems (TODO)

// CJimmyGame::PauseGame(bool) - g_gametype element-family slot 68 (+0x110), 0x485410.
static void GameTypePauseGame(bool pause)
{
	g_gametype->PauseGame(pause);
}

// g_gametype->f_490 == id (the running level's 4cc, e.g. 'LV5F').
static bool GameTypeLevelIs(int id)
{
	(void)id;
	NEG_TODO(0x40be3a, "g_gametype->f_490 (level id)");
	return false;
}

// The player's HUD object (a C3DRadar): C3DJimmy (g_545fd0 - 0xc0)->f_d40 - 200, NULL when f_d40 is 0.
static void *PlayerHud(void)			{ return Jimmy()->f_d40; }	// 0x40b676
// hud->f_61c (the HUD panel is shown). The original reads through a NULL hud when there is none.
static bool HudShown(void *hud)			{ return static_cast<C3DRadar *>(hud)->IsShown(); }
static void HudShow(void *hud)			{ static_cast<C3DRadar *>(hud)->Show(); }	// 0x468a50
static void HudHide(void *hud)			{ static_cast<C3DRadar *>(hud)->Hide(); }	// 0x468aa0

// C3DJimmy element-family slots, called on (g_545fd0 - 0xc0).
static void PlayerAnimEvents(void)		{ NEG_TODO(0x44e0b0, "C3DJimmy slot 67 (anim events)"); }
static void PlayerStopSound(void)		{ NEG_TODO(0x44b060, "C3DJimmy slot 116 (stop sound 0x4c20)"); }
static bool PlayerAltButton(void)		{ NEG_TODO(0x4655d0, "C3DPlayerObject slot 103 (2nd button)"); return false; }
static bool PlayerFireButton(void)		{ NEG_TODO(0x465650, "C3DPlayerObject slot 104 (fire button)"); return false; }

// (g_56af7c - 0xc0)->slot 121("VR01.gam", "PHONEBOOTH", +0xb89, +0x9c8, 0x21b): go to the phone booth.
static void PhoneBoothTeleport(void)	{ NEG_TODO(0x4648a0, "phone booth teleport (g_56af7c slot 121)"); }

static void SetMousePos(int x, int y)		{ neg::SetMousePos((short)x, (short)y); }	// 0x4b1d30
using neg::GetMousePos;						// 0x4b1d10 (Mouse.h)
using neg::CenterMouse;						// 0x4b1d50

// CJimmyGame (ecx = g_gametype): score and "+N" pop-up text.
static void DrawScoreHud(float dt)		{ (void)dt; NEG_TODO(0x4859b0, "CJimmyGame score / pop-up text"); }
// (game over: NewGameAfterGameOver 0x498c70, SaveGame.h)

// --------------------------------------------------------------------------- input / system helpers

// 0x4a38c0: show or hide the mouse cursor (ShowCursor in the original).
static void SetMouseCursor(bool show)
{
	if (!show)
	{
		if (g_56aec9)
		{
			g_56aed1 = 0;
			g_56aec9 = 0;
			if (!g_56aed0) SDL_ShowCursor(SDL_DISABLE);
		}
	}
	else if (!g_56aec9)
	{
		if (!g_56aed2) g_56aed1 = 1;
		else g_56aed2 = 0;
		g_56aec9 = 1;
		if (!g_56aed0) SDL_ShowCursor(SDL_ENABLE);
	}
}

// 0x4a3850: "EXIT REQUEST, CALLING QUIT (CLEAN EXIT)" - the application's quit flag (+0x18).
static void RequestQuit(void)
{
	g_app->quit();
}

// 0x4a39e0
static void ClearMouseDelta(void)
{
	g_56aebe = 0;
	g_56aec0 = 0;
}

// A key-binding list (zero-terminated ints from dat\keys.txt): Windows key codes, 500 = right
// mouse button (0x56aea6), 501 = left mouse button (0x56ad9c). Shared body of 0x4659f0..0x465a60.
static bool BindingDown(const int *list)
{
	for (; *list; list++)
	{
		if (*list == 501 || *list == 500)
		{
			if ((*list == 501 ? g_56ad9c : g_56aea6) != 0) return true;
		}
		else if (g_keys[*list])
		{
			ClearMouseDelta();
			return true;
		}
	}
	return false;
}
static bool CursorLeft(void)	{ return g_56b038 < -0.5f || BindingDown(g_5460dc); }	// 0x4659f0
static bool CursorRight(void)	{ return 0.5f < g_56b038 || BindingDown(g_54617c); }	// 0x465a60
static bool CursorDown(void)	{ return g_56b03c < -0.5f || BindingDown(g_54612c); }	// 0x465980 (adds to y)
static bool CursorUp(void)		{ return 0.5f < g_56b03c || BindingDown(g_546154); }	// 0x465910
// 0x465b40: the inventory key ('I') or joystick button
static bool InventoryKey(void)	{ return g_56b05f != 0 || g_keys['I'] != 0; }

// --------------------------------------------------------------------------- game mode

// 0x485360: change the game mode; every game object hears about it (slot 270). (In the exe it sits
// among CJimmyGame's functions; exported through C2DInGameMenu.h.)
void neg::SetGameMode(int mode)
{
	if (mode == g_game_mode) return;
	g_prev_game_mode = g_game_mode;
	g_game_mode = mode;
	if (mode == 1)
	{
		if (g_545d98)
		{
			if (!g_54a469)
			{
				g_54a468 = 1;
				CutsceneLetterbox(false);
			}
			else g_54a469 = 0;
		}
	}
	else
	{
		if (g_54a468 && mode == 3) CutsceneLetterbox(true);
		g_54a468 = 0;
	}
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
	{
		if (!*it) continue;
		// the original calls slot 270 of every listed object (all are CLocalGameObjects)
		if (CLocalGameObject *o = dynamic_cast<CLocalGameObject *>(*it)) o->v270(g_prev_game_mode, mode);
	}
}

// --------------------------------------------------------------------------- page items

// 0x404dd0: set an item's state. Some pickups come in pairs: setting one makes sure the other
// is there too (and counts it).
static void SetItemState(short page, short item, unsigned short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	MenuPage *p = C2DInGameMenu::GetPage(page);
	if (p) p->items[item].type = (short)state;
	if (page != 2) return;
	MenuPage *p2 = C2DInGameMenu::GetPage(2);
	if (item == 10)
	{
		if (!p2 || !p2->items[21].type) { SetItemState(2, 21, 1); g_pickup_counts[21 * 2]++; }
	}
	else if (item == 21)
	{
		if (!p2 || !p2->items[10].type) { SetItemState(2, 10, 1); g_pickup_counts[10 * 2]++; }
	}
	else if (item == 38)
	{
		if (!p2 || !p2->items[31].type) { SetItemState(2, 31, 1); g_pickup_counts[31 * 2]++; }
	}
	else if (item == 31)
	{
		if (!p2 || !p2->items[38].type) { SetItemState(2, 38, 1); g_pickup_counts[38 * 2]++; }
	}
}

// 0x404ef0
static void ClearItemState(short page, short item)
{
	MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::GetPage(page)) != NULL)
		p->items[item].type = 0;
}

// 0x40a860: the level items page (3) for level-progress value `which` (kept in 0x545efc).
// Exported through C2DInGameMenu.h (CJimmyGame::ReadState calls it).
void neg::SetLevelItems(int which)
{
	MenuPage *p3 = C2DInGameMenu::GetPage(3);
	g_545efc = (unsigned int)which;
	switch (which)
	{
		case 0:
			ClearItemState(3, 0x21); ClearItemState(3, 0x1c); ClearItemState(3, 0x22);
			break;
		case 1:
			g_545f00 = 1;
			SetItemState(3, 0x21, 1);
			if (p3) { p3->items[28].type = 0; if (p3) p3->items[34].type = 0; }
			break;
		case 2:
			g_545f00 = 2;
			if (p3) p3->items[33].type = 0;
			SetItemState(3, 0x1d, 1);
			break;
		case 3:
			g_545f00 = 3;
			if (p3) p3->items[29].type = 0;
			SetItemState(3, 0x19, 1);
			break;
		case 4:
			g_545f00 = 4;
			if (p3) p3->items[25].type = 0;
			SetItemState(3, 0x1a, 1);
			break;
		case 5:
			g_545f00 = 5;
			if (p3) p3->items[26].type = 0;
			SetItemState(3, 0x1b, 1);
			break;
		case 6:
			g_545f00 = 6;
			if (!p3) goto skip_f00;				// sic: skips the test of 0x545f00 below
			p3->items[27].type = 0;
			break;
		case 7:
			g_545f00 = 7;
			if (p3) p3->items[8].type = 0;
			SetItemState(3, 0x1c, 1);
			SetItemState(3, 0x22, 1);
			break;
		case 8:
			g_545f04 = 8;
			if (p3) p3->items[33].type = 0;
			SetItemState(3, 0x1c, 1);
			if (p3) p3->items[34].type = 0;
			break;
		case 9:
			g_545f04 = 9;
			if (p3) p3->items[28].type = 0;
			SetItemState(3, 0x23, 1);
			break;
		case 10:
			g_545f04 = 10;
			if (p3) p3->items[35].type = 0;
			SetItemState(3, 0x24, 1);
			break;
		case 11:
			g_545f04 = 11;
			if (p3) p3->items[36].type = 0;
			SetItemState(3, 0x22, 1);
			SetItemState(3, 0x21, 1);
			break;
		case 12:
			g_545f08 = 12;
			if (p3) { p3->items[33].type = 0; if (p3) p3->items[28].type = 0; }
			SetItemState(3, 0x22, 1);
			break;
		case 13:
			g_545f08 = 13;
			if (p3) p3->items[34].type = 0;
			SetItemState(3, 0x21, 1);
			SetItemState(3, 0x1c, 1);
			break;
		case 14:
			SetItemState(3, 0x1e, 1);
			ClearItemState(3, 0x21); ClearItemState(3, 0x1c); ClearItemState(3, 0x22);
			break;
		case -1:
			SetItemState(3, 0x21, 1); SetItemState(3, 0x1c, 1); SetItemState(3, 0x22, 1);
			break;
	}
	if (g_545f00 == 7) ClearItemState(3, 0x21);
skip_f00:
	if (g_545f04 == 11) ClearItemState(3, 0x1c);
	if (g_545f08 == 13) ClearItemState(3, 0x22);
}

// 0x405260: stop the pickup pictures of the inventory screen (Reset each element).
static void ResetPickupPictures(void)
{
	static const char *const names[] = {
		"Brain", "Formula1", "Formula2", "Formula3", "Formula4", "Pic_Cola", "Pic_Electromagnet",
		"Pic_Balloon", "Pic_Flurpbottle", "Pic_PVCPipe", "Pic_Shades", "Pic_Watergun", "Pic_MegaPipe",
		"Pic_MegaLamp", "Pic_Megaphone", "Pic_Megaburp" };
	InterfaceElt *e[16];
	for (int i = 0; i < 16; i++) e[i] = g_interface.Find(names[i]);
	for (int i = 0; i < 16; i++) e[i]->Reset();		// (the original resets them in another order; same result)
}

// --------------------------------------------------------------------------- 0x40c0f0

// A menu method (ecx = g_menu). on: a cut scene starts - out of the menu (back to Playing) if
// it was open (g_545f10 remembers that), mode 3 (In CutScene), the pickup pictures stopped and
// the cinema bars slide in. off: back to Playing unless a level is loading (g_54a468), and the
// bars slide out.
void neg::CutsceneLetterbox(bool on)
{
	if (on)
	{
		if (g_game_mode == 4)
		{
			g_545f10 = 1;
			g_menu->DeactivateActiveMenu();					// +0x498
			SetGameMode(5);
			GameTypePauseGame(false);						// g_gametype +0x110
		}
		else
			g_545f10 = 0;
		SetGameMode(3);
		ResetPickupPictures();								// 0x405260
		if (g_545d98) return;
		g_545d98 = 1;
		InterfaceElt *top = g_interface.Find("LetterBoxTop");			// 0x480970
		InterfaceElt *bottom = g_interface.Find("LetterBoxBottom");
		top->QueueMorph(0, 1.0f, true, true, true, 640.0f, 0.0f, 640.0f, 64.0f, 0.0f, -1.0f, 0.0f, -1.0f);	// 0x480ba0
		bottom->QueueMorph(0, 1.0f, true, true, true, 640.0f, 0.0f, 640.0f, 64.0f, 0.0f, 480.0f, 0.0f, 416.0f);
		return;
	}
	if (!g_54a468) SetGameMode(5);
	g_545e5b = 0;
	if (g_545d98 != 1) return;
	g_545d98 = 0;
	InterfaceElt *top = g_interface.Find("LetterBoxTop");
	InterfaceElt *bottom = g_interface.Find("LetterBoxBottom");
	top->QueueMorph(0, 1.0f, true, true, true, 640.0f, 64.0f, 640.0f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f);
	bottom->QueueMorph(0, 1.0f, false, true, true, 640.0f, 0.0f, 640.0f, 64.0f, 0.0f, 480.0f, 0.0f, 416.0f);
}

// --------------------------------------------------------------------------- inlined blocks

// The text box (0x545844 / 0x545a64) closed - inline in many places.
static void CloseTextBox(void)
{
	g_545e5c = 0;
	g_545dcc = 0;
	g_545844->hide(true);
	g_545a64->hide(true);
	g_545e68 = 1;
}

// The indicator sprites of UpdateIndicator hidden and its timer stopped - inline in v270.
static void HideIndicators(void)
{
	g_545e74 = 0;
	g_ind_a0->hide(true); g_ind_a1->hide(true); g_ind_a2->hide(true);
	g_ind_b0->hide(true); g_ind_b1->hide(true); g_ind_b2->hide(true);
	g_ind_b3->hide(true); g_ind_b4->hide(true); g_ind_b5->hide(true);
}

// The 8 sprites of the yes/no dialog hidden - inline in UpdateDialog.
static void HideDialog(void)
{
	g_dialog_5448e0->hide(true); g_dialog_5446f0->hide(true); g_dialog_544904->hide(true);
	g_dialog_545c7c->hide(true); g_dialog_545850->hide(true); g_dialog_545848->hide(true);
	g_dialog_no_hi->hide(true); g_dialog_yes_hi->hide(true);
}

// --------------------------------------------------------------------------- help

// 0x409fe0: the "?" help icon of the current help entry: shown unless the help text is open.
static void ShowHelpIcon(C2DInGameMenu *m, bool show)
{
	if (!show || m->f_594 != 0 || g_help_index == 0) g_help_icons[g_help_index]->hide(true);
	else g_help_icons[g_help_index]->force_show();
}

// 0x40a030: close the help text (f_59c open, f_594 = -130 while open, f_598 time left).
static void CloseHelp(C2DInGameMenu *m, bool keep_icon)
{
	m->f_598 = 0;
	g_elt_544908[g_help_index].Hide();
	if (!keep_icon) ShowHelpIcon(m, true);
	g_help_menu_elt->Hide();
	m->f_594 = 0;
	m->f_59c = 0;
}

// 0x40a0c0: open the help text for `secs` (only shown while playing; otherwise kept for later in
// 0x545ef4).
static void OpenHelp(C2DInGameMenu *m, float secs)
{
	if (m->f_594 != 0) return;
	if (m->f_598 == 0) m->f_598 = secs;
	if (secs == 0) return;
	if (g_game_mode == 5)
	{
		g_help_icons[g_help_index]->hide(true);
		g_elt_544908[g_help_index].Show();
		g_help_menu_elt->Show();
		m->f_594 = -130;
		m->f_59c = 1;
		g_545ef4 = 0;
	}
	else g_545ef4 = (int)secs;
}

// --------------------------------------------------------------------------- score box

// 0x409d50: the score box (canvas 0x221 at (x, y), or canvas 0x222 at (41, 221) when 0x545f0d).
static void ShowScoreBox(C2DInGameMenu *m, bool show, int x, int y)
{
	if (!show)
	{
		g_score_box->hide(true);
		g_score_box2->hide(true);
	}
	else if (!g_545f0d)
	{
		m->SetupSprite(g_score_box, (float)x, (float)y, 0x221, 1.0f, true, 1.0f);
		g_score_box->force_show();
	}
	else
	{
		m->SetupSprite(g_score_box2, 41.0f, 221.0f, 0x222, 1.0f, true, 1.0f);
		g_score_box2->force_show();
	}
}

// 0x410600: the end-of-game score screen (0x545f0f): the box and the score; after the delay
// (0x532508) any key closes it - and starts a new game when 0x545f11 (game over) is set.
static void UpdateScoreScreen(C2DInGameMenu *m, float dt)
{
	bool any_key = false;
	if (g_5324e4 == 0)
	{
		if (!g_545f0d)
		{
			m->SetupSprite(g_score_box, 407.0f, (float)g_532500, 0x221, 1.0f, true, 1.0f);
			g_score_box->force_show();
		}
		else
		{
			m->SetupSprite(g_score_box2, 41.0f, 221.0f, 0x222, 1.0f, true, 1.0f);
			g_score_box2->force_show();
		}
		DrawText(0x12a, 0x1bd, 'BIG', "%6.0d", g_score);
	}
	if (g_532508 <= 0)
	{
		if (g_545f11)
		{
			g_545a60->force_show();
			g_532508 = 0.0f;
			for (int i = 0; i < 255; i++)
				if (g_keys[i] == 1) { any_key = true; break; }
			if (g_56ad9c == 1 || g_56aea6 == 1 || g_545dc5 == 1 || any_key)
			{
				g_score_box->hide(true);
				g_score_box2->hide(true);
				g_545a60->hide(true);
				GameTypePauseGame(false);
				g_545f0f = 0;
				if (g_545f11)
				{
					NewGameAfterGameOver();
					g_545f11 = 0;
				}
			}
		}
	}
	else
	{
		g_532508 -= dt;
		if (g_532508 <= 0) g_532508 = 0.0f;
		memset(g_keys, 0, 255);
		g_56ad9c = 0;
		g_56aea6 = 0;
		g_545dc5 = 0;
		if (!g_545f11 && g_532508 == 0)
		{
			g_score_box->hide(true);
			g_score_box2->hide(true);
			g_545a60->hide(true);
			GameTypePauseGame(false);
			g_545f0f = 0;
		}
	}
}

// --------------------------------------------------------------------------- Process helpers

// 0x407240: Escape opens / closes the pause menu (page 3) after a one second delay (f_4e0).
// 0x5324f8: 0 menu shown, 1 menu being hidden, 2 menu active, 3 menu closed.
static void HandleEscape(C2DInGameMenu *m, float dt)
{
	if (!g_keys[VK_ESC])
	{
		g_545e57 = 0;
		g_545e58 = 0;
	}
	else if (g_545e5c) CloseTextBox();
	if ((g_keys[VK_ESC] || (unsigned char)g_56b060) && !g_545e57 && !g_545e58 && !g_545dc5)
	{
		g_545dc5 = 1;
		if (g_5324f8 == 2)
		{
			m->v345(0);
			m->PauseAll(false);
			m->f_4e0 = 1.0f;
			g_5324f8 = 1;
		}
		else if (g_5324f8 == 3)
		{
			GameTypePauseGame(true);
			m->v345(1);
			m->f_4e0 = 1.0f;
			g_5324f8 = 0;
		}
	}
	if (m->f_4e0 > 0)
	{
		m->f_4e0 -= dt;
		if (m->f_4e0 <= 0)
		{
			m->f_4e0 = 0;
			g_545e58 = 1;
			if (g_545dc4)
			{
				m->DeactivateActiveMenu();
				GameTypePauseGame(false);
				return;
			}
			m->PauseAll(true);
			m->ActivateActiveMenu(3);
			m->f_590 = 3;
		}
	}
}

// 0x4073c0: the inventory key opens the pickups page (2); on that page the fire / second button
// or the inventory key closes it (back to the pause menu when it was opened from there).
static void HandleInventoryKey(C2DInGameMenu *m)
{
	if (!InventoryKey()) m->f_5a7 = 0;
	if (InventoryKey() && !m->f_5a7)
	{
		m->f_5a7 = 1;
		if (m->f_4ce == -1)
		{
			SetGameMode(4);
			ResetPickupPictures();
			m->f_590 = -1;
			m->ActivateActiveMenu(2);
			return;
		}
	}
	if (m->v301() != 2) return;
	if (g_5324f8 == 2) m->v346();
	if (!PlayerAltButton() && !PlayerFireButton() && !InventoryKey())
	{
		m->f_5a8 = 0;
		return;
	}
	if (m->f_5a8) return;
	m->f_5a8 = 1;
	if (m->f_590 != -1)
	{
		m->v132(1);
		m->DeactivateActiveMenu();
		m->ActivateActiveMenu(3);
		m->f_590 = 3;
		return;
	}
	m->DeactivateActiveMenu();
	SetGameMode(5);
	GameTypePauseGame(false);
}

// 0x406ee0: H / F1 (or a joystick button) toggles the help text; it closes by itself after 20 s.
static void HandleHelpKey(C2DInGameMenu *m, float dt)
{
	if (!g_keys['H'] && !g_keys[VK_F1_] && !g_56b065) g_545e59 = 0;
	else if (!g_545e59 && !g_545dc5)
	{
		g_545dc5 = 1;
		g_545e59 = 1;
		if (m->f_59c) CloseHelp(m, false);						// inline copy of 0x40a030(false)
		else if (m->f_594 == 0) OpenHelp(m, 20.0f);				// inline copy of 0x40a0c0(20)
	}
	if (m->f_598 > 0)
	{
		m->f_598 -= dt;
		if (m->f_598 <= 0) CloseHelp(m, false);					// inline copy of 0x40a030(false)
	}
}

// 0x4070f0: R toggles the player's HUD panel (0.25 s repeat delay in f_5a0).
static void HandleHudKey(C2DInGameMenu *m, float dt)
{
	if (g_keys['R'] && !g_545dc5 && (g_545dc5 = 1, m->f_5a0 == 0))
	{
		void *hud = PlayerHud();
		if (!HudShown(hud)) HudShow(hud);
		else
		{
			// (the original tests f_61c once more here: if clear, 0x545f0e = 0 - unreachable)
			HudHide(hud);
			g_545f0e = 1;
		}
		m->f_5a0 = 0.25f;
	}
	if (m->f_5a0 > 0)
	{
		m->f_5a0 -= dt;
		if (m->f_5a0 <= 0) m->f_5a0 = 0;
	}
}

// 0x407510: the pickup pop-up: the last picked up item's sprite (page 2 item 0x545d88) shown at
// the top right with its count for 3 s (0x545d90); 0x545d8c queues the next one.
static void UpdatePickupPopup(C2DInGameMenu *m, float dt)
{
	if (!(g_545d90 > 0)) return;
	g_545d90 -= dt;
	if (g_keys['I'] || g_56b05f) g_545d90 = 0.0f;
	if (g_5324ec)
	{
		int dx = HudShown(PlayerHud()) ? -0x9e : 0;
		DrawText(WideX((short)(dx + 0x21c), WIDE_RIGHT), 15, 'MED', "%d", g_pickup_counts[g_545d88 * 2]);	// port: WideX
	}
	MenuPage *p2 = C2DInGameMenu::GetPage(2);
	if (g_545d8c != -1)
	{
		MenuItem &it = p2->items[g_545d88];
		if (it.active)
		{
			m->PositionSprite(it.active, it.x, it.y);
			it.active->hide(true);
			g_545d90 = 3.0f;
			g_545d88 = g_545d8c;
			g_545d8c = -1;
		}
	}
	if (g_545d88 != -1 && p2->items[g_545d88].active)
	{
		OMediaCanvasElementWrapper *e = p2->items[g_545d88].active;
		short dx = (short)(HudShown(PlayerHud()) ? -0x9e : 0);
		m->PositionSprite(e, (float)WideX((short)(dx + 0x21c), WIDE_RIGHT), 10.0f);	// port: WideX
		e->force_show();
		if (g_545d90 <= 0)
		{
			MenuItem &it = p2->items[g_545d88];
			m->PositionSprite(it.active, it.x, it.y);
			it.active->hide(true);
			g_545d90 = 0.0f;
			g_545d88 = -1;
		}
	}
}

// 0x40a170: the cursor (mouse, or moved by the direction bindings) and the yes / no dialog
// (open while 0x545dd4; 0x545960 says what "yes" does).
static void UpdateDialog(C2DInGameMenu *m, float dt)
{
	short mx, my;
	GetMousePos(&mx, &my);
	if (CursorLeft()) m->f_4f0 -= dt * 80.0f;
	else if (CursorRight()) m->f_4f0 += dt * 80.0f;
	else m->f_4f0 = 0;
	if (CursorDown()) m->f_4f4 += dt * 80.0f;
	else if (CursorUp()) m->f_4f4 -= dt * 80.0f;
	else m->f_4f4 = 0;
	int cx = (int)((float)mx + m->f_4f0);
	int cy = (int)((float)my + m->f_4f4);
	if (!g_56aed0) SetMousePos(cx, cy);

	if (!g_545dd4)
	{
		HideDialog();
		return;
	}
	float sx = ScreenSX(), sy = ScreenSY();
	float fx = (float)(short)cx, fy = (float)(short)cy;
	bool on_yes = sx * 225.0f < fx && fx < sx * 305.0f && sy * 240.0f < fy && fy < sy * 300.0f;
	if (on_yes || g_keys['Y'])
	{
		g_dialog_yes_hi->force_show();
		g_dialog_no_hi->hide(true);
		m->PlayMenuSound("click shrink ray");
		if (!PlayerFireButton() && !g_keys['Y'])
		{
			m->f_5aa = 0;
		}
		else if (!m->f_5aa)
		{
			switch (g_545960)
			{
				case 7:								// quit the game
					m->f_5aa = 1;
					m->PlayMenuSound("pickup2");
					GameTypePauseGame(false);
					RequestQuit();
					break;
				case 8:
					m->f_5aa = 1;
					m->v319();
					m->PlayMenuSound("pickup2");
					GameTypePauseGame(false);
					break;
				case 9:
					m->f_5aa = 1;
					m->v318();
					GameTypePauseGame(false);
					break;
				case 10:							// back to the phone booth
					m->f_5aa = 1;
					if (g_camera) PhoneBoothTeleport();
					GameTypePauseGame(false);
					break;
				case 11:							// give up: the score screen, then a new game
					m->f_5aa = 1;
					m->PlayMenuSound("pickup2");
					g_545f11 = 1;
					m->v317(0, 1, 0, 0xffffffffu);
					if (!g_545f0d)
					{
						m->SetupSprite(g_score_box, 407.0f, (float)g_532500, 0x221, 1.0f, true, 1.0f);
						g_score_box->force_show();
					}
					else
					{
						m->SetupSprite(g_score_box2, 41.0f, 221.0f, 0x222, 1.0f, true, 1.0f);
						g_score_box2->force_show();
					}
					g_545f0f = 1;
					g_532508 = 8.0f;
					break;
				case 13:
					m->f_5aa = 1;
					m->v320();
					m->PlayMenuSound("pickup2");
					m->v132(1);
					m->DeactivateActiveMenu();
					GameTypePauseGame(false);
					break;
				default:							// (12 and others: just close)
					break;
			}
			g_545dd4 = 0;
			HideDialog();
		}
		g_5445dc->hide(true);
		m->f_5ac = 0;
		return;
	}
	bool on_no = sx * 315.0f < fx && fx < sx * 395.0f && sy * 240.0f < fy && fy < sy * 300.0f;
	if (on_no || g_keys['N'])
	{
		g_dialog_yes_hi->hide(true);
		g_dialog_no_hi->force_show();
		m->PlayMenuSound("click shrink ray");
		if (!PlayerFireButton() && !g_keys['N'])
		{
			m->f_5ab = 0;
			return;
		}
		if (m->f_5ab) return;
		m->f_5ab = 1;
		g_545dd4 = 0;
		GameTypePauseGame(false);
		HideDialog();
		return;
	}
	g_dialog_yes_hi->hide(true);
	g_dialog_no_hi->hide(true);
	m->v330(-1);
	m->f_5aa = 0;
	m->f_5ab = 0;
}

// The Escape / text box block inline at the start of Process for modes 0..3.
static void EscapeLatch(C2DInGameMenu *m)
{
	if (!g_keys[VK_ESC])
	{
		g_545e57 = 0;
		g_545e58 = 1;
		m->f_4e0 = 0;
	}
	else if (g_545e5c) CloseTextBox();
	if (((g_keys[VK_ESC] && !g_545e57) || !g_540c60) && !g_545dc5)
	{
		g_545dc5 = 1;
		g_545ef8 = 1;
	}
}

// --------------------------------------------------------------------------- SLOT 240 0x407740

void C2DInGameMenu::Process(float dt)
{
	CGameObject::Process(dt);					// sic: CGameObject's (0x4ae0b0), not CLocalGameObject's
	g_545dc5 = 0;								// "a key was handled this frame"
	switch (g_game_mode)
	{
		case 1:									// Loading
			if (g_545f0f && !g_545f11) UpdateScoreScreen(this, dt);
			// fall through
		case 0:									// Starting
		case 2:									// Saving
			EscapeLatch(this);
			v341(dt);
			break;

		case 3:									// In CutScene
			if (g_545f0f && g_545f11)
			{
				UpdateScoreScreen(this, dt);
				break;
			}
			v340(dt);
			UpdatePulse(dt);
			v341(dt);
			EscapeLatch(this);
			break;

		case 4:									// In Menu
			v340(dt);
			HandleEscape(this, dt);
			HandleInventoryKey(this);
			HandleMenuInput(dt);
			UpdatePickupPopup(this, dt);
			UpdateDialog(this, dt);
			if (g_5324f8 == 1 || g_5324f8 == 3) HideMenu(GetPage(f_4ce));
			break;

		case 5:									// Playing
			if (g_545f0f && g_545f11)
			{
				UpdateScoreScreen(this, dt);
				break;
			}
			if (!g_5324ec) v341(dt);
			else CloseTextBox();
			HandleEscape(this, dt);
			HandleInventoryKey(this);
			HandleHelpKey(this, dt);
			HandleHudKey(this, dt);
			UpdateIndicator(dt);
			if (f_5ac != 0)
			{
				f_5ac -= dt;
				if (f_5ac < 0)
				{
					f_5ac = 0;
					g_5445dc->hide(true);
				}
			}
			UpdatePulse(dt);
			UpdatePickupPopup(this, dt);
			DrawText(WideX(40, WIDE_LEFT), 67, 'BIG', "%d", (int)(short)(int)g_health);	// port: WideX
			SetHealthBar(g_health);			// (the health bar; takes the float)
			DrawScoreHud(dt);
			break;

		case 6:									// Paused
			if (g_545f0f && g_545f11)
			{
				UpdateScoreScreen(this, dt);
				break;
			}
			if (f_4e0 > 0)
			{
				f_4e0 -= dt;
				if (f_4e0 <= 0)
				{
					f_4e0 = 0;
					g_545e58 = 1;
					if (g_545dc4)
					{
						DeactivateActiveMenu();
						GameTypePauseGame(false);
					}
					else
					{
						PauseAll(true);
						ActivateActiveMenu(3);
						f_590 = 3;
					}
				}
			}
			HandleHelpKey(this, dt);
			HandleHudKey(this, dt);
			UpdateIndicator(dt);
			UpdatePickupPopup(this, dt);
			if (g_545dd4) UpdateDialog(this, dt);
			DrawText(WideX(40, WIDE_LEFT), 67, 'BIG', "%d", (int)(short)(int)g_health);	// port: WideX
			SetHealthBar(g_health);
			DrawScoreHud(dt);
			break;
	}
	g_interface.UpdateAll(dt);
}

// --------------------------------------------------------------------------- SLOT 260 0x40ac30

void C2DInGameMenu::v260()
{
	g_editor->m_load_pending = 0;							// +0x74c: the level has loaded
	g_545f0e = 0;
}

// --------------------------------------------------------------------------- SLOT 270 0x40b460

// The pickup pop-up's sprite put back in its page position - shared tail of several modes.
static void RestorePickupPopup(C2DInGameMenu *m)
{
	if (g_545d88 == -1) return;
	MenuItem &it = C2DInGameMenu::GetPage(2)->items[g_545d88];
	if (!it.active) return;
	m->PositionSprite(it.active, it.x, it.y);
	it.active->hide(true);
	g_545d90 = 0.0f;
	g_545d88 = -1;
}

void C2DInGameMenu::v270(int old_mode, int new_mode)
{
	CLocalGameObject::v270(old_mode, new_mode);
	switch (new_mode)
	{
		case 0:
		case 1:
		{
			strcpy(g_game_mode_name, new_mode == 0 ? "Starting" : "Loading");
			SetMouseCursor(false);
			g_hud_health_bar->hide(true);
			g_545854->hide(true);
			g_545968->hide(true);
			if (new_mode == 0) v306();
			v316(0);
			g_elt_545858.Hide();
			HideIndicators();
			CloseHelp(this, false);				// (inline, with 0x409fe0(true))
			g_help_icons[g_help_index]->hide(true);
			if (new_mode == 0)
			{
				if (g_545d98 == 1)
				{
					// the cut scene letterbox bars slide out
					g_545d98 = 0;
					InterfaceElt *top = g_interface.Find("LetterBoxTop");
					InterfaceElt *bottom = g_interface.Find("LetterBoxBottom");
					top->QueueMorph(0, 1.0f, true, true, true, 640.0f, 64.0f, 640.0f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f);
					bottom->QueueMorph(0, 1.0f, false, true, true, 640.0f, 0.0f, 640.0f, 64.0f, 0.0f, 480.0f, 0.0f, 416.0f);
				}
				g_score_box->hide(true);
				g_score_box2->hide(true);
			}
			else if (g_5324e4 == 0)
			{
				v317(0, 1, 0, 0xffffffffu);
				ShowScoreBox(this, true, 0x197, g_532500);
				g_545f0f = 1;
				g_532508 = 8.0f;
			}
			void *hud = PlayerHud();
			if (hud) HudHide(hud);
			RestorePickupPopup(this);
			break;
		}

		case 2:
			strcpy(g_game_mode_name, "Saving");
			break;

		case 3:
		{
			strcpy(g_game_mode_name, "In CutScene");
			SetMouseCursor(false);
			g_help_icons[g_help_index]->hide(true);
			g_hud_health_bar->hide(true);
			g_545854->hide(true);
			g_545968->hide(true);
			v306();
			v316(0);
			g_elt_545858.Hide();
			HideIndicators();
			CloseHelp(this, false);				// (inline, with 0x409fe0(true))
			g_help_icons[g_help_index]->hide(true);
			g_score_box->hide(true);
			g_score_box2->hide(true);
			void *hud = PlayerHud();
			if (HudShown(hud))
			{
				HudHide(hud);
				g_545f0e = 1;
			}
			RestorePickupPopup(this);
			memset(g_keys, 0, 255);
			g_56ad9c = 0;
			g_56aea6 = 0;
			g_545dc5 = 0;
			CenterMouse();
			break;
		}

		case 4:
		{
			strcpy(g_game_mode_name, "In Menu");
			SetMouseCursor(true);
			CloseHelp(this, false);				// (inline, with 0x409fe0(true))
			g_help_icons[g_help_index]->hide(true);
			CloseTextBox();
			SetLevelItems((int)g_545efc);
			g_hud_health_bar->hide(true);
			g_545854->hide(true);
			g_545968->hide(true);
			v306();
			v316(0);
			g_elt_545858.Hide();
			HideIndicators();
			g_score_box->hide(true);
			g_score_box2->hide(true);
			g_5445dc->hide(true);
			void *hud = PlayerHud();
			if (HudShown(hud))
			{
				HudHide(hud);
				g_545f0e = 1;
			}
			RestorePickupPopup(this);
			break;
		}

		case 5:
			strcpy(g_game_mode_name, "Playing");
			SetMouseCursor(false);
			g_hud_health_bar->force_show();
			g_545854->force_show();
			g_545968->force_show();
			g_score_box->hide(true);
			g_score_box2->hide(true);
			if (g_53763c > 0 || g_537634 > 0) v316(1);
			if (v312(10) || v312(2)) v305();
			if (GameTypeLevelIs('LV5F')) g_elt_545858.Show();
			if (g_545ef4 == 0)
			{
				if (f_594 == 0 && g_help_index != 0) g_help_icons[g_help_index]->force_show();	// 0x40db50
				else g_help_icons[g_help_index]->hide(true);
			}
			else OpenHelp(this, (float)g_545ef4);	// (inline copy of 0x40a0c0)
			HudShow(PlayerHud());					// (also with a NULL hud)
			break;

		case 6:
			strcpy(g_game_mode_name, "Paused");
			SetMouseCursor(false);
			g_score_box->hide(true);
			g_score_box2->hide(true);
			CloseTextBox();
			g_hud_health_bar->force_show();
			g_545854->force_show();
			g_545968->force_show();					// 0x40db50
			if (g_53763c > 0 || g_537634 > 0) v316(1);
			if (v312(10) || v312(2)) v305();
			if (GameTypeLevelIs('LV5F')) g_elt_545858.Show();	// 0x40b2a0(true)
			break;

		case 9:
			strcpy(g_game_mode_name, "None");
			break;
	}
}

// --------------------------------------------------------------------------- SLOT 286 0x40b240

// The pulse used by blinking things: 0x545ddc = sin(5 t) (read by 0x40b270).
void C2DInGameMenu::UpdatePulse(float dt)
{
	double t = (double)g_545dd8 + dt;			// x87: the sine takes the unrounded sum
	g_545dd8 = (float)t;
	g_545ddc = (float)sin(t * 5.0);
}

// --------------------------------------------------------------------------- SLOT 287 0x405200

void C2DInGameMenu::ResetInterface()
{
	g_interface.ResetAll();
	strcpy((char *)g_545de4, (const char *)g_545f14);	// the text line emptied
	if (f_500 != -1)
	{
		StopChannel(f_500, false);			// 0x4b88e0
		f_500 = -1;
	}
}

// --------------------------------------------------------------------------- SLOT 288 0x404480

void C2DInGameMenu::v288()
{
	if (g_5324fc > 1)
	{
		g_5324e4 = 0;
		SetupSprite(g_545844, 0.0f, 0.0f, 0, 1.0f, true, 0.0f);
	}
}

// --------------------------------------------------------------------------- SLOT 289 0x4044c0

// Place a sprite at (x, y) in 640x480 space: centred alignment, its top at y.
void C2DInGameMenu::PositionSprite(OMediaCanvasElementWrapper *e, float x, float y)
{
	float sx = ScreenSX(), sy = ScreenSY();
	if (!e) return;
	e->set_canvas_flags(omcanef_FreeWorldSize);
	e->set_auto_align(omaac_Center, omaac_Center);
	float px = sx * x - (float)((short)g_screen_w / 2);
	float py = ((float)((short)g_screen_h / 2) - e->get_height()) - sy * y;
	px += WideShift(e);								// port: widescreen edge HUD pieces
	e->place(px, py, 0);
}

// --------------------------------------------------------------------------- SLOT 290 0x404570

// Give a sprite canvas `canvas_id` of screens.omt (scaled to the screen when `scaled`) and place
// it at (x, y), 640x480 space, depth z. `unused` (always 1.0) is ignored.
void C2DInGameMenu::SetupSprite(OMediaCanvasElementWrapper *e, float x, float y, int canvas_id, float unused,
								bool scaled, float z)
{
	(void)unused;
	float sx = ScreenSX(), sy = ScreenSY();
	if (!e) return;
	SetSpriteCanvas(e, canvas_id, scaled);
	e->set_canvas_flags(omcanef_FreeWorldSize);
	e->set_auto_align(omaac_Center, omaac_Center);
	e->m_c8 = canvas_id;
	float px = x * sx - (float)((short)g_screen_w / 2);
	float py = ((float)((short)g_screen_h / 2) - e->get_height()) - sy * y;
	px += WideShift(e);								// port: widescreen edge HUD pieces
	e->place(px, py, z);
}

// --------------------------------------------------------------------------- SLOT 291 0x404650

void C2DInGameMenu::SetSpriteCanvas(OMediaCanvasElementWrapper *e, int canvas_id, bool scaled)
{
	float sx = ScreenSX(), sy = ScreenSY();
	OMediaCanvas *c;
	if (canvas_id == -1)
	{
		// sic: the original hands set_canvas the `scaled` argument itself as the canvas pointer
		// (NULL when scaled is false, a wild pointer otherwise). No canvas here.
		c = NULL;
	}
	else
	{
		c = LoadMenuCanvas(canvas_id);
		if (c)
		{
			if (scaled) e->set_size((float)c->get_width() * sx, (float)c->get_height() * sy);
			else e->set_size((float)c->get_width(), (float)c->get_height());
		}
	}
	e->m_c8 = canvas_id;
	e->set_canvas(c);
}

// --------------------------------------------------------------------------- SLOT 292 0x404710

OMediaCanvas *C2DInGameMenu::LoadMenuCanvas(int canvas_id)
{
	OMediaCanvas *c = LoadCanvasID(g_screens_db, canvas_id);
	if (c) c->db_unlock();						// inline: lock count down if > 0
	return c;
}

// --------------------------------------------------------------------------- SLOT 293 0x40ed60

void C2DInGameMenu::ActivateActiveMenu(short page)
{
	if (g_545dc4) return;
	g_545dc4 = 1;								// (debug log "Calling ActivateActiveMenu()")
	GameTypePauseGame(true);
	g_5446ec->hide(true);
	CloseHelp(this, false);						// (inline copy of 0x40a030(false))
	PlayerAnimEvents();
	PlayerStopSound();
	f_4ab = 1;
	v302(page);
	f_4d0 = 1;
	f_4d4 = 0;
	v132(0);
	ShowCurrentMenu();
	g_5324f8 = 2;
	g_545e5b = 0;
	SetGameMode(4);
}

// --------------------------------------------------------------------------- SLOT 294 0x40eeb0

void C2DInGameMenu::DeactivateActiveMenu()
{
	if (g_545dc4)
	{
		g_545dc4 = 0;							// (debug log "Calling DeactivateActiveMenu()")
		f_4ab = 0;
		f_4d4 = 0;
		f_4d0 = 1;
		g_54a4b8 = 1;
		v132(1);
		v330(-1);
		g_5324f8 = 3;
		v302(-1);
	}
	for (int i = 0; i < MENU_PAGES; i++) HideMenu(GetPage(i));
	ResetInterface();
}

// --------------------------------------------------------------------------- SLOT 295 0x407c70

// A new page in g_545da8[index]; its background sprite is set to NULL, so the set-up that
// follows in the original never runs (kept).
static MenuPage *NewPage(C2DInGameMenu *m, int index, short canvas, float bg_y)
{
	MenuPage *p = new MenuPage;					// items: constructor 0x408b60 (eh vector constructor)
	p->background = NULL;
	g_545da8[index] = p;
	p->background = NULL;
	p->background_id = canvas;
	if (p->background)
	{
		p->background->link_layer(g_layer_hud2d);
		m->SetupSprite(p->background, 0.0f, bg_y, p->background_id, 1.0f, true, 0.0f);
	}
	return p;
}

void C2DInGameMenu::CreateMenuPages()
{
	v332();
	v339();

	// page 0
	MenuPage *p = NewPage(this, 0, 0x14, -1.0f);
	SetMenuItem(p, 0, 0x0b, 0x34, 0x00c, 0x0b6);
	SetMenuItem(p, 1, 0x0c, 0x2e, 0x09e, 0x0b6);
	SetMenuItem(p, 2, 0x0d, 0x2f, 0x00c, 0x147);
	SetMenuItem(p, 3, 0x0e, 0x30, 0x09d, 0x147);
	SetMenuItem(p, 4, 0x0f, 0x31, 0x156, 0x0b6);
	SetMenuItem(p, 5, 0x10, 0x32, 0x1e7, 0x0b6);
	SetMenuItem(p, 6, 0x11, 0x33, 0x156, 0x147);
	SetMenuItem(p, 7, 0x12, 0x2d, 0x1e7, 0x147);
	SetMenuItem(p, 8, 0x54, 0x55, 0x1e7, 0x014);
	SetMenuItem(p, 9, 0xb4, 0xb3, 0x1e7, 0x05f);
	SetMenuItem(p, 10, 0xb6, 0xb5, 0x00c, 0x05f);

	// page 6
	p = NewPage(this, 6, 0xf6, 0.0f);
	SetMenuItem(p, 1, 0x112, -1, 0, 0x17c);
	SetItemState(6, 1, 1);

	// page 2: the pickups
	p = NewPage(this, 2, 0x35, 0.0f);
	SetMenuItem(p, 0x01, -1, 0x205, 0x0e9, 0x020);
	SetMenuItem(p, 0x02, -1, 0x209, 0x0e9, 0x174);
	SetMenuItem(p, 0x03, -1, 0x20c, 0x21d, 0x0ca);
	SetMenuItem(p, 0x04, -1, 0x038, 0x146, 0x077);
	SetMenuItem(p, 0x05, -1, 0x208, 0x0e9, 0x11f);
	SetMenuItem(p, 0x06, -1, 0x03a, 0x013, 0x022);
	SetMenuItem(p, 0x07, -1, 0x20e, 0x21d, 0x174);
	SetMenuItem(p, 0x08, -1, 0x207, 0x0e9, 0x0ca);
	SetMenuItem(p, 0x09, -1, 0x206, 0x0e9, 0x075);
	SetMenuItem(p, 0x0a, -1, 0x03e, 0x0a0, 0x022);
	SetMenuItem(p, 0x0b, -1, 0x03f, 0x05a, 0x022);
	SetMenuItem(p, 0x0c, -1, 0x040, 0x1d3, 0x077);
	SetMenuItem(p, 0x0d, -1, 0x041, 0x18c, 0x077);
	SetMenuItem(p, 0x2b, -1, 0x1c5, 0x1d3, 0x121);
	SetMenuItem(p, 0x1d, -1, 0x1f5, 0x18c, 0x022);
	SetMenuItem(p, 0x10, -1, 0x042, 0x19f, 0x0db);
	SetMenuItem(p, 0x11, -1, 0x045, 0x1f9, 0x0db);
	SetMenuItem(p, 0x12, -1, 0x20d, 0x21d, 0x11f);
	SetMenuItem(p, 0x13, -1, 0x20b, 0x21d, 0x075);
	SetMenuItem(p, 0x14, -1, 0x20a, 0x21d, 0x020);
	SetMenuItem(p, 0x15, -1, 0x202, 0x05a, 0x077);
	SetMenuItem(p, 0x2e, -1, 0x1f0, 0x0a0, 0x077);
	SetMenuItem(p, 0x17, -1, 0x1f1, 0x013, 0x077);
	SetMenuItem(p, 0x18, -1, 0x1c7, 0x18c, 0x176);
	SetMenuItem(p, 0x19, -1, 0x1ff, 0x146, 0x176);
	SetMenuItem(p, 0x1a, -1, 0x1f2, 0x1d3, 0x176);
	SetMenuItem(p, 0x1b, -1, 0x1f3, 0x1d3, 0x022);
	SetMenuItem(p, 0x1c, -1, 0x1f4, 0x146, 0x022);
	SetMenuItem(p, 0x0f, -1, 0x050, 0x145, 0x0db);
	SetMenuItem(p, 0x1e, -1, 0x201, 0x013, 0x0cd);
	SetMenuItem(p, 0x1f, -1, 0x203, 0x05a, 0x0cd);
	SetMenuItem(p, 0x20, -1, 0x200, 0x0a0, 0x0cd);
	SetMenuItem(p, 0x21, -1, 0x1fc, 0x0a0, 0x121);
	SetMenuItem(p, 0x22, -1, 0x1fd, 0x05a, 0x121);
	SetMenuItem(p, 0x23, -1, 0x1fe, 0x013, 0x121);
	SetMenuItem(p, 0x24, -1, 0x1f8, 0x05a, 0x176);
	SetMenuItem(p, 0x26, -1, 0x204, 0x013, 0x176);
	SetMenuItem(p, 0x25, -1, 0x1f9, 0x0a0, 0x176);
	SetMenuItem(p, 0x27, -1, 0x1c0, 0x18c, 0x0cd);
	SetMenuItem(p, 0x28, -1, 0x1fb, 0x1d3, 0x0cd);
	SetMenuItem(p, 0x29, -1, 0x1fa, 0x146, 0x0cd);
	SetMenuItem(p, 0x2a, -1, 0x1f6, 0x18c, 0x121);
	SetMenuItem(p, 0x2c, -1, 0x1f7, 0x146, 0x121);
	SetItemState(2, 0x20, 1);
	g_pickup_counts[0x20 * 2]++;
	SetItemState(2, 0x19, 1);
	g_pickup_counts[0x19 * 2]++;
	SetItemState(3, 0, 1);
	SetItemState(3, 1, 1);
	SetItemState(3, 2, 1);
	SetItemState(3, 3, 1);
	SetItemState(3, 4, 1);
	SetItemState(3, 6, 1);
	SetItemState(3, 0xd, 1);

	// page 4
	p = NewPage(this, 4, 0xef, -1.0f);
	SetMenuItem(p, 0x1, 0x85, -1, 0x1e, 0x179);
	SetMenuItem(p, 0x2, 0x86, -1, 0x8a, 0x19b);
	SetMenuItem(p, 0x3, 0x89, -1, 0x8a, 0x179);
	SetMenuItem(p, 0x4, 0x8e, -1, 0xf6, 0x179);
	SetMenuItem(p, 0x5, 0x82, -1, 0xf6, 0x19b);
	SetMenuItem(p, 0x6, 0x81, -1, 0x36, 0x070);
	SetMenuItem(p, 0x7, 0x87, -1, 0x36, 0x087);
	SetMenuItem(p, 0x8, 0x88, -1, 0x36, 0x09e);
	SetMenuItem(p, 0x9, 0x84, -1, 0x36, 0x0b5);
	SetMenuItem(p, 0xa, 0x8b, -1, 0x36, 0x0cc);
	SetMenuItem(p, 0xb, 0x8c, -1, 0x36, 0x0e3);
	SetMenuItem(p, 0xc, 0x8d, -1, 0x36, 0x0fa);
	SetMenuItem(p, 0xd, 0x90, -1, 0x36, 0x111);
	SetMenuItem(p, 0xe, 0x91, -1, 0x36, 0x128);
	SetMenuItem(p, 0xf, 0x80, -1, 0x36, 0x13f);
	SetItemState(4, 6, 1);
	SetItemState(4, 7, 1);
	SetItemState(4, 8, 1);
	SetItemState(4, 9, 1);
	SetItemState(4, 10, 1);
	SetItemState(4, 11, 1);
	SetItemState(4, 12, 1);
	SetItemState(4, 13, 1);
	SetItemState(4, 14, 1);
	SetItemState(4, 15, 1);
	SetItemState(4, 1, 1);
	SetItemState(4, 2, 1);
	SetItemState(4, 3, 1);
	SetItemState(4, 4, 1);
	SetItemState(4, 5, 1);

	// page 5
	p = NewPage(this, 5, 0xec, -1.0f);
	SetMenuItem(p, 1, 0x0ea, -1, 0x090, 0x05f);
	SetMenuItem(p, 2, 0x0e9, -1, 0x090, 0x0ff);
	SetMenuItem(p, 3, 0x0eb, -1, 0x01e, 0x178);
	SetMenuItem(p, 4, 0x10e, -1, 0x1a3, 0x05f);
	SetMenuItem(p, 5, 0x10f, -1, 0x1a3, 0x0ff);
	SetMenuItem(p, 6, 0x110, -1, 0x119, 0x05f);
	SetMenuItem(p, 7, 0x111, -1, 0x119, 0x0ff);
	SetItemState(5, 1, 1);
	SetItemState(5, 2, 1);
	SetItemState(5, 3, 1);
	SetItemState(5, 4, 1);
	SetItemState(5, 5, 1);
	SetItemState(5, 6, 1);
	SetItemState(5, 7, 1);

	v323(2);
	v323(0);
	v323(3);
	v323(4);
	v323(5);
	v323(6);
	HideMenu(GetPage(0));
	HideMenu(GetPage(2));
	HideMenu(GetPage(3));
	HideMenu(GetPage(4));
	HideMenu(GetPage(5));
	HideMenu(GetPage(6));
}

// --------------------------------------------------------------------------- SLOT 296 0x404740

void C2DInGameMenu::ShowCurrentMenu()
{
	if (f_4ce == -1) return;
	g_5324f8 = 0;
	DisplayMenu(GetPage(f_4ce));
}

// --------------------------------------------------------------------------- SLOT 297 0x408e40

void C2DInGameMenu::DisplayMenu(MenuPage *page)
{
	if (!page) return;							// (debug log "starting  displayMenu")
	if (page->background) page->background->force_show();
	if (page == GetPage(2)) v346();				// the pickups page lays out its items first
	for (int i = 0; i < MENU_ITEMS; i++)
	{
		MenuItem &it = page->items[i];
		if (it.type == 0 || it.type == 4) continue;
		if (it.rollover) it.rollover->hide(true);
		if (it.active) it.active->force_show();
	}
}

// --------------------------------------------------------------------------- SLOT 298 0x404770

void C2DInGameMenu::HideCurrentMenu()
{
	if (f_4ce == -1) return;
	g_5324f8 = 1;
	HideMenu(GetPage(f_4ce));
}

// --------------------------------------------------------------------------- SLOT 299 0x408df0

void C2DInGameMenu::HideMenu(MenuPage *page)
{
	if (!page) return;
	if (page->background) page->background->hide(true);
	for (int i = 0; i < MENU_ITEMS; i++)
	{
		MenuItem &it = page->items[i];
		if (it.rollover) it.rollover->hide(true);
		if (it.active) it.active->hide(true);
	}
}

// --------------------------------------------------------------------------- SLOT 300 0x409980

// Define item `index` of a page: its canvases and position. Quirk kept: both sprite pointers are
// set to NULL just before the code that would set them up, so no sprite is created here (the
// size fields stay 0); the items' sprites come from elsewhere if at all.
void C2DInGameMenu::SetMenuItem(MenuPage *page, short index, short canvas_alt, short canvas, short x, short y)
{
	if (index < 0 || index >= MENU_ITEMS) return;
	MenuItem &it = page->items[index];
	it.active = NULL;
	it.active_id = canvas;
	it.type = 0;
	if (it.active)
	{
		it.active->link_layer(g_layer_hud2d);
		if (canvas != -1)
		{
			SetupSprite(it.active, (float)x, (float)y, canvas, 1.0f, true, 0.0f);
			it.w = (short)(int)it.active->get_width();
			it.h = (short)(int)it.active->get_height();
		}
	}
	if (!g_windowed)
	{
		it.x = (float)x;
		it.y = (float)y;
	}
	else
	{
		it.x = (float)x + 1.0f;
		it.y = (float)y + 2.0f;
	}
	it.rollover = NULL;
	it.rollover_id = canvas_alt;
	if (it.rollover)
	{
		it.rollover->link_layer(g_layer_hud2d);
		SetupSprite(it.rollover, (float)x, (float)y, canvas_alt, 1.0f, true, 0.0f);
		it.w = (short)(int)it.rollover->get_width();
		it.h = (short)(int)it.rollover->get_height();
	}
}

// --------------------------------------------------------------------------- SLOT 303 0x405740

// The PDA text screen drawing itself: the backdrops fade in, the frame lines (Pix4A..H) grow
// one after the other, the cursor line and cursor slide, the black bar opens.
void C2DInGameMenu::PlayPdaIntro()
{
	InterfaceElt *backdrop2  = g_interface.Find("TextBackdrop2");
	InterfaceElt *backdrop   = g_interface.Find("TextBackdrop");
	InterfaceElt *pix4a      = g_interface.Find("Pix4A");
	InterfaceElt *pix4b      = g_interface.Find("Pix4B");
	InterfaceElt *pix4c      = g_interface.Find("Pix4C");
	InterfaceElt *pix4d      = g_interface.Find("Pix4D");
	InterfaceElt *pix4e      = g_interface.Find("Pix4E");
	InterfaceElt *pix4f      = g_interface.Find("Pix4F");
	InterfaceElt *pix4g      = g_interface.Find("Pix4G");
	InterfaceElt *pix4h      = g_interface.Find("Pix4H");
	InterfaceElt *cursorline = g_interface.Find("CursorLine");
	InterfaceElt *cursor1    = g_interface.Find("Cursor1");
	InterfaceElt *horizblack = g_interface.Find("HorizBlack");

	backdrop2->QueueWait(0, 1.3f);
	backdrop2->QueueMorph(0, 0.1f, true, true, false, 267, 61, 267, 61, 112, 159, 112, 159);
	backdrop2->QueueFade(0, 0.5f, true, true, 0.0f, 0.7f);

	pix4a->QueueWait(0, 0.2f);
	pix4a->QueueMorph(0, 0.5f, true, true, true, 4, 4, 272, 4, 110, 157, 110, 157);
	pix4b->QueueWait(0, 0.7f);
	pix4b->QueueMorph(0, 0.15f, true, true, true, 4, 4, 4, 64, 378, 157, 378, 157);
	pix4c->QueueWait(0, 0.85f);
	pix4c->QueueMorph(0, 0.5f, true, true, true, 4, 4, 272, 4, 378, 221, 110, 221);
	pix4d->QueueWait(0, 1.35f);
	pix4d->QueueMorph(0, 0.15f, true, true, true, 4, 4, 4, 64, 110, 221, 110, 157);
	pix4e->QueueWait(0, 1.2f);
	pix4e->QueueMorph(0, 0.5f, true, true, true, 4, 4, 124, 4, 410, 100, 410, 100);
	pix4f->QueueWait(0, 1.7f);
	pix4f->QueueMorph(0, 0.5f, true, true, true, 4, 4, 4, 124, 410, 100, 410, 100);
	pix4g->QueueWait(0, 1.2f);
	pix4g->QueueMorph(0, 0.5f, true, true, true, 4, 4, 124, 4, 530, 220, 410, 220);
	pix4h->QueueWait(0, 1.7f);
	pix4h->QueueMorph(0, 0.5f, true, true, true, 4, 4, 4, 124, 530, 100, 530, 100);

	cursorline->QueueMorph(0, 0.5f, true, true, true, 5, 30, 270, 30, 109, 99, 109, 99);
	cursor1->QueueMove(0, 0.5f, true, true, true, 1.0f, 0.0f, 250.0f);
	cursorline->QueueFade(0, 0.5f, false, true, 0.0f, 1.0f);
	cursor1->QueueFade(0, 0.5f, false, true, 0.0f, 1.0f);

	backdrop->QueueWait(0, 0.3f);
	backdrop->QueueMorph(0, 0.1f, true, true, true, 270, 30, 270, 30, 109, 99, 109, 99);
	backdrop->QueueFade(0, 0.5f, true, true, 0.0f, 0.7f);

	horizblack->QueueWait(0, 1.8f);
	horizblack->QueueMorph(0, 0.1f, true, true, true, 115, 115, 115, 115, 414, 105, 414, 105);
	horizblack->QueueWait(0, 0.1f);
	horizblack->QueueMorph(0, 2.0f, false, true, true, 115, 1, 115, 115, 414, 105, 414, 105);
}

// --------------------------------------------------------------------------- SLOT 311 0x40ad90

// The indicator (set by 0x40ac50 for 3 s): one of three sprites (0x545e70 = 1..3) and one of six
// (0x545e6c = 0..5) shown, the whole fading out during the last second.
void C2DInGameMenu::UpdateIndicator(float dt)
{
	OMediaCanvasElementWrapper *const all[9] = { g_ind_a0, g_ind_a1, g_ind_a2, g_ind_b0, g_ind_b1, g_ind_b2,
												 g_ind_b3, g_ind_b4, g_ind_b5 };
	if (g_545e74 <= 0) return;
	float t = g_545e74 - dt;
	g_545e74 = t;
	if (t <= 1.0f)
	{
		if (t <= 0)
		{
			for (int i = 0; i < 9; i++) all[i]->hide(true);
			g_ind_a0->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));	// (a0 twice in the original)
			for (int i = 0; i < 9; i++) all[i]->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
		else
		{
			for (int i = 0; i < 9; i++) all[i]->set_diffuse(OMediaFARGBColor(t, 1.0f, 1.0f, 1.0f));
		}
		return;
	}
	switch (g_545e70)
	{
		case 1:
			g_ind_a0->force_show();
			g_ind_a1->hide(true);
			g_ind_a2->hide(true);
			break;
		case 2:
			g_ind_a0->hide(true);
			g_ind_a2->hide(true);
			g_ind_a1->force_show();
			break;
		case 3:
			g_ind_a2->force_show();
			g_ind_a1->hide(true);
			g_ind_a0->hide(true);
			break;
		default:
			g_ind_a0->hide(true);
			g_ind_a1->hide(true);
			g_ind_a2->hide(true);
			break;
	}
	if (g_545e6c <= 5) all[3 + g_545e6c]->force_show();	// (the other five are left as they are)
	else
	{
		for (int i = 3; i < 9; i++) all[i]->hide(true);
	}
}
