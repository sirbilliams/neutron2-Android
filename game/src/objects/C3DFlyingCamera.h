/*  C3DFlyingCamera ('3FLY') - a camera that flies a cut scene: a C3DAI that patrols its chain
    of C3DPatrolPoints ("PatrolPoint", AIState 3, at "Speed", turning at "TurnSpeedX/Y/Z") with
    gravity cancelled, while the view rides on it and looks at "CameraTarget" (or, with "none",
    at the point "ViewDirX/Y/Z" in front of it). It stands still and hidden until it is fired
    (TriggerCamera, element slot 144: by a C3DTriggerType's NextTrigger, by another flying
    camera, or by the menu's fade ending), then goes back to where it was placed, opens
    "SoundDatabase" and plays its voice clip ("SoundIndex", or "SoundName"; chunk 0 as a
    fallback). When the clip ends, or the player skips, it fires "NextTrigger" (CallNextTrigger,
    slot 147): a C3DFlyingCamera (TriggerCamera), a C3DTriggerType (its CallNextTrigger), or
    "DEFAULT" (ResetToDefault, slot 146: the camera back to Jimmy and his level music).
    "LetterBox" 1 shows the cinema bars and mutes the music; "BypassOff" clears an earlier skip.
    The model is camera.omt's shape 0 (animation system 2: no animations at all), seen only in
    the editor, where Ctrl+L previews the flight and ` stops it.
    In the shipped levels: Level5 "fowlcam1" (path fcampath1, voice "introfowl", then
    "fowlstop") and "introcammus1" (mus2path1 looking at "museumdoor", "intromusica", then
    "introcammus2"); Level3 "labflyin1" (campatha1 looking at "window", "intronarrator03",
    then "fadecam2").
    Reconstructed from Neutron2.exe: constructor 0x433720 (factory '3FLY', 0x1d18 bytes),
    destructor 0x433a50 (only resets the vtables, then ~C3DAI; deleting thunks 0x433a20 /
    0x434660 / 0x434670 / 0x434680).

    Original layout: as C3DAI (element vtable 0x4dc538, 148 slots: C3DAI's 144 + 144..147;
    listener 0x4dc524; OMediaAnim 0x4dc4e8; CLocalGameObject vtable 0x4dc06c at +0xc0, slots
    0, 7, 10, 231, 240, 242, 283 overridden), own members 0x1b50..0x1d10, vtordisp 0x1d10,
    streamer 0x1d14.

    Element-family slots are `ev<n>` or named (`// ESLOT n`); members are f_<offset in the
    original object> (CGameObject-family code of the original sees them at offset - 0xc0).    */
#pragma once
#include "../C3DAI.h"

class C3DFlyingCamera : public C3DAI
{
	public:
	C3DFlyingCamera();														// 0x433720
	virtual ~C3DFlyingCamera();												// 0x433a50

	// ---- element-family slots it adds
	virtual void TriggerCamera(void);										// ESLOT 144 0x433f50 (+0x240) fire: back to the start, open the sound database, play the clip
	virtual unsigned char StartSound(void);									// ESLOT 145 0x434090 (+0x244) play SoundIndex / SoundName (chunk 0 if that fails)
	virtual void ResetToDefault(void);										// ESLOT 146 0x434480 (+0x248) NextTrigger "DEFAULT": the camera back to Jimmy
	virtual void CallNextTrigger(void);										// ESLOT 147 0x433e90 (+0x24c) fire NextTrigger

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x433ad0 properties
	virtual void ResetObject(void);											// SLOT 10  0x433aa0
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x434470 collides with nothing (0)
	virtual void Process(float dt);											// SLOT 240 0x433c50 fly and look while the clip plays
	virtual void v242(float t);												// SLOT 242 0x434180 editor: ` ends the preview
	virtual void v283(unsigned int dt_bits);								// SLOT 283 0x434220 editor (float dt): Ctrl+L previews

	// ---- data members (object offsets)
	char				f_1b50[100];	// 0x1b50 "CameraTarget" ("none": look along ViewDir)
	CGameObject			*f_1bb4;		// 0x1bb4 the CameraTarget object (looked up once)
	OMedia3DVector		f_1bb8;			// 0x1bb8 "ViewDirX/Y/Z" (0, 20, 100): local point looked at; w 0x1bc4 = 0
	char				f_1bc8[100];	// 0x1bc8 "NextTrigger" ("none")
	int					f_1c2c;			// 0x1c2c "SoundIndex" (-1: play SoundName)
	char				f_1c30[100];	// 0x1c30 "SoundName" ("none")
	char				f_1c94[100];	// 0x1c94 "SoundDatabase" ("none")
	int					f_1cf8;			// 0x1cf8 (not used)
	unsigned char		f_1cfc;			// 0x1cfc flying (the clip is playing)
	int					f_1d00;			// 0x1d00 the clip's sound channel (-1)
	int					f_1d04;			// 0x1d04 "LetterBox" (1)
	int					f_1d08;			// 0x1d08 "BypassOff" (0)
	int					f_1d0c;			// 0x1d0c editor preview running (Ctrl+L)
};

namespace neg {
// TriggerCamera of a C3DFlyingCamera known as a CGameObject (C3DTriggerType, the menu's fade).
void FlyingCameraTrigger(CGameObject *o);
}
