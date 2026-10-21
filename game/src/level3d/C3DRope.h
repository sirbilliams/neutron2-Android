/*  C3DRope ('3ROP') - a rope hanging from its position ("RopeScale" 1: shape 63 of
    "objects.omt"; other scales: "rope01.ase" / "rope.png" stretched along z by RopeScale,
    once scale > 1). Two kinds:
    * CanSwing 1 (= 1): Jimmy touching it (or brushing it after the 2 s regrab delay) hangs on
      it (his ev143: anchor = our start position, length RopeScale x 600) and swings ("ropeswing");
      the rope turns to point at him. A jump press that started after he grabbed it, without
      the action button, lets go (his ev142 and ActivateMenuItem). Saved: whether he hangs on.
    * CanSwing 0: a zip line. It builds a C3DHookSlide (the hook) that runs along its local z;
      the hook waits at the rope's far end (it runs back there, 10 a frame, when let go
      early); Jimmy touching the hook
      grabs it (ev94: SWING, "GRAPLEUP" loops, no physics) and slides with it, 64 below it,
      faster with back held (8 / s up, 6 / s down with go, else 4 / s down), the "GRAPLEUP"
      pitch following the speed; it lets go (ev95: FALL, sounds stopped) at the far end + 100,
      near the start (< 150) or on a jump press.
    Reconstructed from Neutron2.exe: constructor 0x46cd90 (factory '3ROP', 0x990 bytes),
    destructor 0x46d000 (only resets the vtables, then ~C3DAnimated; deleting thunks 0x46e000 /
    0x46cfd0 / 0x46dff0 / 0x46e010).
    Original layout: as C3DAnimated (element vtable 96 slots: C3DAnimated's 94 + 94, 95;
    OMediaAnim vtable at +0x90; CLocalGameObject vtable 0x4f58ec at +0xc0), own members
    0x924..0x988, vtordisp 0x988, streamer 0x98c.

    C3DHookSlide ('3HOS' as its ObjectID; no factory ID) - the zip line's hook ("hookslide.ase",
    "hookslide.png"), radius 200, turned like its rope. Jimmy touching it calls the rope's ev94.
    Reconstructed from Neutron2.exe: constructor 0x43f3a0 (0x94c bytes), destructor 0x43f550
    (deleting thunks 0x43f710 / 0x43f520 / 0x43f700 / 0x43f720). Own members 0x924..0x944,
    vtordisp 0x944, streamer 0x948. Slot 231 is 0x45ce70 (the camera and '3JIM' only).      */
#pragma once
#include "../C3DAnimated.h"
#include "OMedia3DVector.h"
#include "OMedia3DPoint.h"

class OMediaStreamOperators;
class C3DHookSlide;

class C3DRope : public C3DAnimated
{
	public:
	C3DRope();																// 0x46cd90
	virtual ~C3DRope();														// 0x46d000

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69 0x46d050 the stretched rope animation (scale > 1)
	virtual void ev94(CGameObject *o);										// ESLOT 94 0x46d4d0 o grabs the hook (zip line)
	virtual void ev95(CGameObject *o);										// ESLOT 95 0x46d6b0 o lets go of the hook

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x46d0e0 "CanSwing", "RopeScale"
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x46d360 Jimmy grabs a swinging rope
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x46d880 swing / zip line
	virtual void v260(void);												// SLOT 260 0x46d140 the shape; the zip line's hook
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x46d800 + hanging on
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x46d830 + hanging on

	unsigned char		m_attached;		// 0x924  Jimmy hangs on (swing; saved)
	unsigned char		m_released;		// 0x925  jump was let go since grabbing
	OMedia3DPoint		m_anchor;		// 0x928  where he hangs from (our start position)
	OMedia3DVector		m_grab_pos;		// 0x938  where he was when he grabbed it
	float				m_regrab;		// 0x948  no grabbing for this long after letting go (2 s)
	int					m_can_swing;	// 0x94c  "CanSwing" (= 1)
	float				m_scale;		// 0x950  "RopeScale" (= 1)
	unsigned char		m_sliding;		// 0x954  zip line: someone hangs on the hook
	OMedia3DVector		m_dir;			// 0x958  v226(0, 0, 130) when grabbed (not used)
	CGameObject			*m_rider;		// 0x968  who hangs on (zip line) / "JIM1" (swing)
	CGameObject			*m_hook;		// 0x96c  the zip line's C3DHookSlide
	float				m_hook_dist;	// 0x970  the hook's distance along the rope
	float				m_hook_speed;	// 0x974  its speed
	unsigned char		m_hook_moving;	// 0x978  it runs out to the end
	int					m_swing_channel;// 0x97c  "ropeswing" (= -1)
	int					m_freq;			// 0x980  base of the "GRAPLEUP" pitch (never set; port: 0)
	int					m_slide_channel;// 0x984  "GRAPLEUP" (= -1)
};

class C3DHookSlide : public C3DAnimated
{
	public:
	C3DHookSlide();															// 0x43f3a0
	virtual ~C3DHookSlide();												// 0x43f550

	virtual void ev69(void);												// ESLOT 69 0x43f5a0 the hook animation and texture
	virtual void InitObject(void);											// SLOT 7   0x43f600
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43f6a0 Jimmy: the rope's ev94
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x45ce70 the camera and '3JIM' only
	virtual void v260(void);												// SLOT 260 0x43f630 turned like the rope

	unsigned char		f_924;			// 0x924  (= 0)
	float				f_928;			// 0x928  (= 1)
	float				f_92c[3];		// 0x92c  (= 0)
	CGameObject			*f_93c;			// 0x93c  the rope's rider when it was built (not used)
	CGameObject			*m_rope;		// 0x940  the rope (its CGameObject part)
};
