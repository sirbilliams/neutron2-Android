/*  C3DPickupItem ('3PIC') - the pickups lying in the levels (sprites.omt canvas 8 by default):
    gadget parts, keys and crystals, sodas, Sporkos, the timed powerups (SPEEDUP, SHIELDS, SONAR,
    XRAY, FREEZETIME... named by their ObjectTag) and ambient sound sources ("IsAmbient", a
    looping 3D sound). Touched by the camera (Jimmy) and allowed (CanPickUp: "RequiredPicNum" x
    "ReqPicNumAmount" in the pickup menu, else the NeedMore sound) it is collected: its object
    state is set (unless it respawns after "RespawnTime"), it hides (or shows "PickedUpIndex"
    instead), toggles "ActivateObject" / "ToggleObject", ticks its menu item "PIC_NUMBER"
    (with a voice clip when "IsVoice"), starts its powerup for "PowerupTime" seconds, gives
    "PointValue" points, fires NextTrigger and plays "SoundIndex" / "SoundName" (at most
    "TimesToTrigger" times). "ShowCorona" gives it a C3DCorona glow in "Red/Green/Blue".
    Reconstructed from Neutron2.exe: constructor 0x45dc00 (factory '3PIC', 0x944 bytes),
    destructor 0x45ded0 (only resets the vtables, then ~CPickupType; deleting thunks 0x45dea0 /
    0x45fa10 / 0x45fa20). Vtables 0x4f11ec (element, 60 slots: CPickupType's 56 + 56..59) /
    0x4f11d8 (listener) / 0x4f0d5c (game object: 7, 10, 16, 231, 240, 260, 268, 284, 285
    overridden) / 0x4f0d4c (streamer). Own members 0x7b0..0x93c, vtordisp 0x93c, the streamer
    at 0x940.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "CPickupType.h"

class C3DCorona;

class C3DPickupItem : public CPickupType
{
	public:
	C3DPickupItem();														// 0x45dc00
	virtual ~C3DPickupItem();												// 0x45ded0

	// ---- element-family slots it adds
	virtual void Powerup(void);												// ESLOT 56 0x45f120 (+0xe0) the effect of a powerup tag
	virtual unsigned char CanPickUp(void);									// ESLOT 57 0x45f750 (+0xe4) the required pickups (used up), else the NeedMore sound
	virtual void SetActive(unsigned char on);								// ESLOT 58 0x45e750 (+0xe8) shown and colliding, or not
	virtual void SyncWithMenu(void);										// ESLOT 59 0x45efb0 (+0xec) a gadget powerup: active while its menu item is

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x45dfb0
	virtual void ResetObject(void);											// SLOT 10  0x45df10
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x45e200 collected by the camera
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x45eda0 ambient sound, corona, respawn
	virtual void v260(void);												// SLOT 260 0x45ea00 corona, collected / picked-up look
	virtual void v268(int state);											// SLOT 268 0x45e820 ToggleObject: 0 hidden, 1 back (not collected)
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x45f980
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x45f880

	// FIELD 0x7b0 0x18c
	int			m_sound_index;			// 0x7b0  "SoundIndex" (= -1: SoundName)
	int			m_need_more_sound;		// 0x7b4  "NeedMoreSound" (= -1: NeedMoreName)
	int			m_times_to_trigger;		// 0x7b8  "TimesToTrigger" (= -1: always)
	int			m_times_triggered;		// 0x7bc  0
	int			m_is_ambient;			// 0x7c0  "IsAmbient" (= 0)
	int			m_ambient_channel;		// 0x7c4  the looping ambient sound (-1)
	unsigned char m_ambient_started;	// 0x7c8  0
	int			m_point_value;			// 0x7cc  "PointValue" (= 0)
	int			m_picked_up_index;		// 0x7d0  "PickedUpIndex" (= -1: hidden when collected; else this canvas)
	int			m_required_pic_num;		// 0x7d4  "RequiredPicNum" (= -1: none)
	int			m_req_pic_num_amount;	// 0x7d8  "ReqPicNumAmount" (= 1)
	int			m_initially_active;		// 0x7dc  "InitallyActive" (sic; = 1)
	char		m_activate_object[100];	// 0x7e0  "ActivateObject" (= "none")
	char		m_need_more_name[100];	// 0x844  "NeedMoreName" (= "none")
	char		m_sound_name[100];		// 0x8a8  "SoundName" (= "none")
	int			f_90c;					// 0x90c  0 (not used)
	int			m_pass_thru;			// 0x910  "PassThru" (= 1; not used)
	int			m_show_arrow;			// 0x914  "ShowArrow" (= 0; not used)
	C3DCorona	*m_corona;				// 0x918  the glow (the original keeps its CGameObject part)
	int			m_show_corona;			// 0x91c  "ShowCorona" (= 0)
	float		m_red;					// 0x920  "Red" (= 1)
	float		m_green;				// 0x924  "Green" (= 1)
	float		m_blue;					// 0x928  "Blue" (= 1)
	float		m_respawn_time;			// 0x92c  "RespawnTime" (= -1: never; then the state stays set)
	float		m_respawn_left;			// 0x930  0 (saved)
	float		m_powerup_time;			// 0x934  "PowerupTime" (= 30)
	int			m_is_voice;				// 0x938  "IsVoice" (= 0: its sound is a voice clip on the voice channel)
};
