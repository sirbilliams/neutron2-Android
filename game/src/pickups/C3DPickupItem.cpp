/*  C3DPickupItem - see C3DPickupItem.h. Reconstructed from Neutron2.exe 0x45dc00..0x45fa50. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DPickupItem.h"
#include "../objects/C3DCorona.h"
#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../C3DRadar.h"
#include "../CJimmyGame.h"
#include "../DatabaseCache.h"
#include "../InterfaceElement.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include "OMediaViewPort.h"
#include "OMediaStreamOperators.h"
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

static C3DJimmy *JimmyOf(CGameObject *o)	{ return o ? dynamic_cast<C3DJimmy *>(o) : NULL; }	// (JIM1 - 0xc0)

// 0x404dd0: set the type of item `item` of menu page `page` (as in CLoadLevel.cpp). The pickups
// come in pairs (10/21 and 31/38): giving one also gives (and counts) the other.
static void SetItemState(short page, short item, short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p) p->items[item].type = state;
	if (page != C2DInGameMenu::MENU_PICKUPS) return;
	C2DInGameMenu::MenuPage *pk = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
	if (item == 10)
	{
		if (!pk || !pk->items[21].type) { SetItemState(2, 21, 1); g_pickup_counts[2 * 21]++; }
	}
	else if (item == 21)
	{
		if (!pk || !pk->items[10].type) { SetItemState(2, 10, 1); g_pickup_counts[2 * 10]++; }
	}
	else if (item == 38)
	{
		if (!pk || !pk->items[31].type) { SetItemState(2, 31, 1); g_pickup_counts[2 * 31]++; }
	}
	else if (item == 31)
	{
		if (!pk || !pk->items[38].type) { SetItemState(2, 38, 1); g_pickup_counts[2 * 38]++; }
	}
}

// 0x404ef0: item `item` of menu page `page` disabled.
static void ClearItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::Page(page)) != NULL)
		p->items[item].type = 0;
}

// 0x404f20: the type of item `item` of menu page `page` (0 when there is no such item).
static short GetItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p && item >= 0 && item < C2DInGameMenu::MENU_ITEMS) return p->items[item].type;
	return 0;
}

static int  PickupCount(int pic)			{ return g_pickup_counts[2 * pic]; }	// 0x408ef0
static void AddPickupCount(int pic, int n)	{ g_pickup_counts[2 * pic] += n; }		// 0x408f00

// 0x408f80 (2496 bytes, not reconstructed yet): the pickup menu's announcement of pickup `pic`
// (interface elements, the "jimmyself121" voice and the gadget-complete checks).
static void AnnouncePickup(int pic, int index)
{
	(void)pic; (void)index;
	NEG_TODO(0x408f80, "pickup announcement (menu interface elements, jimmyself121)");
}

// C3DWaterPlane (not reconstructed yet): game object +0x87c = 3.0 (the oxygen refill).
static void WaterPlaneOxygen(CGameObject *wp)
{
	(void)wp;
	NEG_TODO(0x45f2eb, "C3DWATERPLANE +0x87c = 3 (oxygen)");
}

// The power timers g_power_timers[0..9] (0x54a424..): which powerup each one runs.
enum { PT_SPEED = 0, PT_SHIELDS, PT_TRACTION, PT_SONAR, PT_XRAY, PT_RAPIDFIRE, PT_FREEZE,
	   PT_EARTHQUAKE, PT_SPREADFIRE, PT_SPRINGSHOES };

// --------------------------------------------------------------------------- construction

C3DPickupItem::C3DPickupItem()								// 0x45dc00
{
	SetName((char *)"C3DPICKUPITEM");
	Log("C3DPICKUPITEM()");
	m_initially_active = 1;
	m_corona = NULL;
	C3DPickupItem::InitObject();							// (called directly, as in the original)
	SetObjectID('3PIC');									// element slot 48
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 8;
	m_picked_up_index = -1;
	m_required_pic_num = -1;
	m_req_pic_num_amount = 1;
	m_required_level = -1;
	m_sprite_size = 50;
	m_sound_index = -1;
	m_need_more_sound = -1;
	v67(100.0f);
	v46(0);
	v52(1);
	m_times_to_trigger = -1;
	m_times_triggered = 0;
	v34(1);
	m_is_ambient = 0;
	m_ambient_channel = -1;
	m_ambient_started = 0;
	m_point_value = 0;
	C3DSprite::InitPhysics();								// 0x49ca30
	strcpy(m_activate_object, "none");
	strcpy(m_need_more_name, "none");
	strcpy(m_sound_name, "none");
	m_toggle = -1;
	f_90c = 0;
	m_pass_thru = 1;
	m_show_arrow = 0;
	f_008 = 1;												// element +0xd0
	m_show_corona = 0;
	m_red = 1.0f;
	m_green = 1.0f;
	m_blue = 1.0f;
	m_respawn_time = -1.0f;
	m_respawn_left = 0;
	m_show_on_radar = 2;
	m_powerup_time = 30.0f;
	m_is_voice = 0;
	// (an empty debug log)
}

C3DPickupItem::~C3DPickupItem() {}							// 0x45ded0 (the corona is left to DeleteAllObjects)

namespace neg {
CGameObject *New_C3DPickupItem(void) { return new C3DPickupItem; }	// '3PIC' (0x944 bytes)
}

// Shown again (not as the picked-up canvas 106), with the corona when it may be picked up.
// (Inlined many times in the original.)
#define SHOW_WITH_CORONA()														\
	do {																		\
		if (m_sprite_index != 106)												\
		{																		\
			ShowAgain();									/* element +0xd4 */	\
			if (m_corona && CanPickUp()) m_corona->ShowAgain();	/* +0xe4 */		\
		}																		\
	} while (0)

