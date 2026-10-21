/*  C3DCheckPoint ('3CHK') - a checkpoint (sprites.omt canvas 206): when the camera (Jimmy) first
    touches it while playing (or, touched in another game mode, as soon as the game is back in
    mode 5) it becomes his checkpoint: his camera type, "ResetWhenDamaged" and "FallSpeedDamage"
    are set from it, and the game is autosaved with him standing on it (facing its way, unless
    the current gadget is 1), then put back where he was. The first time it also sends Goddard
    ("SendGoddardTo": WAIT, FOLLOW, a patrol point to walk to by path, or one to patrol from) and
    hides. "AlwaysAutoSave" saves on every touch. "CheckpointNum" is remembered (0x545f38).
    Reconstructed from Neutron2.exe: constructor 0x426d50 (factory '3CHK', 0x6f8 bytes),
    destructor 0x426f60 (only resets the vtables, then ~C3DSpriteType; deleting thunks 0x426f30 /
    0x4275c0 / 0x4275d0). Vtables 0x4d3f34 (element, 57 slots: C3DSprite's 55 + 55, 56) /
    0x4d3f20 (listener) / 0x4d3aa4 (game object: 7, 16, 231, 240, 260, 270, 284, 285
    overridden) / 0x4d3a94 (streamer). Own members 0x668..0x6f0, vtordisp 0x6f0, the streamer
    at 0x6f4.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DCheckPoint : public C3DSpriteType
{
	public:
	C3DCheckPoint();														// 0x426d50
	virtual ~C3DCheckPoint();												// 0x426f60

	// ---- element-family slots it adds
	virtual void SendGoddard(void);											// ESLOT 55 0x427340 (+0xdc) send Goddard (SendGoddardTo)
	virtual void Touched(CGameObject *who);									// ESLOT 56 0x427000 (+0xe0) checked by who (the camera)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x427290
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x426fb0 the camera: Touched (now or in game mode 5)
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x426fa0 (C3DSpriteType's)
	virtual void v260(void);												// SLOT 260 0x427470 radius = SpriteSize, unchecked
	virtual void v270(int prev, int mode);									// SLOT 270 0x427560 game mode 5: a pending touch
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x427510
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x4274a0

	// FIELD 0x668 0x88
	int			f_668;					// 0x668  (not used)
	unsigned char m_checked;			// 0x66c  checked once (saved)
	int			m_always_autosave;		// 0x670  "AlwaysAutoSave" (= 0; saved, as 0 / 1)
	unsigned char f_674;				// 0x674  1; 0 once checked (not read)
	char		m_send_goddard_to[100];	// 0x675  "SendGoddardTo" (= "none")
	int			m_checkpoint_num;		// 0x6dc  "CheckpointNum" (= -1)
	unsigned char f_6e0;				// 0x6e0  0 (not used)
	int			m_reset_when_damaged;	// 0x6e4  "ResetWhenDamaged" (= -1: leave Jimmy's)
	float		m_fall_speed_damage;	// 0x6e8  "FallSpeedDamage" (= -1: leave Jimmy's)
	int			m_camera_type;			// 0x6ec  "CameraType" (= -1: leave Jimmy's)
};
