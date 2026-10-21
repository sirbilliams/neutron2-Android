	// ---- C3DJimmy part A: the slots below 0x448b40 (jimmy/C3DJimmy_a.cpp)

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60  0x4369f0 (no animation of that name) ev58("IDLE", 1)
	virtual void ev127(void) {}												// ESLOT 127 0x463890 (empty)
	virtual void ev131(void) {}												// ESLOT 131 0x463890 (empty)
	virtual void ev132(void);												// ESLOT 132 0x446b40 jetpack (state 1) / glide (5) over: FALL
	virtual void ev146(float dt, OMedia3DVector pos);						// ESLOT 146 0x446100 the bubble (f_d44) at pos while its time lasts, fading
	virtual void ev147(float time);											// ESLOT 147 0x446010 show the bubble for `time` s (animation by the gadget)
	virtual void ev154(void);												// ESLOT 154 0x447c50 next sparkle of f_4d00 (0..19)
	virtual void ev155(void);												// ESLOT 155 0x447c70 next sparkle of f_4d50 (only 0)
	virtual void ev156(float t, float r, float g, float b);					// ESLOT 156 0x447950 f_4d58 += t; colour of all f_4d00 sparkles
	virtual void ev157(float t, float r, float g, float b, OMedia3DVector pos);	// ESLOT 157 0x4478d0 f_4d64 += t, f_4d6c = pos; colour of f_4d50
	virtual void ev165(CGameObject *start);									// ESLOT 165 0x445d20 enter the level at a start point

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x444780 the Granny model and animations
	virtual void UnInitObject(void);										// SLOT 8   0x444750 + close SoundEffects.omt
	virtual void v242(float t);												// SLOT 242 0x445540 stop the shrink ray; new game: forget object states / pickups
	virtual void v258(void);												// SLOT 258 0x445510 + become the camera
	virtual void v260(void);												// SLOT 260 0x445740 "PostLoad": reset, go to the start point
	virtual void v266(void);												// SLOT 266 0x4455a0 task states changed: ev152 by level; "water stream"
	virtual void v277(void);												// SLOT 277 0x447660 pause: stop the music and the loops
	virtual void v278(void);												// SLOT 278 0x447780 resume: music and loops again

	// ---- PART B: slots of part B that part A calls, declared here until part B is merged
	// (weak stand-ins in C3DJimmy_a.cpp). Remove these when C3DJimmy_b.inc declares them.
	virtual void ev138(CGameObject *o, CGameObject *who, int item, float value);	// PART B ESLOT 138 0x449df0 "ActivateMenuItem"
	virtual void ev152(int which);											// PART B ESLOT 152 0x44d9e0
	virtual void ev153(int set);											// PART B ESLOT 153 0x44c080 surface sound set
