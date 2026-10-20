/*  The savegame system (save\*.tsk), the story task list, the demo levels and the texture
    scrolling of CJimmyGame: the free functions of Neutron2.exe 0x497090..0x4990a0.
    Reconstructed from the decompilation; each function names its original address.

    A .tsk file (OMediaStreamOperators, big-endian; strings are a length byte + the chars):
        string   version            "18:31:01" (the build time). The shipped save\*.tsk files are
                                    placeholders with "DUMMY   ": ReadTaskFile ignores those.
        int      level id           the game type's 4cc ('LV3E' ...)
        string   game file          .gam of the level (g_game_file)
        string   task file          the name it was saved under
        float    health             g_health
        short    lives              g_lives
        int      score              g_score
        float    x, y, z            camera (viewport) position
        int[50]  page 3 items       menu page 3 (level items) item types
        int[4200] object states     g_object_states
        int[50]  pickup counts      g_pickup_counts[2 * i]
        per task (as many as g_tasks has): string name, int state
        int      object count       objects saved (slot 108 set, or f_4c4)
        per object: its slot 284 (WriteState) data, in g_objects order
    ReadTaskFile reads up to the score and the next int (the camera x as an int).          */
#pragma once
#include <list>

class OMedia3DPolygon;

namespace neg {

// --------------------------------------------------------------------------- task list

// One task (0x68 bytes): the name objects give as "TaskName", and its state.
struct Task
{
	char	name[100];			// 0x00
	int		state;				// 0x64
};
extern std::list<Task *> g_tasks;							// 0x54e698 (head 0x54e69c, count 0x54e6a0)

void AddTask(const char *name, int state);					// 0x4978e0 ("AddItem")
void SetTaskState(const char *name, int state);				// 0x497980 (tasks and objects)
int  GetTaskState(const char *name);						// 0x497a40 (0 if unknown)

// --------------------------------------------------------------------------- savegames

// 0x497b10: reads the header of save\<name> into g_save_level_id .. g_save_cam_x and
// g_game_file. False when it is missing, cannot be opened, or is a "DUMMY   " placeholder.
bool ReadTaskFile(const char *name);
// 0x498af0: continue the game saved in save\<name> (new_level false: restored in place when it
// is the running level, else that level is loaded and LoadGame restores it), or (new_level
// true) start the level file <name> afresh.
void LoadTask(const char *name, bool new_level);
// 0x498440: puts the state of save\<name> into the running level (after its objects exist).
bool RestoreTask(const char *name);
// 0x498060: writes save\<name> (game state 2 "saving" while it does).
void WriteTaskFile(const char *name);
// 0x4983f0: WriteTaskFile, then back to game state 5 (C3DCheckPoint, C3DJimmy).
void SaveTask(const char *name);
// 0x498c70: after a game over: a new game from level1d.gam (autosaves deleted, items, pickups
// and tasks reset). Nothing while the score screen (0x545f0f) is up.
void NewGameAfterGameOver(void);

// --------------------------------------------------------------------------- demo

// 0x4990a0: the next level of the demo cycle (g_demo_level), started afresh.
void NextDemoLevel(void);

// --------------------------------------------------------------------------- texture scrolling

// A polygon whose texture coordinates move: v by dv per second, or - both speeds -99999 - a
// gentle wave of u and v (water). 12 bytes in the original.
struct TexScroll
{
	OMedia3DPolygon	*polygon;
	float			du, dv;		// du is never used for a plain scroll
};
enum { TEXSCROLL_WAVE_BITS = 0xc7c34f80 };					// -99999.0f, compared as bits
extern std::list<TexScroll *> g_tex_scrolls;				// 0x54e680 (head 0x54e684, count 0x54e688)

void ClearTexScrolls(void);									// 0x497090 (CJimmyGame, level end)
void UpdateTexScrolls(float dt);							// 0x4970e0 (CJimmyGame::Process)
void AddTexScroll(OMedia3DPolygon *polygon, float du, float dv);	// 0x4972b0 (CJimmyGame slot 368)

}
