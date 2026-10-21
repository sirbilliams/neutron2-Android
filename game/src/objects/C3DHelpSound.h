/*  C3DHelpSound ('3HEL') - a voice / sound trigger, drawn as a spinning sprite ("sprites.omt"
    canvas 303, size 170, translucent). Two kinds:
    * IsAmbient 1: loops SoundName from where it stands (3D) once the task TaskName has reached
      RequiredLevel, while the sound effects database is open.
    * IsAmbient 0: played by TriggerHelp (slot 55: when touched with TouchActivated 1 and no
      NextTrigger, or by a ToggleObject through v268) up to TimesToTrigger times: "bring" and
      SoundName as the help voice (g_voice_channel, unless one is playing). With a NextTrigger
      (a C3DTriggerType) the camera's touch calls that trigger instead (AltViewport: its flag)
      and DisableHelp names another C3DHelpSound to switch off.
    "SoundIndex" (a numbered sound) is no longer supported: it logs an error and plays
    "not available".
    Reconstructed from Neutron2.exe: constructor 0x43d720 (factory '3HEL', 0x820 bytes),
    destructor 0x43d9a0 (stops the ambient loop, then ~C3DSpriteType; deleting thunks 0x43d970 /
    0x43e260 / 0x43e270). Vtables 0x4e0e60 (element, 56 slots: C3DSprite's 55 + TriggerHelp) /
    0x4e0e4c (listener) / 0x4e09d0 (game object) / 0x4e09c0 (streamer: C3DSprite's read_class /
    write_class copies 0x43e280 / 0x43e290). C3DSpriteType's 0x668, own members 0x668..0x818,
    vtordisp 0x818, the streamer at 0x81c.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DHelpSound : public C3DSpriteType
{
	public:
	C3DHelpSound();															// 0x43d720
	virtual ~C3DHelpSound();												// 0x43d9a0

	// ---- element-family slots it adds
	virtual void TriggerHelp(void);											// ESLOT 55 0x43dd80 (+0xdc) play the help voice

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43da30 properties
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43db60 the camera touched it
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x43dea0 spin; the ambient loop
	virtual void v260(void);												// SLOT 260 0x43e080 translucent (windowed: opaque blend)
	virtual void v268(int a2);												// SLOT 268 0x43dd60 ToggleObject: TriggerHelp
	virtual void v277(void);												// SLOT 277 0x43e1b0 pause: stop the loop
	virtual void v278(void);												// SLOT 278 0x43e1d0 resume: the loop again
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x43e180 + times triggered
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x43e110 + times triggered, used up: hidden

	// FIELD 0x668 0x1b0
	int			m_sound_index;			// 0x668  "SoundIndex" (= -1; anything else: error)
	int			m_times_to_trigger;		// 0x66c  "TimesToTrigger" (= 1; -1: no limit)
	int			m_times_triggered;		// 0x670  (= 0; saved)
	int			m_is_ambient;			// 0x674  "IsAmbient" (= 0)
	int			m_channel;				// 0x678  the ambient loop's channel (= -1)
	int			m_required_level;		// 0x67c  "RequiredLevel" (= 0): TaskName's state needed
	char		m_sound_name[100];		// 0x680  "SoundName" (= "none")
	int			m_touch_activated;		// 0x6e4  "TouchActivated" (= 1)
	char		m_next_trigger[100];	// 0x6e8  "NextTrigger" (= "none")
	char		m_player_control[100];	// 0x74c  "PlayerControl" (= "none"; not read here)
	char		m_disable_help[100];	// 0x7b0  "DisableHelp" (= "none")
	int			m_alt_viewport;			// 0x814  "AltViewport" (= 0): the flag for NextTrigger
};
