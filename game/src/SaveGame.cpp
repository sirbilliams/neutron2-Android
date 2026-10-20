/*  The savegame system, the task list, the demo levels and the texture scrolling: the free
    functions of Neutron2.exe 0x497090..0x4990a0. See SaveGame.h for the .tsk format.

    The original's debug-log calls (0x463890, compiled empty in the shipped build) are kept as
    Trace(): silent unless NEG_SAVE_TRACE is set in the environment.
    Constants: 0x4caa44 = 0.                                                                */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "SaveGame.h"
#include "CGameObject.h"
#include "CLocalGameObject.h"
#include "CGameType.h"
#include "CEditor.h"
#include "C2DInGameMenu.h"
#include "InterfaceElement.h"
#include "Game.h"
#include "Globals.h"
#include "Todo.h"
#include "Sound.h"
#include "gen/globals.h"

#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMedia3DPolygon.h"
#include "OMediaViewPort.h"
#pragma GCC diagnostic pop

#include <SDL.h>
#include <math.h>
#include <new>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

std::string omf_SDLResolvePath(const std::string &in);		// engine/sdl: DOS path -> real file

namespace neg {
std::list<Task *>		g_tasks;			// 0x54e698
std::list<TexScroll *>	g_tex_scrolls;		// 0x54e680
}

// The original's empty debug log.
static void Trace(const char *fmt, ...)
{
	static int on = -1;
	if (on < 0) on = getenv("NEG_SAVE_TRACE") != NULL;
	if (!on) return;
	char buf[512];
	va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
	SDL_Log("[save] %s", buf);
}

static const char kVersion[] = "18:31:01";		// 0x53e7b8, the build time: written and required
static const char kDummy[] = "DUMMY   ";		// 0x53e724, the shipped placeholder saves

// String buffers. The file's strings are a length byte and up to 255 chars; the original reads
// them into smaller stack buffers (76..136 bytes). These are 256 so a long one cannot overrun.
enum { STR_MAX = 256 };

// --------------------------------------------------------------------------- helpers

static void IdString(char out[5], int id)		// a 4cc as text, as the logs print it
{
	out[0] = (char)(id >> 24); out[1] = (char)(id >> 16); out[2] = (char)(id >> 8); out[3] = (char)id; out[4] = 0;
}

static void DeleteFile(const char *dos_path)	// KERNEL32 DeleteFileA
{
	remove(omf_SDLResolvePath(dos_path).c_str());
}

static CLocalGameObject *Local(CGameObject *o)	{ return static_cast<CLocalGameObject *>(o); }

// Objects whose state goes into a save: slot 108 (f_00b, "saved in the .gam") or f_4c4.
static bool IsSaved(CGameObject *o)		{ return o->v108() || Local(o)->f_4c4; }

static void MenuShowLoading(int a)	{ NEG_TODO(0x40c0f0, "C2DInGameMenu loading screen"); (void)a; }

// 0x485360: the game state (0 menu/new game, 1 loading, 2 saving, 3 cut scene, 5 playing...),
// told to every object (copy of CEditor.cpp's).
static void SetGameState(int state)
{
	if (state == g_game_state) return;
	g_prev_game_state = g_game_state;
	g_game_state = state;
	if (state == 1)
	{
		if (g_545d98)
		{
			if (!g_level_requested) { g_54a468 = 1; MenuShowLoading(0); }
			else g_level_requested = 0;
		}
	}
	else
	{
		if (g_54a468 && state == 3) MenuShowLoading(1);
		g_54a468 = 0;
	}
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i) Local(*i)->v270(g_prev_game_state, state);
}

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// Menu items (menu helpers, copied): 0x404f20 get, 0x404dd0 set, 0x404ef0 clear the type of
// item `item` of menu page `page` (page 2 = pickups, 3 = level items).
static short GetItemState(short page, short item)				// 0x404f20
{
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p && item >= 0 && item < C2DInGameMenu::MENU_ITEMS) return p->items[item].type;
	return 0;
}

static void SetItemState(short page, short item, short state)	// 0x404dd0
{
	// The pickups come in pairs (10/21 and 31/38): giving one also gives (and counts) the other.
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

static void ClearItemState(short page, short item)				// 0x404ef0
{
	C2DInGameMenu::MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::Page(page)) != NULL)
		p->items[item].type = 0;
}

