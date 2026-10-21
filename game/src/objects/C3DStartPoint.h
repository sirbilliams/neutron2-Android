/*  C3DStartPoint ('STRT') - where Jimmy enters a level: a sprite ("Icons.omt" canvas 5) whose
    ObjectTag is the name a CLoadLevel / C3DJimmy "StartPoint" refers to. It also keeps the view
    to put the camera at ("ViewportP*" / "ViewportR*", taken from the viewport when its
    properties change in the editor), the level's music, and "StartTrigger": a C3DTriggerType
    fired when Jimmy arrives here (the opening cut scene of level1d is 'cam2').
    Reconstructed from Neutron2.exe: constructor 0x473750, destructor 0x473980 (only resets the
    vtables, then ~C3DSprite; deleting thunks 0x473d90 / 0x473950 / 0x473da0). Vtables 0x4fccd0
    (element, 55 slots, all C3DSprite's) / 0x4fccbc (listener) / 0x4fc840 (game object) /
    0x4fc830 (streamer: 0x4275e0 / 0x4275f0, shared with C3DCheckPoint, are C3DSprite's
    read_class / write_class). 0x6f8 bytes: C3DSprite's 0x600, own members 0x600..0x6f0,
    vtordisp 0x6f0, the streamer at 0x6f4. Derives from C3DSprite directly (RTTI), not from
    C3DSpriteType.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSprite.h"

class C3DStartPoint : public C3DSprite
{
	public:
	C3DStartPoint();														// 0x473750
	virtual ~C3DStartPoint();												// 0x473980

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4739c0
	virtual void Process(float dt);											// SLOT 240 0x473700 (folded with C3DSpriteType's)
	virtual void v258(void);												// SLOT 258 0x473ad0 properties changed: the view from the viewport
	virtual void v260(void);												// SLOT 260 0x473bd0 (re)load: Jimmy arriving here arms StartTrigger
	virtual void v266(void);												// SLOT 266 0x473b10 fire the armed StartTrigger
	virtual void v270(int a2, int a3);										// SLOT 270 0x473d50 (CLocalGameObject's)

	// FIELD 0x600 0xee
	float		m_viewport_pos[4];		// 0x600  "ViewportPx/Py/Pz" (w 0x60c = 1)
	float		m_viewport_rot[4];		// 0x610  "ViewportRx/Ry/Rz" (w 0x61c = 1)
	char		m_music_db[100];		// 0x620  "MusicDatabase" (= "none")
	int			m_music_index;			// 0x684  "MusicIndex"
	char		m_start_trigger[100];	// 0x688  "StartTrigger" (= "none")
	unsigned char m_trigger_armed;		// 0x6ec  set by v260, fired (and cleared) by v266
	unsigned char f_6ed;				// 0x6ed  = 0 (not used by the class itself)
};
