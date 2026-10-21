/*  C3DMultiCutSceneCamera ('3MCA') - a cut scene of up to eight shots (the story scenes of the
    lab). Shot i points the camera at "CameraTarget<i>" (starting "TargetAnim<i>" on it), plays
    "SoundIndex<i>" (or "SoundNames<i>") from "SoundDatabase" and places the camera by
    "CameraType<i>" (an offset in the target's frame: fixed, zooming, or circling it at
    "CameraSpeed" / "CameraDist"; type 10 shows the shot in the small picture-in-picture viewport,
    11 closes it; +128 measures from "FixedTarget" instead), looking at the target raised by
    "LookatVOffset<i>" - 60. When a clip ends the next shot starts; after the last (or a skip)
    the camera goes to "PlayerControlled" ("NULL": nobody), the C3DAI held by the scene is
    released, and "NextTrigger" fires. "LetterBox" adds the cinema bars and mutes the music.
    In the editor, slot 61 starts a preview: keys 0..7 pick the shot, ` ends it.
    Reconstructed from Neutron2.exe: constructor 0x456de0, destructor 0x457070 (only resets the
    vtables, then ~C3DTriggerType; deleting thunks 0x458d20 / 0x457040 / 0x458d30). Vtables
    0x4ebb04 (element, 64 slots: C3DTriggerType's 56 + 56..63) / 0x4ebaf0 (listener) / 0x4eb674
    (game object) / 0x4eb664 (streamer). 0x1354 bytes: C3DTriggerType's 0x7a0, own members
    0x7a0..0x134c, vtordisp 0x134c, the streamer at 0x1350.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DTriggerType.h"
#include "OMedia3DVector.h"

class C3DMultiCutSceneCamera : public C3DTriggerType
{
	public:
	enum { kShots = 8 };

	C3DMultiCutSceneCamera();												// 0x456de0
	virtual ~C3DMultiCutSceneCamera();										// 0x457070

	// ---- element-family slots it adds
	virtual void TriggerCamera(unsigned char in_window);					// ESLOT 56 0x457760 (+0xe0) fire: shot 0
	virtual void TriggerCallback(CGameObject *caller, unsigned char flag);	// ESLOT 57 0x457be0 (+0xe4) fired by CallNextTrigger
	virtual void SetView(OMedia3DVector pos, OMedia3DVector look);			// ESLOT 58 0x457c10 (+0xe8) place the (cut-scene) viewport
	virtual bool NextShot(void);											// ESLOT 59 0x4588c0 (+0xec) "GetNextSoundChannel": start shot m_shot
	virtual void EndCutScene(void);											// ESLOT 60 0x4576c0 (+0xf0) hand the camera on, fire NextTrigger
	virtual void StartPreview(void);										// ESLOT 61 0x4588a0 (+0xf4) editor preview from shot 0
	virtual void UpdateView(float t, int camera_type);						// ESLOT 62 0x457d80 (+0xf8) camera at time t of a shot
	virtual void OpenAltViewport(void);										// ESLOT 63 0x458500 (+0xfc) the small viewport

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4570b0
	virtual void Process(float dt);											// SLOT 240 0x457400 the shots run until the last clip ends
	virtual void v242(float t);												// SLOT 242 0x4586f0 editor: the preview

	// FIELD 0x7a0 0xbac
	char		m_sound_db[100];				// 0x7a0  "SoundDatabase" (= "none")
	char		m_target_deact_anim[100];		// 0x804  "TargetDeactAnim" (= "STOP")
	char		m_camera_target[kShots][100];	// 0x868  "CameraTarget<i>" (= "none")
	char		m_target_anim[kShots][100];		// 0xb88  "TargetAnim<i>" (= "TALK")
	float		m_lookat_voffset[kShots];		// 0xea8  "LookatVOffset<i>" (= 100)
	int			m_sound_index[kShots];			// 0xec8  "SoundIndex<i>" (= -1: play SoundNames<i>)
	int			m_camera_type[kShots];			// 0xee8  "CameraType<i>" (= 0; 0..17, +128: from FixedTarget)
	char		m_player_controlled[100];		// 0xf08  "PlayerControlled" (= "NULL": the camera goes to nobody)
	char		m_sound_names[kShots][100];		// 0xf6c  "SoundNames<i>" (= "none")
	char		m_fixed_target[100];			// 0x128c "FixedTarget" (= "none")
	CGameObject	*m_fixed_target_obj;			// 0x12f0 the FixedTarget object (looked up once)
	unsigned char m_in_window;					// 0x12f4 TriggerCamera(in_window): shown in the small viewport
	int			m_channel;						// 0x12f8 the clip's sound channel (= -1)
	unsigned char m_active;						// 0x12fc a shot is running
	CGameObject	*m_target;						// 0x1300 the shot's CameraTarget object
	float		m_cam_offset[3];				// 0x1304 UpdateView: the camera point in the target's frame
	int			f_1310;							// 0x1310 = 0 (not used)
	int			m_shot;							// 0x1314 the next shot
	float		m_time;							// 0x1318 time into the shot
	float		m_lookat;						// 0x131c the running shot's LookatVOffset
	int			m_cur_camera_type;				// 0x1320 the running shot's CameraType
	CGameObject	*m_held_ai;						// 0x1324 the C3DAI the scene holds (state 9), released at the end
	int			m_deactivate_inv;				// 0x1328 "DeactivateInv" (= 1)
	int			f_132c;							// 0x132c (not used)
	int			m_letterbox;					// 0x1330 "LetterBox" (= 1)
	int			m_bypass_off;					// 0x1334 "BypassOff" (= 0)
	int			m_preview_shot;					// 0x1338 editor preview: the shot
	float		m_preview_time;					// 0x133c editor preview: its time
	int			m_preview;						// 0x1340 editor preview running
	float		m_camera_speed;					// 0x1344 "CameraSpeed" (= 2: circling, radians per unit of time)
	float		m_camera_dist;					// 0x1348 "CameraDist" (= 500: circling radius)
};
