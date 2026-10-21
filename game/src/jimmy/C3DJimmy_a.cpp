/*  C3DJimmy, part A: the slots below 0x448b40 (see C3DJimmy.h). */
#include "../C3DJimmy.h"
#include "../C3DPermanentSprite.h"
#include "../C2DInGameMenu.h"
#include "../CJimmyGame.h"
#include "../CTaskList.h"
#include "../objects/C3DStartPoint.h"
#include "../Game.h"
#include "../Bridge.h"
#include "../Todo.h"
#include "../Sound.h"
#include "../SaveGame.h"
#include "../DatabaseCache.h"
#include "../InterfaceElement.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x404ef0 (menu helper, copied as in SaveGame.cpp): item `item` of menu page `page` disabled.
static void ClearItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::Page(page)) != NULL)
		p->items[item].type = 0;
}

static CJimmyGame *JimmyGame(void)
{
	return g_gametype ? dynamic_cast<CJimmyGame *>(g_gametype) : NULL;
}

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'L1_A').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// 0x48fd30 (thiscall, `this` unused): count *t down by dt; true while it is still running.
static bool CountDown(float *t, float dt)
{
	if (*t <= 0) return false;
	*t -= dt;
	if (*t > 0) return true;
	*t = 0;
	return false;
}

static void StopSound(int *channel)						// 0x48f0d0(channel, 0); channel = -1
{
	if (*channel != -1)
	{
		StopGameSound(*channel, false);
		*channel = -1;
	}
}

static OMedia3DVector Vec(const float p[4]) { return OMedia3DVector(p[0], p[1], p[2], p[3]); }

// --------------------------------------------------------------------------- PART B
// Slots of part B (jimmy/C3DJimmy_b.cpp) this part calls: weak stand-ins until it is merged.
__attribute__((weak)) void C3DJimmy::ev138(CGameObject *o, CGameObject *who, int item, float value)
{
	(void)o; (void)who; (void)item; (void)value;
	NEG_TODO(0x449df0, "C3DJimmy::ActivateMenuItem (ESLOT 138, part B)");
}
__attribute__((weak)) void C3DJimmy::ev152(int which)
{
	(void)which;
	NEG_TODO(0x44d9e0, "C3DJimmy ESLOT 152 (part B)");
}
__attribute__((weak)) void C3DJimmy::ev153(int set)
{
	(void)set;
	NEG_TODO(0x44c080, "C3DJimmy ESLOT 153 surface sound (part B)");
}

// --------------------------------------------------------------------------- element family

void C3DJimmy::ev60(void)									// 0x4369f0
{
	ev58((char *)"IDLE", 1);
}

void C3DJimmy::ev132(void)									// 0x446b40
{
	if (f_a68 == 1)
	{
		f_a8c = 0;
		ev58((char *)"FALL", 1);
		JimmyJetpackOff(static_cast<CGameObject *>(this));	// 0x443370 on f_ccc
		return;
	}
	if (f_a68 == 5)
	{
		f_a8c = 0;
		ev58((char *)"FALL", 1);
		f_968 = 600.0f;
		f_96c = f_96c * 0.5f;
	}
}

void C3DJimmy::ev146(float dt, OMedia3DVector pos)			// 0x446100
{
	if (!f_d44) return;
	if (CountDown(&f_4da4, dt))								// 0x48fd30
	{
		f_d44->v136(0);										// +0x220
		f_d44->v196(pos.x, pos.y, pos.z, pos.w);			// +0x310
		float r[4];
		v201(r);											// +0x324
		f_d44->v202(r[0], r[1], r[2], r[3]);				// +0x328
		if (f_4da8 > 0)
		{
			f_d44->ev71(f_4da8 * f_4da4 / f_4da0);			// element +0x11c
			return;
		}
	}
	f_d44->HideAgain();										// element +0xdc
}

void C3DJimmy::ev147(float time)							// 0x446010
{
	// (the original calls through a NULL f_d44 when there is no bubble)
	if (f_d44)
	{
		if (g_cur_gadget == 1 || g_cur_gadget == 9) f_d44->ev58((char *)"ANIM2", 1);
		else if (g_cur_gadget == 5) f_d44->ev58((char *)"ANIM1", 1);
		else f_d44->ev58((char *)"ANIMATE", 1);
	}
	f_4da8 = 0.35f;
	f_4da0 = time;
	f_4da4 = time;
	if (f_d44) f_d44->ShowAgain();							// element +0xd8
}

