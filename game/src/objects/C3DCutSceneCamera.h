/*  C3DCutSceneCamera ('3CAM') - one shot of a cut scene: when fired (by a trigger chain, its
    TriggerCallback, or a C3DStartPoint) it takes the view, plays a voice clip ("SoundIndex" or
    "SoundName" from "SoundDatabase"), points the camera at "CameraTarget" (optionally starting
    "TargetActAnim" on it) and, when the clip ends or the player skips, fires "NextTrigger".
    "CameraType" 2 keeps the camera where the sprite is; otherwise the camera rides on the target
    ("TargOffset", zooming from "InitialDist" by "ZoomSpeed" within MinDist..MaxDist), or with
    "ViewFromCamera" looks at the target from the sprite's direction. "AltViewport" shows the
    shot in a small picture-in-picture viewport instead; "LetterBox" adds the cinema bars and
    mutes the music. In the editor, Ctrl+L previews the shot.
    Reconstructed from Neutron2.exe: constructor 0x429f30, destructor 0x42a2b0 (only resets the
    vtables, then ~C3DTriggerType; deleting thunks 0x42b760 / 0x42a280 / 0x42b770). Vtables
    0x4d7744 (element, 60 slots: C3DTriggerType's 56 + 56..59) / 0x4d7730 (listener) / 0x4d72b4
    (game object) / 0x4d72a4 (streamer). 0xb30 bytes: C3DTriggerType's 0x7a0, own members
    0x7a0..0xb28, vtordisp 0xb28, the streamer at 0xb2c.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DTriggerType.h"
#include "OMedia3DVector.h"

class C3DCutSceneCamera : public C3DTriggerType
{
	public:
	C3DCutSceneCamera();													// 0x429f30
	virtual ~C3DCutSceneCamera();											// 0x42a2b0

	// ---- element-family slots it adds
	virtual void TriggerCamera(unsigned char in_window);					// ESLOT 56 0x42abd0 (+0xe0) fire: take the view (or a small one), start the clip
	virtual void SetView(OMedia3DVector pos, OMedia3DVector look);			// ESLOT 57 0x42b460 (+0xe4) place the (cut-scene) viewport
	virtual void UpdateView(float t);										// ESLOT 58 0x42a7f0 (+0xe8) camera position at time t of the shot
	virtual void ev59(void) {}												// ESLOT 59 0x463890 (+0xec) (empty)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x42a2f0
	virtual void v23(CGameObject *o);										// SLOT 23  0x42b430 TriggerCallback: fire
	virtual void Process(float dt);											// SLOT 240 0x42a580 the shot runs until its clip ends
	virtual void v242(float t);												// SLOT 242 0x42b710 editor: ` ends the preview
	virtual void v283(unsigned int dt_bits);								// SLOT 283 0x42b5d0 editor (float dt): Ctrl+L previews

	// FIELD 0x7a0 0x388
	char		m_camera_target[100];	// 0x7a0  "CameraTarget" (= "none")
	char		m_sound_db[100];		// 0x868  "SoundDatabase" (= "none")
	char		m_face_object[100];		// 0x8cc  "FaceObject" (= "none"; not used by the class)
	char		m_target_act_anim[100];	// 0x930  "TargetActAnim" (= "none")
	char		m_target_deact_anim[100];	// 0x994  "TargetDeactAnim" (= "none")
	char		m_player_controlled[100];	// 0x9f8  "PlayerControlled" ("none"; "NULL": take the view from the player camera)
	char		m_sound_name[100];		// 0xa5c  "SoundName" (= "none")
	int			m_sound_index;			// 0xac0  "SoundIndex" (= -1: play SoundName)
	int			m_channel;				// 0xac4  the clip's sound channel
	unsigned char m_active;				// 0xac8  the shot is running
	CGameObject	*m_target;				// 0xacc  the CameraTarget object
	float		m_targ_offset[4];		// 0xad0  "TargOffsetX/Y/Z" (= 0, 100, 200; Z becomes the zoomed distance), w 0xadc = 0
	int			m_view_from_camera;		// 0xae0  "ViewFromCamera" (= 1)
	float		m_look_voffset;			// 0xae4  "LookVoffset" (= 100)
	int			f_ae8;					// 0xae8  (not used)
	int			m_camera_type;			// 0xaec  "CameraType" (= 0; 2: the camera stays at the sprite)
	float		m_zoom_speed;			// 0xaf0  "ZoomSpeed" (= 15)
	float		m_max_dist;				// 0xaf4  "MaxDist" (= 600)
	float		m_min_dist;				// 0xaf8  "MinDist" (= 200)
	float		m_initial_dist;			// 0xafc  "InitialDist" (= 300)
	float		m_time;					// 0xb00  time into the shot
	int			m_loop_act_anim;		// 0xb04  "LoopActAnim" (= -1)
	int			m_deactivate_inv;		// 0xb08  "DeactivateInv" (= 1)
	int			m_alt_viewport;			// 0xb0c  "AltViewport" (= -1)
	int			m_letterbox;			// 0xb10  "LetterBox" (= 1)
	int			m_bypass_off;			// 0xb14  "BypassOff" (= 0)
	int			m_preview;				// 0xb18  editor preview running (Ctrl+L)
	int			f_b1c;					// 0xb1c  (not used)
	float		m_preview_time;			// 0xb20  editor preview time
	unsigned char m_in_window;			// 0xb24  TriggerCamera(in_window): shown in the small viewport while play goes on; handed on to NextTrigger
};
