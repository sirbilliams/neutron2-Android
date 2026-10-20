/*  CTaskList - see CTaskList.h. Reconstructed from Neutron2.exe; each function names its
    original address. Constants: 0x4caa44 = 0, 0x4cb218 = 0.001, 0x4dce54 = 15, 0x4caa5c = 0.5. */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "CTaskList.h"
#include "C2DInGameMenu.h"
#include "CEditor.h"
#include "SaveGame.h"
#include "Game.h"
#include "Globals.h"
#include "gen/globals.h"
#pragma GCC diagnostic pop

#include <string.h>
#include <strings.h>

using namespace neg;

// Windows virtual keys the demo presses (g_keys[])
enum { VK_RETURN_ = 0x0d, VK_SPACE_ = 0x20, VK_LEFT_ = 0x25, VK_UP_ = 0x26, VK_RIGHT_ = 0x27, VK_DOWN_ = 0x28 };

// --------------------------------------------------------------------------- 0x497430

CTaskList::CTaskList()
{
	SetName((char *)"CTASKLIST");
	// The game's tasks and their first states (the original inlines AddItem 0x4978e0 for the
	// first nine). Appended to the global list each time a CTaskList is made.
	AddTask("MOM", 0);
	AddTask("LIBBY", 0);
	AddTask("BENNY", 0);
	AddTask("CARLTASK", 100);
	AddTask("SCENE", 50);
	AddTask("DINO", 0);
	AddTask("CLONE", 0);
	AddTask("KITTY1", 0);
	AddTask("KITTY2", 0);
	AddTask("KITTY3", 0);
	AddTask("HYDRANT1", 0);
	AddTask("HYDRANT2", 0);
	AddTask("REACTOR", 0);
	AddTask("PROMOTION", 0);
	v36(0);
	v107(0);											// not written to a .gam
	g_tasks_applied = 0;
	g_54e80c = 0;
	strcpy(g_54e794, g_empty_string);
	g_54e825 = 0;
	v52(0);
}

// 0x4978d0: nothing of its own (the tasks stay in g_tasks)
CTaskList::~CTaskList()
{
}

// --------------------------------------------------------------------------- slots

void CTaskList::Process(float dt)						// 0x497d60
{
	CGameObject::Process(dt);							// (CGameObject's, 0x4ae0b0, not CLocalGameObject's)
	if (g_editor->m_editing)
	{
		g_tasks_applied = 0;
		UpdateDemo(dt);
		return;
	}
	if (!g_tasks_applied)
	{
		g_tasks_applied = 1;
		v286();
	}
	UpdateDemo(dt);
}

void CTaskList::v242(float t)							// 0x497d30
{
	CGameObject::v242(t);								// (0x464ee0, empty)
	if (g_editor->m_editing) g_tasks_applied = 0;
}

void CTaskList::v260()									// 0x497a80
{
	g_tasks_applied = 0;
}

void CTaskList::v270(int a2, int a3)					// 0x498f70
{
	// a2 = the previous game state, a3 = the new one (SetGameState 0x485360)
	CLocalGameObject::v270(a2, a3);
	if (g_demo_mode == 0) return;
	if (a3 == 4)										// menu
	{
		g_menu->f_4e0 = 2.0f;
		return;
	}
	if (a3 == 3)										// cut scene
	{
		if (g_demo_mode == 1) g_cutscene_skip = 1;
		else if (g_demo_mode == 2) g_demo_cutscene_time = 20.0f;
		return;
	}
	if (a3 != 5) return;								// playing: (re)start the demo timers
	switch (g_demo_mode)
	{
		case 1:
			if (!(g_demo_time > 0.0f)) g_demo_time = 20.0f;
			break;
		case 2:
			if (!(g_demo_time > 0.0f)) g_demo_time = 30.0f;
			break;
		case 3:
			if (!(g_demo_time > 0.0f)) g_demo_time = 90.0f;
			g_demo_key_time = 1.0f;
			break;
		case 4:
			if (!(g_demo_time > 0.0f)) g_demo_time = 300.0f;
			g_demo_key_time = 1.0f;
			break;
	}
}

void CTaskList::v286()									// 0x497a90
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		if (!*i) continue;
		CLocalGameObject *o = static_cast<CLocalGameObject *>(*i);
		int state = 0;
		for (std::list<Task *>::iterator t = g_tasks.begin(); t != g_tasks.end(); t++)
			if (strcasecmp((*t)->name, o->m_task_name) == 0) { state = (*t)->state; break; }
		o->f_4a0 = state;								// (slot 267's field, set directly)
		o->v266();
	}
}

// --------------------------------------------------------------------------- 0x497dc0

void CTaskList::UpdateDemo(float dt)
{
	if (g_demo_mode == 0) return;
	if (g_game_state == 3)								// cut scene: mode 2 skips it after a while
	{
		if (!(g_demo_cutscene_time > 0.0f)) return;
		if (g_demo_mode != 2) return;
		g_demo_cutscene_time = g_demo_cutscene_time - dt;
		if (g_demo_cutscene_time > 0.0f) return;
		g_demo_cutscene_time = 0;
		g_cutscene_skip = 1;
		return;
	}
	if (g_game_state != 5) return;

	if (g_demo_time > 0.0f)
	{
		DrawText(320, 190, 'SMAL', "%d", (int)g_demo_time);	// (0x4a1a80, a copy of DrawText)
		if (g_keys['X']) g_demo_mode = 0;				// X stops the demo
		g_demo_time = g_demo_time - dt;
		if (!(g_demo_time > 0.0f))
		{
			g_demo_time = 0;
			NextDemoLevel();							// 0x4990a0
			return;
		}
	}
	if (g_demo_mode < 3 || g_demo_mode > 4) return;

	// modes 3 and 4: every few seconds release all keys and press a random set
	if (!(g_demo_key_time > 0.0f)) return;
	g_demo_key_time = g_demo_key_time - dt;
	if (g_demo_key_time > 0.0f) return;
	unsigned int r = *g_random_seed * 0x19660d + 0x7fff;
	*g_random_seed = r;
	memset(g_keys, 0, 255);								// (63 dwords, a word and a byte: not g_keys[255])
	const int n = ((int)((r >> 16) * 1001) >> 16) & 0xffff;		// 0..1000
	g_demo_key_time = (float)((double)n * (double)0.001f * 15.0);	// 0..15 s
	r = *g_random_seed * 0x19660d + 0x7fff;
	*g_random_seed = r;
	switch (((int)((r >> 16) * 11) >> 16) & 0xffff)		// 0..10 (10 presses nothing)
	{
		case 0: g_keys[VK_LEFT_] = 1; break;
		case 1: g_keys[VK_SPACE_] = 1; return;
		case 2: g_keys[VK_RETURN_] = 1; return;
		case 3: g_keys[VK_DOWN_] = 1; return;
		case 4: g_keys[VK_UP_] = 1; return;
		case 5: g_keys[VK_RIGHT_] = 1; break;
		case 6: g_keys[VK_RIGHT_] = 1; g_keys[VK_UP_] = 1; return;
		case 7: g_keys[VK_UP_] = 1; g_keys[VK_SPACE_] = 1; return;
		case 8: g_keys[VK_UP_] = 1; g_keys[VK_RETURN_] = 1; return;
		case 9: g_keys[VK_UP_] = 1; g_keys[VK_LEFT_] = 1; return;
		default: return;
	}
	g_demo_key_time = g_demo_key_time * 0.5f;			// left or right alone: half as long
}
