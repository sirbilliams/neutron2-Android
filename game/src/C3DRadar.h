/*  C3DRadar ('3RAD') - the player's HUD panel: a translucent green radar disc (sprite 27 of
    "retainedsprites.omt") floating in front of the camera, a second sprite f_600 (sprite 30, its
    sweep / needle) turned by the heading of the object it is centred on, and up to 65 blips
    (2D interface elements on the HUD layer, g_radar_blips) for the objects whose "ShowOnRadar"
    is 1 (red), 2 (green) or 3 (blue). The range ("%dX", 1..5) follows the nearest kind-1 object.
    C3DJimmy's constructor makes the one radar (C3DJimmy f_d40); the menu toggles it with R
    (Show 0x468a50 / Hide 0x468aa0), C3DPickupItem opens it on a target (0x4689d0), CJimmyGame
    calls its v269 every frame and hides it when the power timer [3] runs out.
    Reconstructed from Neutron2.exe: factory '3RAD', constructor 0x468180 (0x640 bytes),
    destructor 0x468480 (only resets the vtables, then ~C3DPermanentSprite; deleting thunks
    0x469000 / 0x468450 / 0x469010). The blip array is constructed by the static initialiser
    0x468130 (InterfaceElt x 65 at 0x5461d0).

    Original layout: as C3DSprite (element vtable 0x4f3df0, 55 slots; listener 0x4f3ddc at +0x30;
    CLocalGameObject vtable 0x4f3960 at +0xc8), own members 0x600..0x638, vtordisp 0x638,
    streamer 0x63c.                                                                             */
#pragma once
#include "C3DPermanentSprite.h"

struct InterfaceElt;

class C3DRadar : public C3DPermanentSprite
{
	public:
	C3DRadar();																// 0x468180
	virtual ~C3DRadar();													// 0x468480

	// ---- CGameObject-family overrides
	virtual void Process(float dt);											// SLOT 240 0x4684c0 Goddard points at the target while shown
	virtual void v260(void);												// SLOT 260 0x4686e0 sprite, physics, colour, hidden, second pass
	virtual void v269(float dt);											// SLOT 269 0x4685b0 per frame (CJimmyGame): place / count / blips

	// ---- non-virtual methods (thiscall on the element)
	void Open(CGameObject *centre, int unused, int x, int y, int z, CGameObject *target);	// 0x4689d0
	void Show(void);														// 0x468a50 (only after Hide set g_545f0e)
	void Hide(void);														// 0x468aa0
	bool IsShown(void) const { return f_61c != 0; }							// (inline: +0x61c)
	void Place(void);														// 0x468790 in front of the camera, then the blips
	void UpdateBlips(CGameObject *centre, int unused, float pos[4]);		// 0x468b00
	static void Blip(int i, int x, int y, bool show, int kind, float size);	// 0x468e70 (stdcall)

	// ---- data members (object offsets)
	C3DPermanentSprite	*f_600;			// 0x600  the needle sprite (sprite 30, size 2), turned by f_620's heading
	int					f_604;			// 0x604  kind-1 objects seen by the last UpdateBlips (0)
	int					f_608;			// 0x608  range unit (11000)
	int					f_60c;			// 0x60c  blip centre x on the screen (550, set by Place)
	int					f_610;			// 0x610  blip centre y (73)
	int					f_614;			// 0x614  range 1..5 ("%dX"; 99 = none yet)
	float				f_618;			// 0x618  distance scale (7; Place: per level 1, 2 or 6)
	unsigned char		f_61c;			// 0x61c  shown (0)
	CGameObject			*f_620;			// 0x620  the object at the centre (Open)
	int					f_624;			// 0x624  0 (passed to UpdateBlips, unused)
	CGameObject			*f_628;			// 0x628  the target Goddard points at while shown (Open)
	int					f_62c;			// 0x62c  camera-relative position of the disc (Open)
	int					f_630;			// 0x630
	int					f_634;			// 0x634
};

namespace neg {
extern InterfaceElt g_radar_blips[65];		// 0x5461d0
}
