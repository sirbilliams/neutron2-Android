/*  C2DInGameMenu part d: the dat\menu.dat parser, the menu texts and trigger texts, the screen
    fades, the phone-booth level warp, the PDA zoom and the pickups page.
    Reconstructed from Neutron2.exe slots 332..343, 345..347 (see menu/C2DInGameMenu_d.inc) and
    the menu helpers 0x404dd0, 0x404ef0, 0x405260, 0x40a860, 0x4aefb0.

    Constants: 0x4caa98 = 1/255 (double), 0x4caa44 = 0, 0x4caa68 = 0.1, 0x4caa6c = 1.
    The original computes in x87 registers with MSVC's 53-bit precision control; the expressions
    below keep those intermediates in double where the result is truncated or stored.

    Calls into slots of the other parts use their generated signatures. Where such a signature
    cannot carry the real argument (a pointer or a string passed as `int` / `unsigned int`), the
    call casts and is marked (merged: calls now use the real slot types).  */

#include "../C2DInGameMenu.h"
#include "../CEditor.h"
#include "../CGameType.h"
#include "../CanvasWrapper.h"
#include "../CGameObject.h"
#include "../Game.h"
#include "../Todo.h"
#include "../Sound.h"
#include "../C3DTriggerType.h"
#include "OMediaViewPort.h"
#include "OMediaRGBColor.h"
#include <list>
#include <new>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace neg;

std::list<C2DInGameMenu::MenuObject *> C2DInGameMenu::s_objects;		// 0x5444b0

static const char kDelims[] = ",\n,\t";		// 0x53250c (strtok delimiters, ',' given twice)
enum { VK_ESCAPE_KEY = 0x1b };

// --------------------------------------------------------------------------- stubs (other systems)

// Sound: PlaySound 0x4b8380 / StopChannel 0x4b88e0 (Sound.h)
// g_gametype +0x490: the running level's 4cc ('LV5C' ...).
static int GameTypeLevelID(void)		{ NEG_TODO(0x40fe7f, "CGameType +0x490 (level id)"); return 0; }
// 0x4b1d50: puts the cursor in the middle of the viewport (g_viewport +0xd4..0xe0, world +0x38 ...).
static void CenterCursor(void)			{ NEG_TODO(0x4b1d50, "centre the cursor"); }
// 0x485360: the game mode (0x53b5b4), told to every object (vtable +0x438).
// A C3DTriggerType (element vtable +0xdc, its CGameObject base at +0xc8) activated by name.
static void TriggerTypeActivate(CGameObject *o, const char *name, int a)
{
	C3DTriggerType *t = o ? dynamic_cast<C3DTriggerType *>(o) : NULL;	// 0x406cb0 (o - 0xc8)
	if (t) t->CallNextTrigger(name, (unsigned char)a);					// element +0xdc
}
// A C3DFlyingCamera (element vtable +0x240, its CGameObject base at +0xc0).
namespace neg { void FlyingCameraTrigger(CGameObject *o); }			// objects/C3DFlyingCamera.cpp
static void FlyingCameraStart(CGameObject *o)	{ neg::FlyingCameraTrigger(o); }	// 0x406cd4
// A C3DPickupItem (element vtable +0xec, its CGameObject base at +0xc8).
static void PickupItemCollect(CGameObject *o)	{ NEG_TODO(0x408dce, "C3DPickupItem element slot 59"); (void)o; }
// C3DJimmy (element vtable +0x1e4): go to `file` at start point `start`.
static void JimmyLoadLevel(CGameObject *jimmy, const char *file, const char *start,
						   const char *file2, const char *start2, int a)
{
	NEG_TODO(0x41038c, "C3DJimmy element slot 121 (load level)");
	(void)jimmy; (void)file; (void)start; (void)file2; (void)start2; (void)a;
}
// C3DJimmy (element vtable +0x228)(menu, jimmy, 3, 13.0) before level1_C.gam.
static void JimmySlot138(CGameObject *jimmy, C2DInGameMenu *menu, CGameObject *jimmy_go, int a, float b)
{
	NEG_TODO(0x410315, "C3DJimmy element slot 138"); (void)jimmy; (void)menu; (void)jimmy_go; (void)a; (void)b;
}
// C3DJimmy +0xcbc: an object whose vtable +0xdc is called after a warp (if set).
static void JimmyAfterWarp(CGameObject *jimmy)	{ NEG_TODO(0x4103a6, "C3DJimmy +0xcbc vtable +0xdc"); (void)jimmy; }

