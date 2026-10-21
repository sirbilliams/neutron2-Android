// C3DJimmy part b: slot declarations and members (inside the class). See docs/decomp-guide.md and
// the C3DPlayer.h / C3DObject.h header comments for the conventions.
// The methods of Neutron2.exe 0x448b40..0x44e8f0: the gadgets (ActivateMenuItem / ev139), the
// water-gun and burp-gun shots (ev144 / ev148 / ev152), the grappling hook and rope (ev140..ev143),
// damage (ev119 / ev120 / ev134), water (EnterWater / LeaveWater), the footstep sounds (ev115,
// ev116, ev153), Jimmy's animation choice (ev58, ev67, ev162), ChangeState (v270), Read/WriteState.
// Element-family slots are `ev<n>` (`// ESLOT n`); a name from a log string gets an `ev<n>` alias.

	// ---- element-family slots (overrides)
	virtual void ev58(char *name, unsigned char loop);						// ESLOT 58  0x44c3e0 play animation <name> (Jimmy's Granny names; loop unused)
	virtual void ev67(void);												// ESLOT 67  0x44e0b0 "AnimEnded": end the pickup / gadget gestures
	virtual void ev115(void);												// ESLOT 115 0x44b040 start the footstep sound (before RUN)
	virtual void ev116(void);												// ESLOT 116 0x44b060 stop the footstep sound
	virtual void ev118(void) {}												// ESLOT 118 0x44bc30 (f_cc0's empty 0x463890: nothing)
	virtual void ev119(OMedia3DVector plane);								// ESLOT 119 0x44bc40 touched an electric wall (material 120..129): SHOCK
	virtual void ev120(void);												// ESLOT 120 0x44be70 touched a wall of material 110..119: hurt

	// ---- element-family slots (new in C3DJimmy)
	virtual void ev127(void) {}												// ESLOT 127 0x463890 (empty) CJimmyGame: in water, every frame
	virtual void EnterWater(void);											// ESLOT 128 0x44bf20 "Calling EnterWater"
	void ev128(void) { EnterWater(); }
	virtual void LeaveWater(void);											// ESLOT 129 0x44bfa0 "Calling LeaveWater"
	void ev129(void) { LeaveWater(); }
	virtual void ev130(void);												// ESLOT 130 0x44bfe0 fell into water: SPLASH + "pain agh ahh"
	virtual void ev131(void) {}												// ESLOT 131 0x463890 (empty) CJimmyGame: left that water
	virtual void ev134(float energy, int points, float time, unsigned char shield);	// ESLOT 134 0x44bd90 hurt: energy, score, invulnerable for <time> (shield: pickup 20 halves it)
	virtual void ev137(int which);											// ESLOT 137 0x44de40 gadget gestures: 1/4 water gun, 2/5 burp gun, 3 PDA prop
	virtual void ActivateMenuItem(CGameObject *o, CGameObject *who, short item, float value);	// ESLOT 138 0x449df0 "C3DJimmy::ActivateMenuItem()"
	void ev138(CGameObject *o, CGameObject *who, short item, float value) { ActivateMenuItem(o, who, item, value); }
	virtual void ev139(void);												// ESLOT 139 0x449450 deactivate the current gadget (g_menu GetCurrentGadget)
	virtual unsigned char ev140(float dt);									// ESLOT 140 0x44b490 on the rope: swing, climb, hook and shadow (returns 0)
	virtual unsigned char ev141(float dt);									// ESLOT 141 0x449050 grappling hook (gadget 7): fire upwards, or reel in
	virtual void ev142(void);												// ESLOT 142 0x44b3a0 let go of the rope
	virtual void ev143(OMedia3DPoint anchor, OMedia3DVector pos, int swing, float len);	// ESLOT 143 0x448e80 hang on a rope at <anchor>
	virtual void ev144(void);												// ESLOT 144 0x448b40 fire the current gun (water / burp / shrink "click")
	virtual void ev148(float x, float y, float z, float pitch, float speed, float a6, float a7, float a8);	// ESLOT 148 0x44b090 launch the next water balloon from v224(x, y, z)
	virtual void ev149(float spin, CGameObject *target, C3DObject *obj);	// ESLOT 149 0x44e420 put <obj> 300 in front of the camera towards <target> (`this` unused)
	virtual void ev150(void);												// ESLOT 150 0x44c2a0 remember the level id and our position
	virtual void ev151(void);												// ESLOT 151 0x44c2f0 the same + HUD picture 19
	virtual void ev152(int kind);											// ESLOT 152 0x44d9e0 water balloon look: 0 water gun, 1 burp gun, 3 (small)
	virtual void ev153(int material);										// ESLOT 153 0x44c080 footstep sound of the floor material
	virtual unsigned char ev158(float dt);									// ESLOT 158 0x448de0 out of energy: "not available" (1), else 0
	virtual void ev159(float dt);											// ESLOT 159 0x449420 ev160(dt * -0.05) while there is a Goddard
	virtual void ev160(float d);											// ESLOT 160 0x44bed0 f_d60 += d, kept in 0..100
	virtual void ev161(unsigned char show, int which);						// ESLOT 161 0x44e3b0 which 1: show / hide Goddard's f_1b80 object
	virtual unsigned char ev162(char *name, char *anim);					// ESLOT 162 0x44c390 name is anim and not the state yet: state = anim, 1

	// ---- CGameObject-family overrides
	virtual unsigned char v133(void);										// SLOT 133 0x44d650 on the floor: also while f_4d90
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x44bbe0 no wall contact while on the rope
	virtual void v261(void);												// SLOT 261 0x44c360 stop + ev116 + "STOP"
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x44e590 "C3DJimmy::ChangeState"
	virtual void v271(void);												// SLOT 271 0x44e850 power-up over: hook hidden too
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x44d860
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x44d670

	// ---- data members part b uses (object offsets; types as far as part b sees them)
	// FIELD 0x938 40
	char				f_938[40];		// 0x938  (C3DPlayerObject's unused range) animation to go back to after a cut scene (v270)
	// FIELD 0xcbc 4
	C3DObject			*f_cbc;			// 0xcbc  hidden when out of energy (ev158)
	// FIELD 0xcc4 12
	C3DPermanentSprite	*f_cc4;			// 0xcc4  C3DShadow (its size: 0x46ec30)
	C3DPlayerObject		*f_cc8;			// 0xcc8  C3DRocketShip (C3DFlyingObject)
	C3DPlayerObject		*f_ccc;			// 0xccc  C3DJetPack (C3DFlyingObject)
	// FIELD 0xcd0 80
	C3DPermanentSprite	*f_cd0[20];		// 0xcd0  C3DWaterBalloon (the shots)
	// FIELD 0xd20 4
	C3DAnimated			*f_d20;			// 0xd20  C3DGraplingHook
	// FIELD 0xd28 20
	C3DGrannyObj		*f_d28;			// 0xd28  water gun in the right hand ("newgunbase.grn")
	C3DGrannyObj		*f_d2c;			// 0xd2c  burp gun ("megaburpgun.grn")
	C3DGrannyObj		*f_d30;			// 0xd30  shades on the head ("Shades.grn")
	C3DGrannyObj		*f_d34;			// 0xd34  the PDA ("PDA.grn")
	CGameObject			*f_d38;			// 0xd38  held object (the HOLD animations; ReadState clears it)
	// FIELD 0xd50 24
	short				f_d50;			// 0xd50  next water balloon (0..19)
	int					f_d54;			// 0xd54  0 after a shot
	short				f_d58;			// 0xd58
	C3DPlayerObject		*f_d5c;			// 0xd5c  C3DGoddard (C3DAI)
	float				f_d60;			// 0xd60  0..100 (ev160)
	float				f_d64;			// 0xd64  0.5 after leaving the jeep
	// FIELD 0x4bf0 20
	OMedia3DPoint		f_4bf0;			// 0x4bf0 position remembered by ev150 / ev151
	int					f_4c00;			// 0x4c00 level id (g_gametype ObjectID) remembered with it
	// FIELD 0x4c08 12
	int					f_4c08;			// 0x4c08 "ropeclimb" channel (-1)
	int					f_4c0c;			// 0x4c0c "water stream" channel (-1)
	int					f_4c10;			// 0x4c10 channel stopped when out of energy (-1)
	// FIELD 0x4c18 4
	int					f_4c18;			// 0x4c18 level music argument (v270)
	// FIELD 0x4c20 108
	int					f_4c20;			// 0x4c20 footstep channel (-1)
	int					f_4c24;			// 0x4c24 "not available" channel (-1)
	char				f_4c28[100];	// 0x4c28 level music name ("none")
	// FIELD 0x4c9c 11
	float				f_4c9c;			// 0x4c9c 0 when hurt
	int					f_4ca0;			// 0x4ca0 gesture to do when the pickup animation ends (ev137)
	unsigned char		f_4ca4;			// 0x4ca4 hurt by a 110..119 wall (ev120)
	unsigned char		f_4ca5;			// 0x4ca5 shocked (ev119)
	unsigned char		f_4ca6;			// 0x4ca6 in water (CJimmyGame)
	// FIELD 0x4cb8 1
	unsigned char		f_4cb8;			// 0x4cb8 hurt (ev134; cleared by ChangeState)
	// FIELD 0x4ccc 16
	float				f_4ccc;			// 0x4ccc 20 / 0 (gadget 2)
	float				f_4cd0;			// 0x4cd0 burp gun charge (fires at 20)
	unsigned char		f_4cd4;			// 0x4cd4 cleared by a shot
	float				f_4cd8;			// 0x4cd8 aim pitch of the shots
	// FIELD 0x4cdc 4
	float				f_4cdc;			// 0x4cdc invulnerable time after a hurt (5)
	// FIELD 0x4ce8 4
	float				f_4ce8;			// 0x4ce8 no shock while > 0
	// FIELD 0x4cf8 7
	int					f_4cf8;			// 0x4cf8 rope: 1 swing (keys move the swing), 0 hang still
	unsigned char		f_4cfc;			// 0x4cfc grappling hook gadget selected once (12 s help shown)
	unsigned char		f_4cfd;			// 0x4cfd on the scooter (jeep)
	unsigned char		f_4cfe;			// 0x4cfe
	// FIELD 0x4d58 4
	float				f_4d58;			// 0x4d58 (saved)
	// FIELD 0x4d7c 1
	unsigned char		f_4d7c;			// 0x4d7c "bNoPhysics" (driving the crane, saved)
	// FIELD 0x4d90 8
	unsigned char		f_4d90;			// 0x4d90 counts as on the floor (v133)
	int					f_4d94;			// 0x4d94 (saved)
	// FIELD 0x4da0 8
	float				f_4da0;			// 0x4da0 0 when a LADDER / FENCE climb starts
	float				f_4da4;			// 0x4da4 invulnerable while > 0 (saved)
	// FIELD 0x4dac 8
	int					f_4dac;			// 0x4dac "ResetWhenDamaged" (saved)
	float				f_4db0;			// 0x4db0 "FallSpeedDamage" (saved)
	// FIELD 0x4db8 194
	unsigned char		f_4db8;			// 0x4db8 hanging on the rope
	OMedia3DPoint		f_4dbc[10];		// 0x4dbc rope anchor points ([f_4e78])
	float				f_4e5c;			// 0x4e5c first rope length (the hook model is scaled to it once)
	float				f_4e60;			// 0x4e60 rope length now
	float				f_4e64;			// 0x4e64 swing speed x
	float				f_4e68;			// 0x4e68 swing speed z
	float				f_4e6c;			// 0x4e6c time on the rope
	float				f_4e70;			// 0x4e70 anchor height above us - 100 / 200
	float				f_4e74;			// 0x4e74 longest rope
	short				f_4e78;			// 0x4e78 current anchor
	// FIELD 0x4e80 100
	char				f_4e80[100];	// 0x4e80 pickup animation waiting for the floor (ev58)

	// ---- PART A (temporary declarations of part A's slots that part b calls)
	virtual void ev147(float t);											// PART A ESLOT 147 0x446010
