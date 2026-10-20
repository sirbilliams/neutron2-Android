/*  CLoadLevel ('LOAD') - a level exit: a sprite that, when Jimmy (or his hover cycle) touches
    it, hands Jimmy the level to load ("LevelName", at its "StartPoint"). "RETURN" sends him back
    to the level and start point he came from. "RequiredTask" (with "RequiredLevel" as a minimum
    or "ExactLevel" as the only state) and "GoIfActive" (a level item of the menu) can hold the
    exit closed. Its sprite is "icons.omt" canvas 2.
    Reconstructed from Neutron2.exe: constructor 0x48e370, destructor 0x48e5e0 (deleting thunks
    0x48ec60 / 0x48e5b0 / 0x48ec70), vtables 0x5159c4 (element, 56 slots) / 0x5159b0 /
    0x515534 (game object) / 0x515524. 0x718 bytes: C3DSprite's 0x600, own members
    0x600..0x710, vtordisp 0x710, the streamer at 0x714.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower). C3DTriggerType's level
    hand-over (0x479610) reads them too: LevelName, StartPoint, LoadScreen, Promotion,
    FadeType, FadeTime.                                                                      */
#pragma once
#include "../C3DSprite.h"

class CLoadLevel : public C3DSprite
{
	public:
	CLoadLevel();															// 0x48e370
	virtual ~CLoadLevel();													// 0x48e5e0

	// ---- element-family slot it adds
	virtual void ev55(void);												// ESLOT 55 0x48ebf0 (+0xdc) fired: the lab-to-neighbourhood scene step

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x48e620
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x48e790 touch starts
	virtual void v258(void);												// SLOT 258 0x48ebe0 (jump to C3DSprite's)
	virtual void v260(void);												// SLOT 260 0x48eba0 (re)load the sprite canvas

	// FIELD 0x600 0x110
	char		m_level_name[80];		// 0x600  "LevelName": the .gam to load, "RETURN", or "none"
	char		m_start_point[80];		// 0x650  "StartPoint": the C3DStartPoint tag in that level
	char		m_required_task[80];	// 0x6a0  "RequiredTask", "none" = no condition
	int			m_required_level;		// 0x6f0  "RequiredLevel": the task's state must be at least this
	int			m_sound_index;			// 0x6f4  "SoundIndex": played when used, -1 = none
	int			m_exact_level;			// 0x6f8  "ExactLevel": ... and exactly this, -1 = any
	int			m_fade_type;			// 0x6fc  "FadeType", -1 = none
	float		m_fade_time;			// 0x700  "FadeTime"
	int			m_load_screen;			// 0x704  "LoadScreen": the loading picture
	int			m_promotion;			// 0x708  "Promotion": given to Jimmy (+0xc18)
	int			m_go_if_active;			// 0x70c  "GoIfActive": a menu page 3 (level items) item that must be on, -1 = none
};