// --------------------------------------------------------------------------- menu helpers

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// 0x404dd0: item `item` of page `page` enabled := on. Pickups 10/21 and 31/38 come in pairs:
// enabling one also enables (and counts) the other if that one is not enabled yet.
static void SetItemEnabled(short page, short item, short on)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p) p->items[item].type = on;
	if (page != C2DInGameMenu::MENU_PICKUPS) return;
	C2DInGameMenu::MenuPage *pk = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
	// (with no pickups page these recurse until the stack overflows, as in the original)
	if (item == 10)
	{
		if (!pk || !pk->items[21].type) { SetItemEnabled(2, 21, 1); g_pickup_counts[2 * 21]++; }
	}
	else if (item == 21)
	{
		if (!pk || !pk->items[10].type) { SetItemEnabled(2, 10, 1); g_pickup_counts[2 * 10]++; }
	}
	else if (item == 38)
	{
		if (!pk || !pk->items[31].type) { SetItemEnabled(2, 31, 1); g_pickup_counts[2 * 31]++; }
	}
	else if (item == 31)
	{
		if (!pk || !pk->items[38].type) { SetItemEnabled(2, 38, 1); g_pickup_counts[2 * 38]++; }
	}
}

// 0x404ef0: item `item` of page `page` disabled.
static void ClearItemEnabled(short page, short item)
{
	C2DInGameMenu::MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::Page(page)) != NULL)
		p->items[item].type = 0;
}

// Inlined in 0x40a860: main-menu item `item` disabled (no range check).
static void MainItemOff(int item)
{
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(C2DInGameMenu::MENU_MAIN);
	if (p) p->items[item].type = 0;
}

// 0x40a860 (a menu method that does not use `this`): selects the level pictures of the main
// menu for warp `which` (-1..14); remembers it in 0x545efc and, per world, in 0x545f00 /
// 0x545f04 / 0x545f08.
static void ShowLevelPictures(int which)
{
	g_545efc = (unsigned int)which;
	switch (which)
	{
		case -1:
			SetItemEnabled(3, 33, 1); SetItemEnabled(3, 28, 1); SetItemEnabled(3, 34, 1);
			break;
		case 0:
			ClearItemEnabled(3, 33); ClearItemEnabled(3, 28); ClearItemEnabled(3, 34);
			break;
		case 1:  g_545f00 = 1;  SetItemEnabled(3, 33, 1); MainItemOff(28); MainItemOff(34); break;
		case 2:  g_545f00 = 2;  MainItemOff(33); SetItemEnabled(3, 29, 1); break;
		case 3:  g_545f00 = 3;  MainItemOff(29); SetItemEnabled(3, 25, 1); break;
		case 4:  g_545f00 = 4;  MainItemOff(25); SetItemEnabled(3, 26, 1); break;
		case 5:  g_545f00 = 5;  MainItemOff(26); SetItemEnabled(3, 27, 1); break;
		case 6:
			g_545f00 = 6;
			if (!C2DInGameMenu::Page(C2DInGameMenu::MENU_MAIN))
				goto skip_world1;						// (without a main page the world-1 test is skipped)
			MainItemOff(27);
			break;
		case 7:  g_545f00 = 7;  MainItemOff(8); SetItemEnabled(3, 28, 1); SetItemEnabled(3, 34, 1); break;
		case 8:  g_545f04 = 8;  MainItemOff(33); SetItemEnabled(3, 28, 1); MainItemOff(34); break;
		case 9:  g_545f04 = 9;  MainItemOff(28); SetItemEnabled(3, 35, 1); break;
		case 10: g_545f04 = 10; MainItemOff(35); SetItemEnabled(3, 36, 1); break;
		case 11: g_545f04 = 11; MainItemOff(36); SetItemEnabled(3, 34, 1); SetItemEnabled(3, 33, 1); break;
		case 12: g_545f08 = 12; MainItemOff(33); MainItemOff(28); SetItemEnabled(3, 34, 1); break;
		case 13: g_545f08 = 13; MainItemOff(34); SetItemEnabled(3, 33, 1); SetItemEnabled(3, 28, 1); break;
		case 14:
			SetItemEnabled(3, 30, 1);
			ClearItemEnabled(3, 33); ClearItemEnabled(3, 28); ClearItemEnabled(3, 34);
			break;
		default:
			break;
	}
	if (g_545f00 == 7) ClearItemEnabled(3, 33);
skip_world1:
	if (g_545f04 == 11) ClearItemEnabled(3, 28);
	if (g_545f08 == 13) ClearItemEnabled(3, 34);
}

// 0x405260 (a menu method that does not use `this`): stops the pickup pictures of the PDA.
// (A missing element crashes, as in the original.)
static void ResetPickupPictures(void)
{
	static const char *const kNames[16] = {
		"Brain", "Formula1", "Formula2", "Formula3", "Formula4", "Pic_Cola", "Pic_Electromagnet",
		"Pic_Balloon", "Pic_Flurpbottle", "Pic_PVCPipe", "Pic_Shades", "Pic_Watergun", "Pic_MegaPipe",
		"Pic_MegaLamp", "Pic_Megaphone", "Pic_Megaburp" };
	InterfaceElt *e[16];
	for (int i = 0; i < 16; i++) e[i] = g_interface.Find(kNames[i]);
	for (int i = 0; i < 16; i++) e[i]->Reset();
}

// --------------------------------------------------------------------------- parser helpers

