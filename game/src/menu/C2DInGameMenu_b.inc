// C2DInGameMenu part b: slot declarations (inside the class). See docs/decomp-guide.md.
// Per-frame update (Process), game-mode changes, the menu pages and their items, sprite helpers.

	// ---- menu pages. Seven pages (g_545da8[7]); CreateMenuPages (slot 295) builds pages 0, 2, 4,
	// 5 and 6 (page 1 and 3 are never allocated there). A page has an optional background sprite
	// and 50 items; an item is shown when its `state` is not 0 (and not 4).
	struct MenuItem											// 0x28 bytes, constructor 0x408b60
	{
		OMediaCanvasElementWrapper	*elt_alt;		// 0x00  sprite of canvas_alt (hidden by DisplayMenu)
		OMediaCanvasElementWrapper	*elt;			// 0x04  sprite of canvas (shown by DisplayMenu)
		short						 f_08;			// 0x08
		float						 x;				// 0x0c  position, 640x480 space
		float						 y;				// 0x10
		float						 f_14;			// 0x14  = 0
		float						 f_18;			// 0x18  = 1
		short						 state;			// 0x1c  0 = not shown (SetItemState / 0x404dd0)
		short						 w;				// 0x1e  sprite size after SetupSprite
		short						 h;				// 0x20
		short						 canvas_alt;	// 0x22  -1 = none
		short						 canvas;		// 0x24  -1 = none
		short						 f_26;			// 0x26

		MenuItem() : elt_alt(0), elt(0), f_08(0), x(0), y(0), f_14(0), f_18(1.0f), state(0), w(0), h(0),
					 canvas_alt(-1), canvas(-1), f_26(0) {}
	};
	enum { MENU_PAGES = 7, MENU_ITEMS = 50 };
	struct MenuPage											// 0x7d8 bytes
	{
		OMediaCanvasElementWrapper	*bg;			// 0x00  background sprite (always NULL: see CreateMenuPages)
		short						 canvas;		// 0x04  canvas of bg
		MenuItem					 items[MENU_ITEMS];	// 0x08
	};
	// The page table g_545da8[7] (pointer slots). Page 2 = pickups, 3 = keys/level items.
	static MenuPage *GetPage(int i) { return (MenuPage *)g_545da8[i]; }

	virtual void Process(float dt);																	// SLOT 240 0x407740
	virtual void v260();																			// SLOT 260 0x40ac30
	virtual void v270(int old_mode, int new_mode);													// SLOT 270 0x40b460
	virtual void UpdatePulse(float dt);																// SLOT 286 0x40b240
	virtual void ResetInterface();																	// SLOT 287 0x405200
	virtual void v288();																			// SLOT 288 0x404480
	virtual void PositionSprite(OMediaCanvasElementWrapper *e, float x, float y);						// SLOT 289 0x4044c0
	virtual void SetupSprite(OMediaCanvasElementWrapper *e, float x, float y, int canvas_id, float unused, bool scaled, float z);	// SLOT 290 0x404570
	virtual void SetSpriteCanvas(OMediaCanvasElementWrapper *e, int canvas_id, bool scaled);		// SLOT 291 0x404650
	virtual OMediaCanvas *LoadMenuCanvas(int canvas_id);											// SLOT 292 0x404710
	virtual void ActivateActiveMenu(short page);													// SLOT 293 0x40ed60
	virtual void DeactivateActiveMenu();															// SLOT 294 0x40eeb0
	virtual void CreateMenuPages();																	// SLOT 295 0x407c70
	virtual void ShowCurrentMenu();																	// SLOT 296 0x404740
	virtual void DisplayMenu(MenuPage *page);														// SLOT 297 0x408e40
	virtual void HideCurrentMenu();																	// SLOT 298 0x404770
	virtual void HideMenu(MenuPage *page);															// SLOT 299 0x408df0
	virtual void SetMenuItem(MenuPage *page, short index, short canvas_alt, short canvas, short x, short y);	// SLOT 300 0x409980
	virtual void PlayPdaIntro();																	// SLOT 303 0x405740
	virtual void UpdateIndicator(float dt);															// SLOT 311 0x40ad90

	// TYPE 0x4e0 float
	// TYPE 0x4f0 float
	// TYPE 0x4f4 float
	// TYPE 0x590 int
	// TYPE 0x594 int
	// TYPE 0x598 float
	// TYPE 0x5a0 float
	// TYPE 0x5ac float