// --------------------------------------------------------------------------- CGameObject family

void C3DPickupItem::InitObject(void)						// 0x45dfb0
{
	CPickupType::InitObject();
	RegisterProperty("SoundIndex", &m_sound_index, PROP_LONG, 0);
	RegisterProperty("SoundName", m_sound_name, PROP_ALPHA, 0);
	RegisterProperty("NeedMoreSound", &m_need_more_sound, PROP_LONG, 0);
	RegisterProperty("NeedMoreName", m_need_more_name, PROP_ALPHA, 0);
	RegisterProperty("TimesToTrigger", &m_times_to_trigger, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("IsAmbient", &m_is_ambient, PROP_LONG, 0);
	RegisterProperty("PointValue", &m_point_value, PROP_LONG, 0);
	RegisterProperty("PickedUpIndex", &m_picked_up_index, PROP_LONG, 0);
	RegisterProperty("RequiredPicNum", &m_required_pic_num, PROP_LONG, 0);
	RegisterProperty("ReqPicNumAmount", &m_req_pic_num_amount, PROP_LONG, 0);
	RegisterProperty("InitallyActive", &m_initially_active, PROP_LONG, 0);
	RegisterProperty("ActivateObject", m_activate_object, PROP_ALPHA, 0);
	RegisterProperty("PassThru", &m_pass_thru, PROP_LONG, 0);
	RegisterProperty("ShowArrow", &m_show_arrow, PROP_LONG, 0);
	RegisterProperty("ShowCorona", &m_show_corona, PROP_LONG, 0);
	RegisterProperty("Red", &m_red, PROP_FLOAT, 0);
	RegisterProperty("Green", &m_green, PROP_FLOAT, 0);
	RegisterProperty("Blue", &m_blue, PROP_FLOAT, 0);
	RegisterProperty("RespawnTime", &m_respawn_time, PROP_FLOAT, 0);
	RegisterProperty("PowerupTime", &m_powerup_time, PROP_FLOAT, 0);
	RegisterProperty("IsVoice", &m_is_voice, PROP_LONG, 0);
}

void C3DPickupItem::ResetObject(void)						// 0x45df10
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	v107(1);												// +0x1ac
	const int i = m_pickup_index;
	if (i > -1 && i < 4200 && g_object_states[i] == 0) SHOW_WITH_CORONA();
}