// strtok's next field of the current line into `dst` (a 128-byte local in the original; the
// copy is bounded here - the original strcpy would overrun on a field of 128+ characters).
static bool NextField(char (&dst)[128])
{
	const char *t = strtok(NULL, kDelims);
	if (!t) return false;
	snprintf(dst, sizeof dst, "%s", t);
	return true;
}

// The original strcpy's the fields into the record unbounded; bounded copies here.
template <size_t N> static void CopyField(char (&dst)[N], const char *src) { snprintf(dst, N, "%s", src); }

// Inlined: font name -> DrawText font (anything else leaves -1).
static void ParseFont(int &font, const char *s)
{
	if (strcasecmp(s, "SMAL") == 0) font = 'SMAL';
	else if (strcasecmp(s, "MED") == 0) font = 'MED';
	else if (strcasecmp(s, "LARG") == 0) font = 'LARG';
}

// Inlined: alignment name -> 0 CENTERED, 2 LEFT, 1 RIGHT (RIGHT is tested on its own).
static void ParseAlign(int &align, const char *s)
{
	if (strcasecmp(s, "CENTERED") == 0) align = 0;
	else if (strcasecmp(s, "LEFT") == 0) align = 2;
	if (strcasecmp(s, "RIGHT") == 0) align = 1;
}

static float ParseColour(const char *s) { return (float)(atof(s) * (1.0 / 255.0)); }	// * 0x4caa98

// MSVC _read in text mode: reads `size` raw bytes into buf, then translates CR LF -> LF in place
// (Ctrl-Z ends the file). Returns the byte count after translation, 0 on an empty file, -1 if
// the file cannot be read. As in the CRT, the bytes after the translated text keep the stale end
// of the raw data (one byte per CR removed).
static int ReadTextFile(FILE *f, char *buf, int size)
{
	int n = (int)fread(buf, 1, (size_t)size, f);
	if (n <= 0) return ferror(f) ? -1 : 0;
	int o = 0;
	for (int i = 0; i < n; i++)
	{
		if (buf[i] == 0x1a) break;
		if (buf[i] == '\r' && i + 1 < n && buf[i + 1] == '\n') continue;
		buf[o++] = buf[i];
	}
	return o;
}

// --------------------------------------------------------------------------- 0x40c360

// LoadMenuDat: reads dat\menu.dat and hands each line to the parser of its keyword. Returns true
// on failure: the file cannot be opened or is empty, a line is incomplete, or a keyword is
// unknown - parsing stops there (objects parsed so far stay in s_objects).
// Quirk kept: the text is not terminated after the byte count _read returns, so strtok goes on
// into the stale raw bytes _read's CR LF translation left behind it. With the shipped menu.dat
// (CR LF lines) every line is parsed, then the stale tail starts with the token "0", an unknown
// keyword: the function always returns true (InitMenus v295 ignores it).
bool C2DInGameMenu::v332()
{
	FILE *f = fopen("dat/menu.dat", "rb");			// _open(".\\dat\\menu.dat", _O_RDONLY) - text mode
	if (!f)
	{
		// (debug log) "open failed on input file"
		return true;
	}
	// 60000-byte stack buffer in the original, not terminated: strtok stops at the first zero
	// byte of the (fresh) stack after the raw data. Zero-filled here, one byte longer.
	char *buf = new char[60000 + 1]();
	int n = ReadTextFile(f, buf, 60000);
	if (n == 0)		// unsigned test in the original: a read error (-1) passes as success
	{
		// (debug log) "Problem reading file menu.txt"
		fclose(f);
		delete[] buf;
		return true;
	}
	// (debug log) "Read %u bytes from file\n"
	fclose(f);

	bool failed = false;
	for (const char *t = strtok(buf, kDelims); t; t = strtok(NULL, kDelims))
	{
		char key[128];
		CopyField(key, t);
		bool err;
		if (strcasecmp("BACKGROUND", key) == 0)			err = ParseBackground();
		else if (strcasecmp("TEXTSTRING", key) == 0)	err = ParseTextString();
		else if (strcasecmp("MENUITEM", key) == 0)		err = ParseMenuItem();
		else if (strcasecmp("TEXTMENUITEM", key) == 0)	err = ParseTextMenuItem();
		else if (strcasecmp("HELPMENUITEM", key) == 0)	err = ParseHelpMenuItem();
		else if (strcasecmp("TRIGTEXTSTRING", key) == 0)	err = ParseTriggerTextString();
		else err = true;
		if (err) { failed = true; break; }
	}
	delete[] buf;
	return failed;
}

// --------------------------------------------------------------------------- 0x40c550