// 0x40c2b0 (a menu method, ecx = g_menu, `this` unused): the cut scene letterbox bars slide out.
static void LetterboxOut(void)
{
	if (g_545d98 == 1)
	{
		g_545d98 = 0;
		InterfaceElt *top = g_interface.Find("LetterBoxTop");
		InterfaceElt *bottom = g_interface.Find("LetterBoxBottom");
		top->QueueMorph(0, 1.0f, true, true, true, 640.0f, 64.0f, 640.0f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f);
		bottom->QueueMorph(0, 1.0f, false, true, true, 640.0f, 0.0f, 640.0f, 64.0f, 0.0f, 480.0f, 0.0f, 416.0f);
	}
}

// 0x4a3850: "EXIT REQUEST, CALLING QUIT (CLEAN EXIT)".
static void RequestQuit(void)		{ g_app->quit(); }

// --- not reconstructed yet
// 0x4104c0: element slot 59 (vtable +0xec) of every C3DPICKUPITEM (its CGameObject base at +0xc8).
static void PickupItemSlot59(CGameObject *o)	{ NEG_TODO(0x4104c0, "C3DPickupItem element slot 59"); (void)o; }
static void ResetPickupItems(void)												// 0x4104c0
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && (*i)->NameContains((char *)"C3DPICKUPITEM")) PickupItemSlot59(*i);
}
// A C3DAITrigger (element vtable +0xdc, its CGameObject base at +0xc8) activated by name.
static void AITriggerActivate(CGameObject *o, const char *name, int a)
{
	NEG_TODO(0x498aa3, "C3DAITrigger element slot 55 (activate)"); (void)o; (void)name; (void)a;
}
// C3DJimmy (g_jimmy - 0xc0, 2002 layout): the start point names at +0x9c8, +0xa18 and +0xb24.
static void JimmySetStartPoints(CGameObject *jimmy, const char *name)
{
	NEG_TODO(0x498cab, "C3DJimmy +0x9c8/+0xa18/+0xb24 = start point"); (void)jimmy; (void)name;
}
// C3DJimmy element slot 138 (vtable +0x228)(menu, jimmy's CLocalGameObject, a, b).
static void JimmySlot138(CGameObject *jimmy, C2DInGameMenu *menu, CGameObject *jimmy_go, int a, float b)
{
	NEG_TODO(0x498e54, "C3DJimmy element slot 138"); (void)jimmy; (void)menu; (void)jimmy_go; (void)a; (void)b;
}

// --------------------------------------------------------------------------- task list

void neg::AddTask(const char *name, int state)					// 0x4978e0
{
	// ("AddItem"; the CTaskList constructor has nine more copies of it inlined)
	Task *t = new (std::nothrow) Task;
	if (!t)
	{
		ErrorLog("ERROR: Unable to allocate variable %s in AddItem", name);
		return;
	}
	strcpy(t->name, name);
	t->state = state;
	g_tasks.push_back(t);
}

void neg::SetTaskState(const char *name, int state)				// 0x497980
{
	Trace("SETTING task %s to %d", name, state);
	Trace("SETTING task %s to %d", name, state);			// (logged twice)
	for (std::list<Task *>::iterator i = g_tasks.begin(); i != g_tasks.end(); i++)
		if (strcasecmp((*i)->name, name) == 0) (*i)->state = state;	// every match, no break
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		// (no NULL test of the object; the original tests its m_task_name - an array, never
		// NULL - and would log "ERROR: invalid TaskName for %s" with m_tag)
		CLocalGameObject *o = Local(*i);
		if (strcasecmp(name, o->m_task_name) == 0) o->v267(state);
	}
}

int neg::GetTaskState(const char *name)							// 0x497a40
{
	for (std::list<Task *>::iterator i = g_tasks.begin(); i != g_tasks.end(); i++)
		if (strcasecmp((*i)->name, name) == 0) return (*i)->state;
	return 0;
}

// --------------------------------------------------------------------------- 0x497b10

bool neg::ReadTaskFile(const char *name)
{
	char path[STR_MAX];
	strcpy(path, "save\\");
	strcat(path, name);
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	if (!fs.fileexists())
	{
		Trace("WARNING file %s does not exist for TaskGameLoad", name);
		fs.close();
		return false;
	}
	fs.open(omcfp_Read, false, false);
	if (!fs.isopen())
	{
		ErrorLog("ERROR opening file %s for TaskGameLoad", name);
		return false;
	}
	char version[STR_MAX];
	fs >> version;
	if (strcasecmp(version, kDummy) == 0)					// a placeholder: no game in it
	{
		fs.close();
		return false;
	}
	// (any other version is accepted here; RestoreTask insists on kVersion)
	char task_name[STR_MAX];
	fs >> g_save_level_id;
	fs >> g_game_file;
	fs >> task_name;										// read and dropped
	fs >> g_save_health;
	fs >> g_save_lives;
	fs >> g_save_score;
	fs >> g_save_cam_x;										// sic: the camera x (a float), read as an int
	fs.close();
	return true;
}

