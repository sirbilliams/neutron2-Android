// C2DInGameMenu part c: slot declarations (inside the class). See docs/decomp-guide.md.
// Slots 304..331 (not 311): HUD bars, menu pages (load / unload / rollovers / clicks), the main
// menu's item info, the save and options menus, the menu's sound channel.

	// ---- slots
	virtual void ArrangePickups(void);									// SLOT 304 0x408ba0
	virtual void ShowGadgetBar(void);									// SLOT 305 0x404bd0
	virtual void HideGadgetBar(void);									// SLOT 306 0x404c70
	virtual void GadgetBarEmpty(void);									// SLOT 307 0x404ca0
	virtual void GadgetBarFull(void);									// SLOT 308 0x404d10
	virtual unsigned char IsGadgetBarShown(void);						// SLOT 309 0x403f40
	virtual void SetHealthBar(float health);							// SLOT 310 0x404f50
	virtual unsigned char IsCurrentGadget(int gadget);					// SLOT 312 0x403f50
	virtual int  GetCurrentGadget(void);								// SLOT 313 0x403f70
	virtual int  MenuIndex(const char *name);							// SLOT 314 0x40e600
	virtual short UpdateRollovers(short mx, short my);					// SLOT 315 0x405030
	virtual void v316(bool show);										// SLOT 316 0x409fb0
	virtual void ShowHudPicture(int canvas_id, bool show, int x, int y);	// SLOT 317 0x409e20
	virtual void LoadSaveSlot(void);									// SLOT 318 0x40e900
	virtual void SaveToSlot(void);										// SLOT 319 0x40e7a0
	virtual void DeleteSaveSlot(void);									// SLOT 320 0x40e6e0
	virtual void MainMenuAction(short item);							// SLOT 321 0x40faf0
	virtual unsigned char HandleMenuInput(float dt);					// SLOT 322 0x40efa0
	virtual void LoadMyMenu(int menu);									// SLOT 323 0x4047a0
	virtual void UnloadMyMenu(int menu);								// SLOT 324 0x404aa0
	virtual void LoadOptions(const char *path);							// SLOT 325 0x40eb70
	virtual void SaveOptions(const char *path);							// SLOT 326 0x40e9e0
	virtual void v327(bool show);										// SLOT 327 0x404ba0
	virtual void DeleteAllMenuObject(void);								// SLOT 328 0x404400
	virtual void PlayMenuSound(const char *name);						// SLOT 329 0x40b370
	virtual void PlayMenuSoundIndex(int index);							// SLOT 330 0x40b2c0
	virtual void ShowMainMenuInfo(int item);							// SLOT 331 0x405e30

	// The generated names of the renamed slots, for callers in the other parts.
	void v304()								{ ArrangePickups(); }
	void v305()								{ ShowGadgetBar(); }
	void v306()								{ HideGadgetBar(); }
	void v307()								{ GadgetBarEmpty(); }
	void v308()								{ GadgetBarFull(); }
	unsigned char v309()					{ return IsGadgetBarShown(); }
	void v310(float health)					{ SetHealthBar(health); }
	unsigned char v312(int gadget)			{ return IsCurrentGadget(gadget); }
	int  v313()								{ return GetCurrentGadget(); }
	int  v314(const char *name)				{ return MenuIndex(name); }
	short v315(short mx, short my)			{ return UpdateRollovers(mx, my); }
	void v317(int id, bool show, int x, int y)	{ ShowHudPicture(id, show, x, y); }
	void v318()								{ LoadSaveSlot(); }
	void v319()								{ SaveToSlot(); }
	void v320()								{ DeleteSaveSlot(); }
	void v321(short item)					{ MainMenuAction(item); }
	unsigned char v322(float dt)			{ return HandleMenuInput(dt); }
	void v323(int menu)						{ LoadMyMenu(menu); }
	void v324(int menu)						{ UnloadMyMenu(menu); }
	void v325(const char *path)				{ LoadOptions(path); }
	void v326(const char *path)				{ SaveOptions(path); }
	void v328()								{ DeleteAllMenuObject(); }
	void v329(const char *name)				{ PlayMenuSound(name); }
	void v330(int index)					{ PlayMenuSoundIndex(index); }
	void v331(int item)						{ ShowMainMenuInfo(item); }

	// ---- members used by this part
	// TYPE 0x588 int
	// TYPE 0x590 int
	// TYPE 0x5ac float