// BACKGROUND,name,page,x,y,canvas: creates the page (a page made earlier is replaced, not
// freed) with its background sprite on the HUD layer at (x, y).
bool C2DInGameMenu::ParseBackground()
{
	char name[128], page[128], x[128], y[128], canvas[128];
	if (!NextField(name) || !NextField(page) || !NextField(x) || !NextField(y) || !NextField(canvas))
		return true;
	// (debug log) "Parse BACKGROUND successful"
	MenuObject *o = new (std::nothrow) MenuObject;
	if (!o)
	{
		ErrorLog("ERROR: Unable to allocate in ParseBackground");
		return true;
	}
	o->type = MOBJ_BACKGROUND;
	CopyField(o->name, name);
	o->page = MenuIndex(page);
	o->x = atoi(x);
	o->y = atoi(y);
	o->canvas_hi = atoi(canvas);
	if (o->page >= 0 && o->page < MENU_PAGES)
	{
		Page(o->page) = new (std::nothrow) MenuPage;
		OMediaCanvasElementWrapper *bg = new (std::nothrow) OMediaCanvasElementWrapper;
		Page(o->page)->background = bg;				// (crashes if the page could not be allocated)
		Page(o->page)->background_id = (short)o->canvas_hi;
		if (Page(o->page)->background)
		{
			MenuPage *p = Page(o->page);
			p->background->link_layer(g_layer_hud2d);
			SetupSprite(p->background, (float)o->x, (float)o->y, (int)p->background_id, 1.0f, true, 0.0f);
		}
	}
	s_objects.push_back(o);
	return false;
}

// --------------------------------------------------------------------------- 0x40c8f0

// TEXTSTRING,name,page,x,y,text,font,align,r,g,b: a text drawn on its page (v340).
bool C2DInGameMenu::ParseTextString()
{
	char name[128], page[128], x[128], y[128], text[128], font[128], align[128], r[128], g[128], b[128];
	if (!NextField(name) || !NextField(page) || !NextField(x) || !NextField(y) || !NextField(text) ||
		!NextField(font) || !NextField(align) || !NextField(r) || !NextField(g) || !NextField(b))
		return true;
	// (debug log) "Parse TEXTSTRING successful"
	MenuObject *o = new (std::nothrow) MenuObject;
	if (!o)
	{
		ErrorLog("ERROR: Unable to allocate in ParseBackground");
		return true;
	}
	o->type = MOBJ_TEXTSTRING;
	CopyField(o->name, name);
	o->page = MenuIndex(page);
	o->x = atoi(x);
	o->y = atoi(y);
	CopyField(o->text, text);
	ParseFont(o->font, font);
	ParseAlign(o->align, align);
	o->r = ParseColour(r);
	o->g = ParseColour(g);
	o->b = ParseColour(b);
	s_objects.push_back(o);
	return false;
}

// --------------------------------------------------------------------------- 0x40d2c0

// MENUITEM,name,page,x,y,canvas,canvas_hi,id: page item `id` (v300 sets it up).
bool C2DInGameMenu::ParseMenuItem()
{
	char name[128], page[128], x[128], y[128], canvas[128], canvas_hi[128], id[128];
	if (!NextField(name) || !NextField(page) || !NextField(x) || !NextField(y) || !NextField(canvas) ||
		!NextField(canvas_hi) || !NextField(id))
		return true;
	// (debug log) "Parse MENUITEM successful"
	MenuObject *o = new (std::nothrow) MenuObject;
	if (!o)
	{
		// (debug log - not the error log here) "ERROR: Unable to allocate in ParseBackground"
		return true;
	}
	o->type = MOBJ_MENUITEM;
	CopyField(o->name, name);
	o->page = MenuIndex(page);
	o->x = atoi(x);
	o->y = atoi(y);
	o->canvas = atoi(canvas);
	o->canvas_hi = atoi(canvas_hi);
	o->id = atoi(id);
	if (o->page == -1)
		ErrorLog("ERROR: Invalid menuname in ParseMenuItem");
	else	// real arguments: (MenuPage *, id, canvas, canvas_hi, x, y) - all 16-bit but the page
		SetMenuItem(Page(o->page), (short)o->id, (short)o->canvas, (short)o->canvas_hi, (short)o->x, (short)o->y);
	s_objects.push_back(o);
	return false;
}

// --------------------------------------------------------------------------- 0x40d620

// TEXTMENUITEM,name,page,dx,dy,text,font,align,r,g,b: a text at (dx, dy) from the MENUITEM
// called `name` (the last one with a position).
bool C2DInGameMenu::ParseTextMenuItem()
{
	char name[128], page[128], x[128], y[128], text[128], font[128], align[128], r[128], g[128], b[128];
	if (!NextField(name) || !NextField(page) || !NextField(x) || !NextField(y) || !NextField(text) ||
		!NextField(font) || !NextField(align) || !NextField(r) || !NextField(g) || !NextField(b))
		return true;
	// (debug log) "Parse TEXTMENUITEM successful"
	MenuObject *o = new (std::nothrow) MenuObject;
	if (!o)
	{
		ErrorLog("ERROR: Unable to allocate in ParseBackground");
		return true;
	}
	o->type = MOBJ_TEXTMENUITEM;
	// The offsets are uninitialised stack in the original when no MENUITEM matches.
	int item_x = 0, item_y = 0;
	for (std::list<MenuObject *>::iterator it = s_objects.begin(); it != s_objects.end(); ++it)
	{
		MenuObject *m = *it;
		if (m->type == MOBJ_MENUITEM && strcasecmp(m->name, name) == 0 && m->x != -1 && m->y != -1)
		{
			// (debug log) "Found matching MENUITEM, %d %d"
			item_x = m->x;
			item_y = m->y;
		}
	}
	CopyField(o->name, name);
	o->page = MenuIndex(page);
	o->x = atoi(x) + item_x;
	o->y = atoi(y) + item_y;
	CopyField(o->text, text);
	ParseFont(o->font, font);
	ParseAlign(o->align, align);
	o->r = ParseColour(r);
	o->g = ParseColour(g);
	o->b = ParseColour(b);
	s_objects.push_back(o);
	return false;
}