// 0x45e200: picked up by the camera.
void C3DPickupItem::AddTouching(CGameObject *other)
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	// (debug log "Picked up a %s")
	if (other != g_camera) return;
	if (other->NameContains((char *)"C3DGODDARD") && strcasecmp(m_tag, "SEWERPART") != 0 &&
		strcasecmp(m_next_trigger, "none") != 0) return;	// Goddard only picks up sewer parts of trigger pickups
	const int i = m_pickup_index;
	if (i >= 4200)
		ErrorLog("ERROR: OVER max pickup items in C3DPickupItem");
	else if (i > 0)											// saved in the object states
	{
		if (g_object_states[i] != 0) return;
		// (debug log "1 %d")
		if (!CanPickUp()) return;							// element +0xe4
		if (m_picked_up_index != -1)						// it stays, looking different
		{
			if (m_respawn_time < 0) g_object_states[i] = 2;
			OMediaDataBase *db = GetDatabase(m_sprite_db);	// 0x4a4180
			if (db)
			{
				InitSpriteID(db, m_picked_up_index, (float)m_sprite_size);	// element +200
				SHOW_WITH_CORONA();
				// (the original also has a branch calling slot +0xd4 of NULL when there is no
				// corona; it cannot be reached)
			}
		}
		else
		{
			if (m_respawn_time < 0) g_object_states[i] = 1;
			HideAgain();									// element +0xd8
			v136(0);										// +0x220
			if (m_corona) m_corona->HideAgain();
		}
		CGameObject *o;
		if (strcasecmp(m_activate_object, "none") != 0 && (o = FindObjectByTag(m_activate_object)) != NULL)
			static_cast<CLocalGameObject *>(o)->v268(m_toggle);	// +0x430
		if (strcasecmp(m_toggle_object, "none") != 0 && (o = FindObjectByTag(m_toggle_object)) != NULL)
			static_cast<CLocalGameObject *>(o)->v268(m_toggle);
		if (m_pic_number >= 0 && m_pic_number < 50)			// a pickup of the menu
		{
			if (m_is_voice && !IsChannelPlaying(g_voice_channel))	// 0x4b89d0, 0x537640
				g_voice_channel = PlayGameSound(-1, std::string(m_sound_name), false);	// 0x48f420
			SetItemState(2, (short)m_pic_number, 1);		// 0x404dd0
			AnnouncePickup(m_pic_number, m_pickup_index);	// 0x408f80
		}
		else if (m_is_voice && !IsChannelPlaying(g_voice_channel))
			g_voice_channel = PlayGameSound(-1, std::string(m_sound_name), false);
		if (g_object_states[m_pickup_index] > 2)
			ErrorLog("ERROR: Pickup item %d has been used %d times", m_pickup_index, g_object_states[m_pickup_index]);
	}
	else if (i == 0)										// not saved
	{
		if (m_pic_number >= 0) SetItemState(2, (short)m_pic_number, 1);
		CGameObject *o;
		if (strcasecmp(m_activate_object, "none") != 0 && (o = FindObjectByTag(m_activate_object)) != NULL)
			static_cast<CLocalGameObject *>(o)->v268(m_toggle);
		if (strcasecmp(m_toggle_object, "none") != 0 && (o = FindObjectByTag(m_toggle_object)) != NULL)
			static_cast<CLocalGameObject *>(o)->v268(m_toggle);
		HideAgain();
		v124(0);											// +0x1f0
		v136(0);
		if (m_corona) m_corona->HideAgain();
	}
	// (i < 0: on to the rest)

	Powerup();												// element +0xe0
	if (m_respawn_time > 0) m_respawn_left = m_respawn_time;
	if (m_point_value > 0 && (m_times_to_trigger == -1 || m_times_to_trigger > m_times_triggered))
	{
		AddScore(m_point_value);							// 0x482b50
		v136(0);
	}
	if (strcasecmp(m_next_trigger, "none") != 0) CallNextTrigger(m_next_trigger, 0);	// element +0xdc
	if (m_is_ambient == 0)
	{
		if (m_times_to_trigger != -1 && m_times_to_trigger <= m_times_triggered) return;
		m_times_triggered++;
		if (m_sound_index != -1)
		{
			if (!m_is_voice) PlayGameSoundIndex(-1, m_sound_index, false);	// 0x48f030
		}
		else if (!m_is_voice) PlayGameSound(-1, std::string(m_sound_name), false);
	}
	else if (m_sound_index != -1 && m_ambient_channel != -1)
		StopGameSound(m_ambient_channel, false);			// 0x48f0d0
}

unsigned char C3DPickupItem::v231(CGameObject *other)		// 0x43d610
{
	return g_camera == other;
}