void C3DJimmy::ev154(void)									// 0x447c50
{
	if (++f_4d54 > 19) f_4d54 = 0;
}

void C3DJimmy::ev155(void)									// 0x447c70
{
	if (++f_4d60 > 0) f_4d60 = 0;
}

// The colour of a C3DMagicParticle (+0x600..+0x608).
static void SetParticleColour(C3DPermanentSprite *p, float r, float g, float b)
{
	(void)p; (void)r; (void)g; (void)b;
	NEG_TODO(0x447970, "C3DMagicParticle colour (+0x600..+0x608)");
}

void C3DJimmy::ev156(float t, float r, float g, float b)	// 0x447950
{
	f_4d58 = t + f_4d58;
	for (int i = 0; i < 20; i++)
		if (f_4d00[i])
		{
			SetParticleColour(f_4d00[i], r, g, b);
			f_4d5c = 0;
		}
}

void C3DJimmy::ev157(float t, float r, float g, float b, OMedia3DVector pos)	// 0x4478d0
{
	f_4d6c.x = pos.x;
	f_4d6c.y = pos.y;
	f_4d64 = t + f_4d64;
	f_4d6c.z = pos.z;
	f_4d6c.w = pos.w;
	if (f_4d50[0])
	{
		SetParticleColour(f_4d50[0], r, g, b);
		f_4d68 = 0;
	}
}

void C3DJimmy::ev165(CGameObject *start)					// 0x445d20
{
	f_4d7c = 0;
	InitPhysics();											// +0x2c
	float p[4], r[4];
	start->v88(p);											// +0x160: its position
	f_4cbc.x = p[0]; f_4cbc.y = p[1]; f_4cbc.z = p[2]; f_4cbc.w = p[3];
	start->v88(p);
	v196(p[0], p[1], p[2], p[3]);							// +0x310
	start->v88(p);
	v90(p[0], p[1], p[2]);									// +0x168
	start->v91(r);											// +0x16c: its rotation
	v92(r[0], r[1], r[2]);									// +0x170
	if (f_d5c)
	{
		start->v88(p);
		f_d5c->v89(p[0], p[1], p[2], *(unsigned int *)&p[3]);	// Goddard there too (+0x164)
	}
	start->v88(p);
	place(p[0], p[1], p[2]);								// element +0x44
	if (f_bd9)
	{
		// coming back to where PlayerLoadLevel left this level
		f_bd9 = 0;
		v90(f_bdc.x, f_bdc.y, f_bdc.z);
		place(f_bdc.x, f_bdc.y, f_bdc.z);
		if (f_d5c) f_d5c->v89(f_bdc.x, f_bdc.y, f_bdc.z, *(unsigned int *)&f_bdc.w);	// port: NULL-checked
	}
	// The view and the music of the start point (C3DStartPoint +0x600, +0x620, +0x684).
	C3DStartPoint *sp = dynamic_cast<C3DStartPoint *>(start);
	if (sp)
	{
		g_viewport->place(sp->m_viewport_pos[0], sp->m_viewport_pos[1], sp->m_viewport_pos[2]);	// +0x44
		strcpy(f_4c28, sp->m_music_db);
		f_4c18 = sp->m_music_index;
	}
	else
	{
		// port: v260 falls back to the "PHONEBOOTH" object, which is no start point; the original
		// reads the same offsets of it.
		NEG_TODO(0x445f77, "C3DJimmy::ev165 on an object that is not a C3DStartPoint");
	}
}

// --------------------------------------------------------------------------- CGameObject family

