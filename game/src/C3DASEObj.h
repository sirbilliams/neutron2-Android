/*  C3DASEObj ('3ASE') - a level object made from .ase morph animations: ANIMATE ("ASEWalk",
    default "flamemove.ase"), STOP ("ASEStop", "flamestop.ase"), optional ANIM1 / ANIM2, one
    texture ("PNGFile", "flame.png"). Options: "DoubleSided", texture scrolling ("SlideXSpeed" /
    "SlideYSpeed"), a bob along its own z axis ("InOutDist" x sin("PeriodSpeed" t)), spinning
    ("RotateX/Y/Z", degrees / s) and a repeat ("RepeatAnimTime": shown again with "RepeatSound"
    every so many seconds, hidden at the end of each ANIMATE). Collides only with the camera
    object, Goddard, Jimmy and the bus. C3DJimmy's constructor makes one for his bubble (f_d44:
    "energybubble.ase", "vrtankbubble.ase", "stratobubble.ase", "bubble.png").
    Reconstructed from Neutron2.exe: factory '3ASE', constructor 0x4207e0 (0xbb0 bytes),
    destructor 0x420a80 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x4212c0 /
    0x420a50 / 0x4212d0 / 0x4212b0).

    Original layout: as C3DAnimated (element vtable 0x4ceb38, 94 slots; OMediaAnim vtable
    0x4ceae8 at +0x90; listener 0x4ceb24 at +0x30; CLocalGameObject vtable 0x4ce66c at +0xc0),
    own members 0x924..0xba8, vtordisp 0xba8, streamer 0xbac.                                  */
#pragma once
#include "C3DAnimated.h"

class OMediaStreamOperators;

class C3DASEObj : public C3DAnimated
{
	public:
	C3DASEObj();															// 0x4207e0
	virtual ~C3DASEObj();													// 0x420a80

	// ---- element-family game slots
	virtual void ev67(void);												// ESLOT 67 0x4211a0 ANIMATE over: hidden until the repeat
	virtual void ev69(void);												// ESLOT 69 0x420ad0 the animations and the texture (once)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x420bd0 properties
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x420d80 collides with the camera, '3GOD', '3JIM', '3BUS' only
	virtual void Process(float dt);											// SLOT 240 0x420dc0 repeat, slide, bob, spin
	virtual void v260(void);												// SLOT 260 0x421080 properties loaded
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x421240 + visible
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x4211f0 + visible

	// ---- data members (object offsets)
	char				f_924[100];		// 0x924  "ASEWalk": the ANIMATE animation ("flamemove.ase")
	char				f_988[100];		// 0x988  "ASEStop": STOP ("flamestop.ase")
	char				f_9ec[100];		// 0x9ec  "ASEAnim1": ANIM1 ("none")
	char				f_a50[100];		// 0xa50  "ASEAnim2": ANIM2 ("none")
	char				f_ab4[100];		// 0xab4  "PNGFile" ("flame.png")
	char				f_b18[100];		// 0xb18  "RepeatSound" ("none")
	int					f_b7c;			// 0xb7c  "DoubleSided" (0)
	float				f_b80;			// 0xb80  "SlideXSpeed" (0): texture offset per second
	float				f_b84;			// 0xb84  "SlideYSpeed" (0)
	float				f_b88;			// 0xb88  "RepeatAnimTime" (-1; > 0: repeat every so many s)
	float				f_b8c;			// 0xb8c  repeat time left (-1)
	float				f_b90;			// 0xb90  "InOutDist" (0; > 0: bob)
	float				f_b94;			// 0xb94  bob time (0)
	float				f_b98;			// 0xb98  "PeriodSpeed" (1)
	float				f_b9c;			// 0xb9c  "RotateX" (0)
	float				f_ba0;			// 0xba0  "RotateY" (0)
	float				f_ba4;			// 0xba4  "RotateZ" (0)
};
