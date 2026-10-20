// C2DInGameMenu part b: slot declarations (inside the class). See docs/decomp-guide.md.
// Per-frame update (Process), game-mode changes, the menu pages and their items, sprite helpers.

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