// 0x45eda0: starts the ambient sound once; keeps the corona between us and the viewport (10
// units behind us); counts the respawn time down, then reloads (v260).
void C3DPickupItem::Process(float dt)
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	if (m_is_ambient && !m_ambient_started)
	{
		m_ambient_started = 1;
		if (m_sound_index != -1)
		{
			if (m_ambient_channel != -1) StopGameSound(m_ambient_channel, false);
			m_ambient_channel = PlayGameSound3DIndex(this, -1, m_sound_index, true);	// 0x48f080
		}
	}
	if (m_show_corona == 1 && m_corona)
	{
		const float vx = g_viewport->getx(), vy = g_viewport->gety(), vz = g_viewport->getz();	// +0x44..
		float p[4];
		v195(p);											// +0x30c
		OMedia3DVector d(vx - p[0], vy - p[1], vz - p[2]);
		d.w = 0;
		const float inv = 1.0f / d.quick_magnitude();		// 0x4caa6c
		d.x = inv * d.x; d.y = inv * d.y;
		const float z = inv * d.z;
		d.x = d.x * 10.0f; d.y = d.y * 10.0f; d.z = z * 10.0f;	// 0x4cb1cc
		v195(p);
		m_corona->v196(p[0] - d.x, p[1] - d.y, p[2] - d.z, p[3]);	// +0x310
	}
	if (m_respawn_left > 0)
	{
		m_respawn_left -= dt;
		if (m_respawn_left <= 0)
		{
			m_respawn_left = 0;
			v260();											// +0x410
		}
	}
}

// 0x45ea00: (re)load: the corona (once, not in a window), then shown unless collected (or as
// PickedUpIndex when it was picked up and stays), InitallyActive 0 hides it, SyncWithMenu.
void C3DPickupItem::v260(void)
{
	CPickupType::v260();									// 0x496de0
	// (debug logs "PostLoad::PI=%d", "%s   %f %f  %f" of the position)
	if (m_show_corona == 1)
	{
		// (debug logs "MEBTEMP showcorona=1", "MEBTEMP Allocating corona=1")
		if (!m_corona && !g_windowed)						// 0x56afab
			m_corona = new C3DCorona;						// 0x694 bytes
		if (m_corona)
		{
			m_corona->v107(0);								// +0x1ac
			m_corona->m_sprite_size = m_sprite_size * 2;
			m_corona->m_sprite_index = 194;
			m_corona->m_red = m_red;
			m_corona->m_green = m_green;
			m_corona->m_blue = m_blue;
			float p[4];
			v195(p);
			unsigned int w;
			memcpy(&w, &p[3], sizeof w);
			m_corona->v89(p[0], p[1], p[2], w);				// +0x164: placed there
			m_corona->v260();								// +0x410
		}
	}
	const int i = m_pickup_index;
	if (i > -1 && i < 4200)
	{
		const int state = i == 0 ? 0 : g_object_states[i];
		if (state == 0)
		{
			SHOW_WITH_CORONA();
			v136(1);
		}
		else if (m_picked_up_index != -1 && state == 2)
		{
			OMediaDataBase *db = GetDatabase(m_sprite_db);
			if (db)
			{
				InitSpriteID(db, m_picked_up_index, (float)m_sprite_size);
				SHOW_WITH_CORONA();
			}
		}
	}
	if (m_initially_active == 0)
	{
		v232();												// +0x3a0
		HideAgain();
		v136(0);
		if (m_corona) m_corona->HideAgain();
	}
	SyncWithMenu();											// element +0xec
}

// 0x45e820: toggled: 0 hides it; 1 brings it back (its object state cleared).
void C3DPickupItem::v268(int state)
{
	// (debug log "Calling NextState for %s")
	if (state == 0)
	{
		HideAgain();
		v232();
		v136(0);
		if (m_corona) m_corona->HideAgain();
		return;
	}
	if (state != 1) return;
	// (debug log "NextState with %d for %s")
	if (m_pickup_index >= 4200) return;
	if (m_pickup_index == 0)
	{
		SHOW_WITH_CORONA();
		v234();												// +0x3a8
		return;
	}
	if (m_pickup_index < 0)
	{
		ErrorLog("ERROR: Incalid PickupIndex for %s", m_tag);	// (sic)
		return;
	}
	if (g_object_states[m_pickup_index] != 0) g_object_states[m_pickup_index] = 0;
	SHOW_WITH_CORONA();
	v234();
}

void C3DPickupItem::WriteState(OMediaStreamOperators &s)	// 0x45f980
{
	CLocalGameObject::WriteState(s);
	// (debug log "test" for FREEZETIME)
	bool collides = v135() != 0;							// +0x21c
	s << collides;
	s << m_respawn_left;
}

