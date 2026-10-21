/*  C3DYokDoor ('3DOR') - a Yokian door that slides up: a shape from an OMT database
    ("OmtDatabase", default "objects.omt", "OmtIndex" 0), no animations (f_6d0 = 2). NextState
    (v268: 0 open, 1 close, other toggles) moves it by "OpenAmount" (500) in y over "DoorSpeed"
    (5 s; reversed half-way if asked) with the "Door opening loop" sound, then "door closing";
    open, it closes by itself after "OpenTime" (3 s). Each NextState also fires the "Next"
    object (an AI trigger or a cut-scene camera). Opened when a Yokian touches it. "ItemClosed"
    (1) is its state on reset. The same door as C3DDoorUpDown, with a database shape.
    Reconstructed from Neutron2.exe: constructor 0x47f800 (factory '3DOR', 0xa1c bytes),
    destructor 0x47fa10 (deleting thunks 0x47f9e0 / 0x480170 / 0x480180 / 0x480190).
    Vtables 0x505cac (game object at +0xc0: 0, 7, 10, 16, 231, 240, 260, 268, 270, 284, 285
    overridden; 231 is code shared at 0x47fc20) / 0x505c9c (streamer).
    Own members 0x924..0xa14, vtordisp 0xa14, streamer 0xa18.                                */
#pragma once
#include "../C3DAnimated.h"

class C3DYokDoor : public C3DAnimated
{
	public:
	C3DYokDoor();															// 0x47f800
	virtual ~C3DYokDoor();													// 0x47fa10

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x47fac0 properties
	virtual void ResetObject(void);											// SLOT 10  0x47fa60 back to ItemClosed
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x47fc70 a Yokian: open
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x47fc20 collides with the camera, '3JIM', '3ENE', '3GOD', '3WBL' only
	virtual void Process(float dt);											// SLOT 240 0x47fcb0 moving, closing by itself
	virtual void v260(void);												// SLOT 260 0x47fbb0 properties loaded: the database shape
	virtual void v268(int state);											// SLOT 268 0x47fe70 "NextState": 0 open, 1 close, else toggle
	virtual void v270(int a, int msg);										// SLOT 270 0x47fff0 cut scene (3): stop the loop sound
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x480010
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x480050

	// ---- data members (object offsets)
	int					f_924;			// 0x924  "ItemClosed" (1): closed on reset
	int					f_928;			// 0x928  closed (or closing)
	char				f_92c[100];		// 0x92c  "Next" ("none")
	char				f_990[100];		// 0x990  "OmtDatabase" ("objects.omt")
	int					f_9f4;			// 0x9f4  "OmtIndex" (0)
	float				f_9f8;			// 0x9f8  moving time left
	float				f_9fc;			// 0x9fc  "DoorSpeed" (5 s for the whole way)
	float				f_a00;			// 0xa00  "OpenTime" (3 s)
	float				f_a04;			// 0xa04  time open
	int					f_a08;			// 0xa08  = 0 (unused)
	float				f_a0c;			// 0xa0c  "OpenAmount" (500)
	int					f_a10;			// 0xa10  the "Door opening loop" channel (-1: none)
};
