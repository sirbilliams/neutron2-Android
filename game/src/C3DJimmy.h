/*  C3DJimmy ('3JIM') - the player: Jimmy Neutron. A C3DPlayer (keyboard / mouse / joystick
    movement, jump, jetpack, climbing, chase camera) plus everything only Jimmy has: his Granny
    model and animation set, his gadgets (shrink ray, water balloons, grappling hook, bubbles,
    burp gun, the PDA menu), health, the vehicles he can ride (rocket ship, jet pack) and the
    level start / restart logic. The one C3DJimmy of the game is made by CreateStaticObjects
    (CreateObject('3JIM') after the first level load) and kept in g_jimmy (0x545fd0, its
    CGameObject part).
    Reconstructed from Neutron2.exe: factory '3JIM', constructor 0x443830 (0x4efc bytes),
    destructor 0x4446a0 (deleting thunks 0x444670 / 0x44e8a0 / 0x44e8b0 / 0x44e8c0..0x44e8e0).

    Original layout (classes.json): as C3DPlayer (element vtable 0x4e5350, 166 slots: C3DPlayer's
    127 + 127..165; OMediaAnim at +0x90, vtable 0x4e5300; listener +0x30, vtable 0x4e533c;
    CLocalGameObject at +0xc0, vtable 0x4e4e84, 286 slots), own members 0xcac..0x4ef4 (C3DPlayer's
    data ends at 0xcac), vtordisp 0x4ef4, streamer 0x4ef8.

    The constructor builds Jimmy's helpers and keeps them (not deleted by the destructor):
      g_menu (C2DInGameMenu, the PDA), f_4c90 (CTaskList), f_cc8 C3DRocketShip, f_ccc C3DJetPack,
      f_cc4 C3DShadow (his blob shadow), f_4c8c C3DTargetCursor, f_d4c C3DPointCursor (points at
      the nearest pickup, ev164), f_cd0[20] C3DWaterBalloon, f_d20 C3DGraplingHook,
      f_4d00[20] / f_4d50[1] C3DMagicParticle (burp-gun / bubble sparkles, ev154..ev157),
      f_d5c C3DGoddard (his dog), f_d40 C3DRadar (the HUD panel the menu shows with R),
      f_d44 C3DASEObj (the bubble he floats in, ev146 / ev147).
    Most of those classes are not reconstructed yet: their pointers are typed with the nearest
    reconstructed base (C3DPlayerObject, C3DAnimated, C3DPermanentSprite) and created through the
    factory's New_<Class> where the class has a factory ID (weak TODO stubs until it exists), else
    through a TODO stub here; until then they stay NULL and every use is NULL-checked (port:).

    This class is written in two parts (jimmy/C3DJimmy_a.{inc,cpp}: everything below 0x448b40,
    jimmy/C3DJimmy_b.{inc,cpp}: 0x448b40 and above). Each part declares the slots it writes in
    its .inc. Element-family game slots are `ev<n>` (`// ESLOT n`, n = vtable offset / 4) unless a
    log string names them; CGameObject-family slots keep their CGameObject names (`// SLOT n`).
    Members are f_<offset in the original object> (CGameObject-family code sees them at
    offset - 0xc0).

    Hooks: C3DPlayer.h declares neg::JimmyJetpackOff / JimmyJetpackSound / JimmyJetpackFlame /
    JimmyActivateMenuItem weak; C3DJimmy.cpp defines them.                                    */
#pragma once
#include "C3DPlayer.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"

class CTaskList;
class C3DPermanentSprite;
class OMediaStreamOperators;

class C3DJimmy : public C3DPlayer
{
	public:
	C3DJimmy();																// 0x443830
	virtual ~C3DJimmy();													// 0x4446a0

#include "jimmy/C3DJimmy_a.inc"
#include "jimmy/C3DJimmy_b.inc"