void C3DJimmy::InitObject(void)								// 0x444780
{
	Log("C3DJimmy::InitObject()");
	C3DPlayer::InitObject();								// 0x460470
	InitAnim();												// element +0x110
	InitModel(std::string("jimmybase.grn"));				// element +0x144
	static const char *const anims[][2] = {					// element +0xe0 CreateAnim(name, file)
		{ "DRIVE", "jimmysit.grn" },
		{ "FULLSTOP", "jimmyfullstop.grn" },
		{ "SCOOT", "jimmyscooter.grn" },
		{ "SCOOTSTOP", "jimmyscooterstop.grn" },
		{ "UPPERRUNHOLD", "jimmyupperrunhold.grn" },
		{ "UPPERRUN", "jimmyupperrun.grn" },
		{ "UPPERWALK", "jimmyupperwalk.grn" },
		{ "UPPERSTOP", "jimmyupperstop.grn" },
		{ "UPPERSTOPHOLD", "jimmyupperstophold.grn" },
		{ "UPPERSTOPGUN", "jimmyupperstopgun.grn" },
		{ "UPPERRUNGUN", "jimmyupperrungun.grn" },
		{ "UPPERPOINTGUN", "jimmyupperpointgun.grn" },
		{ "UPPERBAZOOKA", "jimmyupperbazooka.grn" },
		{ "SHOOTGUN", "jimmyshootgun.grn" },
		{ "LOWERRUN", "jimmylowerrun.grn" },
		{ "LOWERWALK", "jimmylowerwalk.grn" },
		{ "LOWERLEFT", "jimmylowerleft.grn" },
		{ "LOWERRIGHT", "jimmylowerright.grn" },
		{ "LOWERSTOP", "jimmylowerstop.grn" },
		{ "LOWERBACK", "jimmylowerbackpedal.grn" },
		{ "FULLRUN", "jimmyfullrun.grn" },
		{ "FULLLEFT", "jimmyfullleft.grn" },
		{ "FULLRIGHT", "jimmyfullright.grn" },
		{ "FULLRUNGUN", "jimmyfullrungun.grn" },
		{ "FULLGUNPOINT", "jimmyfullgunpoint.grn" },
		{ "FULLGUNSHOOT", "jimmyfullgunshoot.grn" },
		{ "FULLRIGHTHOLDGUN", "jimmyfullrightholdgun.grn" },
		{ "FULLLEFTHOLDGUN", "jimmyfullleftholdgun.grn" },
		{ "FULLSTOPHOLD", "jimmyfullstophold.grn" },
		{ "FULLRUNHOLD", "jimmyfullrunhold.grn" },
		{ "FULLLEFTHOLD", "jimmyfulllefthold.grn" },
		{ "FULLRIGHTHOLD", "jimmyfullrighthold.grn" },
		{ "FULLBACK", "jimmyfullbackpedal.grn" },
		{ "FULLBURPSTOP", "jimmyfullburpgunstop.grn" },
		{ "FULLBURPRUN", "jimmyfullrunburpgun.grn" },
		{ "FULLBURPRIGHT", "jimmyfullrightburpgun.grn" },
		{ "FULLBURPLEFT", "jimmyfullleftburpgun.grn" },
		{ "SLIDE", "jimmyslide.grn" },
		{ "JUMPGUN", "jimmyjumpgun.grn" },
		{ "JUMP", "jimmyjump.grn" },
		{ "JUMPJOY", "jimmyjumpjoy.grn" },
		{ "FALL", "jimmyfall.grn" },
		{ "FLY", "jimmyfly.grn" },							// (the name is the shared string 0x533460)
		{ "LEVER", "jimmylever.grn" },
		{ "SWIM", "jimmyswim.grn" },
		{ "FLOAT", "jimmyfloat.grn" },
		{ "SITWAVE", "jimmysitwave.grn" },
		{ "SHOCK", "jimmyshock.grn" },
		{ "HIT", "jimmyhit.grn" },							// (0x53612c)
		{ "SWING", "jimmyswing.grn" },
		{ "INVENT", "jimmyinvent.grn" },
		{ "AFTERBURN", "jimmyafterburn.grn" },
		{ "LADDER", "jimmyladder.grn" },
		{ "FENCE", "jimmyfence.grn" },
		{ "TYPE", "jimmybuttons.grn" },
		{ "PICKUP", "jimmypickup.grn" },
		{ "PICKUPFLOOR", "jimmypickupfloor.grn" },
		{ "PICKUPGLASSES", "jimmypickupglasses.grn" },
		{ "PUTAWAY", "jimmyputinbackpack.grn" },
		{ "GET", "jimmypickup2.grn" },						// (0x536db0)
		{ "GETITEM", "jimmygetfrombackpack.grn" },
		{ "GETPDA", "jimmygetpda.grn" },
		{ "LOOKPDA", "jimmylookpda.grn" },
		{ "TALKPDA", "jimmytalkpda.grn" },
		{ "TALK", "jimmyfulltalk.grn" },
		{ "NO", "jimmyno.grn" },							// (0x53780c)
		{ "PANIC", "jimmysitpanic.grn" },
		{ "FACESTOP", "jimmyfacestop.grn" },
		{ "TALKBODY", "jimmytalkbody.grn" },
		{ "TALKHERMAN", "jimmytalkherman.grn" },
		{ "WORRYIDLE", "jimmyscared.grn" },
		{ "WORRYTALK", "jimmyscaredtalk.grn" },
		{ "PROUD", "jimmyproud.grn" },
		{ "EXCITETALK", "jimmytalkexcited.grn" },
		{ "EXCITEIDLE", "jimmyExcited.grn" },
		{ "ARGGH", "jimmyAhhrrgg.grn" },
		{ "YAWN", "jimmyawn.grn" },
		{ "ORDER", "jimmyorderpizza.grn" },
		{ "IDLE", "jimmyidle.grn" },
		{ "HANGTALK", "jimmyspork.grn" },
		{ "CLIMBROPE", "jimmyclimbrope.grn" },
		{ "GIVEFLURP", "jimmygiveflurp.grn" },
	};
	for (unsigned i = 0; i < sizeof(anims) / sizeof(anims[0]); i++)
		CreateAnim((char *)anims[i][0], (char *)anims[i][1]);
	f_594 = 0;
	ev58((char *)"STOP", 1);
}

