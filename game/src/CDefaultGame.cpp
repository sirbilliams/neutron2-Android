/*  CDefaultGame and the level-load helpers next to it in Neutron2.exe (0x480230, 0x480320) and the
    waypoint list reset (0x4b9b60). See CDefaultGame.h.                                         */

#include "CDefaultGame.h"
#include "CEditor.h"
#include "Game.h"
#include "Todo.h"
#include "gen/globals.h"
#include <SDL.h>

namespace neg {
bool					g_static_objects_created = false;
std::list<CWayPoint *>	g_waypoints;
}
using namespace neg;

// --------------------------------------------------------------------------- CEditor calls

// CEditor element vtable +0x118 (0x4aa0b0): opens omt\Editor.omt (0x56af18 stream, 0x56af14 database).
static void EditorOpenDatabase(void)		{ g_editor->OpenEditorDatabase(); }
// CEditor element vtable +0x12c (0x4aa180): "DrawSplash" - shows splash canvas `id` of Editor.omt.
static void EditorDrawSplash(int id)		{ g_editor->DrawSplash(id); }

// --------------------------------------------------------------------------- CDefaultGame

CDefaultGame::CDefaultGame()
{
	g_static_objects_created = false;		// 0x54a3d8
}

CDefaultGame::~CDefaultGame()												// 0x4801f0
{
}

// --------------------------------------------------------------------------- 0x480230

// Called by LoadNewLevel with the level id ('MAIN' at start-up, the AWEMSG_LOAD parameter later),
// which it does not use: it always makes a CDefaultGame, then has the editor open its database and
// show splash 3.
CGameType *neg::CreateGameType(unsigned int level)
{
	(void)level;
	CGameType *game = NULL;
	if (g_editor)						// original: if (g_editor)
	{
		game = new CDefaultGame;			// 0x55c bytes
		EditorOpenDatabase();				// g_editor vtable +0x118
		EditorDrawSplash(3);				// g_editor vtable +0x12c
	}
	return game;
}

// --------------------------------------------------------------------------- 0x480320

// Called from CEditor (0x4a8420) once a level has been loaded.
void neg::CreateStaticObjects(void)
{
	if (!g_static_objects_created)
	{
		g_static_objects_created = true;
		SDL_Log("CreateStaticObjects, spawning 3JIM");	// (debug log, empty in the shipped build)
		CreateObject('3JIM');
	}
}

// --------------------------------------------------------------------------- 0x4b9b60

void neg::ClearWayPoints(void)
{
	g_waypoints.clear();					// the nodes only; the waypoints are game objects (DeleteAllObjects)
	g_6f75e4 = 0;
}
