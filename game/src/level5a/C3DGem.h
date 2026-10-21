/*  C3DGem ('3GEM') - a gem of Level5a's door puzzle (tags REDGEM, YELLOWGEM, BLUEGEM): a granny
    prop ("gembase.grn", radius 10, scale 1.265) tinted "Red/Green/Blue" at "Opacity" (the first
    polygon's material), shown or hidden by "InitiallyVisible". Jimmy picks it up by touching it
    while he holds nothing and no gem is placed (g_545f68): he holds it (his f_d38) and, if no gem
    was taken yet, g_545f58 says which (1 red, 2 yellow, 4 blue).
    Reloading a savegame (ReadState) puts the puzzle back: g_545f64 is read (and then set to 3 for
    the doors' reset), GREENDOOR / ORANGEDOOR / PURPLEDOOR and the gem are reset, the gem globals
    cleared and SPHYNXRAY tinted (0.6; 1, 1, 1). WriteState saves g_545f64.
    Reconstructed from Neutron2.exe: constructor 0x436fe0 (factory '3GEM', 0xddc bytes),
    destructor 0x4371c0 (only resets the vtables, then ~C3DGrannyObj; deleting thunks 0x437a90 /
    0x437a80 / 0x437170 / 0x437aa0). Vtables: element 0x4de73c (99 slots, all C3DGrannyObj's),
    OMediaAnim 0x4de6ec, listener 0x4de728, game object (+0xc0) 0x4de270 (0, 7, 10, 16, 260, 284,
    285 overridden; 10 is the code of C3DSpriteObject's 0x473010), streamer 0x4de260. Own members
    0xdb8..0xdd4, vtordisp 0xdd4, streamer 0xdd8.                                             */
#pragma once
#include "../C3DGrannyObj.h"

class C3DGem : public C3DGrannyObj
{
	public:
	C3DGem();																// 0x436fe0
	virtual ~C3DGem();														// 0x4371c0

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4371f0 colour, opacity, InitiallyVisible, Radius
	virtual void ResetObject(void);											// SLOT 10  0x473010 + stopped
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x437650 Jimmy takes it
	virtual void v260(void);												// SLOT 260 0x4372a0 shown / hidden, tinted
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x437a40 + g_545f64
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x437960 + the puzzle reset

	float				m_red;			// 0xdb8  "Red" (1)
	float				m_green;		// 0xdbc  "Green" (1)
	float				m_blue;			// 0xdc0  "Blue" (1)
	float				m_opacity;		// 0xdc4  "Opacity" (not set by the constructor)
	int					f_dc8;			// 0xdc8  0
	int					f_dcc;			// 0xdcc  0
	int					f_dd0;			// 0xdd0  (not used)
};