// --------------------------------------------------------------------------- 0x498af0

void neg::LoadTask(const char *name, bool new_level)
{
	if (!new_level)
	{
		if (!ReadTaskFile(name)) return;
		strcpy(g_task_file, name);
		char now[5], was[5];
		IdString(now, g_save_level_id);
		IdString(was, g_gametype->m_object_id);
		Trace("Was in %s, now loading level %s", was, now);
		if (g_save_level_id == g_gametype->m_object_id)
		{
			// the saved level is running: restore it in place
			SetGameState(1);
			ResetPickupItems();								// 0x4104c0
			RestoreTask(name);
			SetGameState(5);
			return;
		}
		// else load its level; CEditor::LoadGame restores g_task_file once the objects exist
		g_editor->RequestLevel(g_game_file);				// vtable +0x110
		return;
	}
	strcpy(g_task_file, g_empty_string);
	if (g_gametype->m_object_id == 'LPRO')
		g_editor->RequestLevelDelayed((char *)name, 4.0f);	// vtable +0x114
	else
		g_editor->RequestLevel((char *)name);
}

// --------------------------------------------------------------------------- 0x498440

bool neg::RestoreTask(const char *name)
{
	char path[STR_MAX];
	strcpy(path, "save\\");
	strcat(path, name);
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	if (!fs.fileexists())
	{
		Trace("WARNING file %s does not exist for TaskGameLoad", name);
		fs.close();
		return false;
	}
	fs.open(omcfp_Read, false, false);
	if (!fs.isopen())
	{
		ErrorLog("ERROR opening file %s for TaskGameLoad", name);
		return false;
	}
	StopAllChannels();										// 0x4b8990

	char unused[STR_MAX];
	strcpy(unused, g_empty_string);							// (a local the original fills and never reads)
	(void)unused;
	char expected[12];
	strcpy(expected, kVersion);
	char version[STR_MAX];
	fs >> version;
	if (strcasecmp(expected, version) != 0)
	{
		// Not written by this build: the game quits.
		ErrorLog("ERROR: Invalid header file");
		RequestQuit();										// 0x4a3850
		DrawText(200, 200, 'MED', "INVALID FILE VERSION");
		return false;										// (not closed: the stream's destructor does it)
	}

	int level_id;
	fs >> level_id;											// read and dropped
	fs >> g_game_file;
	char task_name[STR_MAX];
	fs >> task_name;										// read and dropped

	// health, at least 50 / 35 / 20 by difficulty
	float health;
	fs >> health;
	switch (g_difficulty)
	{
		case 0: if (health < 50.0f) health = 50.0f; break;		// 0x4ca4b0
		case 1: if (health < 35.0f) health = 35.0f; break;		// 0x4e560c
		case 2: if (health < 20.0f) health = 20.0f; break;		// 0x4caa40
	}
	g_health = health;
	fs >> g_lives;
	int score;
	fs >> score;
	g_score = score;

	float x, y, z;
	fs >> x;
	fs >> y;
	fs >> z;
	g_viewport->setx(x);									// (2002 +0x44 / +0x48 / +0x4c)
	g_viewport->sety(y);
	g_viewport->setz(z);

	for (int i = 0; i < 50; i++)							// the level items
	{
		ClearItemState(3, (short)i);
		int state;
		fs >> state;
		SetItemState(3, (short)i, (short)state);
	}
	for (int i = 0; i < 4200; i++) fs >> g_object_states[i];
	for (int i = 0; i < 50; i++)							// the pickups
	{
		ClearItemState(2, (short)i);
		fs >> g_pickup_counts[2 * i];
		if (g_pickup_counts[2 * i] > 0) SetItemState(2, (short)i, 1);
	}

	// The task states: as many as there are tasks now (the file does not say).
	int ntasks = 0;
	for (std::list<Task *>::iterator i = g_tasks.begin(); i != g_tasks.end(); i++) ntasks++;
	if (ntasks > 0)
	{
		for (int k = 0; k < ntasks; k++)
		{
			char task[STR_MAX];
			fs >> task;
			// (the original also stops when the buffer's address is NULL)
			int state;
			fs >> state;
			if (state < 0) break;
			Trace("LoadTask 5  %s  %d", task, state);
			SetTaskState(task, state);
			// ...and once more for the objects (SetTaskState just did it, without the "none" test)
			for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
			{
				CLocalGameObject *o = Local(*i);			// (no NULL test)
				if (strcasecmp(o->m_task_name, "none") != 0 && strcasecmp(o->m_task_name, task) == 0)
				{
					o->v267(state);
					Trace("TaskState %s found, state set to %d", task, state);
				}
			}
		}
		for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
			if (*i) Local(*i)->v266();
	}

	// The objects: as many as the level has, in g_objects order, each numbered (f_438).
	int level_count = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && IsSaved(*i)) level_count++;
	int file_count;
	fs >> file_count;
	if (file_count == level_count)
		Trace("Save File has %d objects, level has %d objects", file_count, level_count);
	else
		ErrorLog("Error: Save File has %d objects, level has %d objects", file_count, level_count);
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && IsSaved(*i)) (*i)->f_438 = (unsigned int)n++;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o || !IsSaved(o)) continue;
		const unsigned int old = o->f_438;
		Local(o)->ReadState(fs);							// slot 285
		if (old != o->f_438)
			ErrorLog("ERROR: Object Index's dont match Old=%d New=%d", (int)old, (int)o->f_438);
		Trace("Loaded object %d %s", (int)o->f_438, o->m_tag);
	}
	fs.close();

	g_tasks_applied = 0;
	SetGameState(5);
	g_gametype->PauseGame(false);							// vtable +0x110
	CGameObject *t = FindObjectByTag("LOADEDGAME");
	if (t && t->NameContains((char *)"C3DAITRIGGER")) AITriggerActivate(t, "LOADEDGAME", 0);
	return true;
}

