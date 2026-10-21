/*  TEMPORARY - a minimal C3DJimmy header so that part b (game/src/jimmy/C3DJimmy_b.*) compiles on
    its own. Part A's real C3DJimmy.h (constructor 0x443830, the hooks, jimmy/C3DJimmy_a.inc)
    replaces this file at the merge; nothing part b needs lives here except the include of its
    .inc and the stand-ins for part A's slots below.                                         */
#pragma once
#include "C3DPlayer.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"
#include "Todo.h"

class OMedia3DPolygon;
class OMediaStreamOperators;
class C3DPermanentSprite;
class C3DGrannyObj;

class C3DJimmy : public C3DPlayer
{
	public:
#include "jimmy/C3DJimmy_b.inc"
};

// TEMPORARY stand-ins for part A's slots that part b calls (part A defines them).
inline void C3DJimmy::ev147(float t) { (void)t; NEG_TODO(0x446010, "C3DJimmy element slot 147 (part A)"); }