void C3DPickupItem::ReadState(OMediaStreamOperators &s)		// 0x45f880
{
	CLocalGameObject::ReadState(s);
	// (debug log "test" for FREEZETIME)
	bool collides;
	s >> collides;
	v136(collides);
	if (!collides)
	{
		HideAgain();
		if (m_corona) m_corona->HideAgain();
	}
	else
	{
		v234();
		ShowAgain();
		if (m_corona && CanPickUp()) m_corona->ShowAgain();
	}
	s >> m_respawn_left;
}

// --------------------------------------------------------------------------- element family

// 0x45f120: what a pickup does by its tag (the timed powerups set their g_power_timers and
// colour Jimmy's sparkles).
void C3DPickupItem::Powerup(void)
{
	const char *tag = m_tag;
	if (strcasecmp(tag, "PICKEDUPCRYSTAL") == 0)
	{
		const int s = GetTaskState("SCENE");
		if (s == 200) SetTaskState("SCENE", 210);
		else if (s == 220) SetTaskState("SCENE", 230);
	}
	if (strcasecmp(tag, "PICKEDUPKEY") == 0)
	{
		const int s = GetTaskState("SCENE");
		if (s == 200) SetTaskState("SCENE", 220);
		else if (s == 210) SetTaskState("SCENE", 230);
	}
	C3DJimmy *j;
	if (strcasecmp(tag, "SPEEDUP") == 0)
	{
		g_power_timers[PT_SPEED] = m_powerup_time;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL) j->ev156(m_powerup_time, 1.0f, 1.0f, 0);	// element +0x270
	}
	if (strcasecmp(tag, "SODA") == 0)
	{
		g_power_timers[PT_SPEED] = 5.0f;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL)
		{
			j->ev156(5.0f, 1.0f, 1.0f, 0);
			j->f_4cd0 = j->f_4cd0 + 8.0f;					// 0x4f12e4
		}
	}
	if (strcasecmp(tag, "TRACTION") == 0)
	{
		g_power_timers[PT_TRACTION] = m_powerup_time;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL) j->ev156(g_power_timers[PT_TRACTION], 1.0f, 0, 1.0f);
	}
	CGameObject *o;
	if (strcasecmp(tag, "OXYGEN") == 0 && (o = FindObjectByTag("C3DWATERPLANE")) != NULL)
	{
		WaterPlaneOxygen(o);								// +0x87c = 3.0
		PlayGameSound(-1, std::string("waterbutton1"), false);
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL) j->ev156(10.0f, 0, 0, 1.0f);
	}
	if (strcasecmp(tag, "SPORKOS") == 0 && (j = JimmyOf(FindObjectByTag("JIM1"))) != NULL)
	{
		j->ev156(10.0f, 0, 1.0f, 0);
		j->f_4d94 += 10;
		if (j->f_4d94 > 100) j->f_4d94 = 100;
	}
	if (strcasecmp(tag, "SONAR") == 0)
	{
		j = JimmyOf(FindObjectByTag("JIM1"));
		g_power_timers[PT_SONAR] = m_powerup_time;
		if (j)
		{
			j->ev156(15.0f, 0, 1.0f, 0);
			C3DJimmy *pj = JimmyOf(g_player);				// 0x545fd0 - 0xc0
			C3DRadar *radar = pj ? pj->f_d40 : NULL;
			CGameObject *johnny = FindObjectByTag("JOHNNY");
			if (radar) radar->Open(g_camera, '3DIN', 23, 16, 40, johnny);	// 0x4689d0 (port: the original calls it on NULL)
		}
	}
	if (strcasecmp(tag, "RAPIDFIRE") == 0)
	{
		g_power_timers[PT_RAPIDFIRE] = m_powerup_time;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL) j->ev156(15.0f, 0.3f, 0.227f, 0.945f);
	}
	if (strcasecmp(tag, "SPREADFIRE") == 0)
	{
		g_power_timers[PT_SPREADFIRE] = m_powerup_time;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL) j->ev156(15.0f, 0.3f, 0.3f, 1.0f);
	}
	if (strcasecmp(tag, "SHIELDS") == 0)
	{
		g_power_timers[PT_SHIELDS] = m_powerup_time;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL)
		{
			j->ev156(15.0f, 0, 0.87f, 1.0f);
			j->ev147(g_power_timers[PT_SHIELDS]);			// element +0x24c: the bubble
		}
	}
	if (strcasecmp(tag, "FREEZETIME") == 0)
	{
		for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		{
			CGameObject *e = *it;
			if (!e) continue;
			const int id = e->m_object_id;
			if (id != '3WBL' && id != '3EPR' && id != '3VRT') e->f_430 = 0.25f;	// slowed down
		}
		static_cast<CJimmyGame *>(g_gametype)->StartGadgetEffect("FREEZETIME");	// element +0x1a4
		g_enemy_time_scale = 0.25f;							// 0x540594
		g_power_timers[PT_FREEZE] = m_powerup_time;
		if ((j = JimmyOf(FindObjectByTag("JIM1"))) != NULL)
		{
			j->ev156(15.0f, 0.96f, 0.55f, 0.33f);
			j->f_430 = 1.0f;								// (element +0x4f0) he is not
		}
	}
	if (strcasecmp(tag, "XRAY") == 0)
	{
		j = JimmyOf(FindObjectByTag("JIM1"));
		g_power_timers[PT_XRAY] = m_powerup_time;
		static_cast<CJimmyGame *>(g_gametype)->StartGadgetEffect("XRAY");
		if (j) j->ev156(15.0f, 0.9f, 0, 1.0f);
	}
	if (strcasecmp(tag, "EARTHQUAKE") == 0)
	{
		j = JimmyOf(FindObjectByTag("JIM1"));
		g_power_timers[PT_EARTHQUAKE] = m_powerup_time;
		if (j)
		{
			j->ev156(m_powerup_time, 1.0f, 0.2f, 0);
			j->f_bf8 = m_powerup_time;						// camera shake time
			j->f_c00 = 0.03f;								// and strength
		}
	}
	if (strcasecmp(tag, "SPRINGSHOES") == 0)
	{
		j = JimmyOf(FindObjectByTag("JIM1"));
		g_power_timers[PT_SPRINGSHOES] = m_powerup_time;
		if (j) j->ev156(m_powerup_time - 2.0f, 1.0f, 1.0f, 1.0f);	// 0x4caa70
	}
}

