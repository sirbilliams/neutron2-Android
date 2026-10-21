/*  C3DJimmy, part A: the slots below 0x448b40 (see C3DJimmy.h). */
#include "../C3DJimmy.h"
#include "../C3DPermanentSprite.h"
#include "../C3DShadow.h"
#include "../C3DPointCursor.h"
#include "../C3DGrannyObj.h"
#include "../pickups/C3DPickupItem.h"
#include "../C3DRocketShip.h"
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
#include "../Physics.h"
#include "../CanvasWrapper.h"
#include "../levels/CLevel04Games.h"
#include "../lab/C3DBot.h"
#include "OMediaViewPort.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include <math.h>
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

static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// The game's random number generator (as C3DPlayer's): 0..1000.
static int Random1001(void)
{
	unsigned int *seed = g_random_seed;
	*seed = *seed * 0x19660d + 0x7fff;
	return ((int)((*seed >> 16) * 1001) >> 16) & 0xffff;
}

// The end (hit point) of the last ray test (0x6311f8).
static OMedia3DVector RayEnd(void)
{
	return OMedia3DVector(g_phys_ray.end.x, g_phys_ray.end.y, g_phys_ray.end.z, g_phys_ray.end.w);
}

// The material of polygon 0 of an element's shape (2002: shape +0x88 -> polygon +4).
static OMedia3DMaterial *FirstMaterial(OMedia3DShapeElement *e)
{
	OMedia3DShape *sh = e->get_shape();
	omt_PolygonList *polys = sh ? sh->get_polygons() : NULL;
	if (!polys || polys->empty()) return NULL;				// port: the original takes polygon 0 unchecked
	return (*polys->begin()).get_material();
}

static OMedia3DVector Position(CGameObject *o)			// vtable +0x30c
{
	float p[4];
	o->v195(p);
	return Vec(p);
}

static OMedia3DVector Velocity(CGameObject *o)			// vtable +0x2b8
{
	float v[4];
	o->v174(v);
	return Vec(v);
}

// 0x404f20 (menu helper, copied as in SaveGame.cpp): the type of item `item` of page `page`.
static short GetItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p && item >= 0 && item < C2DInGameMenu::MENU_ITEMS) return p->items[item].type;
	return 0;
}

// 0x485730: health (g_health, 0..100) += d; under 19 "low neutron energy" once (g_545fd4)
// until it is over 20 again.
static void AddHealth(float d)
{
	g_health = g_health + d;
	if (g_health > 100.0f)
	{
		g_health = 100.0f;
		g_545fd4 = 0;
		return;
	}
	if (g_health < 0)
	{
		g_health = 0;
		return;
	}
	if (g_health < 19.0f && g_health != 0)
	{
		if (!g_545fd4)
		{
			PlayGameSound(-1, std::string("low neutron energy"), false);	// 0x48f420
			g_545fd4 = 1;
		}
		return;
	}
	if (g_health > 20.0f) g_545fd4 = 0;
}

// 0x404d60 (a menu method, `this` unused): the gadget power bar sprite g_54584c is 20 x
// (1.04 * v) (in 640 x 480 units; v 0..100, negative counts as 0).
static void SetPowerBar(float v)
{
	if (v < 0) v = 0;
	if (g_54584c)
		g_54584c->set_size((float)((double)(short)g_screen_w * (double)0.0015625f * 20.0),
						   (float)((double)(short)g_screen_h * (double)0.0020833334f * v * 1.04));
}

// 0x48f730 (CLocalGameObject family): the nearest object with ObjectID id1 or id2 that is
// active (v123) and f_140, by quick_magnitude of (o - it).
static CGameObject *FindNearest(CGameObject *o, int id1, int id2)
{
	CGameObject *best = NULL;
	float best_d = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *it = *i;
		if (!it || (it->m_object_id != id1 && it->m_object_id != id2)) continue;
		if (!it->v123() || !it->f_140) continue;
		OMedia3DVector a = Position(o), b = Position(it);
		OMedia3DVector d(a.x - b.x, a.y - b.y, a.z - b.z, 0);
		const float m = d.quick_magnitude();
		if (!best || m < best_d)
		{
			best_d = m;
			best = it;
		}
	}
	return best;
}

