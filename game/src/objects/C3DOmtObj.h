/*  C3DOmtObj ('3OMT') - a scenery object whose shape comes from an OMT database ("OmtDatabase",
    default "objects.omt"; 3D shape chunk "OmtIndex", default 4), unlit. "SpecialCase" makes it
    behave: 1 a platform (tag "PLATFORM": carries the objects locked onto it, ev94), 2/3/4 spins
    about x/y/z at "RotateSpeed" deg/s, 5 scrolls its texture, 6 translucent (0.6), 7 a falling
    tile ("tilefall" when the camera lands on it, then it shakes), 8 spins (y 500, z 1000),
    9/10 resets: touched by the camera it calls "ResetTrigger" (or turns a rocket / minisub around
    with "TURNAROUND"), 12 fades in when the camera comes within "DistToView" (or glows while
    g_54a434 is set). A "ResetTrigger" other than "none" works as 9.
    Reconstructed from Neutron2.exe: constructor 0x45b270 (factory '3OMT', 0xa28 bytes),
    destructor 0x45b520 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x45b4f0 /
    0x45c470 / 0x45c480 / 0x45c490).

    Original layout: as C3DAnimated (element vtable 0x4eee58, 95 slots: C3DAnimated's 94 + 94;
    OMediaAnim vtable 0x4eee08 at +0x90; listener 0x4eee44 at +0x30; CLocalGameObject vtable
    0x4ee98c at +0xc0), own members 0x924..0xa20, vtordisp 0xa20, streamer 0xa24.        */
#pragma once
#include "../C3DAnimated.h"

class OMediaStreamOperators;

class C3DOmtObj : public C3DAnimated
{
	public:
	C3DOmtObj();															// 0x45b270
	virtual ~C3DOmtObj();													// 0x45b520

	// ---- element-family slots
	virtual void ev94(CGameObject *o);										// ESLOT 94 0x45b950 lock o onto us (crane); the 4th: "CRANEDONE"

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x45bb30 falling tile / reset trigger
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x45c0a0 collides with the camera and a few object IDs
	virtual void Process(float dt);											// SLOT 240 0x45bcf0 the special cases
	virtual void v260(void);												// SLOT 260 0x45b570 shape from the database, special case setup
	virtual void v271(void);												// SLOT 271 0x45c130 case 12: invisible again
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x45c2d0
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x45c180

	// ---- data members (object offsets)
	char				f_924[100];		// 0x924  "OmtDatabase" ("objects.omt")
	int					f_988;			// 0x988  "OmtIndex" (4): its 3D shape chunk
	int					f_98c;			// 0x98c  "SpecialCase" (0)
	CGameObject			*f_990[6];		// 0x990  objects locked onto us (ev94)
	unsigned char		f_9a8;			// 0x9a8  0 (not used otherwise)
	float				f_9ac;			// 0x9ac  "RotateSpeed" (20)
	int					f_9b0;			// 0x9b0  ev94 calls (0)
	char				f_9b4[100];		// 0x9b4  "ResetTrigger" ("none")
	float				f_a18;			// 0xa18  falling tile: time to the next shake (3)
	float				f_a1c;			// 0xa1c  "DistToView" (500)
};
