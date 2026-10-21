/*  C3DNeutron ('3NEU') - a neutron: a spinning energy pickup (sprites.omt canvases 0..7 cycled
    every 0.05 s, scaled by the stopwatch's time scale). Touched by the camera (Jimmy) it gives 5
    neutron energy and 10 points, flashes Jimmy, ticks the menu's pickup item 45 (unless the
    energy is full), plays "neutron" and bursts (canvases 8..12), then hides and stops colliding.
    It comes back 240 s later. Its state goes into the savegame.
    Reconstructed from Neutron2.exe: constructor 0x4597d0 (factory '3NEU', 0x6c0 bytes),
    destructor 0x459b20 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x459af0 /
    0x459f20 / 0x459f30). Vtables 0x4ed4a0 (element, 55 slots, none overridden) / 0x4ed48c
    (listener) / 0x4ed010 (game object: 16, 231, 240, 258, 260, 284, 285 overridden; 231 and 260
    are code shared with C3DCheckPoint / C3DLeaves) / 0x4ed000 (streamer). Own members
    0x668..0x6b8, vtordisp 0x6b8, the streamer at 0x6bc.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class OMediaCanvas;

class C3DNeutron : public C3DSpriteType
{
	public:
	C3DNeutron();															// 0x4597d0
	virtual ~C3DNeutron();													// 0x459b20

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x459d40 collected by the camera
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x459b60 animation, burst, coming back
	virtual void v258(void);												// SLOT 258 0x459d10 reload: first frame
	virtual void v260(void);												// SLOT 260 0x44f430 first frame (no base call)
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x459ec0
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x459e10

	// FIELD 0x668 0x50
	OMediaCanvas	*m_frames[13];		// 0x668  sprites.omt canvases 0..12 (0..7 spin, 8..12 burst)
	int				m_frame;			// 0x69c  current frame
	float			m_time;				// 0x6a0  time on this frame (time since gone, once gone)
	unsigned char	m_collected;		// 0x6a4  touched, bursting
	int				m_channel;			// 0x6a8  the "neutron" sound channel
	unsigned char	m_gone;				// 0x6ac  hidden until it comes back
};
