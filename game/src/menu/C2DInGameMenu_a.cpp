/*  C2DInGameMenu part a: constructor 0x401380, destructor 0x403fa0 (slot 0 0x403f80 is the
    compiler's deleting destructor around it), UnInitObject (slot 8, 0x4043d0), and the
    dat\keys.txt reader the constructor calls (0x465bc0 / 0x465eb0 / 0x4660d0).

    The constructor opens omt\screens.omt (g_screens_db), builds the menu pages (v295), creates
    the menu's fixed sprites (OMediaCanvasElementWrapper, all hidden), sets up the static
    interface elements and creates the named ones (g_interface.Create), then publishes itself
    in g_menu and reads the key bindings.

    Sprite coordinates are in the game's 640x480 space; v290 (part b) places them.            */
#include "../C2DInGameMenu.h"
#include "../CanvasWrapper.h"
#include "../Game.h"
#include "../Todo.h"
#include "../C3DRadar.h"
#include "../port/Widescreen.h"
#include "../port/BillsUi.h"
#include "OMediaFileStream.h"
#include "OMediaError.h"
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

using namespace neg;

#define COUNTOF(a) ((int)(sizeof(a) / sizeof((a)[0])))

// --------------------------------------------------------------------------- helpers

static unsigned int fbits(float f) { unsigned int u; memcpy(&u, &f, 4); return u; }

// The constructor's sprite idiom: a new OMediaCanvasElementWrapper linked to `layer`.
static OMediaCanvasElementWrapper *NewSprite(OMediaLayer *layer)
{
	OMediaCanvasElementWrapper *s = new OMediaCanvasElementWrapper;
	if (s) s->link_layer(layer);			// vtable +0x34 (some sites in the original skip the check)
	return s;
}

// v290 (part b, 0x404570) as every constructor call uses it: canvas `canvas_id` of the
// screens database, scaled to the screen, placed at (x, y) in 640x480 space, z 0. From the
// machine code its arguments are (OMediaCanvasElementWrapper *elt, float x, float y,
// int canvas_id, float unused (always 1.0), bool scaled, float z).
// MERGE NOTE: the generated signature of v290 cannot carry these types (the element pointer
// is an `int` there); this is the one call to adapt once part b gives v290 its real signature.
static void PlaceSprite(C2DInGameMenu *m, OMediaCanvasElementWrapper *s, float x, float y, int canvas_id)
{
	m->SetupSprite(s, x, y, canvas_id, 1.0f, true, 0.0f);
}

// The fields the constructor writes into a static interface element (what Create would set).
static void SetupElt(InterfaceElt &e, short canvas_id, short x, short y)
{
	e.db = g_screens_db;
	e.canvas_id = canvas_id;
	e.x = x;
	e.y = y;
	e.src_blend = omblendfc_Src_Alpha;
	e.dest_blend = omblendfc_Inv_Src_Alpha;
	e.home_x = x;
	e.home_y = y;
}

static void DeleteSprite(OMediaCanvasElementWrapper *s)
{
	if (s) delete s;						// virtual deleting destructor (through the +0x30 subobject)
}

// --------------------------------------------------------------------------- dat\keys.txt

