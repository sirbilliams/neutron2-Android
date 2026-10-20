// C2DInGameMenu part d: slot declarations (inside the class). See docs/decomp-guide.md.
// Part d: the dat\menu.dat parser (slots 332..339), the menu texts and the trigger "teletype"
// texts (340), the screen fades (341), the phone-booth level warp (342, 343), the PDA zoom (345)
// and the pickups page (346, 347). Bodies in menu/C2DInGameMenu_d.cpp.


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
