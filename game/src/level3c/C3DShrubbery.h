/*  C3DShrubbery ('3SHU') - a shrub: a C3DSpriteType billboard ("sprites.omt" canvas 65, size 400)
    with Red/Green/Blue properties (all 1; nothing in the class reads them).
    Reconstructed from Neutron2.exe: constructor 0x46fc60, destructor 0x46fe60 (only resets the
    vtables, then ~C3DSpriteType; deleting thunks 0x46fe30 / 0x46ff30 / 0x46ff40). Vtables
    0x4f8a10 (element) / 0x4f89fc (listener) / 0x4f8580 (game object: 0, 7 overridden; 260 is a
    jump to C3DSpriteType's 0x473650) / 0x4f8570 (streamer). Own members 0x668..0x674,
    vtordisp 0x674, the streamer at 0x678 (0x67c bytes).                                      */
#pragma once
#include "../C3DSpriteType.h"

class C3DShrubbery : public C3DSpriteType
{
	public:
	C3DShrubbery();															// 0x46fc60
	virtual ~C3DShrubbery();												// 0x46fe60

	virtual void InitObject(void);											// SLOT 7   0x46fea0 colour properties

	// FIELD 0x668 0xc
	float			m_red;					// 0x668 "Red"   (= 1)
	float			m_green;				// 0x66c "Green" (= 1)
	float			m_blue;					// 0x670 "Blue"  (= 1)
};