// 0x4660d0 "C3DPlayerObject::TextToKeys": key name -> Windows virtual-key code (500/501 for
// the mouse buttons), 0 for KEY_NONE. The JOY_xx names of keys.txt are not in the table: they
// log the error and give 0. That is the shipped game's behaviour (checked against the exe: it has
// no JOY_ key names, and ErrorLog is active in release), so on Windows keys.txt's JOY_01..JOY_09
// were dead too and the 0 ends that binding list early. The joystick reaches the game only through
// the poll's globals (g_joy_buttons[DirectInput button number], g_joy_x / g_joy_y), which the key
// tests read next to these lists - see Input.cpp and docs/controls.md.
static int TextToKey(const char *name)
{
	static const struct { const char *name; int code; } kKeys[] = {
		{"KEY_NONE",0}, {"LeftMouse",501}, {"RightMouse",500}, {"KEY_BACKSPACE",8}, {"KEY_TAB",9},
		{"KEY_ENTER",13}, {"KEY_SHIFT",16}, {"KEY_CTRL",17}, {"KEY_PAUSE",19}, {"KEY_ESC",27},
		{"KEY_SPACE",32}, {"KEY_PGUP",33}, {"KEY_PGDN",34}, {"KEY_END",35}, {"KEY_HOME",36},
		{"KEY_LEFT",37}, {"KEY_UP",38}, {"KEY_RIGHT",39}, {"KEY_DOWN",40}, {"KEY_COMMA",188},
		{"KEY_INS",45}, {"KEY_PERIOD",190}, {"KEY_DEL",46}, {"KEY_0",48}, {"KEY_1",49},
		{"KEY_2",50}, {"KEY_3",51}, {"KEY_4",52}, {"KEY_5",53}, {"KEY_6",54}, {"KEY_7",55},
		{"KEY_8",56}, {"KEY_9",57}, {"KEY_LESSTHAN",188}, {"KEY_GREATERTHAN",190}, {"KEY_A",65},
		{"KEY_B",66}, {"KEY_C",67}, {"KEY_D",68}, {"KEY_E",69}, {"KEY_F",70}, {"KEY_G",71},
		{"KEY_H",72}, {"KEY_I",73}, {"KEY_J",74}, {"KEY_K",75}, {"KEY_L",76}, {"KEY_M",77},
		{"KEY_N",78}, {"KEY_O",79}, {"KEY_P",80}, {"KEY_Q",81}, {"KEY_R",82}, {"KEY_S",83},
		{"KEY_T",84}, {"KEY_U",85}, {"KEY_V",86}, {"KEY_W",87}, {"KEY_X",88}, {"KEY_Y",89},
		{"KEY_Z",90}, {"KEY_LEFTBRACKET",219}, {"KEY_RIGHTBRACKET",221}, {"KEY_F1",112},
		{"KEY_F2",113}, {"KEY_F3",114}, {"KEY_F4",115}, {"KEY_F5",116}, {"KEY_F6",117},
		{"KEY_F7",118}, {"KEY_F8",119}, {"KEY_F9",120}, {"KEY_F10",121}, {"KEY_F11",122},
		{"KEY_F12",123}, {"KEY_PAD0",96}, {"KEY_PAD1",97}, {"KEY_PAD2",98}, {"KEY_PAD3",99},
		{"KEY_PAD4",100}, {"KEY_PAD5",101}, {"KEY_PAD6",102}, {"KEY_PAD7",103}, {"KEY_PAD8",104},
		{"KEY_PAD9",105}, {"KEY_PADSTAR",106}, {"KEY_PADPLUS",107}, {"KEY_PADMINUS",109},
		{"KEY_PADDOT",110}, {"KEY_PADSLASH",111}, {"KEY_PADNUM",144}, {"KEY_TILDE",192},
		{"KEY_BACKSLASH",220},
	};
	for (size_t i = 0; i < sizeof(kKeys) / sizeof(kKeys[0]); i++)
		if (strcasecmp(kKeys[i].name, name) == 0) return kKeys[i].code;		// _stricmp
	ErrorLog("ERROR: C3DPlayerObject::TextToKeys %s not found", name);
	return 0;
}

static const char kKeyDelims[] = ",\n,\t,=";	// 0x539838 (sic: ',' three times)

// 0x465eb0: the six key codes of one binding line, continuing the strtok of ReadKeys.
// Returns true when the file ends early.
static bool ReadKeyBinding(int *codes)
{
	for (int i = 0; i < 6; i++)
	{
		char *tok = strtok(NULL, kKeyDelims);
		if (!tok) return true;
		char name[128];
		snprintf(name, sizeof(name), "%s", tok);		// strcpy into a 128-byte buffer in the original
		codes[i] = TextToKey(name);
	}
	return false;
}

