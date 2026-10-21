/*  C3DShock ('3SHO') - an electric-shock sprite: a short sprite animation (up to nine frames of
    the object's database, "sprites.omt": 226, 227, 228 by default; the count is the frames
    before the first -1) at f_668 frames per second, 220 units, added onto the scene (blend
    Src_Alpha, One; Inv_Src_Alpha in windowed mode). While f_694 is set it plays: mode f_6a0
    1 loops, 0 plays once, 2 plays once and hides; with a fade time f_6a4 > 0 it fades out over
    it and stops. C3DBot makes one and shows it while stunned.
    Reconstructed from Neutron2.exe: constructor 0x46f120 (factory '3SHO', 0x6b4 bytes),
    destructor 0x46f300 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x46f2d0 /
    0x46f7c0 / 0x46f7d0).

    Original layout: as C3DSpriteType (element vtable 0x4f7e28, 55 slots; listener 0x4f7e14 at
    +0x30; CLocalGameObject vtable 0x4f7998 at +0xc8), own members 0x668..0x6ac, vtordisp 0x6ac,
    streamer 0x6b0.                                                                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DShock : public C3DSpriteType
{
	public:
	C3DShock();																// 0x46f120
	virtual ~C3DShock();													// 0x46f300

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x46f340 the animation, the fade
	virtual void v260(void);												// SLOT 260 0x46f680 frame count, first frame, blend

	// ---- data members (object offsets)
	float				f_668;			// 0x668  frames per second (10)
	int					f_66c;			// 0x66c  number of frames (3; v260: those before the first -1)
	int					f_670[9];		// 0x670  sprite index of each frame (226, 227, 228, -1 ...)
	unsigned char		f_694;			// 0x694  playing (0)
	int					f_698;			// 0x698  current frame (0)
	float				f_69c;			// 0x69c  time since the last frame (0)
	int					f_6a0;			// 0x6a0  at the end: 1 loop, 0 stop, 2 stop and hide (1)
	float				f_6a4;			// 0x6a4  fade time left (-1: no fade)
	float				f_6a8;			// 0x6a8  fade time (1)
};