// --------------------------------------------------------------------------- 0x498060 / 0x4983f0

void neg::WriteTaskFile(const char *name)
{
	int score = g_score;									// (taken before the state change)
	SetGameState(2);
	char path[STR_MAX];
	strcpy(path, "save\\");
	strcat(path, name);
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	fs.open(omcfp_Write, true, true);

	float health = g_health;
	char version[12];
	strcpy(version, kVersion);
	fs << version;
	fs << g_gametype->m_object_id;							// (element +0x490)
	fs << g_game_file;
	fs << (char *)name;
	fs << health;
	fs << g_lives;
	fs << score;
	float x = g_viewport->getx(), y = g_viewport->gety(), z = g_viewport->getz();
	fs << x;
	fs << y;
	fs << z;
	for (int i = 0; i < 50; i++)
	{
		int state = GetItemState(3, (short)i);
		fs << state;
	}
	for (int i = 0; i < 4200; i++) fs << g_object_states[i];
	for (int i = 0; i < 50; i++) fs << g_pickup_counts[2 * i];
	for (std::list<Task *>::iterator i = g_tasks.begin(); i != g_tasks.end(); i++)
	{
		fs << (*i)->name;
		fs << (*i)->state;
	}

	int count = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && IsSaved(*i)) count++;
	Trace("MEBTEMP saving %d objects", count);
	fs << count;
	int n = 0;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		if (*i && IsSaved(*i)) (*i)->f_438 = (unsigned int)n++;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		CGameObject *o = *i;
		if (!o || !IsSaved(o)) continue;
		Trace("Saving object %d %s", (int)o->f_438, o->m_tag);
		Local(o)->WriteState(fs);							// slot 284
	}
	fs.close();
}

void neg::SaveTask(const char *name)
{
	char unused[52];
	strcpy(unused, g_empty_string);							// (a local the original fills and never reads)
	(void)unused;
	WriteTaskFile(name);
	SetGameState(5);
}

// --------------------------------------------------------------------------- 0x498c70

