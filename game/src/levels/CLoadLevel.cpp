/*  CLoadLevel - see CLoadLevel.h. Reconstructed from Neutron2.exe 0x48e370..0x48ec70. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CLoadLevel.h"
#include "../C2DInGameMenu.h"
#include "../CEditor.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- not reconstructed yet

// C3DJimmy (a C3DObject: element = object - 0xc0) - its members the level exits use, at their
// 2002 element offsets: +0xb89 the level file Jimmy is in and +0x9c8 his start point there,
// +0xc25 / +0xb24 the level and start point to return to, +0xc18 the promotion, +0xb88 and
// +0xbd9 two flags RETURN sets.
static const char *JimmyString(CGameObject *jim, int off)
{
	(void)jim; (void)off;
	NEG_TODO(0x48e809, "C3DJimmy level / start point strings (+0x9c8, +0xb24, +0xb89, +0xc25)");
	return "";
}
static void JimmySetFlag(CGameObject *jim, int off)
{
	(void)jim; (void)off;
	NEG_TODO(0x48e907, "C3DJimmy +0xb88 / +0xbd9 = 1");
}
static void JimmySetPromotion(CGameObject *jim, int promotion)
{
	(void)jim; (void)promotion;
	NEG_TODO(0x48e9e4, "C3DJimmy +0xc18 = Promotion");
}
// C3DJimmy element slot 121 (+0x1e4): go to `level` at start point `start`, remembering
// `ret_level` / `ret_start` to come back to, with loading screen `screen`.
static void JimmyLoadLevel(CGameObject *jim, const char *level, const char *start,
						   const char *ret_level, const char *ret_start, int screen)
{
	(void)jim; (void)level; (void)start; (void)ret_level; (void)ret_start; (void)screen;
	NEG_TODO(0x48ea0f, "C3DJimmy element slot 121 (load level)");
}
// C3DJimmy element slot 79 (+0x13c)(1), when the exit has a FadeType.
static void JimmySlot79(CGameObject *jim, int a)
{
	(void)jim; (void)a;
	NEG_TODO(0x48eb7f, "C3DJimmy element slot 79");
}

// --------------------------------------------------------------------------- small helpers

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// 0x404f20: the type of item `item` of menu page `page` (0 when there is no such item).
static short GetItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p && item >= 0 && item < C2DInGameMenu::MENU_ITEMS) return p->items[item].type;
	return 0;
}

// 0x404dd0: set the type of item `item` of menu page `page`. The pickups come in pairs (10/21
// and 31/38): giving one also gives (and counts) the other.
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

// --------------------------------------------------------------------------- construction

CLoadLevel::CLoadLevel()													// 0x48e370
{
	static char name[] = "C3DLOADLEVEL";		// SetName upper-cases in place
	SetName(name);
	Log("C3DLoadLevel()");
	CLoadLevel::InitObject();					// (direct call)
	C3DSprite::SetObjectID('LOAD');				// 0x49b550
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_level_name, "none");
	m_sprite_index = 2;
	m_sprite_size = 100;
	strcpy(m_start_point, "none");
	strcpy(m_required_task, "none");
	m_required_level = 0;
	C3DSprite::InitSpriteProperties(200.0f);	// 0x49ce10
	CGameObject::v67(250.0f);					// radius ("Radius", f_034)
	CGameObject::v46(0);
	CGameObject::v32(1);
	CGameObject::v34(1);
	f_008 = 1;
	C3DSprite::InitPhysics();					// 0x49ca30
	m_fade_type = 0;
	m_sound_index = -1;
	m_exact_level = -1;
	m_fade_time = 1.0f;
	g_view_restore_x = 0.0f;					// the view C2DInGameMenu puts back after a fade
	g_view_restore_y = 0.0f;
	g_view_restore_z = 0.0f;
	g_view_restore_ax = 0.0f;
	g_view_restore_ay = 0.0f;
	g_view_restore = 0;
	m_promotion = 0;
	m_load_screen = 0;
	m_go_if_active = -1;
	// (debug log, empty in the shipped build)
}

CLoadLevel::~CLoadLevel()													// 0x48e5e0
{
}

// --------------------------------------------------------------------------- element-family slot

// 0x48ebf0: when a trigger fires the exit tagged "LABTONEIGH" at story stage 60, the menu's
// phone page gets item 1 and the stage goes on to 65.
void CLoadLevel::ev55(void)
{
	if (strcasecmp(m_tag, "LABTONEIGH") != 0) return;		// 0x4c4520 _stricmp
	if (GetTaskState("SCENE") == 60)						// 0x497a40
	{
		SetItemState(1, 1, 1);								// 0x404dd0
		SetTaskState("SCENE", 65);							// 0x497980
	}
}

// --------------------------------------------------------------------------- CGameObject-family slots

void CLoadLevel::InitObject(void)											// 0x48e620
{
	Log("InitObject()");
	C3DSprite::InitObject();
	RegisterProperty("LevelName",     m_level_name,      PROP_ALPHA, 0);
	RegisterProperty("StartPoint",    m_start_point,     PROP_ALPHA, 0);
	RegisterProperty("RequiredTask",  m_required_task,   PROP_ALPHA, 0);
	RegisterProperty("RequiredLevel", &m_required_level, PROP_LONG,  0);
	RegisterProperty("ExactLevel",    &m_exact_level,    PROP_LONG,  0);
	RegisterProperty("Radius",        &f_034,            PROP_FLOAT, 0);
	RegisterProperty("SoundIndex",    &m_sound_index,    PROP_LONG,  0);
	RegisterProperty("FadeType",      &m_fade_type,      PROP_LONG,  0);
	RegisterProperty("FadeTime",      &m_fade_time,      PROP_FLOAT, 0);
	RegisterProperty("Promotion",     &m_promotion,      PROP_LONG,  0);
	RegisterProperty("LoadScreen",    &m_load_screen,    PROP_LONG,  0);
	RegisterProperty("GoIfActive",    &m_go_if_active,   PROP_LONG,  0);
	SetObjectID('LOAD');									// element +0xc0
	// (debug log)
}

// 0x48e790: Jimmy (or his hover cycle) walked into the exit. The original's debug logs
// ("MENTEMP Touched a CLoadLevel to %s at %s using %d screen", "CLoadLevel::Touch A".."D", ...)
// are empty in the shipped build.
void CLoadLevel::AddTouching(CGameObject *other)
{
	CGameObject::AddTouching(other);						// 0x4ab970
	CGameObject *jim = NULL;
	if (other->NameContains((char *)"C3DJIMMY") || other->NameContains((char *)"C3DHOVERCYCLE"))	// vtable +0x18
		jim = FindObjectByTag("JIM1");						// 0x4aefb0
	if (m_go_if_active != -1 && !GetItemState(3, (short)m_go_if_active)) return;
	if (!jim) return;

	strcpy(g_task_file, g_empty_string);					// no savegame to restore in the next level
	if (strcasecmp(m_required_task, "none") != 0)
	{
		// Quirk: GetTaskState gives 0, not -1, for an unknown task, so the error is only logged
		// for a task whose state is -1.
		const int state = GetTaskState(m_required_task);
		if (state == -1) ErrorLog("ERROR: Task %s not found in in %s", m_required_task, m_tag);
		else
		{
			if (state < m_required_level) return;
			if (m_exact_level != -1 && m_exact_level != state) return;
		}
	}

	if (strcasecmp(m_level_name, "RETURN") == 0)
	{
		// "MENTEMP hit a RETURN loadpoint returning"
		JimmySetFlag(jim, 0xb88);
		JimmySetFlag(jim, 0xbd9);
		char level[80], start[80], ret_level[80], ret_start[80];
		strcpy(level, JimmyString(jim, 0xc25));
		strcpy(start, JimmyString(jim, 0xb24));
		strcpy(ret_level, JimmyString(jim, 0xc25));
		strcpy(ret_start, JimmyString(jim, 0xb24));
		JimmySetPromotion(jim, m_promotion);
		JimmyLoadLevel(jim, level, start, ret_level, ret_start, m_load_screen);	// element +0x1e4
		if (m_sound_index != -1) SetChannelVolume((short)PlaySoundIndex(-1, m_sound_index, false), g_sfx_volume);	// 0x4b8240, 0x4b8e30
		jim->v176(0.0f, 0.0f, 0.0f);						// vtable +0x2c0: stop
		return;
	}
	if (strcasecmp(m_level_name, "none") == 0) return;

	ev55();													// element +0xdc
	JimmySetPromotion(jim, m_promotion);
	JimmyLoadLevel(jim, m_level_name, m_start_point, JimmyString(jim, 0xb89), JimmyString(jim, 0x9c8), m_load_screen);
	if (m_sound_index != -1) SetChannelVolume((short)PlaySoundIndex(-1, m_sound_index, false), g_sfx_volume);
	if (m_fade_type != -1)
	{
		g_editor->m_load_pending = 0;						// the fade loads the level when it ends
		g_view_restore = 1;
		JimmySlot79(jim, 1);								// element +0x13c
	}
}

void CLoadLevel::v258(void)	{ C3DSprite::v258(); }							// 0x48ebe0

void CLoadLevel::v260(void)													// 0x48eba0
{
	strcpy(m_sprite_db, "icons.omt");
	C3DSprite::v260();
}

// --------------------------------------------------------------------------- factory

namespace neg {
CGameObject *New_CLoadLevel(void)	{ return new CLoadLevel; }				// factory 'LOAD' (0x718 bytes)
}