// Members of helper classes that are not reconstructed yet (their objects are never created
// until they are; see C3DJimmy.h).
static void ParticleBurst(C3DMagicParticle *p, OMedia3DVector pos)	// +0x610 = 1, element slot 55 (+0xdc)(pos)
{
	p->f_610 = 1.0f;										// (set before the camera point in the original)
	p->Burst(pos);
}
// 0x4489ac: the pickup's "ShowArrow" - C3DGrannyObj +0xdb4, anything else read as a
// C3DPickupItem (+0x914). port: other classes have none (the original reads +0x84c of them).
static bool CursorWanted(CGameObject *pickup)
{
	if (pickup->NameContains((char *)"C3DGRANNY"))			// +0x18
	{
		C3DGrannyObj *g = dynamic_cast<C3DGrannyObj *>(pickup);
		return g && g->f_db4 != 0;
	}
	C3DPickupItem *p = dynamic_cast<C3DPickupItem *>(pickup);
	return p && p->m_show_arrow != 0;
}
static C3DObject *GoddardPointer(C3DGoddard *goddard)		// C3DGoddard +0x1b84 (its CGameObject part)
{
	return goddard->f_1b84 ? dynamic_cast<C3DObject *>(goddard->f_1b84) : NULL;
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
static void SetParticleColour(C3DMagicParticle *p, float r, float g, float b)
{
	p->f_600 = r;
	p->f_604 = g;
	p->f_608 = b;
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
		f_d5c->v89(f_bdc.x, f_bdc.y, f_bdc.z, *(unsigned int *)&f_bdc.w);
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
	C3DObject *props[5] = { f_d28, f_d2c, f_d30, f_d38, f_d34 };
	for (int i = 0; i < 5; i++)
		if (props[i])
		{
			props[i]->HideAgain();							// element +0xdc
			ev88((char *)"none", static_cast<CGameObject *>(props[i]));	// element +0x160
		}
	f_d20->HideAgain();
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
	if (f_cc8 && static_cast<C3DRocketShip *>(f_cc8)->f_9f4) f_cc8->ShowAgain();	// 0x445ce5: our rocket with a rider (element +0xd8)
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
	if (C3DRocketShip *rs = static_cast<C3DRocketShip *>(f_cc8))	// 0x447736: its sounds (the channels are kept)
	{
		if (rs->f_123c != -1) StopGameSound(rs->f_123c, false);
		if (rs->f_1240 != -1) StopGameSound(rs->f_1240, false);
	}
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

// --------------------------------------------------------------------------- the per-frame slots

void C3DJimmy::ev133(float dt)								// 0x4473f0
{
	if (f_4ca5)
	{
		// knocked back: no control for a second
		f_4c9c = dt + f_4c9c;
		v261();												// +0x414
		if (f_4c9c > 1.0f)
		{
			f_4c9c = 0;
			f_4ce8 = 0.5f;
			f_4ca5 = 0;
			if (g_camera == static_cast<CGameObject *>(this))
			{
				if (v133()) ev58((char *)"STOP", 1);
				else ev58((char *)"FALL", 1);
			}
		}
		return;
	}
	if (f_4ce8 > 0) f_4ce8 = f_4ce8 - dt;
	if (f_4cb8)
	{
		// flashing (diffuse of the first polygon's material: alpha 1, r = g = k, b 1)
		f_4c9c = dt + f_4c9c;
		double k;
		if (f_4c9c > f_4cdc)
		{
			k = 1.0;
			f_4cb8 = 0;
		}
		else k = (sin((double)f_4c9c * 20.0) + 1.0) * 0.5;
		if (f_4da0 <= 0 && f_4da4 <= 0)
		{
			OMedia3DMaterial *m = FirstMaterial(this);		// element +0x8c -> polygon 0
			if (m) m->set_diffuse(OMediaFARGBColor(1.0f, (float)k, (float)k, 1.0f));
		}
	}
	if (f_4ca4)
	{
		// the same with the morph material f_5d4[0]: alpha k, r 1, g k, b 1
		f_4c9c = dt + f_4c9c;
		double k;
		if (f_4c9c > 4.0f)
		{
			k = 1.0;
			f_4ca4 = 0;
		}
		else k = (sin((double)f_4c9c * 20.0) + 1.0) * 0.5;
		if (f_4da0 <= 0 && f_4da4 <= 0 && f_5d4[0])
			f_5d4[0]->set_diffuse(OMediaFARGBColor((float)k, 1.0f, (float)k, 1.0f));
	}
}

void C3DJimmy::ev135(float dt)								// 0x446bc0
{
	(void)dt;
	OMedia3DVector p = Position(this);
	OMedia3DVector up = p;
	up.y = p.y + 150.0f;
	if (!PhysicsLineBlocked(p, up, false)) return;			// 0x4b7170
	if (Velocity(this).y <= 0) return;
	float vy = (float)(-(double)Velocity(this).y * 0.6);
	if (vy > -85.0f) vy = -85.0f;
	v176(Velocity(this).x, vy, Velocity(this).z);			// +0x2c0
	if (f_4ce4 > 5.0f) PlayGameSound(-1, std::string("HitHead"), false);
	f_4ce4 = 0;
	ev132();												// +0x210
}

void C3DJimmy::ev136(float dt)								// 0x446d40
{
	if (f_4db8) return;
	OMedia3DVector p = Position(this);
	OMedia3DVector a(p.x, p.y + 150.0f, p.z, p.w);
	OMedia3DVector b(p.x, a.y - 2500.0f, p.z, p.w);
	if (!PhysicsClosestHit(a, b, false))					// 0x4b7640
	{
		f_cc4->HideAgain();									// +0xd8
		f_c04 = 0;
		return;
	}
	const OMedia3DVector hit = RayEnd();
	if (!f_cc4) return;										// (the original checks here)
	if (!g_windowed) f_cc4->ShowAgain();					// +0xd4 (0x56afab)
	// the shadow sits on the floor, half its height (+0x602) - 2 up
	const float half = (float)(f_cc4->f_602 / 2 - 2);
	p = Position(this);
	f_cc4->v197(p.x, hit.y + half, p.z);					// +0x314
	{
		OMedia3DVector s = Position(f_cc4);
		p = Position(this);
		const float dx = s.x - p.x, dy = s.y - p.y, dz = s.z - p.z;
		f_b08 = sqrtf(dx * dx + dy * dy + dz * dz);
	}
	if (f_c08 <= 0)
	{
		f_c08 = 0;
		f_c04 = 0;
		return;
	}
	// slopes: probe the floor ahead of Jimmy, left and right
	OMedia3DVector l0 = v224(-5.0f, 150.0f, 30.0f), l1 = v224(-5.0f, -2500.0f, 30.0f);
	if (!PhysicsClosestHit(l0, l1, false))
	{
		f_c04 = 0;
		return;
	}
	const OMedia3DVector left = RayEnd();
	OMedia3DVector r0 = v224(5.0f, 150.0f, 30.0f), r1 = v224(5.0f, -2500.0f, 30.0f);
	if (f_c08 <= 0)
	{
		f_c08 = 0;
		f_c04 = 0;
		return;
	}
	if (!PhysicsClosestHit(r0, r1, false)) return;
	const OMedia3DVector right = RayEnd();
	OMedia3DVector low;										// the lower side
	if (left.y == right.y) low = left;
	else
	{
		if (left.y > right.y)
		{
			AddRotation(0, dt * -15.0f, 0);					// +0x330
			low = right;
		}
		else
		{
			AddRotation(0, dt * 15.0f, 0);
			low = left;
		}
		if (right.y + 5.0f < left.y)
		{
			AddRotation(0, dt * -29.0f, 0);
			low = right;
		}
		else if (right.y - 5.0f > left.y)
		{
			AddRotation(0, dt * 29.0f, 0);
			low = left;
		}
	}
	float d = hit.y - low.y;
	if (d > 10.0f)
	{
		v196(hit.x, hit.y + 70.0f, hit.z, hit.w);			// +0x310
		f_c04 = (float)((double)d * 5.5);
		return;
	}
	if (f_c08 <= 0)
	{
		f_c04 = 0;
		return;
	}
	g_5461cc = 0;
	v196(hit.x, hit.y + 70.0f, hit.z, hit.w);
	if (d < 0)
	{
		f_c04 = 0;
		// (debug text "UPHILL %d")
		return;
	}
	if (d < 10.0f) d = 10.0f;
	f_c04 = (float)((double)d * 5.5);
}

void C3DJimmy::ev145(float dt)								// 0x4468f0
{
	if (!g_menu) return;
	if (!g_camera)
	{
		g_menu->HideGadgetBar();							// +0x4c8
		return;
	}
	if (!g_menu->IsCurrentGadget(2)) return;				// +0x4e0
	g_menu->ShowGadgetBar();								// +0x4c4
	if (PlayerKeyAction() && !PlayerKeyJump() && f_4ccc == 0 && f_4cd0 == 20.0f)
	{
		AddHealth(-2.0f);									// 0x485730
		f_4ccc = 20.0f;
		ev65(0, 6, 1);										// element +0x104
		ev71(0.7f);											// +0x11c
		f_4a9 = 0;
		f_4cd0 = 0;
	}
	if (f_4ccc == 0)
	{
		f_4cd0 = dt + f_4cd0;
		if (g_menu) SetPowerBar((float)((double)f_4cd0 * (double)0.05f * 100.0));	// 0x404d60
		if (f_4cd0 >= 20.0f)
		{
			if (g_menu) g_menu->GadgetBarFull();			// +0x4d0
			f_4cd0 = 20.0f;
		}
	}
	if (f_4ccc > 0)
	{
		f_4ccc = f_4ccc - dt;
		float v = (float)((double)f_4ccc * (double)0.05f * 100.0);
		if (v <= 0) v = 0;
		if (g_menu) SetPowerBar(v);
		if (f_4ccc <= 0)
		{
			f_4ccc = 0;
			ev65(0, 6, 7);
			ev71(0);
			f_4a9 = 1;
			if (g_menu) g_menu->GadgetBarEmpty();			// +0x4cc
			return;
		}
		if (f_4ccc <= 3.0f)
		{
			// the last 3 s it blinks
			if ((int)((double)f_4ccc * 5.0) % 2 != 0) ev65(0, 6, 7);	// _ftol
			else ev65(0, 6, 1);
		}
	}
}

void C3DJimmy::ev163(void)									// 0x447c90
{
	if (f_ae8 > 35.0f) f_ae8 = 35.0f;
	if (f_ae8 < -15.0f) f_ae8 = -15.0f;
	OMedia3DVector p = Position(this);
	const float ax = p.x, ay = p.y + 80.0f, az = p.z;
	OMedia3DVector t = v224(0, f_ae8 + 80.0f, 45.0f);
	OMedia3DVector d(t.x - ax, t.y - ay, t.z - az, 0);
	omt_Angle rx, ry;
	d.angles(rx, ry);										// OMedia3DVector::angles
	f_d20->v203((float)rx * kAngleToDeg, (float)ry * kAngleToDeg, 0);	// +0x32c
	OMedia3DVector at = v224(24.0f, 14.0f, 32.0f);
	f_d20->v196(at.x, at.y, at.z, at.w);					// +0x310
	f_d20->ShowAgain();										// +0xd8
	f_c9c = 1;
	f_4cd8 = f_ae8;
}

void C3DJimmy::ev164(float dt)								// 0x4488e0
{
	if (f_4ef0)
	{
		ev149(dt, f_4ef0, GoddardPointer(f_d5c));			// element +0x254
	}
	CGameObject *o = FindNearest(static_cast<CGameObject *>(this), '3PIC', '3GRO');	// 0x48f730
	if (!o)
	{
		if (f_d48 && f_d5c)
		{
			C3DObject *gp = GoddardPointer(f_d5c);
			if (gp) gp->HideAgain();						// +0xdc
		}
		f_d48 = 0;
	}
	else
	{
		OMedia3DVector a = Position(o), b = Position(this);
		const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
		const float d = sqrtf(dx * dx + dy * dy + dz * dz);
		if (d < 950.0f)
		{
			if (CursorWanted(o))
			{
				if (f_d4c)									// (checked in the original)
				{
					f_d4c->ev71((float)(1.0 - (double)d * (double)0.0010526316f));	// +0x11c: fades with distance
					OMedia3DVector q = Position(o);
					f_d4c->v196(q.x, q.y, q.z, q.w);		// +0x310
					f_d4c->AddRotation(0, dt * -100.0f, 0);	// +0x330
					f_d4c->ShowAgain();						// +0xd8
				}
				return;
			}
		}
	}
	if (f_d4c) f_d4c->HideAgain();							// +0xdc (checked in the original)
}

void C3DJimmy::Slide(float dt)								// 0x4479b0
{
	if (f_c04 <= 0 && f_c08 <= 0)
	{
		StopSound(&f_4d98);
		return;
	}
	if (f_c08 > 0)
	{
		f_c08 = f_c08 - dt;
		if (f_c08 <= 0) f_c08 = 0;
	}
	if (!v133())
	{
		v178(0, -5500.0f, 0);								// +0x2c8
		StopSound(&f_4d98);
		return;
	}
	v136(1);												// +0x220
	if (f_c04 > 0)
	{
		v178(0, -5500.0f, 0);
		if (f_4d98 == -1)
		{
			ev116();										// +0x1d0
			ev58((char *)"SLIDE", 1);
			f_4d98 = PlayGameSound(-1, std::string("slide"), true);
		}
		if (g_flag_545e54) f_ccc->HideAgain();				// +0xdc
	}
	else
	{
		StopSound(&f_4d98);
		f_96c = f_96c - 10.0f;
		if (f_96c < 0) f_96c = 0;
		if (g_flag_545e54) f_ccc->ShowAgain();				// +0xd8
	}
	const float top = f_968 * 2.7f;
	// (debug text "%d" of f_96c)
	if (f_96c < top)
	{
		if (f_c08 > 0) f_96c = f_c04 * 4.5f + f_96c;
		if (f_96c <= 0)
		{
			f_96c = 0;
			StopSound(&f_4d98);
		}
		if (f_96c > top) f_96c = top;
	}
}

void C3DJimmy::AddTouching(CGameObject *other)				// 0x444fc0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (other->NameContains((char *)"C3DPENDULUM"))
	{
		const int l = LevelID();
		if (l == 'L1_A' || l == 'L1_B' || l == 'L1_C') return;
		ev134(-10.0f, -10, 5.0f, 1);						// element +0x218
		return;
	}
	if (other->NameContains((char *)"C3DWATERBALLOON"))
	{
		other->v132(0);										// +0x210
		other->v136(0);										// +0x220
		C3DSprite *s = dynamic_cast<C3DSprite *>(other);
		if (s) s->HideAgain();								// element +0xd8
		return;
	}
	if (other->NameContains((char *)"C3DMISSILE"))
	{
		if (f_4cb8) return;
		ev134(-10.0f, -10, 5.0f, 1);
		v261();												// +0x414
		ev116();											// +0x1d0
		PlayGameSound(-1, std::string("YokianBump"), false);	// 0x48f420
		return;
	}
	if (other->NameContains((char *)"C3DGIRLEATINGPLANT") || other->NameContains((char *)"C3DDARWIN"))
	{
		// their members (+0xc40 stunned; knocked back: element slot 107 (5), +0xc10 = 500,
		// +0xc3c = 5): C3DAI is not reconstructed yet, so none can be met.
		NEG_TODO(0x445405, "C3DJimmy::AddTouching: plant / Darwin members (+0xc40, +0xc10, +0xc3c, element slot 107)");
		if (f_4cb8) return;
		if (strcasecmp(other->m_tag, "LIBBYPLANT") == 0) return;
		ev134(-10.0f, -10, 5.0f, 1);
		ev114((char *)"HIT");								// +0x1c8
		v261();
		ev116();
		PlayGameSound(-1, std::string("YokianBump"), false);
		return;
	}
	if (other->NameContains((char *)"C3DBOT"))
	{
		if (f_a68 == 2) return;
		C3DBot *bot = dynamic_cast<C3DBot *>(other);		// 0x44512a (port: the original subtracts 0xc0 without looking)
		if (!bot) return;
		if (bot->f_c40) return;								// state locked
		if (f_4cb8)											// we are hurt: an unstunned bot only backs off
		{
			if (bot->f_1b50 > 0) return;
			bot->ev107(5);									// flee ...
			bot->f_c10 = 1500.0f;							// ... beyond 1500
			bot->f_c3c = 5.0f;								// for 5 s
			return;
		}
		if (bot->f_96c != 0 && bot->f_96c != 11) return;	// only an attacking bot hurts
		if (bot->f_1b50 > 0) return;						// stunned
		bot->ev107(5);
		bot->f_c10 = 1500.0f;
		bot->f_c3c = 10.0f;
		if (f_8a7) return;
		ev134(-2.0f, -1, 3.0f, 1);
		ev114((char *)"HIT");
		v261();
		ev116();
		PlayGameSound(-1, std::string("YokianBump"), false);
		return;
	}
	if (other->NameContains((char *)"C3DHUMPHREY")) return;
	if (other->NameContains((char *)"C3DBUTTON"))
	{
		if (strcasecmp(other->m_tag, "REACTOR") != 0) return;
		f_d6d = 1;
		if (LevelID() == 'LV4B')
		{
			CLevel04BGame *g = dynamic_cast<CLevel04BGame *>(g_gametype);
			if (g) g->f_688 = 1;
		}
		return;
	}
	if (other->NameContains((char *)"C3DAITRIGGER"))
	{
		// (the original compares Jimmy's own tag, not the trigger's)
		if (strcasecmp(m_tag, "DEACTIVATE") != 0) return;
		ev138(static_cast<CGameObject *>(this), static_cast<CGameObject *>(this), 0, 0.0f);	// element +0x228
		return;
	}
	if (other->NameContains((char *)"C3DLOADLEVEL"))
	{
		if (!(g_53763c > 0)) return;
		g_53763c = -1.0f;
		if (g_menu) g_menu->v316(false);					// +0x4f0
	}
}

void C3DJimmy::Process(float dt)							// 0x446260
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (f_d34 && g_game_state != 3)
	{
		f_d34->HideAgain();									// +0xdc
		f_d34->v136(0);										// +0x220
	}
	g_54a400 = g_54a400 + dt;
	g_545f30 = 0;
	// sparkles of f_4d50
	if (f_4d64 > 0)
	{
		f_4d64 = f_4d64 - dt;
		f_4d68 = f_4d68 - dt;
		if (f_4d64 <= 0) f_4d64 = 0;
		else
		{
			ev155();										// element +0x26c
			if (f_4d50[f_4d60])
			{
				const int r1 = Random1001(), r2 = Random1001();
				if (g_camera)
				{
					OMedia3DVector at = g_camera->v224(
						(float)(((double)r1 * (double)0.001f - 0.5) * 10.0 + f_4d6c.x),
						(float)(((double)r2 * (double)0.001f - 0.5) * 10.0 + f_4d6c.y),
						f_4d6c.z);
					ParticleBurst(f_4d50[f_4d60], at);
				}
			}
		}
	}
	// sparkles of f_4d00 (note: < 0 here, <= 0 above)
	if (f_4d58 > 0)
	{
		f_4d58 = f_4d58 - dt;
		f_4d5c = f_4d5c - dt;
		if (f_4d58 < 0) f_4d58 = 0;
		else
		{
			ev154();										// element +0x268
			if (f_4d00[f_4d54])
			{
				const int r1 = Random1001(), r2 = Random1001();
				if (g_camera)
				{
					OMedia3DVector at = g_camera->v224(
						(float)(((double)r1 * (double)0.001f - 0.5) * 100.0),
						(float)(((double)r2 * (double)0.001f - 0.5) * 100.0 + 10.0),
						-40.0f);
					ParticleBurst(f_4d00[f_4d54], at);
				}
			}
		}
	}
	if (!g_camera)
	{
		if (f_4da4 > 0)
		{
			f_4da4 = 0;
			if (f_d44) f_d44->HideAgain();					// (unchecked in the original)
		}
		if (f_4c10 != -1)
		{
			StopChannel(f_4c10, false);						// 0x4b88e0
			f_4c10 = -1;
			if (f_cbc) f_cbc->HideAgain();
		}
	}
	if (v133())
	{
		// landed: play the animation asked for while in the air
		if (strcasecmp(f_4e80, g_empty_string) != 0)
		{
			ev58(f_4e80, 1);
			strcpy(f_4e80, g_empty_string);
		}
		v152(60.0f);										// +0x260
	}
	if (f_d6c) DrawText(20, 240, 'MED', "%d", GetTaskState("SCENE"));	// 0x4a0f60 / 0x497a40
	f_4ce4 = dt + f_4ce4;
	if (g_56afaa)
	{
		int n = 0;
		for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
			if ((*i)->v135()) n++;							// +0x21c
		DrawText(0x123, 100, 'TIME', "%.4d", n);
	}
	if (f_4cec && f_4cf0 > 0)
	{
		f_4cf0 = f_4cf0 - dt;
		if (f_4cf0 <= 0) f_4cf0 = 0;
	}
	if (Velocity(this).y > 0) ev135(dt);					// element +0x21c
	if (!f_4cfd) ev136(dt);									// element +0x220
	if (!v133() && f_4ca6) ev129();							// element +0x204
	C3DPlayer::Process(dt);									// 0x460510
	ev145(dt);												// element +0x244
	if (v133())
	{
		OMedia3DVector p = Position(this);
		f_4cbc.x = p.x; f_4cbc.y = p.y; f_4cbc.z = p.z; f_4cbc.w = p.w;
	}
	ev133(dt);												// element +0x214
	if (f_d5c && f_d5c->f_1b5c == 4 && f_d5c->f_960 > 9.0f)	// Goddard in front for 9 s: back beside us
	{
		f_d5c->f_960 = 0;
		if (f_d5c->f_1b5e == 1) { f_d5c->ev144(2); return; }	// element +0x240
		if (f_d5c->f_1b5e == 3) { f_d5c->ev144(0); return; }
	}
	if (f_4d7c) C3DAnimated::Process(dt);					// 0x41c120
}