// --------------------------------------------------------------------------- 0x40cdb0

// HELPMENUITEM,name,page,x,y,text,font,align,id,r,g,b: a text shown while item `id` of its page
// has f_26 set.
bool C2DInGameMenu::ParseHelpMenuItem()
{
	char name[128], page[128], x[128], y[128], text[128], font[128], align[128], id[128], r[128], g[128], b[128];
	if (!NextField(name) || !NextField(page) || !NextField(x) || !NextField(y) || !NextField(text) ||
		!NextField(font) || !NextField(align) || !NextField(id) || !NextField(r) || !NextField(g) || !NextField(b))
		return true;
	// (debug log) "Parse HELPMENUITEM successful"
	MenuObject *o = new (std::nothrow) MenuObject;
	if (!o)
	{
		ErrorLog("ERROR: Unable to allocate in ParseBackground");
		return true;
	}
	o->type = MOBJ_HELPMENUITEM;
	CopyField(o->name, name);
	o->page = MenuIndex(page);
	o->x = atoi(x);
	o->y = atoi(y);
	CopyField(o->text, text);
	ParseFont(o->font, font);
	ParseAlign(o->align, align);
	o->id = atoi(id);
	o->r = ParseColour(r);
	o->g = ParseColour(g);
	o->b = ParseColour(b);
	s_objects.push_back(o);
	return false;
}

// --------------------------------------------------------------------------- 0x40db70

// TRIGTEXTSTRING,name,trigger,x,y,text,font,align,r,g,b,line: line `line` of the text typed out
// while trigger `trigger` is g_trig_text_group (v340). No page.
bool C2DInGameMenu::ParseTriggerTextString()
{
	char name[128], trigger[128], x[128], y[128], text[128], font[128], align[128], r[128], g[128], b[128], line[128];
	if (!NextField(name) || !NextField(trigger) || !NextField(x) || !NextField(y) || !NextField(text) ||
		!NextField(font) || !NextField(align) || !NextField(r) || !NextField(g) || !NextField(b) || !NextField(line))
		return true;
	// (debug log) "Parse TEXTSTRING successful" (sic)
	MenuObject *o = new (std::nothrow) MenuObject;
	if (!o)
	{
		ErrorLog("ERROR: Unable to allocate in ParseTriggerTextString");
		return true;
	}
	o->type = MOBJ_TRIGTEXTSTRING;
	CopyField(o->name, name);
	CopyField(o->trigger, trigger);
	o->x = atoi(x);
	o->y = atoi(y);
	CopyField(o->text, text);
	ParseFont(o->font, font);
	ParseAlign(o->align, align);
	o->r = ParseColour(r);
	o->g = ParseColour(g);
	o->b = ParseColour(b);
	o->id = atoi(line);
	s_objects.push_back(o);
	return false;
}

// --------------------------------------------------------------------------- 0x40e090

// DumpMenuObjects: one debug-log line per object - the log is compiled empty, so nothing happens.
void C2DInGameMenu::v339()
{
	for (std::list<MenuObject *>::iterator it = s_objects.begin(); it != s_objects.end(); ++it)
		if ((*it)->type != MOBJ_NONE)
		{
			// (debug log) "Type:%d  ID:%s  MenuName=%d  Px:%d  Py%d  omtid:%d TextString:%s",
			//             type, name, page, x, y, canvas_hi, text
		}
}

// --------------------------------------------------------------------------- 0x40e0e0

// Inlined for every text: colour, and centring for CENTERED.
static void SetTextStyle(const C2DInGameMenu::MenuObject *o)
{
	g_text_rgb[0] = o->r;
	g_text_rgb[1] = o->g;
	g_text_rgb[2] = o->b;
	if (o->align == 0) g_text_center = 1;
}

static void ResetTextStyle(bool red)
{
	if (red) g_text_rgb[0] = 1.0f;
	g_text_rgb[1] = 1.0f;
	g_text_rgb[2] = 1.0f;
	g_text_center = 0;
}

