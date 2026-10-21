/*  C3DSoundEffect ('3SOU') - a sound trigger, drawn (in the editor) as "icons.omt" canvas 8,
    size 50. Like C3DHelpSound without the help voice parts:
    * IsAmbient 1: loops SoundName from where it stands (3D) once the task TaskName has reached
      RequiredLevel, while the sound effects database is open.
    * IsAmbient 0: TriggerSound (slot 55: when the camera touches it with TouchActivated 1, or a
      ToggleObject through v268) plays SoundName up to TimesToTrigger times (-1: no limit); with
      IsVoice 1 as the voice (g_voice_channel, unless one is playing). QuakeTime > 0 also shakes
      the camera of "JIM1" for that long.
    "SoundIndex" (a numbered sound) is no longer supported: it logs an error and plays
    "not available".
    Reconstructed from Neutron2.exe: constructor 0x470b50 (factory '3SOU', 0x6f8 bytes),
    destructor 0x470d50 (stops the ambient loop, then ~C3DSpriteType; deleting thunks 0x470d20 /
    0x4712e0 / 0x4712f0). Vtables 0x4fa108 (element, 56 slots: C3DSprite's 55 + TriggerSound) /
    0x4fa0f4 (listener) / 0x4f9c78 (game object) / 0x4f9c68 (streamer). Its layout matches
    C3DHelpSound's up to 0x6e4, and slots 231, 268, 277, 278 and 284 are C3DHelpSound's code.
    Own members 0x668..0x6f0, vtordisp 0x6f0, the streamer at 0x6f4.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DSpriteType.h"

class C3DSoundEffect : public C3DSpriteType
{
	public:
	C3DSoundEffect();														// 0x470b50
	virtual ~C3DSoundEffect();												// 0x470d50

	// ---- element-family slots it adds
	virtual void TriggerSound(void);										// ESLOT 55 0x470f20 (+0xdc) play the sound

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x470de0 properties
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x470ee0 the camera touched it
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x4710c0 the ambient loop
	virtual void v260(void);												// SLOT 260 0x46ff00 (a jump to C3DSpriteType's)
	virtual void v268(int a2);												// SLOT 268 0x43dd60 ToggleObject: TriggerSound
	virtual void v277(void);												// SLOT 277 0x43e1b0 pause: stop the loop
	virtual void v278(void);												// SLOT 278 0x43e1d0 resume: the loop again
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x43e180 + times triggered
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x471290 + times triggered

	// FIELD 0x668 0x88
	int			m_sound_index;			// 0x668  "SoundIndex" (= -1; anything else: error)
	int			m_times_to_trigger;		// 0x66c  "TimesToTrigger" (= -1: no limit)
	int			m_times_triggered;		// 0x670  (= 0; saved)
	int			m_is_ambient;			// 0x674  "IsAmbient" (= 0)
	int			m_channel;				// 0x678  the ambient loop's channel (= -1)
	int			m_required_level;		// 0x67c  "RequiredLevel" (= 0): TaskName's state needed
	char		m_sound_name[100];		// 0x680  "SoundName" (= "none")
	float		m_quake_time;			// 0x6e4  "QuakeTime" (= 0): camera shake (s)
	int			m_touch_activated;		// 0x6e8  "TouchActivated" (= 1)
	int			m_is_voice;				// 0x6ec  "IsVoice" (= 1): played as the voice
};