void C3DJimmy::UnInitObject(void)							// 0x444750
{
	C3DAnimated::UnInitObject();							// 0x41ab10
	if (g_active_sound_db)
	{
		CloseDatabase("SoundEffects.omt");					// 0x4a4730
		g_active_sound_db = NULL;
		g_sfx_db_loaded = false;
	}
}

void C3DJimmy::v242(float t)								// 0x445540
{
	(void)t;
	if (f_4c10 != -1)
	{
		StopGameSound(f_4c10, false);						// 0x48f0d0
		f_4c10 = -1;
		StopAllChannels();									// 0x4b8990
	}
	if (g_56adf2)
	{
		memset(g_object_states, 0, 0x1068 * sizeof(int));
		for (int i = 0; i < 50; i++) ClearItemState(2, (short)i);	// 0x404ef0
	}
}

void C3DJimmy::v258(void)									// 0x445510
{
	C3DPlayer::v258();										// 0x435ea0
	g_gametype->SetCamera(static_cast<CGameObject *>(this));	// +0x124
}

void C3DJimmy::v260(void)									// 0x445740
{
	// (debug log "Calling PostLoad for C3DJimmy()")
	f_d30 = NULL;											// (cleared before it is looked at below)
	f_d6d = 0;
	f_4c04 = 0;
	f_4c94 = 0;
	f_4c98 = 0;
	f_4cfd = 0;
	g_sfx_db_loaded = false;
	g_active_sound_db = GetDatabase("SoundEffects.omt");	// 0x4a4180
	f_4ef0 = NULL;
	v67(60.0f);												// +0x10c
	v64(1.265f);											// +0x100
	f_bec = 0;
	g_sound_falloff = 0.02f;
	g_54a498 = 0;
	f_4ab = 0;
	f_4cb8 = 0;
	f_4ca5 = 0;
	f_4ca4 = 0;
	ev79(0);												// element +0x13c: animation not paused
	g_music_volume = g_6f7518;
	f_4ef0 = NULL;
	if (f_d5c) f_d5c->ev79(0);
	// drop the props on Jimmy's bones
	C3DAnimated *props[5] = { f_d28, f_d2c, f_d30, f_d38, f_d34 };
	for (int i = 0; i < 5; i++)
		if (props[i])
		{
			props[i]->HideAgain();							// element +0xdc
			ev88((char *)"none", static_cast<CGameObject *>(props[i]));	// element +0x160
		}
	if (f_d20) f_d20->HideAgain();							// port: NULL-checked (the hook always exists)
	if (f_d44)
	{
		f_d44->f_8ac = 0;									// "CanClip"
		f_d44->HideAgain();
		f_d44->v52(0);										// +0xd0
		f_d44->v46(0);										// +0xb8
	}
	if (g_active_sound_db) g_sfx_db_loaded = true;
	StopSound(&f_4c10);
	StopSound(&f_4c24);
	f_a8c = 0;
	C3DPlayer::v260();										// 0x4637d0
	ev51();													// element +0xcc: texture filtering on
	if (LevelID() == 'LV5A') g_sound_falloff = 0.09f;
	if (LevelID() == 'LV3D') g_sound_falloff = 0.09f;
	if (LevelID() == 'LV3A') SetTaskState("SCENE", 200);	// 0x497980
	if (strcasecmp(f_a18, "none") != 0) ev125(f_a18);		// element +0x1f4

	// the start point named by "StartPoint" (the last one of that tag)
	CGameObject *start = NULL;
	bool found = false;
	if (strcasecmp(f_9c8, g_empty_string) != 0)
	{
		for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		{
			CGameObject *o = *i;
			if (o && o->NameContains((char *)"C3DSTARTPOINT") && strcasecmp(f_9c8, o->m_tag) == 0)
			{
				found = true;
				start = o;
			}
		}
	}
	if (found) ev165(start);								// element +0x294
	else
	{
		ErrorLog("ERROR: Startpoint not found, SP=%s", f_9c8);
		CGameObject *booth = FindObjectByTag("PHONEBOOTH");	// 0x4aefb0
		if (booth) ev165(booth);
		else ErrorLog("ERROR: Startpoint not found and PHONEBOOTH not found");
	}

	OMedia3DVector cam = v224(0, 200.0f, -300.0f);			// +0x380
	g_viewport->place(cam.x, cam.y, cam.z);					// +0x44
	if (f_d5c) f_d5c->v92(0, 0, 0);							// +0x170
	if (f_d4c)
	{
		f_d4c->HideAgain();
		f_d4c->ev91(1);										// element +0x16c
	}
	ev138(static_cast<CGameObject *>(this), static_cast<CGameObject *>(this), 0, 0.0f);	// element +0x228: nothing in hand
	g_537634 = -1.0f;
	g_53763c = -1.0f;
	if (LevelID() == 'LV5C') ev152(1);						// element +0x260
	else if (LevelID() == 'LV5F') ev152(0);
	v261();													// +0x414
	if (f_d5c)
	{
		OMedia3DVector p = v224(0, 0, -150.0f);
		f_d5c->v89(p.x, p.y, p.z, *(unsigned int *)&p.w);	// Goddard behind Jimmy (+0x164)
	}
	if (f_cc8) NEG_TODO(0x445ce5, "C3DRocketShip +0x9f4: show it (element +0xd8) when set");
	if (g_menu) g_menu->ShowHudPicture(-1, false, 0, 0);	// +0x4f4
}