// DrawMenuTexts(dt): draws the TEXTSTRINGs, TEXTMENUITEMs and HELPMENUITEMs of the current page
// (f_4ce) and the TRIGTEXTSTRINGs of the current trigger group, typing the line g_trig_text_line
// out at g_teletype_rate characters per second with the "telemetry1" sound (channel f_500;
// f_4f8 = time typing, f_4fc = characters shown). Escape (or 0x545e57) skips the group while the
// sound plays.
void C2DInGameMenu::v340(float dt)
{
	for (std::list<MenuObject *>::iterator it = s_objects.begin(); it != s_objects.end(); ++it)
	{
		MenuObject *o = *it;
		if (o->type == MOBJ_NONE) continue;
		switch (o->type)
		{
			case MOBJ_BACKGROUND:
			case MOBJ_MENUITEM:
				break;

			case MOBJ_TEXTSTRING:
				if (o->page != (int)f_4ce) break;
				SetTextStyle(o);
				DrawText((short)o->x, (short)o->y, o->font, "%s", o->text);
				ResetTextStyle(false);			// quirk: the red stays the text's
				break;

			case MOBJ_TEXTMENUITEM:
				if (o->page != (int)f_4ce) break;
				SetTextStyle(o);
				DrawText((short)o->x, (short)o->y, o->font, "%s", o->text);
				ResetTextStyle(true);
				break;

			case MOBJ_HELPMENUITEM:
				// (item index not range-checked)
				if (o->page != (int)f_4ce || Page(f_4ce)->items[o->id].shown == 0) break;
				SetTextStyle(o);
				DrawText((short)o->x, (short)o->y, o->font, "%s", o->text);
				ResetTextStyle(true);
				break;

			case MOBJ_TRIGTEXTSTRING:
			{
				if (strcasecmp(g_trig_text_group, o->trigger) != 0) break;
				if (g_trig_text_line > o->id)
					f_4fc = (int)strlen(o->text);		// an earlier line: all of it
				else
				{
					if ((g_keys[VK_ESCAPE_KEY] || g_545e57) && f_500 != -1)
					{
						// skipped: end the group. Returns - the rest of the list is not drawn
						// this frame.
						StopChannel(f_500, false);
						f_500 = -1;
						g_trig_text_len = 0;
						strcpy(g_trig_text_group, g_empty_string);
						g_trig_text_stop = true;
						return;
					}
					if (g_trig_text_line != o->id) break;	// a later line: not yet
					if (g_trig_text_len == 0)
					{
						// start typing this line
						if (f_500 == -1) f_500 = PlaySound(-1, "telemetry1", true);
						g_trig_text_len = (int)strlen(o->text);
						f_4fc = 1;
						f_4f8 = 0.0f;
					}
					else
					{
						double t = (double)dt + f_4f8;		// (the product uses the unrounded sum)
						f_4f8 = (float)t;
						f_4fc = (int)(t * g_teletype_rate);
						if (f_4fc > g_trig_text_len)
						{
							// line done: the next one starts on the next frame
							if (f_500 != -1)
							{
								StopChannel(f_500, false);
								f_500 = -1;
							}
							g_trig_text_len = 0;
							g_trig_text_line++;
						}
					}
				}
				SetTextStyle(o);
				// 128 bytes in the original, whose terminating store below can land past it
				// (f_4fc >= 128); one zero-filled byte more here and the store is bounded.
				char shown[128 + 1] = "";
				strcpy(shown, g_empty_string);
				if (f_4fc >= 1 && !g_trig_text_stop)
				{
					if (f_4fc >= 128)
					{
						memcpy(shown, o->text, 127);
						ErrorLog("ERROR: TRIGTEXTSTRING %s too long", o->text);
					}
					else
					{
						memcpy(shown, o->text, (size_t)f_4fc);
						if (shown[f_4fc - 1] == ' ')
						{
							// the sound pauses on spaces
							if (f_500 != -1)
							{
								StopChannel(f_500, false);
								f_500 = -1;
							}
						}
						else if (f_500 == -1 && g_trig_text_len != 0)
							f_500 = PlaySound(-1, "telemetry1", true);
					}
					if (f_4fc < (int)sizeof shown) shown[f_4fc] = 0;
				}
				if ((g_trig_text_len == 0 || g_trig_text_stop) && f_500 != -1)
				{
					StopChannel(f_500, false);
					f_500 = -1;
				}
				DrawText((short)o->x, (short)o->y, o->font, "%s", shown);
				ResetTextStyle(true);
				break;
			}

			default:
				ErrorLog("ERROR: Invalid MenuType");
				break;
		}
	}
}

// --------------------------------------------------------------------------- 0x406aa0

// Inlined twice: puts the viewport back where 0x54a480.. saved it (when 0x54a498 is set) and
// stops the object in 0x56af7c.
static void RestoreViewAndStop(void)
{
	if (g_54a498)
	{
		g_viewport->place(g_54a480, g_54a484, g_54a488);
		short ay = (short)(int)g_54a494;
		short ax = (short)(int)g_54a490;
		g_viewport->set_angle(ax, ay, 0);
	}
	if (g_camera) g_camera->v176(0, 0, 0);
}

