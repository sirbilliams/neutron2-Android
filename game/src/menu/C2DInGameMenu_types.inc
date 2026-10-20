// C2DInGameMenu: types shared by the parts (inside the class). Merged from parts b and c.
	// ---- data shared with the other parts

	// One item of a menu page (0x28 bytes; the page constructor 0x408b60 sets the defaults).
	struct MenuItem
	{
		OMediaCanvasElementWrapper	*rollover;		// 0x00  shown / hidden by UpdateRollovers
		OMediaCanvasElementWrapper	*active;		// 0x04  the item's picture
		short	f_08;								// 0x08
		float	x, y;								// 0x0c  position (640x480 space)
		int		f_14;								// 0x14
		float	f_18;								// 0x18  = 1.0
		short	type;								// 0x1c  0 off, 1 rollover on hover, 2 rollover always,
													//       3 rollover hidden on hover, 4 off; pickups: 0 = not owned
		short	w, h;								// 0x1e  size of the last sprite LoadMyMenu created
		short	rollover_id;						// 0x22  canvas IDs in screens.omt, -1 = none
		short	active_id;							// 0x24
		short	shown;								// 0x26  rollover visible
		MenuItem() : rollover(0), active(0), f_08(0), x(0), y(0), f_14(0), f_18(1.0f), type(0),
					 w(0), h(0), rollover_id(-1), active_id(-1), shown(0) {}
	};
	enum { MENU_ITEMS = 50, MENU_COUNT = 7 };
	// A menu page (0x7d8 bytes), g_menu_pages[MenuIndex(name)], built from the BACKGROUND line.
	struct MenuPage
	{
		OMediaCanvasElementWrapper	*background;	// 0x00
		short	background_id;						// 0x04
		MenuItem items[MENU_ITEMS];					// 0x08
	};
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
		MENU_OPTIONS = 5, MENU_CREDITS = 6
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



	static std::list<MenuObject *> s_objects;		// 0x5444b0 (head node 0x5444b4, count 0x5444b8)
	static MenuPage *&Page(int i) { return g_menu_pages[i]; }	// 0x545da8[i]

	// Save menu: what the header of save\JimmyGame<n>.tsk says (20 bytes each, g_save_slots).
	struct SaveSlotInfo
	{
		int		score;			// 0x00  -1 = no save in this slot
		int		f_04, f_08;
		int		level;			// 0x0c  level 4cc ('LV3D'...)
		int		f_10;			// 0x10
	};

	enum { MENU_PAGES = MENU_COUNT };
	// The page table g_menu_pages[7] (pointer slots). Pages 0, 2, 4, 5, 6 are built by CreateMenuPages;
	// 2 = pickups, 3 = keys/level items.
	static MenuPage *GetPage(int i) { return g_menu_pages[i]; }

