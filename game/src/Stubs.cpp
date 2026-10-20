/*  Bodies for calls into code that is not reconstructed yet. See Bridge.h. */
#include "Bridge.h"
#include "CGameObject.h"
#include "Todo.h"
#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>

namespace neg {

bool EditorIsEditing(void)				{ NEG_TODO(0x4a4aa0, "CEditor +0x8c (editing)"); return false; }
bool GameIsRunning(void)				{ NEG_TODO(0x4b0ca0, "GameIsRunning"); return true; }
int  GameTypeTouchStart(CGameObject *, int)	{ NEG_TODO(0x14c, "CGameType vtable +0x14c"); return -1; }
int  GameTypeTouchEnd(CGameObject *, int)	{ NEG_TODO(0x150, "CGameType vtable +0x150"); return -1; }
int  GameTypeTouch154(CGameObject *, unsigned int) { NEG_TODO(0x154, "CGameType vtable +0x154"); return -1; }
void PhysicsRemoveObject(CGameObject *)	{ NEG_TODO(0x4b4070, "CPhysics::RemoveObject"); }
void StopObjectSounds(CGameObject *)	{ NEG_TODO(0x4b8dc0, "StopObjectSounds"); }
bool IsChannelPlaying(int)				{ NEG_TODO(0x4b89d0, "IsChannelPlaying"); return false; }

void PostGameMessage(int msg)
{
	// The original posts WM_USER+msg to its own window; CAweReal handles it on the next event pass.
	NEG_TODO(0x400, "PostGameMessage"); (void)msg;
}

void ErrorLog(const char *fmt, ...)		// 0x4b3b10: appends a line to dat\debug.log
{
	char buf[512]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
	SDL_Log("%s", buf);
}

}