void neg::NewGameAfterGameOver(void)
{
	if (g_545f0f) return;
	g_health = 100.0f;
	g_lives = 0;
	g_score = 0;
	JimmySetStartPoints(g_jimmy, "PHONEBOOTH");				// (the original writes through NULL without a Jimmy)
	strcpy(g_task_file, g_empty_string);
	static const char *const autosaves[] = {
		"save\\autosave.tsk",
		"save\\autosave01.tsk", "save\\autosave02.tsk", "save\\autosave03.tsk", "save\\autosave04.tsk",
		"save\\autosave05.tsk", "save\\autosave06.tsk", "save\\autosave07.tsk", "save\\autosave08.tsk",
		"save\\autosave09.tsk", "save\\autosave10.tsk", "save\\autosave11.tsk", "save\\autosave12.tsk",
		"save\\autosave13.tsk", "save\\autosave14.tsk", "save\\autosave15.tsk", "save\\autosave16.tsk",
		"save\\autosave17.tsk" };
	for (unsigned i = 0; i < sizeof(autosaves) / sizeof(autosaves[0]); i++) DeleteFile(autosaves[i]);
	LoadTask("level1d.gam", true);
	LetterboxOut();											// 0x40c2b0
	g_gametype->PauseGame(false);							// vtable +0x110
	g_resume_game_state = 9;
	JimmySlot138(g_jimmy, NULL, g_jimmy, 0, 0.0f);			// (through NULL without a Jimmy)

	memset(g_object_states, 0, 4200 * sizeof(int));
	for (int i = 0; i < 50; i++)
	{
		ClearItemState(2, (short)i);
		g_pickup_counts[2 * i] = 0;
	}
	static const short level_items[] = { 0x21, 0x1d, 0x19, 0x1a, 0x1b, 0x1c, 0x23, 0x24, 0x22, 0x1e, 0x13 };
	for (unsigned i = 0; i < sizeof(level_items) / sizeof(level_items[0]); i++) ClearItemState(3, level_items[i]);
	g_545efc = 0;											// the main menu's level pictures (0x40a860)
	g_545f00 = 0;
	g_545f04 = 0;
	g_545f08 = 0;
	for (std::list<Task *>::iterator i = g_tasks.begin(); i != g_tasks.end(); i++)
		SetTaskState((*i)->name, 0);
	// a new game starts with pickups 32 and 25
	SetItemState(2, 0x20, 1);
	g_pickup_counts[2 * 0x20]++;
	SetItemState(2, 0x19, 1);
	g_pickup_counts[2 * 0x19]++;
}

// --------------------------------------------------------------------------- 0x4990a0

void neg::NextDemoLevel(void)
{
	// (a switch in the original)
	static const char *const levels[17] = {
		"level3c.gam", "level3d.gam", "level3a.gam", "level3e.gam", "level5d.gam", "vr01.gam",
		"level1b.gam", "level1_a.gam", "level1_c.gam", "level1.gam", "level1c.gam", "level5a.gam",
		"level5b.gam", "level5b1.gam", "level5c.gam", "level4b.gam", "level5f.gam" };
	const int i = g_demo_level;
	g_demo_level = i + 1;
	if ((unsigned)i < 17)
	{
		LoadTask(levels[i], true);
		return;
	}
	LoadTask("level1d.gam", true);
	g_demo_level = 0;
}

// --------------------------------------------------------------------------- texture scrolling

void neg::ClearTexScrolls(void)									// 0x497090
{
	while (g_tex_scrolls.size() != 0)
	{
		if (g_tex_scrolls.front()) delete g_tex_scrolls.front();
		g_tex_scrolls.pop_front();
	}
}

void neg::UpdateTexScrolls(float dt)							// 0x4970e0
{
	if (g_gametype->IsPaused()) return;						// vtable +0x114
	if (!g_tex_scroll_on) return;
	g_tex_scroll_time = g_tex_scroll_time + dt;
	for (std::list<TexScroll *>::iterator it = g_tex_scrolls.begin(); it != g_tex_scrolls.end(); ++it)
	{
		TexScroll *s = *it;
		// (a copy of the vertex list is made and thrown away, as shipped)
		omt_3DPolygonVertexList copy = s->polygon->get_vertices();
		(void)copy;
		unsigned int du, dv;
		memcpy(&du, &s->du, 4);
		memcpy(&dv, &s->dv, 4);
		for (short i = 0; i < (int)s->polygon->get_vertices().size(); i++)
		{
			OMedia3DPolygonVertex &v = s->polygon->get_vertices()[i];
			if (du == TEXSCROLL_WAVE_BITS && dv == TEXSCROLL_WAVE_BITS)
			{
				// x87 (53-bit): v first (kept on the stack), then u, then v is stored
				const double t = g_tex_scroll_time;
				const double nv = ((cos(t * 3.0) + 1.0) * 0.5 - 0.5) * (double)dt * (double)0.12f + (double)v.v;	// 0x4cb1e8, 0x516aec
				v.u = (float)(((cos(t * (double)3.2f) + 1.0) * 0.5 - 0.5) * (double)dt * (double)0.15f + (double)v.u);	// 0x516ae8, 0x4d7f7c
				v.v = (float)nv;
			}
			else
				v.v = dt * s->dv + v.v;						// (du is not used)
		}
	}
}

void neg::AddTexScroll(OMedia3DPolygon *polygon, float du, float dv)	// 0x4972b0
{
	TexScroll *s = new TexScroll;
	s->polygon = polygon;
	s->du = du;
	s->dv = dv;
	g_tex_scrolls.push_back(s);
}
