/*  C3DCraneBoom ('3CRB') - the dock crane Jimmy drives: the boom (shape 6 of "objects.omt",
    unlit) with its cable (C3DChain) and claw (C3DCraneClaw), which it builds itself. While
    Jimmy sits in it (C3DJimmy mode 11 makes the boom the camera and sets m_driver; Jimmy is
    held 150 above its origin and plays DRIVE) its v243 runs the controls: left / right (or the
    mouse) turn it at 20 deg/s, go / back (or the mouse) run the trolley out (230 / s) between
    300 and 1290, the action button (at most every 0.5 s, "cranegrab") lowers or raises the claw
    (200 / s, from -1300 down to -1750; at the bottom it drops what it carries and winds up).
    The cable follows the trolley at 50 / s; the viewport sits behind and above the boom
    looking at the claw's side. "cranerun" loops while driven (stopped on pause).
    Reconstructed from Neutron2.exe: constructor 0x428940 (factory '3CRB', 0x998 bytes),
    destructor 0x428bf0 (only resets the vtables, then ~C3DPlayerObject; deleting thunks
    0x429380 / 0x428bc0 / 0x429370 / 0x429390). Vtables 0x4d6404 (element, C3DPlayerObject's) /
    0x4d63f0 (listener) / 0x4d63b4 (OMediaAnim) / 0x4d5f38 (game object at +0xc0) / 0x4d5f28
    (streamer). Own members 0x960..0x990, vtordisp 0x990, streamer 0x994.                    */
#pragma once
#include "../C3DPlayerObject.h"
#include "OMedia3DVector.h"

class C3DChain;
class C3DCraneClaw;

class C3DCraneBoom : public C3DPlayerObject
{
	public:
	C3DCraneBoom();															// 0x428940
	virtual ~C3DCraneBoom();												// 0x428bf0

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x428cd0 the driver, the claw and the cable follow
	virtual void v243(float dt);											// SLOT 243 0x428eb0 driven: the controls and the viewport
	virtual void v260(void);												// SLOT 260 0x428c40 the cable to the trolley
	virtual void v277(void);												// SLOT 277 0x4292e0 pause: stop "cranerun"
	virtual void v278(void);												// SLOT 278 0x429300 resume: "cranerun" again

	float				m_reach;		// 0x960  trolley distance along the boom (= 400; 300..1290)
	C3DChain			*m_chain;		// 0x964  the cable
	C3DCraneClaw		*m_claw;		// 0x968  the claw
	OMedia3DVector		m_chain_pos;	// 0x96c  where the cable hangs (follows the trolley)
	unsigned char		m_up;			// 0x97c  1: the claw winds up (= 1)
	float				m_grab_time;	// 0x980  time since the last action press (= 0.5)
	float				m_drop;			// 0x984  claw height below the boom (= -1300; down to -1750)
	CGameObject			*m_driver;		// 0x988  Jimmy, while driving (= NULL)
	int					m_channel;		// 0x98c  "cranerun" (= -1)
};
