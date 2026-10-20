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
	// One parsed line of dat\menu.dat (0x134 bytes), kept in s_menu_objects.
	struct MenuDatEntry
	{
		int		type;			// 0x00
		char	id[32];			// 0x04
		int		menu;			// 0x24  MenuIndex of the menu name
		int		px, py;			// 0x28
		int		f_30;			// 0x30
		int		omtid;			// 0x34
		char	text[128];		// 0x38
		int		f_b8, f_bc, f_c0, f_c4, f_c8, f_cc;
		char	f_d0[100];		// 0xd0
	};
	static std::list<MenuDatEntry *> s_menu_objects;	// 0x5444b4 (count 0x5444b8)
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