// 0x45f750: no RequiredPicNum, or ReqPicNumAmount of it in the pickup menu (used up; its item
// cleared when none are left): 1. Otherwise the NeedMore sound on the voice channel: 0.
unsigned char C3DPickupItem::CanPickUp(void)
{
	if (m_required_pic_num == -1) return 1;
	if (GetItemState(2, (short)m_required_pic_num) != 0)
	{
		// (debug logs)
		if (m_req_pic_num_amount <= PickupCount(m_required_pic_num))
		{
			AddPickupCount(m_required_pic_num, -m_req_pic_num_amount);
			if (PickupCount(m_required_pic_num) < 1) ClearItemState(2, (short)m_required_pic_num);
			return 1;
		}
	}
	if (m_need_more_sound == -1)
	{
		if (!IsChannelPlaying(g_voice_channel))
			g_voice_channel = PlayGameSound(-1, std::string(m_need_more_name), false);
	}
	else if (!IsChannelPlaying(g_voice_channel))
		g_voice_channel = PlayGameSoundIndex(-1, m_need_more_sound, false);
	return 0;
}

// 0x45e750: on: shown and colliding (unless it is waiting to respawn); off: hidden.
void C3DPickupItem::SetActive(unsigned char on)
{
	if (!on)
	{
		HideAgain();
		v232();
		v136(0);
		if (m_corona) m_corona->HideAgain();
		return;
	}
	if (m_respawn_left != 0) return;
	ShowAgain();
	if (m_corona && CanPickUp()) m_corona->ShowAgain();
	v234();
	v136(1);
}

// 0x45efb0: a gadget powerup lying in the level (not collected) is there only while its gadget
// is in the pickup menu.
void C3DPickupItem::SyncWithMenu(void)
{
	if (g_object_states[m_pickup_index] != 0) return;
	static const struct { const char *tag; short item; } gadgets[] = {
		{ "SONAR", 8 }, { "RAPIDFIRE", 3 }, { "SHIELDS", 20 }, { "FREEZETIME", 18 },
		{ "XRAY", 9 }, { "EARTHQUAKE", 2 }, { "SPEEDUP", 5 }, { "WATERGUN", 1 },
		{ "MEGABURP", 19 }, { "SPRINGSHOES", 7 } };
	for (unsigned k = 0; k < sizeof gadgets / sizeof gadgets[0]; k++)
		if (strcasecmp(m_tag, gadgets[k].tag) == 0)
		{
			SetActive(GetItemState(2, gadgets[k].item) != 0);	// element +0xe8
			return;
		}
}
