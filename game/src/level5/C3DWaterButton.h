/*  C3DWaterButton ('3WAB') - a button that raises the water: a morph object with UP / DOWN
    ("Up.ase" "arrowbutton.ase", "Down.ase" "arrowbuttonin.ase", texture "UpDown.Png"
    "arrow1.png") next to the level's water plane (tagged "C3DWATERPLANE"). Under the water it
    is black and not available; above it, while UP, it pulses red. Pressed (ev95: touched by the
    camera or a water balloon once "TaskName" has reached RequiredLevel, or by NextState) when
    UP, available, not just pressed and the water is still: unless the water already stands at
    its target ("UpLevels" x "LevelAmount" (1 x 200) above the button, less 70), it goes DOWN
    (dark) for 4 s, plays "click shrink ray" + "waterstart" and sends the water there at 100
    (only "waterbutton1" when the water is not below the button). Pressed while DOWN it goes
    back UP. Either way the object tagged "ActivateButton" gets NextState("Toggle").
    Reconstructed from Neutron2.exe: constructor 0x47d3e0 (factory '3WAB', 0xaf8 bytes),
    destructor 0x47d640 (deleting thunks 0x47d610 / 0x47e000 / 0x47e010 / 0x47e020).

    Original layout: as C3DButton up to 0xa6c (element vtable 0x504e40, 96 slots; CLocalGameObject
    vtable 0x504974 at +0xc0: 0, 7, 10, 240, 258, 260 overridden, 16 and 268 are C3DButton's
    0x47d9a0 / 0x47da00), own members 0x924..0xaf0, vtordisp 0xaf0, streamer 0xaf4.         */
#pragma once
#include "../C3DAnimated.h"

class C3DWaterPlane;

class C3DWaterButton : public C3DAnimated
{
	public:
	C3DWaterButton();														// 0x47d3e0
	virtual ~C3DWaterButton();												// 0x47d640

	// ---- element-family slots
	virtual void ev69(void);												// ESLOT 69 0x47df40 load the animations and the texture (once)
	virtual void ev94(int unused) { (void)unused; }							// ESLOT 94 0x464ee0 (empty, one stack argument)
	virtual void ev95(void);												// ESLOT 95 0x47da20 pressed

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x47d810 properties
	virtual void ResetObject(void);											// SLOT 10  0x47d690 available again (or not): UP / DOWN
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x47d9a0 the camera or a water balloon: press
	virtual void Process(float dt);											// SLOT 240 0x47dce0 the water level, reset timer, pulse
	virtual void v258(void);												// SLOT 258 0x47de90 radius 100
	virtual void v260(void);												// SLOT 260 0x47dec0 the water plane, the target level
	virtual void v268(int state);											// SLOT 268 0x47da00 "NextState": press (state unused)

	// ---- data members (object offsets)
	float				f_924;			// 0x924  "Red" (1)
	float				f_928;			// 0x928  "Green" (0.1)
	float				f_92c;			// 0x92c  "Blue" (0.1)
	int					f_930;			// 0x930  available (0)
	int					f_934;			// 0x934  "ButtonAvailable" (0)
	int					f_938;			// 0x938  "NASound" (-1; not used)
	int					f_93c;			// 0x93c  "AvailSound" (-1): a sound index played when pressed
	char				f_940[100];		// 0x940  "ActivateButton" ("none"): tag of the object told
	char				f_9a4[100];		// 0x9a4  "Down.ase" ("arrowbuttonin.ase")
	char				f_a08[100];		// 0xa08  "Up.ase" ("arrowbutton.ase")
	char				f_a6c[100];		// 0xa6c  "UpDown.Png" ("arrow1.png")
	int					f_ad0;			// 0xad0  "NewTaskState" (0; not used)
	int					f_ad4;			// 0xad4  "Toggle" (-1): the state passed on
	float				f_ad8;			// 0xad8  reset timer (0)
	int					f_adc;			// 0xadc  "UpLevels" (1)
	float				f_ae0;			// 0xae0  "LevelAmount" (200)
	C3DWaterPlane		*f_ae4;			// 0xae4  the water plane (the original keeps its CGameObject part)
	float				f_ae8;			// 0xae8  the water level
	float				f_aec;			// 0xaec  the level the button raises the water to

	private:
	bool HasColour(void) const { return f_924 != -1.0f || f_928 != -1.0f || f_92c != -1.0f; }	// (bit compares in the original)
	void FindWater(void);
};
