/*  Calls from reconstructed code into parts that are not reconstructed yet (CEditor, CGameType,
    the object factory, physics, sound). Each one is a small named function so the call sites read
    like the original; Stubs.cpp holds the TODO bodies until the real code replaces them.     */
#pragma once
class CGameObject;

namespace neg {
bool EditorIsEditing(void);								// g_editor +0x8c
bool GameIsRunning(void);								// 0x4b0ca0
void PostGameMessage(int msg);							// PostMessage(NULL, WM_USER, msg, 0) -> CAweReal
int  GameTypeTouchStart(CGameObject *o, int id);		// g_gametype vtable +0x14c
int  GameTypeTouchEnd(CGameObject *o, int id);			// g_gametype vtable +0x150
int  GameTypeTouch154(CGameObject *o, unsigned int a);	// g_gametype vtable +0x154
}