void C3DJimmy::v266(void)									// 0x4455a0
{
	f_4d94 = 0;
	switch (LevelID())
	{
		case 'LV5F': ev152(0); f_4d94 = 0; break;			// element +0x260
		case 'LV4B': break;
		case 'LV5C': ev152(1); f_4d94 = 15; break;
		case 'VR01': ev152(2); break;
		default: ev152(0); break;
	}
	if (LevelID() >= 'L1_A' && LevelID() <= 'L1_C') LoadSound(std::string("water stream"));	// 0x4b82f0
}

void C3DJimmy::v277(void)									// 0x447660
{
	C3DAnimated::v277();									// 0x41ce80
	if (CJimmyGame *g = JimmyGame()) g->StopLevelMusicKeepName();	// +0x1a0
	if (f_4c0c != -1) StopGameSound(f_4c0c, false);			// (the channels are kept for v278)
	g_elt_545a70.Hide();									// 0x40b280(0)
	if (f_4c24 != -1) StopGameSound(f_4c24, false);
	if (f_4c08 != -1) StopGameSound(f_4c08, false);
	if (f_4c10 != -1)
	{
		StopGameSound(f_4c10, false);
		f_4c10 = -1;
		if (f_cbc) f_cbc->HideAgain();						// +0xdc
	}
	if (f_4c1c != -1) StopGameSound(f_4c1c, false);
	if (f_4c20 != -1) StopGameSound(f_4c20, false);
	if (f_4c14 != -1) StopGameSound(f_4c14, false);
	if (f_cc8) NEG_TODO(0x447736, "C3DRocketShip sounds +0x123c / +0x1240 stop");
}

void C3DJimmy::v278(void)									// 0x447780
{
	C3DAnimated::v278();									// 0x41cea0
	if (CJimmyGame *g = JimmyGame()) g->PlayLevelMusic(f_4c28, f_4c18);	// +0x198
	if (f_4c0c != -1) f_4c0c = PlayGameSound(-1, std::string("water stream"), true);	// 0x48f420
	if (f_4c24 != -1) f_4c24 = PlayGameSound(-1, std::string("not available"), true);
	if (f_4c08 != -1) f_4c08 = PlayGameSound(-1, std::string("ropeswing"), false);
	if (f_4c1c != -1) f_4c1c = PlayGameSound(-1, std::string("gravitybubble"), true);
	if (f_4c20 != -1)
	{
		CJimmyGame *g = JimmyGame();
		if (g) ev153(g->m_surface_sound_next);				// element +0x264 (g_gametype +0x560)
	}
	if (f_cc8) NEG_TODO(0x447898, "C3DRocketShip +0x123c: restart its engine sound (element +0x1b0)(2)");
}
