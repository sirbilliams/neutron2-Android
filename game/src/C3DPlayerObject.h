/*  C3DPlayerObject - a C3DAnimated that the player (or the AI) drives: the chase-camera helpers
    and the player's button tests. Base of C3DPlayer (-> C3DJimmy), C3DAI (-> C3DAIGranny,
    C3DFish ...) and C3DFlyingObject (-> C3DRocketShip ...).
    Reconstructed from Neutron2.exe: constructor 0x464d50, destructor 0x464e80 (deleting thunks
    0x464e50 / 0x466a20 / 0x466a30 / 0x466a40). No factory ID, no name of its own.

    Original layout: as C3DAnimated (element vtable 0x4f2484, 106 slots: C3DAnimated's 94 +
    94..105; CLocalGameObject vtable 0x4f1fb8 at +0xc0), own members 0x924..0x960, vtordisp 0x960,
    streamer 0x964. Derived classes put their members from 0x960 on.

    The camera helpers work on the viewport (g_viewport, 2002 +0x44 position, +0x50 angles):
      ev95  pull the camera in front of a wall between it and the player
      ev96  keep the camera within a distance of its target
      ev97  place the camera and point it at a target
      ev98  move the camera towards a wanted position and turn it to the target
    Globals: g_5461cc (cleared by the constructor and v260; set by C3DPlayer), g_5461cd (set by
    ev105: restarting after death), g_5461ce (player controls off: ev104 answers no).        */
#pragma once
#include "C3DAnimated.h"
#include "OMedia3DVector.h"

class OMediaStreamOperators;

class C3DPlayerObject : public C3DAnimated
{
	public:
	C3DPlayerObject();														// 0x464d50
	virtual ~C3DPlayerObject();												// 0x464e80

	// ---- element-family game slots
	virtual void ev94(void) {}												// ESLOT 94  0x463890 (empty)
	virtual float ev95(OMedia3DVector unused, OMedia3DVector *cam);			// ESLOT 95  0x465010 a wall between us and *cam: *cam = 3/4 of the way to it, 1.5; else 1.0
	virtual float ev96(OMedia3DVector target, OMedia3DVector *out, float max);	// ESLOT 96  0x464ef0 viewport farther than max from target: *out = target + (max - 3) towards it, 2.0; else 1.0
	virtual void ev97(OMedia3DVector pos, OMedia3DVector target);			// ESLOT 97  0x465110 viewport at pos, looking at target
	virtual void ev98(float a, float b, float mode, OMedia3DVector cur, OMedia3DVector want,
					  OMedia3DVector target, OMedia3DVector axis);			// ESLOT 98  0x4651c0 mode 1: viewport += axis * (want - cur) * a * b, then look at target; else ev97(want, target)
	virtual unsigned char ev99(void) { return 0; }							// ESLOT 99  0x4655c0
	virtual unsigned char ev100(void) { return 0; }							// ESLOT 100 0x4655c0
	virtual unsigned char ev101(void) { return 0; }							// ESLOT 101 0x4655c0
	virtual unsigned char ev102(void) { return 0; }							// ESLOT 102 0x4655c0
	virtual unsigned char ev103(void);										// ESLOT 103 0x4655d0 jump/use button (right mouse, Insert, Shift, Num0, joystick 1)
	virtual unsigned char ev104(void);										// ESLOT 104 0x465650 fire button (left mouse, Enter, Ctrl, joystick 0), not while g_5461ce
	virtual void ev105(void);												// ESLOT 105 0x465360 Jimmy is out: back to the autosave + a comment

	// ---- CGameObject-family overrides
	virtual void v243(float dt) { (void)dt; }								// SLOT 243 0x464ee0 (empty: no C3DObject camera)
	virtual void v260(void);												// SLOT 260 0x464ed0
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x4651b0 (CLocalGameObject's)
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x4651a0 (CLocalGameObject's)

	// ---- data members (object offsets; 0x924..0x92f and 0x938..0x95f are never used)
	float				f_930;			// 0x930  1.0
	int					f_934;			// 0x934  -1 (C3DJimmy v270 uses it)
};