// UpdateFade(dt): runs the fade set up by 0x406910 / 0x406a40 on a full-screen quad:
// g_fade_sprite for states 0 (to opaque) and 1 (to clear) - the menu's own (v288 each frame) -,
// g_fade_sprite2 for 2/4 (to opaque), 3 (to clear) and 5 (hold opaque; then it fades back
// over g_fade_in_time as state 3 and activates the object tagged g_fade_trigger).
void C2DInGameMenu::v341(float dt)
{
	if (g_fade_setting == 0 || (g_fade_state < 2 && g_fade_setting < 2))
	{
		// fades off (or only trigger fades and this is a menu one)
		g_fade_done = true;
		RestoreViewAndStop();
		return;
	}
	if (g_fade_done) return;

	g_fading = true;
	RestoreViewAndStop();
	switch (g_fade_state)
	{
		case 0: case 1:
			v288();
			g_fade_cur = g_fade_sprite;
			break;
		case 2: case 3: case 4: case 5:
			g_fade_cur = g_fade_sprite2;
			break;
	}

	g_fade_time = g_fade_time - dt;
	if (g_fade_time < 0.0f)
	{
		g_fade_time = 0.0f;
		g_fading = false;
		switch (g_fade_state)
		{
			case 0: case 1: case 2: case 3:
				g_fade_done = true;
				g_editor->m_load_pending = 0;		// 0x406bdc: +0x74c
				g_fade_cur->hide(true);
				break;
			case 4:
				g_fade_state = 5;
				g_fade_time = g_fade_hold_time;
				break;
			case 5:
				if (g_fade_setting == 0)
				{
					g_fade_done = true;
					break;
				}
				g_fade_duration = g_fade_in_time < 0.1f ? 0.1f : g_fade_in_time;
				g_fade_time = g_fade_duration;
				g_fade_state = 3;
				g_fade_done = false;
				g_fading = true;
				if (strcasecmp(g_fade_trigger, g_empty_string) != 0)
				{
					CGameObject *o = FindObjectByTag(g_fade_trigger);
					if (o)
					{
						if (o->NameContains((char *)"C3DTRIGGERTYPE"))
							TriggerTypeActivate(o, g_fade_trigger, 0);
						else if (o->NameContains((char *)"C3DFLYINGCAMERA"))
							FlyingCameraStart(o);
					}
				}
				break;
		}
	}

	float a;
	switch (g_fade_state)
	{
		case 0: case 2: case 4:
			a = (float)(1.0 - (double)g_fade_time / g_fade_duration);
			break;
		case 1: case 3:
			a = (float)((double)g_fade_time / g_fade_duration);
			break;
		case 5:
			a = 1.0f;
			break;
		default:
			return;
	}
	g_fade_cur->set_diffuse(OMediaFARGBColor(a, a, a, a));		// 2002 +0x90..0x9c, all four = a
	g_fade_cur->force_show();									// hide_count = 1 + hide(false) when hidden
}

// --------------------------------------------------------------------------- 0x40fd90

// WarpToLevel(level): the phone booth's level select (`level` = the MENUITEM id). Jimmy goes to
// the level's PHONEBOOTH start point unless it is the running level; holding 1 / 2 / 3 on
// level 8 picks level1_A / _B / _C instead. Stops the trigger texts first.
void C2DInGameMenu::v342(short level)
{
	// The original first copies two C3DJimmy strings (+0xb89, +0x9c8 from the object start) into
	// a local that is never read (and crashes there without a Jimmy): dropped.
	g_flag_545e54 = false;
	if (f_500 != -1)
	{
		StopChannel(f_500, false);
		f_500 = -1;
		g_trig_text_len = 0;
		strcpy(g_trig_text_group, g_empty_string);
		g_trig_text_stop = true;
	}

	int id = 0, code = 0;
	const char *file = 0;
	switch (level)
	{
		case 5: case 34:	ShowLevelPictures(12); id = 'LV5C'; file = "level5c.gam"; code = 0x1b4; break;
		case 7:				id = 'LV5D'; file = "level5d.gam"; code = 0x21d; break;
		case 8:
			if (g_keys['1']) goto level1_a;
			if (g_keys['2']) goto level1_b;
			if (g_keys['3']) goto level1_c;
			id = 'LEV1'; file = "level1.gam"; code = 0x1b2;
			break;
		case 9:				id = 'LV3E'; file = "level3e.gam"; code = 0x21a; break;
		case 10:			id = 'LV3D'; file = "level3d.gam"; code = 0x219; break;
		case 14:			id = 'LV3A'; file = "level3a.gam"; code = 0x21a; break;
		case 15: case 28:	ShowLevelPictures(8); id = 'LV5A'; file = "level5a.gam"; code = 0x1b0; break;
		case 16: case 33:	ShowLevelPictures(1); id = 'VR01'; file = "VR01.gam"; code = 0x21b; break;
		case 17:			id = 'LV3C'; file = "level3c.gam"; code = 0x219; break;
		case 18: case 35:	id = 'LV5B'; file = "level5b.gam"; code = 0x1b0; break;
		case 19: case 31:	id = 'LV5F'; file = "level5f.gam"; code = 0x21c; break;
		case 20: case 29:	id = 'LV1B'; file = "level1B.gam"; code = 0x94; break;
		case 21: case 36:	id = 'L5B1'; file = "level5b1.gam"; code = 0x1b0; break;
		case 23: case 26:
		level1_a:
			id = 'L1_A'; file = "level1_A.gam"; code = 0x1b1;
			break;
		case 22: case 25:
		level1_b:
			id = 'L1_B'; file = "level1_B.gam"; code = 0x1b1;
			break;
		case 24: case 27:
		level1_c:
			if (GameTypeLevelID() == 'L1_C') return;
			g_flag_545e54 = true;
			JimmySlot138(g_jimmy, this, g_jimmy, 3, 13.0f);
			JimmyLoadLevel(g_jimmy, "level1_C.gam", "PHONEBOOTH", "level1_C.gam", "PHONEBOOTH", 0x1b1);
			JimmyAfterWarp(g_jimmy);
			return;
		case 32:			id = 'LV4B'; file = "level4B.gam"; code = 0x21c; break;
		default:			// 6, 11..13, 30 and anything else: no warp
			JimmyAfterWarp(g_jimmy);
			return;
	}
	if (GameTypeLevelID() == id) return;			// already there (and no +0xcbc call)
	JimmyLoadLevel(g_jimmy, file, "PHONEBOOTH", file, "PHONEBOOTH", code);
	JimmyAfterWarp(g_jimmy);
}

