	// ---- C3DJimmy part A: the slots below 0x448b40 (jimmy/C3DJimmy_a.cpp)

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60  0x4369f0 (no animation of that name) ev58("IDLE", 1)
	virtual void ev127(void) {}												// ESLOT 127 0x463890 (empty)
	virtual void ev131(void) {}												// ESLOT 131 0x463890 (empty)
	virtual void ev132(void);												// ESLOT 132 0x446b40 jetpack (state 1) / glide (5) over: FALL
	virtual void ev133(float dt);											// ESLOT 133 0x4473f0 hurt / stunned flashes (f_4cb8, f_4ca4), the knock-back (f_4ca5)
	virtual void ev135(float dt);											// ESLOT 135 0x446bc0 rising into a ceiling: bounce down, "HitHead"
	virtual void ev136(float dt);											// ESLOT 136 0x446d40 the shadow under Jimmy; slopes: turn and slide (f_c04)
	virtual void ev145(float dt);											// ESLOT 145 0x4468f0 gadget 2 (the power bar f_4ccc / f_4cd0)
	virtual void ev146(float dt, OMedia3DVector pos);						// ESLOT 146 0x446100 the bubble (f_d44) at pos while its time lasts, fading
	virtual void ev147(float time);											// ESLOT 147 0x446010 show the bubble for `time` s (animation by the gadget)
	virtual void ev154(void);												// ESLOT 154 0x447c50 next sparkle of f_4d00 (0..19)
	virtual void ev155(void);												// ESLOT 155 0x447c70 next sparkle of f_4d50 (only 0)
	virtual void ev156(float t, float r, float g, float b);					// ESLOT 156 0x447950 f_4d58 += t; colour of all f_4d00 sparkles
	virtual void ev157(float t, float r, float g, float b, OMedia3DVector pos);	// ESLOT 157 0x4478d0 f_4d64 += t, f_4d6c = pos; colour of f_4d50
	virtual void ev163(void);												// ESLOT 163 0x447c90 aim the grappling hook (f_d20) by the camera pitch f_ae8
	virtual void ev164(float dt);											// ESLOT 164 0x4488e0 Goddard's target; the point cursor over the nearest pickup
	virtual void ev165(CGameObject *start);									// ESLOT 165 0x445d20 enter the level at a start point

	// ---- not virtual
	void Slide(float dt);													// 0x4479b0 sliding down slopes (f_c04 / f_c08), "slide"

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x444780 the Granny model and animations
	virtual void UnInitObject(void);										// SLOT 8   0x444750 + close SoundEffects.omt
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x444fc0 bumped into: pendulum, balloon, missile, bots, plants, buttons
	virtual void Process(float dt);											// SLOT 240 0x446260
	virtual void v243(float dt);											// SLOT 243 0x447e40 control: gadgets, jetpack, landing, sliding
	virtual void v242(float t);												// SLOT 242 0x445540 stop the shrink ray; new game: forget object states / pickups
	virtual void v258(void);												// SLOT 258 0x445510 + become the camera
	virtual void v260(void);												// SLOT 260 0x445740 "PostLoad": reset, go to the start point
	virtual void v266(void);												// SLOT 266 0x4455a0 task states changed: ev152 by level; "water stream"
	virtual void v277(void);												// SLOT 277 0x447660 pause: stop the music and the loops
	virtual void v278(void);												// SLOT 278 0x447780 resume: music and loops again

	// ---- PART B: slots of part B that part A calls, declared here until part B is merged
	// (weak stand-ins in C3DJimmy_a.cpp). Remove these when C3DJimmy_b.inc declares them.
	virtual void ev129(void);												// PART B ESLOT 129 0x44bfa0
	virtual void ev134(float a, int b, float c, unsigned char d);			// PART B ESLOT 134 0x44bd90 hurt (AddTouching: -10, -10, 5, 1)
	virtual void ev138(CGameObject *o, CGameObject *who, int item, float value);	// PART B ESLOT 138 0x449df0 "ActivateMenuItem"
	virtual unsigned char ev140(float dt);									// PART B ESLOT 140 0x44b490 (v243 stops when it answers yes)
	virtual void ev144(void);												// PART B ESLOT 144 0x448b40
	virtual void ev149(float dt, CGameObject *target, C3DObject *pointer);	// PART B ESLOT 149 0x44e420 Goddard shows `target` (pointer: Goddard +0x1b84's element)
	virtual unsigned char ev158(float dt);									// PART B ESLOT 158 0x448de0
	virtual void ev159(float dt);											// PART B ESLOT 159 0x449420
	virtual void ev152(int which);											// PART B ESLOT 152 0x44d9e0
	virtual void ev153(int set);											// PART B ESLOT 153 0x44c080 surface sound set