void C3DJimmy::v243(float dt)								// 0x447e40
{
	if (f_aed) return;
	if (g_gametype->IsPaused()) return;
	if (f_4d7c)
	{
		if (!f_4cb8) return;
		f_4cb8 = 0;
		if (GetItemState(2, 20)) return;					// 0x404f20
		OMedia3DMaterial *m = FirstMaterial(this);
		if (m) m->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
		return;
	}
	if (!g_camera) return;
	if (g_54a498) return;
	if (f_a68 == 4) g_5461cc = 0;
	if (f_4ca5) return;
	Slide(dt);												// 0x4479b0
	C3DPlayer::v243(dt);									// 0x4618d0
	ev146(dt, Position(this));								// element +0x248
	if (ev140(dt)) return;									// element +0x230
	if (!v133())
	{
		// in the air
		if (Velocity(this).y < -200.0f) f_4db4 = 1;
		if (f_4cfe && !f_4db8)
		{
			if (Velocity(this).y < f_4db0)
			{
				if (GetItemState(0, 3) == 1)
				{
					// falling fast with the jet pack: it starts by itself
					JimmyJetpackFlame(static_cast<CGameObject *>(this), 3.0f);	// f_ccc +0x9f0 -> +0x928
					ev114((char *)"FLY");							// +0x1c8
					JimmyJetpackSound(static_cast<CGameObject *>(this), 1);		// "jetpack" on f_ccc +0x9f4
					f_4cec = 1;
					f_4cf0 = 5.0f;
					ev58((char *)"FLY", 1);
					f_a8c = 5;
					v178(0, 100.0f, 0);								// +0x2c8
					f_a68 = 1;
				}
				else if (Velocity(this).y < f_4db0) g_5461cc = 1;
			}
			else if (f_4cec && f_4cf0 == 0)
			{
				f_4cec = 0;
				JimmyJetpackOff(static_cast<CGameObject *>(this));	// 0x443370
				f_a8c = 0;
				ev58((char *)"FALL", 1);
			}
		}
	}
	else if (!g_menu->IsCurrentGadget(3))
	{
		// on the floor
		if (!g_5461cc)
		{
			if (f_4db4)
			{
				PlayGameSound(-1, std::string("landing"), false);
				f_4db4 = 0;
			}
		}
		else if (g_54a448 > 0)
		{
			// jump power-up: bounce
			if (g_54a448 < 3.0f) v162(0, g_54a448 * 233.33333f, 0);	// +0x288
			else v162(0, 175.0f, 0);
			PlayGameSound(-1, std::string("boing"), false);
			v176(0, 0, 0);											// +0x2c0
			g_5461cc = 0;
		}
		else if (f_c08 == 0 && f_a68 != 4)
		{
			// landed from too high
			f_4db4 = 0;
			g_5461cc = 0;
			PlayGameSound(-1, std::string("pain ahhouch"), false);
			if (f_4dac) ev134(-300.0f, -10, 5.0f, 1);
			else ev134(-10.0f, -10, 5.0f, 1);
		}
	}
	if (f_a68 == 2)
	{
		AddHealth(dt * -0.5f);								// 0x485730
		OMedia3DVector p = Position(this);
		OMedia3DVector d(p.x - g_viewport->getx(), p.y + 100.0f - g_viewport->gety(), p.z - g_viewport->getz(), 0);
		const float k = 1.0f / d.quick_magnitude();
		d.x = k * d.x;
		d.y = k * d.y;
		d.z = k * d.z;
		if (f_cc0)
		{
			p = Position(this);
			f_cc0->v197(p.x - d.x * 80.0f, p.y + 50.0f - d.y * 80.0f, p.z - d.z * 80.0f);	// +0x314
		}
	}
	ev159(dt);												// element +0x27c
	ev164(dt);												// element +0x290
	if (g_menu->IsCurrentGadget(10))
	{
		// gadget 10 recharges by the skill setting
		switch (g_difficulty)
		{
			case 0: f_4cd0 = dt + dt + f_4cd0; break;
			case 1: f_4cd0 = dt * 1.5f + f_4cd0; break;
			case 2: f_4cd0 = dt + f_4cd0; break;
		}
		if (f_4cd0 >= 20.0f) f_4cd0 = 20.0f;
		const float v = (float)((double)f_4cd0 * (double)0.05f * 100.0);
		if (g_menu) SetPowerBar(v);							// 0x404d60
	}
	if (PlayerKeyAction() && !PlayerKeyJump())
	{
		if (g_5461ce) return;
		if (f_4ee8) return;
		if (g_menu->IsCurrentGadget(6) || g_menu->IsCurrentGadget(10)) f_96c = 0;
		if (ev158(dt)) return;								// element +0x278
		if (g_menu->IsCurrentGadget(4)) return;
		if (v133())
		{
			f_4e7c = 3.0f;
			if (g_54a438 > 0)
			{
				ev163();									// element +0x28c
				bool again = true;
				if (f_4ee4 > 0)
				{
					f_4ee4 = f_4ee4 - dt;
					if (f_4ee4 > 0) again = false;
					else ev144();							// element +0x240
				}
				if (again) f_4ee4 = 0.15f;
			}
			if (ev77((char *)"SHOOT")) return;				// element +0x134
			if (!g_menu->IsCurrentGadget(6) && !g_menu->IsCurrentGadget(10)) return;
			ev116();
			if (!f_4cd4)
			{
				f_4cd4 = 1;
				ev116();
				return;
			}
			ev163();
			ev58((char *)"POINT", 1);
			return;
		}
		if (!g_menu->IsCurrentGadget(2) && GetItemState(0, 3) == 1)
		{
			// the shrink ray, in the air
			if (f_4c10 == -1)
			{
				if (strcasecmp(ev66(), "FALL") == 0) return;	// element +0x108
				f_4c10 = PlayGameSound(-1, std::string("shrinkray"), true);
				ev116();
				return;
			}
			if (strcasecmp(ev66(), "FALL") != 0) return;
			StopSound(&f_4c10);
			if (f_cbc) f_cbc->HideAgain();					// (unchecked in the original)
			return;
		}
		f_4ee8 = 0;
		return;
	}
	f_c9c = 0;
	if (g_54a438 > 0 && f_d20) f_d20->HideAgain();
	if (f_4e7c > 0) f_4e7c = f_4e7c - dt;
	if (f_4e7c <= 0)
	{
		// the camera pitch eases back
		f_4e7c = 0;
		if (f_ae8 > 0)
		{
			f_ae8 = f_ae8 - dt * 10.0f;
			if (f_ae8 <= 0) f_ae8 = 0;
		}
		if (f_ae8 < 0)
		{
			f_ae8 = dt * 10.0f + f_ae8;
			if (f_ae8 >= 0) f_ae8 = 0;
		}
	}
	if (f_4cd4) ev144();
	StopSound(&f_4c10);
	StopSound(&f_4c24);
	if (f_96c < 10.0f && ev77((char *)"SHOOT")) ev116();
	if (f_cbc) f_cbc->HideAgain();
}