	// ---- data members (object offsets). Names say what is known; "?" = not established.
	int					f_cac;			// 0xcac  0
	// 0xcb0..0xcbb never used
	C3DObject			*f_cbc;			// 0xcbc  effect object hidden when the shrink ray stops (0; set elsewhere)
	CGameObject			*f_cc0;			// 0xcc0  0 (ev118 calls an empty function on it)
	C3DPermanentSprite	*f_cc4;			// 0xcc4  C3DShadow: the blob shadow under Jimmy (ev136)
	C3DPlayerObject		*f_cc8;			// 0xcc8  C3DRocketShip he can fly (a C3DFlyingObject)
	C3DPlayerObject		*f_ccc;			// 0xccc  C3DJetPack (a C3DFlyingObject; +0x9f0 its fire, +0x9f4 its sound)
	C3DPermanentSprite	*f_cd0[20];		// 0xcd0  C3DWaterBalloon pool
	C3DAnimated			*f_d20;			// 0xd20  C3DGraplingHook (ev163)
	// 0xd24 never used
	C3DAnimated			*f_d28;			// 0xd28  props attached to Jimmy's bones (ev88), dropped by v260
	C3DAnimated			*f_d2c;			// 0xd2c
	C3DAnimated			*f_d30;			// 0xd30
	C3DAnimated			*f_d34;			// 0xd34  (detached first of the five? see v260's order: d28 d2c d30 d38 d34)
	C3DAnimated			*f_d38;			// 0xd38
	int					f_d3c;			// 0xd3c  0
	C3DPermanentSprite	*f_d40;			// 0xd40  C3DRadar: the HUD panel (the original keeps its CGameObject part)
	C3DAnimated			*f_d44;			// 0xd44  C3DASEObj: the bubble (energybubble.ase; the original keeps its CGameObject part)
	int					f_d48;			// 0xd48  0
	C3DAnimated			*f_d4c;			// 0xd4c  C3DPointCursor (the original keeps its CGameObject part)
	short				f_d50;			// 0xd50  0
	int					f_d54;			// 0xd54  0
	short				f_d58;			// 0xd58  0
	C3DPlayerObject		*f_d5c;			// 0xd5c  C3DGoddard, Jimmy's dog (a C3DAI)
	float				f_d60;			// 0xd60  health, 0..100 (50; ev160 adds)
	int					f_d64;			// 0xd64  0
	short				f_d68;			// 0xd68  -1
	short				f_d6a;			// 0xd6a  4
	unsigned char		f_d6c;			// 0xd6c  0; Process shows the "SCENE" task state when set (debug)
	unsigned char		f_d6d;			// 0xd6d  0
	// 0xd6e..0xd6f never used
	OMedia3DPoint		f_d70[500];		// 0xd70  ?
	OMedia3DPoint		f_2cb0[500];	// 0x2cb0 ?
	OMedia3DPoint		f_4bf0;			// 0x4bf0 position saved by ev150 / ev151
	int					f_4c00;			// 0x4c00 the level (g_gametype ID) of f_4bf0
	int					f_4c04;			// 0x4c04 0
	int					f_4c08;			// 0x4c08 sound channel (-1)
	int					f_4c0c;			// 0x4c0c sound channel (-1) "water stream"
	int					f_4c10;			// 0x4c10 sound channel (-1) "shrinkray"
	int					f_4c14;			// 0x4c14 sound channel (-1)
	int					f_4c18;			// 0x4c18 0 (ev165: copied from the start point)
	int					f_4c1c;			// 0x4c1c sound channel (-1)
	int					f_4c20;			// 0x4c20 sound channel / level music (-1)
	int					f_4c24;			// 0x4c24 sound channel (-1) "not available"
	char				f_4c28[100];	// 0x4c28 "none"; ev165: from the start point (the level music name)
	C3DPermanentSprite	*f_4c8c;		// 0x4c8c C3DTargetCursor
	CTaskList			*f_4c90;		// 0x4c90 the task list
	int					f_4c94;			// 0x4c94 0
	int					f_4c98;			// 0x4c98 0
	float				f_4c9c;			// 0x4c9c 0
	int					f_4ca0;			// 0x4ca0 0
	unsigned char		f_4ca4;			// 0x4ca4 0
	unsigned char		f_4ca5;			// 0x4ca5 0 (ev133: an animation playing out, see there)
	unsigned char		f_4ca6;			// 0x4ca6 0 (surface flag, CJimmyGame)
	unsigned char		f_4ca7;			// 0x4ca7 0 (surface flag, CJimmyGame)
	OMedia3DVector		f_4ca8;			// 0x4ca8
	unsigned char		f_4cb8;			// 0x4cb8 0: AddTouching ignores missiles / bots / plants while set
	OMedia3DPoint		f_4cbc;			// 0x4cbc position of the start point he came in at (ev165)
	float				f_4ccc;			// 0x4ccc 0
	float				f_4cd0;			// 0x4cd0 0
	unsigned char		f_4cd4;			// 0x4cd4 0
	float				f_4cd8;			// 0x4cd8 0 (ev163: the camera pitch when the hook was aimed)
	float				f_4cdc;			// 0x4cdc 5
	int					f_4ce0;			// 0x4ce0 0
	float				f_4ce4;			// 0x4ce4 0
	float				f_4ce8;			// 0x4ce8 0
	unsigned char		f_4cec;			// 0x4cec 1
	float				f_4cf0;			// 0x4cf0 0
	int					f_4cf4;			// 0x4cf4 0
	int					f_4cf8;			// 0x4cf8 1
	unsigned char		f_4cfc;			// 0x4cfc 0
	unsigned char		f_4cfd;			// 0x4cfd 0
	unsigned char		f_4cfe;			// 0x4cfe 1 (0 while the constructor builds the helpers)
	C3DPermanentSprite	*f_4d00[20];	// 0x4d00 C3DMagicParticle pool (next: f_4d54)
	C3DPermanentSprite	*f_4d50[1];		// 0x4d50 C3DMagicParticle (next: f_4d60)
	int					f_4d54;			// 0x4d54 next of f_4d00 (ev154)
	float				f_4d58;			// 0x4d58 0; ev156 adds: sparkle time of f_4d00
	float				f_4d5c;			// 0x4d5c 0
	int					f_4d60;			// 0x4d60 next of f_4d50 (ev155)
	float				f_4d64;			// 0x4d64 0; ev157 adds: sparkle time of f_4d50
	float				f_4d68;			// 0x4d68 0
	OMedia3DVector		f_4d6c;			// 0x4d6c ev157: sparkle position (of f_4d50)
	unsigned char		f_4d7c;			// 0x4d7c 0 (ev165 clears it)
	OMedia3DPoint		f_4d80;			// 0x4d80 (0, 0, 0)
	unsigned char		f_4d90;			// 0x4d90 0
	int					f_4d94;			// 0x4d94 0 (v266: 0, 15)
	int					f_4d98;			// 0x4d98 sound channel (-1) "slide"
	float				f_4d9c;			// 0x4d9c 5000
	float				f_4da0;			// 0x4da0 0 bubble: time it lasts (ev147)
	float				f_4da4;			// 0x4da4 0 bubble: time left (ev146)
	float				f_4da8;			// 0x4da8 0.35 bubble alpha
	int					f_4dac;			// 0x4dac 0
	float				f_4db0;			// 0x4db0 -900
	unsigned char		f_4db4;			// 0x4db4 0
	unsigned char		f_4db8;			// 0x4db8 0: no shadow (ev136) / ...
	OMedia3DPoint		f_4dbc[10];		// 0x4dbc (counted by f_4e78)
	float				f_4e5c;			// 0x4e5c 0
	float				f_4e60;			// 0x4e60 0
	float				f_4e64;			// 0x4e64 0
	float				f_4e68;			// 0x4e68 0
	float				f_4e6c;			// 0x4e6c 0
	float				f_4e70;			// 0x4e70
	float				f_4e74;			// 0x4e74
	short				f_4e78;			// 0x4e78 0: entries of f_4dbc
	float				f_4e7c;			// 0x4e7c 0
	char				f_4e80[100];	// 0x4e80 "" (an animation name ev133 / Process play when set)
	float				f_4ee4;			// 0x4ee4 0
	unsigned char		f_4ee8;			// 0x4ee8 0
	CGameObject			*f_4ef0;		// 0x4ef0 the target Goddard shows (C3DAITrigger "ShowTarget")
};

namespace neg {
C3DJimmy *Jimmy(void);					// g_jimmy as a C3DJimmy (NULL if there is none)
}
