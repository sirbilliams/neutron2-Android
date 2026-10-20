// C2DInGameMenu part d: slot declarations (inside the class). See docs/decomp-guide.md.
// Part d: the dat\menu.dat parser (slots 332..339), the menu texts and the trigger "teletype"
// texts (340), the screen fades (341), the phone-booth level warp (342, 343), the PDA zoom (345)
// and the pickups page (346, 347). Bodies in menu/C2DInGameMenu_d.cpp.

	// ---- menu objects, pages and items. Shared by the whole class (the parser here creates them;
	// the constructor, InitMenus v295, DeleteAllMenuObject v328 ... use them too).

	// MenuObject::type - one per menu.dat keyword
	enum
	{
		MOBJ_NONE			= -1,
		MOBJ_BACKGROUND		= 0,	// BACKGROUND,name,page,x,y,canvas						(creates the page)
		MOBJ_TEXTSTRING		= 1,	// TEXTSTRING,name,page,x,y,text,font,align,r,g,b
		MOBJ_MENUITEM		= 2,	// MENUITEM,name,page,x,y,canvas,canvas_hi,id			(fills page item `id`)
		MOBJ_TEXTMENUITEM	= 3,	// TEXTMENUITEM,name,page,dx,dy,text,font,align,r,g,b	(at the MENUITEM `name` + dx,dy)
		MOBJ_HELPMENUITEM	= 4,	// HELPMENUITEM,name,page,x,y,text,font,align,id,r,g,b	(shown while item `id` has f_26)
		MOBJ_TRIGTEXTSTRING	= 5		// TRIGTEXTSTRING,name,trigger,x,y,text,font,align,r,g,b,line
	};

	// Page indices: what v314 returns for the page names of menu.dat (-1 = NO_MENU / unknown).
	enum
	{
		MENU_INV = 0, MENU_PHONE = 1, MENU_PICKUPS = 2, MENU_MAIN = 3, MENU_SAVE = 4,
		MENU_OPTIONS = 5, MENU_CREDITS = 6,
		MENU_PAGES = 7,				// g_menu_pages[7] (0x545da8)
		MENU_ITEMS = 50				// MenuPage::items
	};

	// One parsed menu.dat line (0x134 bytes in the original), in s_objects in file order.
	struct MenuObject
	{
		int		type;			// 0x00  MOBJ_*
		char	name[32];		// 0x04
		int		page;			// 0x24  page index (v314), -1 = none (always -1 for TRIGTEXTSTRING)
		int		x;				// 0x28  640x480 screen position (DrawText / item position)
		int		y;				// 0x2c
		int		canvas;			// 0x30  MENUITEM: canvas id (screens.omt)
		int		canvas_hi;		// 0x34  MENUITEM: second canvas id; BACKGROUND: the page's canvas id
		char	text[128];		// 0x38
		int		font;			// 0xb8  DrawText font: 'SMAL' 'MED' 'LARG' (-1 for anything else)
		int		align;			// 0xbc  0 = CENTERED (drawn centred), 2 = LEFT, 1 = RIGHT
		int		id;				// 0xc0  MENUITEM / HELPMENUITEM: item index; TRIGTEXTSTRING: line number
		float	r, g, b;		// 0xc4  text colour, menu.dat value / 255
		char	trigger[100];	// 0xd0  TRIGTEXTSTRING: name of the trigger group (see g_trig_text_group)

		MenuObject()			// inlined in every Parse* slot
		{
			type = -1; name[0] = 0; page = x = y = canvas = canvas_hi = id = -1;
			text[0] = 0; font = align = -1; r = g = b = 0.0f; trigger[0] = 0;
		}
	};

	// One item of a page (0x28 bytes). Filled by v300 from a MENUITEM line.
	struct MenuItem
	{
		OMediaCanvasElementWrapper	*sprite;	// 0x00  canvas `canvas` of the MENUITEM
		OMediaCanvasElementWrapper	*sprite_hi;	// 0x04  canvas `canvas_hi` (the pickups page draws the count when set)
		short	f_08;			// 0x08
		float	x;				// 0x0c  640x480 position (v300)
		float	y;				// 0x10
		int		f_14;			// 0x14
		float	f_18;			// 0x18  = 1
		short	enabled;		// 0x1c  item available (pickup owned, level open ...): 0x404dd0 / 0x404ef0
		short	w;				// 0x1e  (v300)
		short	h;				// 0x20
		short	canvas;			// 0x22  = -1
		short	canvas_hi;		// 0x24  = -1
		short	f_26;			// 0x26  HELPMENUITEMs of this item are drawn only while it is set

		MenuItem()				// 0x408b60 (destructor 0x41f890 empty)
		{
			sprite = 0; sprite_hi = 0; f_08 = 0; x = 0; y = 0; f_14 = 0; f_18 = 1.0f;
			enabled = 0; w = 0; h = 0; canvas = -1; canvas_hi = -1; f_26 = 0;
		}
	};

	// A page (0x7d8 bytes): created by its BACKGROUND line (ParseBackground) or by InitMenus v295.
	struct MenuPage
	{
		OMediaCanvasElementWrapper	*background;	// 0x00
		short		canvas;							// 0x04  background canvas id (left uninitialised by new)
		MenuItem	items[MENU_ITEMS];				// 0x08

		MenuPage() : background(0) {}
	};

	static std::list<MenuObject *> s_objects;		// 0x5444b0 (head node 0x5444b4, count 0x5444b8)
	static MenuPage *&Page(int i) { return *(MenuPage **)&g_menu_pages[i]; }	// 0x545da8[i]

	// ---- slots
	virtual bool v332();					// SLOT 332 0x40c360  LoadMenuDat: parse dat\menu.dat into s_objects
	virtual bool ParseBackground();			// SLOT 333 0x40c550
	virtual bool ParseTextString();			// SLOT 334 0x40c8f0
	virtual bool ParseMenuItem();			// SLOT 335 0x40d2c0
	virtual bool ParseTextMenuItem();		// SLOT 336 0x40d620
	virtual bool ParseHelpMenuItem();		// SLOT 337 0x40cdb0
	virtual bool ParseTriggerTextString();	// SLOT 338 0x40db70
	virtual void v339();					// SLOT 339 0x40e090  DumpMenuObjects (debug log only: does nothing)
	virtual void v340(float dt);			// SLOT 340 0x40e0e0  DrawMenuTexts: texts of the current page + trigger texts
	virtual void v341(float dt);			// SLOT 341 0x406aa0  UpdateFade: the full-screen fade quads
	virtual void v342(short level);			// SLOT 342 0x40fd90  WarpToLevel (phone booth level select)
	virtual void v343();					// SLOT 343 0x4104a0  phone booth touched: toggle the active menu
	virtual void v345(bool open);			// SLOT 345 0x405ca0  PDA zoom in (open) / out
	virtual void v346();					// SLOT 346 0x408cd0  DrawPickupCounts
	virtual void v347();					// SLOT 347 0x408d50  GiveAllPickups (cheat)
