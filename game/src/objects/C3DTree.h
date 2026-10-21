/*  C3DTree ('3TRE') - a tree: a plain C3DSpriteType billboard ("sprites.omt" canvas 16, size
    150) with no behaviour of its own.
    Reconstructed from Neutron2.exe: constructor 0x477fc0, destructor 0x478130 (only resets the
    vtables, then ~C3DSpriteType; deleting thunks 0x478190 / 0x478100 / 0x4781a0). Vtables
    0x501634 (element, 55 slots, all C3DSprite's) / 0x501620 (listener) / 0x5011a4 (game object,
    only slot 0 overridden) / 0x501194 (streamer). 0x670 bytes: C3DSpriteType's 0x668, vtordisp
    0x668, the streamer at 0x66c.                                                            */
#pragma once
#include "../C3DSpriteType.h"

class C3DTree : public C3DSpriteType
{
	public:
	C3DTree();																// 0x477fc0
	virtual ~C3DTree();														// 0x478130
};