// 0x465bc0: read the key bindings from .\dat\keys.txt ("Key_Jump=KEY_ENTER, KEY_SHIFT, ...").
// Returns true on error. Names are compared case-insensitively; unknown names are skipped
// token by token.
static bool ReadKeys(void)
{
	static const struct { const char *name; int *codes; } kBindings[] = {
		{"KEY_ClimbDown", g_key_climb_down},	{"KEY_Brake", g_key_brake},
		{"KEY_Action", g_key_action},			{"KEY_Turnaround", g_key_turnaround},
		{"KEY_Speed", g_key_speed},				{"KEY_Jump", g_key_jump},
		{"KEY_Go", g_key_go},					{"KEY_Back", g_key_back},
		{"KEY_TurnLeft", g_key_turn_left},		{"KEY_TurnRight", g_key_turn_right},
		{"KEY_CutSceneBypass", g_key_cutscene_bypass},
		{"KEY_DeactivateItem", g_key_deactivate_item},
	};
	static char buf[60000 + 1];				// on the stack in the original (_chkstk)

	// _open(".\\dat\\keys.txt", O_RDONLY) / _read(60000) / _close
	OMediaFileStream *fs = InitFileStream(".\\dat\\keys.txt");
	if (!fs) return true;					// "open failed on input file" (debug log)
	unsigned int n = 0;
	try
	{
		n = fs->getsize();
		if (n > 60000) n = 60000;
		fs->read(buf, n);
	}
	catch (OMediaError &) { n = 0; }
	UnInitFileStream(fs);
	if (n == 0) return true;				// "Problem reading file menu.txt" (sic)

	// _read in text mode turns CR LF into LF.
	unsigned int m = 0;
	for (unsigned int i = 0; i < n; i++)
		if (!(buf[i] == '\r' && i + 1 < n && buf[i + 1] == '\n')) buf[m++] = buf[i];
	// The original does not terminate the buffer (strtok runs on into the stack); terminated here.
	buf[m] = 0;

	for (char *tok = strtok(buf, kKeyDelims); tok; tok = strtok(NULL, kKeyDelims))
	{
		char name[128];
		snprintf(name, sizeof(name), "%s", tok);		// strcpy in the original
		for (size_t i = 0; i < sizeof(kBindings) / sizeof(kBindings[0]); i++)
			if (strcasecmp(kBindings[i].name, name) == 0 && ReadKeyBinding(kBindings[i].codes))
				return true;
	}
	return false;
}

// --------------------------------------------------------------------------- 0x401380