// --------------------------------------------------------------------------- 0x4104a0

// Jimmy touched a phone booth (C3DPhoneBooth vt1 slot 16, through g_menu): close the active menu
// if it is open, else open it.
void C2DInGameMenu::v343()
{
	if (g_menu_active) DeactivateActiveMenu();					// DeactivateActiveMenu
	else ActivateActiveMenu(1);								// ActivateActiveMenu(1)
}

// --------------------------------------------------------------------------- 0x405ca0

// The PDA ("MainMenuPDA") zoom. open: rise from below the bottom right corner, grow to full
// screen, then hidden (the menu page shows instead). Closed: v287, v298, all keys released, the
// cursor centred, game mode 5, then the PDA shrinks from full screen to the corner and drops out
// of sight. (A missing PDA element crashes, as in the original.)
void C2DInGameMenu::v345(bool open)
{
	InterfaceElt *pda = g_interface.Find("MainMenuPDA");
	pda->Reset();
	if (open)
	{
		ResetPickupPictures();
		pda->QueueMorph(0, 0.5f, true, true, true, 160, 140, 160, 140, 480, 480, 480, 340);
		pda->QueueMorph(0, 0.4f, true, true, true, 160, 140, 640, 480, 480, 340, 0, 0);
		pda->QueueScale(0, 0.15f, true, true, true, 0, 0);
		pda->QueueHide();
		return;
	}
	ResetInterface();
	HideCurrentMenu();
	g_56ad9c = 0;
	memset(g_keys, 0, 255);						// 0x56ada0..0x56ae9e (the last key is left alone)
	g_56aea6 = 0;
	g_545dc5 = 0;
	CenterCursor();
	SetGameMode(5);
	pda->Show();
	pda->QueueMorph(0, 0.5f, true, true, true, 640, 480, 160, 140, 0, 0, 480, 340);
	pda->QueueMorph(0, 0.5f, true, true, true, 160, 140, 160, 140, 480, 340, 480, 481);
}

// --------------------------------------------------------------------------- 0x408cd0

// DrawPickupCounts: on the pickups page, the count of every owned pickup that has a second
// sprite, at the item's position.
void C2DInGameMenu::v346()
{
	for (short i = 0; i < MENU_ITEMS; i++)
	{
		MenuPage *p = Page(MENU_PICKUPS);
		if (p && i >= 0 && i < MENU_ITEMS && p->items[i].type && p->items[i].active &&
			g_pickup_counts[2 * i] != 0)
			DrawText((short)(int)p->items[i].x, (short)(int)p->items[i].y, 'MED', "%d", g_pickup_counts[2 * i]);
	}
}

// --------------------------------------------------------------------------- 0x408d50

// GiveAllPickups (cheat): every pickup not owned yet is enabled and counted once; then every
// C3DPickupItem in the level is collected.
void C2DInGameMenu::v347()
{
	for (int i = 0; i < MENU_ITEMS; i++)
	{
		MenuPage *p = Page(MENU_PICKUPS);
		short si = (short)i;
		if (!p || si < 0 || si > 49 || p->items[si].type == 0)
		{
			SetItemEnabled(2, (short)i, 1);
			g_pickup_counts[2 * i]++;
		}
	}
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && (*it)->NameContains((char *)"C3DPICKUPITEM"))
			PickupItemCollect(*it);
}