C2DInGameMenu::C2DInGameMenu()
{
	char tag[] = "C2DINGAMEMENU";					// SetName upper-cases its argument in place
	SetName(tag);
	Log("C2DInGameMenu()");
	CLocalGameObject::InitObject();
	FindDevice(2);								// 0x4b3460 (Input.cpp): the first joystick (omhidtc_Joystick)
	v64(0.3f);
	m_object_id = 'NEM2';

	g_screens_db = NULL;
	f_4c8 = NULL;
	f_4cc = 0;
	f_520 = 0;
	f_588 = -1;
	f_4c8 = InitFileStream("omt\\screens.omt");
	g_screens_db = InitDatabase(f_4c8);
	f_4ce = -1;
	CreateMenuPages();											// builds the menu pages
	f_590 = -1;

	// Full screen: most frame sprites are drawn additively (dest blend One).
	const omt_BlendFunc frame_dest = g_windowed ? omblendfc_Inv_Src_Alpha : omblendfc_One;
	const int windowed = g_windowed ? 1 : 0;		// also moves the bottom bar down 1 pixel (sic)

	OMediaCanvasElementWrapper *s;
	g_545854 = NULL;
	s = g_545854 = NewSprite(g_layer_hud2d);
	WideAnchor(s, WIDE_LEFT);					// port: widescreen, top left corner
	s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	s->set_canvas(NULL);
	PlaceSprite(this, s, 32, 0, 0x17);

	s = g_5448e4 = NewSprite(g_layer_hud2d);
	WideAnchor(s, WIDE_LEFT);					// port: widescreen, top left corner
	s->set_blend(omblendfc_Src_Alpha, frame_dest);
	s->set_canvas(NULL);
	PlaceSprite(this, s, 8, 4, 0x16);

	const float sx = (float)(short)g_screen_w * (1.0f / 640.0f);
	const float sy = (float)(short)g_screen_h * (1.0f / 480.0f);

	s = g_545968 = NewSprite(g_layer_hud2d);
	WideAnchor(s, WIDE_LEFT);					// port: widescreen, top left corner
	s->set_blend(omblendfc_Src_Alpha, frame_dest);
	s->set_canvas(NULL);
	PlaceSprite(this, s, 3, 7, 0x15);

	s = g_5445e0 = NewSprite(g_layer_hud2d);
	s->set_blend(omblendfc_Src_Alpha, frame_dest);
	s->set_canvas(NULL);
	PlaceSprite(this, s, 320, 7, 0x15);
	s->hide(true);

	g_54584c = NewSprite(g_layer_hud2d);			// set up below

	f_5b0 = NULL;
	s = f_5b0 = NewSprite(g_layer_hud2d);
	WideAnchor(s, WIDE_RIGHT);					// port: widescreen, the radar disc (top right)
	s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	s->set_canvas(NULL);
	PlaceSprite(this, s, 490, 10, 0x223);
	s->set_diffuse(OMediaFARGBColor(0.4f, 1.0f, 1.0f, 1.0f));
	s->hide(true);

	f_4e8 = -1;

	s = g_54584c;
	s->set_blend(omblendfc_Src_Alpha, frame_dest);
	s->set_canvas(NULL);
	PlaceSprite(this, s, 325, 4, 0x16);
	s->set_size(sx * 30.0f, sy * 50.0f);
	s->hide(true);

	strcpy(g_545e7c, g_545f14);

	// The 14 static elements g_elt_544908[] (a bar at y 340) and their 14 sprites (y 410).
	static const short kEltCanvas[14] =
		{ 0x71, 0x73, 0x6f, 0x70, 0x74, 0x1de, 0x76, 0x1dc, 0x6e, 0x1dd, 0x1db, 0x1d8, 0x1d9, 0x1da };
	for (int i = 0; i < 14; i++)
	{
		SetupElt(g_elt_544908[i], kEltCanvas[i], 0, (short)(windowed + 340));
		g_elt_544908[i].f_34 = 1;
		WideAnchor(&g_elt_544908[i], WIDE_LEFT);	// port: widescreen, the help pictures (bottom left)
	}
	// sprite index, canvas - in the order the original creates them
	static const short kItemSprites[14][2] = {
		{ 0, 0x20 }, { 11, 0x1ef }, { 12, 0x1e7 }, { 2, 0x58 }, { 3, 0x77 }, { 13, 0x1e8 }, { 10, 0x1ee },
		{ 9, 0x1eb }, { 1, 0x57 }, { 7, 0x1e9 }, { 4, 0x1ea }, { 8, 0x1ec }, { 5, 0x1ed }, { 6, 0x142 } };
	const float item_y = (float)(windowed + 410);
	for (int i = 0; i < 14; i++)
	{
		s = g_menu_item_sprites[kItemSprites[i][0]] = new OMediaCanvasElementWrapper;
		WideAnchor(s, WIDE_LEFT);				// port: widescreen, the help icons (bottom left)
		s->link_layer(g_layer_hud2d);
		PlaceSprite(this, s, 0, item_y, kItemSprites[i][1]);
		s->hide(true);
	}
	for (int i = 0; i < 14; i++)
		g_menu_item_sprites[i]->set_blend(omblendfc_Src_Alpha, frame_dest);

	g_interface.Create("HelpMenu", g_screens_db, 0x75, 513, 340, omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha, 1);
	g_help_menu_elt = g_interface.Find("HelpMenu");
	// port: widescreen - the help keys panel (bottom right) and the radar blips go to the right edge
	WideAnchor(g_help_menu_elt, WIDE_RIGHT);
	for (int i = 0; i < 65; i++) WideAnchor(&g_radar_blips[i], WIDE_RIGHT);

	SetupElt(g_elt_5444d8, 0, 0, -1);
	f_504 = 0;
	g_elt_5444d8.Hide();
	f_508 = 0;
	f_50c = 2.0f;
	f_510 = 0;
	f_514 = 0;
	f_518 = 0;
	f_594 = -130;
	f_598 = 0;

	// Inlined "deselect the current item" (f_594 was just set, so the else branch always runs).
	g_elt_544908[g_menu_cur_item].Hide();
	if (f_594 == 0 && g_menu_cur_item != 0)
		g_menu_item_sprites[g_menu_cur_item]->force_show();	// hide count 1, then hide(false)
	else
		g_menu_item_sprites[g_menu_cur_item]->hide(true);
	g_help_menu_elt->Hide();
	f_594 = 0;
	f_59c = 0;

	s = g_5444d0 = NewSprite(g_layer_hud2d);
	PlaceSprite(this, s, 260, 1, 0x67);
	s->set_blend(omblendfc_Src_Alpha, omblendfc_One);
	s->hide(true);

	g_5324e4 = -1;
	f_4dc = 0;

	// Sprites with their blend set first: layer, x, y, canvas.
	struct SpriteDef { OMediaCanvasElementWrapper **p; OMediaLayer *layer; float x, y; int canvas; };
	const SpriteDef kGroup1[] = {
		{ &g_5444d4, g_layer_hud2d, 210, 210, 0x18 },
		{ &g_544744, g_layer_hud2d, 285, 256, 0x1a },
		{ &g_5448e0, g_layer_hud2d, 210, 210, 0xa9 },
		{ &g_5446f0, g_layer_hud2d, 210, 210, 0xf1 },
		{ &g_545848, g_layer_hud2d, 210, 210, 0x113 },
	};
	for (int i = 0; i < COUNTOF(kGroup1); i++)
	{
		const SpriteDef &d = kGroup1[i];
		s = *d.p = NewSprite(d.layer);
		s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		PlaceSprite(this, s, d.x, d.y, d.canvas);
		s->hide(true);
	}

	SetupElt(g_elt_545a70, 0xf4, 410, 20);
	SetupElt(g_elt_545858, 0x1cd, 260, 5);

	const SpriteDef kGroup2[] = {
		{ &g_544904, g_layer_hud2d, 210, 210, 0xf5 },
		{ &g_545c7c, g_layer_hud2d, 210, 210, 0x217 },
		{ &g_5445dc, g_layer_hud2d, 210, 210, 0x218 },
	};
	for (int i = 0; i < COUNTOF(kGroup2); i++)
	{
		const SpriteDef &d = kGroup2[i];
		s = *d.p = NewSprite(d.layer);
		s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		PlaceSprite(this, s, d.x, d.y, d.canvas);
		s->hide(true);
	}

	f_5ac = 0;

	const SpriteDef kGroup3[] = {
		{ &g_545a60, g_layer_top2d, 210, 210, 0x228 },
		{ &g_5446ec, g_layer_hud2d, 210, 210, 0x21f },
		{ &g_5444bc, g_layer_top2d, 407, (float)g_532500, 0x221 },
		{ &g_5444c4, g_layer_top2d, 41, 221, 0x222 },
		{ &g_545850, g_layer_hud2d, 210, 210, 0x7b },
		{ &g_545a68, g_layer_hud2d, 210, 252, 0xf2 },
		{ &g_544748, g_layer_hud2d, 325, 252, 0xf3 },
	};
	for (int i = 0; i < COUNTOF(kGroup3); i++)
	{
		const SpriteDef &d = kGroup3[i];
		s = *d.p = NewSprite(d.layer);
		s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		PlaceSprite(this, s, d.x, d.y, d.canvas);
		s->hide(true);
	}

	g_545dd4 = 0;
	f_4ec = 0;

	// Placed first, then blend, hidden, diffuse opaque white.
	const SpriteDef kGroup4[] = {
		{ &g_5448f0, g_layer_hud2d, 150, 3, 0x15a },
		{ &g_5448f4, g_layer_hud2d, 150, 3, 0x15b },
		{ &g_5448fc, g_layer_hud2d, 150, 3, 0x15c },
		{ &g_54472c, g_layer_hud2d, 290, 3, 0x159 },
		{ &g_544730, g_layer_hud2d, 290, 3, 0x154 },
		{ &g_544734, g_layer_hud2d, 290, 3, 0x155 },
		{ &g_544738, g_layer_hud2d, 290, 3, 0x156 },
		{ &g_54473c, g_layer_hud2d, 290, 3, 0x157 },
		{ &g_544740, g_layer_hud2d, 290, 3, 0x158 },
	};
	for (int i = 0; i < COUNTOF(kGroup4); i++)
	{
		const SpriteDef &d = kGroup4[i];
		s = *d.p = NewSprite(d.layer);
		PlaceSprite(this, s, d.x, d.y, d.canvas);
		s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
		s->hide(true);
		s->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// Named interface elements: name, canvas, x, y, last Create argument.
	static const struct { const char *name; short canvas, x, y; int f_34; } kElts[] = {
		{ "LineGlow", 0x165, 100, 356, 0 },
		{ "TextBackdrop", 0x177, 110, 100, 1 },
		{ "LetterBoxTop", 0x15d, 0, -65, 0 },
		{ "LetterBoxBottom", 0x15e, 0, 481, 0 },
		{ "TextBackdrop2", 0x177, 110, 193, 1 },
		{ "Cindy", 0x168, 414, 104, 1 },
		{ "CindyWire", 0x169, 414, 104, 1 },
		{ "Carl", 0x19d, 414, 104, 1 },
		{ "CarlWire", 0x19c, 414, 104, 1 },
		{ "Resume", 0x187, 414, 104, 1 },
		{ "ResumeWire", 0x197, 414, 104, 1 },
		{ "Exit", 0x188, 414, 104, 1 },
		{ "ExitWire", 0x199, 414, 104, 1 },
		{ "Options", 0x189, 414, 104, 1 },
		{ "OptionsWire", 0x19a, 414, 104, 1 },
		{ "Pickups", 0x18a, 414, 104, 1 },
		{ "PickupsWire", 0x196, 414, 104, 1 },
		{ "Credits", 0x18b, 414, 104, 1 },
		{ "CreditsWire", 0x192, 414, 104, 1 },
		{ "Pix4A", 0x166, 110, 157, 1 },
		{ "Pix4B", 0x166, 378, 157, 1 },
		{ "Pix4C", 0x166, 378, 217, 1 },
		{ "Pix4D", 0x166, 110, 217, 1 },
		{ "Pix4E", 0x166, 410, 100, 1 },
		{ "Pix4F", 0x166, 530, 100, 1 },
		{ "Pix4G", 0x166, 530, 220, 1 },
		{ "Pix4H", 0x166, 410, 100, 1 },
		{ "CursorLine", 0x176, 110, 100, 0 },
		{ "Cursor1", 0x175, 110, 97, 0 },
		{ "LS", 0x185, 414, 104, 1 },
		{ "LSWire", 0x190, 414, 104, 1 },
		{ "MainMenuPDA", 0xd9, 0, 0, 1 },
		{ "Johnny", 0x1a1, 414, 104, 1 },
		{ "JohnnyWire", 0x1a0, 414, 104, 1 },
		{ "Fowl", 0x19f, 414, 104, 1 },
		{ "FowlWire", 0x19e, 414, 104, 1 },
		{ "MomDad", 0x1a3, 414, 104, 1 },
		{ "MomDadWire", 0x1a2, 414, 104, 1 },
		{ "Dino", 0x1cc, 414, 104, 1 },
		{ "DinoWire", 0x1cb, 414, 104, 1 },
		{ "HorizBlack", 0x186, 414, 104, 0 },
		{ "Brain", 0x1bb, 814, 104, 0 },
		{ "Formula1", 0x1b7, 814, 104, 0 },
		{ "Formula2", 0x1b8, 814, 104, 0 },
		{ "Formula3", 0x1b9, 814, 104, 0 },
		{ "Formula4", 0x1ba, 814, 104, 0 },
		{ "Pic_Cola", 0x1bc, 814, 104, 0 },
		{ "Pic_Electromagnet", 0x1bd, 814, 104, 0 },
		{ "Pic_Balloon", 0x1bf, 814, 104, 0 },
		{ "Pic_Flurpbottle", 0x1d0, 814, 104, 0 },
		{ "Pic_PVCPipe", 0x1cf, 814, 104, 0 },
		{ "Pic_Shades", 0x1d1, 814, 104, 0 },
		{ "Pic_Watergun", 0x1d2, 814, 104, 0 },
		{ "Pic_MegaPipe", 0x1d3, 814, 104, 0 },
		{ "Pic_MegaLamp", 0x1d4, 814, 104, 0 },
		{ "Pic_Megaphone", 0x1d5, 814, 104, 0 },
		{ "Pic_Megaburp", 0x1d6, 814, 104, 0 },
	};
	for (int i = 0; i < COUNTOF(kElts); i++)
		g_interface.Create(kElts[i].name, g_screens_db, kElts[i].canvas, kElts[i].x, kElts[i].y,
						   omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha, kElts[i].f_34);

	SetupElt(g_elt_545c80, 0x9c, 425, 204);
	g_elt_545c80.effect.move_dy = -1.0f;
	g_elt_545c80.f_34 = 1;

	// y is (float)(windowed - 1): 0 in a window, -1 full screen.
	const float bar_y = (float)(windowed - 1);
	s = g_545844 = NewSprite(g_layer_hud2d);
	PlaceSprite(this, s, 0, bar_y, 0x220);
	s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	s->hide(true);
	s = g_545a64 = NewSprite(g_layer_hud2d);
	PlaceSprite(this, s, 0, bar_y, 0x220);
	s->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
	s->hide(true);

	SetupElt(g_elt_5445e8, 0x14f, 0, 200);

	v107(0);
	v36(0);
	v52(0);
	strcpy(g_545de4, "TEXTTRIG");
	f_4f8 = 0;
	g_545e48 = 0;
	f_4fc = 0;
	g_545e4c = 0;
	f_500 = -1;
	f_5a4 = f_5a5 = f_5a6 = f_5a7 = 1;
	f_5a8 = f_5a9 = f_5aa = f_5ab = 1;
	f_4d4 = 0;
	f_4d0 = 0;
	g_545dc4 = 0;
	f_4e4 = 0;
	f_4d8 = 0;
	g_5324ec = 1;
	g_5324e8 = 1.0f;
	f_51c = 0;
	f_4f0 = 0;
	f_4f4 = 0;
	g_545e64 = 0;
	f_4e0 = 0;
	f_58c = 0;
	g_menu = this;
	f_5a0 = 0;
	g_545f0e = 0;
	if (ReadKeys()) ErrorLog("ERROR: Reading Keys.txt file");
}

// --------------------------------------------------------------------------- 0x403fa0

C2DInGameMenu::~C2DInGameMenu()
{
	WideAnchorClear();							// port: widescreen anchors (Widescreen.h)
	BillsUiShutdown();							// port: Bill's UI pieces (BillsUi.h)
	g_interface.DestroyAll();

	// Deleted, not cleared (sic: the globals keep dangling pointers). f_5b0 and the canvas
	// elements of the other static interface elements are not deleted.
	OMediaCanvasElementWrapper *const kSprites[] = {
		g_544900, g_5448f8, g_545854, g_5448e4, g_545968, g_5444c8, g_54595c, g_5448e8, g_5445e0,
		g_54584c, g_5444d0, g_5448e0, g_5446f0, g_5444d4, g_545a68, g_544748, g_544744, g_544904,
		g_545c7c, g_5445dc, g_545a60, g_5446ec, g_545850, g_5448f0, g_5448f4, g_5448fc, g_54472c,
		g_544730, g_544734, g_544738, g_54473c, g_544740, g_545848, g_545d84, g_5444bc, g_5444c4,
		g_545844, g_545a64,
	};
	for (int i = 0; i < COUNTOF(kSprites); i++) DeleteSprite(kSprites[i]);
	for (int i = 0; i < 14; i++)
	{
		DeleteSprite(g_elt_544908[i].canvas_elt);	// sic: canvas_elt left dangling
		DeleteSprite(g_menu_item_sprites[i]);
	}

	// UnloadMyMenu for the menu pages (sic: page 1 is not unloaded).
	v324(2);
	v324(0);
	v324(3);
	v324(4);
	v324(5);
	v324(6);
	DeleteAllMenuObject();							// inlined in the original (slot 328)
}

// --------------------------------------------------------------------------- 0x4043d0

void C2DInGameMenu::UnInitObject()
{
	CLocalGameObject::UnInitObject();
	// sic: neither pointer is cleared - a second call would release them again.
	if (g_screens_db) UnInitDatabase(g_screens_db);
	if (f_4c8) UnInitFileStream(f_4c8);
}
