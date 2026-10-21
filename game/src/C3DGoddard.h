/*  C3DGoddard ('3GOD') - Jimmy's robot dog, a Granny-animated C3DAI ("goddbase.grn" and its
    animations). As an AI he follows his target (Jimmy, '3JIM') in state 4 at one of six places
    around it (ev144: behind, beside, in front; ev148 / ev145 move to a free one when a wall is
    in the way), teleports behind him when he is left too far behind, sits and does tricks when
    told to wait (state 1), flies when he is not on the floor, fetches bones (C3DBONE). He has a
    blob shadow (f_1b50), a multiplied Granny shadow ("goddshadow.grn", f_1bc0) and two 3D arrows
    (C3DCursor f_1b80 towards the nearest red neutron, f_1b84 towards the nearest pickup) that
    show while the player steers him (v243: keys or mouse turn and run, jump flies him up to
    f_1b94). C3DJimmy's constructor makes the one Goddard (C3DJimmy f_d5c); C3DRocketShip and
    C3DFish carry him; C3DAITrigger "ShowTarget" points his arrow.
    Reconstructed from Neutron2.exe: factory '3GOD', constructor 0x4383d0 (0x1bcc bytes),
    destructor 0x438b40 (only resets the vtables, then ~C3DAI; deleting thunks 0x43b5b0 /
    0x438b10 / 0x43b5c0 / 0x43b5a0). ResetObject is C3DAI's (0x43a130 jumps there).
    The six places (static initialisers 0x4382b0..0x4383a0, OMedia3DVectors with w = 0) are
    constants of the code here (kSides).

    Original layout: as C3DAI (element vtable 0x4df4dc, 150 slots: C3DAI's 144 + 144..149;
    OMediaAnim vtable 0x4df48c at +0x90; listener 0x4df4c8 at +0x30; CLocalGameObject vtable
    0x4df010 at +0xc0), own members 0x1b50..0x1bc4, vtordisp 0x1bc4, streamer 0x1bc8.

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                            */
#pragma once
#include "C3DAI.h"

class C3DShadow;
class C3DGrannyObj;
class OMediaElement;
class OMediaStreamOperators;
class OMedia3DPolygon;

class C3DGoddard : public C3DAI
{
	public:
	C3DGoddard();															// 0x4383d0
	virtual ~C3DGoddard();													// 0x438b40

	// ---- element-family slots
	virtual void ev60(void);												// ESLOT 60  0x43b570 no such animation: SIT
	virtual void ev67(void);												// ESLOT 67  0x43b510 FIX / LOSTWING over: ROCK
	virtual void ev107(int state);											// ESLOT 107 0x439b50 + state 3: speed 400, props on; 7: props off
	virtual void ev116(void);												// ESLOT 116 0x439c30 face the way the target faces (until arrived)
	virtual void ev118(void);												// ESLOT 118 0x43a140 f_1b54 = -1 (no RUN animation)
	virtual void ev124(void);												// ESLOT 124 0x43a160 stop the sound f_1b58
	virtual void ev144(short side);											// ESLOT 144 0x438b90 follow at kSides[side]
	virtual unsigned char ev145(short side);								// ESLOT 145 0x43a470 that side free (nothing between us)? then take it
	virtual void ev146(float dt);											// ESLOT 146 0x438f50 steered: turn, run along the heading
	virtual void ev147(unsigned char on);									// ESLOT 147 0x439a40 "sonar" on / off
	virtual void ev148(void);												// ESLOT 148 0x43a1b0 blocked: try the sides next to ours
	virtual void ev149(void);												// ESLOT 149 0x439c80 state 4: follow (speed, teleport, animations)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x438cd0 model, animations
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43a830 Jimmy / a bone
	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x43a9a0 ceiling: f_bb0 = 0
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x43a190 wall: state 4 -> ev148
	virtual void Process(float dt);											// SLOT 240 0x439170
	virtual void v243(float dt);											// SLOT 243 0x43a9b0 steered by the player (keys / mouse), arrows
	virtual void v260(void);												// SLOT 260 0x439f90 properties loaded (hidden in LV3C)
	virtual void v266(void);												// SLOT 266 0x43a090 the Granny shadow multiplied, second pass
	virtual void v270(int old_mode, int new_mode);							// SLOT 270 0x43b410 "C3DGoddard::ChangeState": wait during cut scenes
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x43a0f0 + state 3: speed 400, props on

	// ---- data members (object offsets)
	C3DShadow			*f_1b50;		// 0x1b50 blob shadow
	int					f_1b54;			// 0x1b54 sound channel (-1; ev118 forgets it)
	int					f_1b58;			// 0x1b58 sound channel (-1; ev124 stops it)
	short				f_1b5c;			// 0x1b5c side we follow at (ev144, 0..5)
	short				f_1b5e;			// 0x1b5e side ev148 started from
	unsigned char		f_1b60;			// 0x1b60 sitting / riding along (state 1, C3DFish, C3DJimmy); ev149 ends it when far
	float				f_1b64;			// 0x1b64 steering: turn rate wanted (+-60, eases back)
	float				f_1b68;			// 0x1b68 steering: turn rate used (ev146)
	float				f_1b6c;			// 0x1b6c the same, as used last
	float				f_1b70;			// 0x1b70 steering: top speed (700)
	float				f_1b74;			// 0x1b74 steering: acceleration (30)
	int					f_1b78;			// 0x1b78 steering: animation playing (0 RUN, 1 WAG, 3 FLY)
	int					f_1b7c;			// 0x1b7c "sonar" sound channel (-1)
	CGameObject			*f_1b80;		// 0x1b80 C3DCursor towards the nearest red neutron (its CGameObject part)
	CGameObject			*f_1b84;		// 0x1b84 C3DCursor towards the nearest pickup (its CGameObject part)
	CGameObject			*f_1b88;		// 0x1b88 the nearest '3PIC'
	CGameObject			*f_1b8c;		// 0x1b8c the nearest '3RED'
	unsigned char		f_1b90;			// 0x1b90 go behind Jimmy (the camera object) next ev149
	float				f_1b94;			// 0x1b94 steering: highest he flies (1500; VR06 3500)
	float				f_1b98;			// 0x1b98 steering: mouse x, summed
	float				f_1b9c;			// 0x1b9c steering: mouse y, summed
	float				f_1ba0;			// 0x1ba0 time since the jump (f_bb0, 5 s)
	unsigned char		f_1ba4;			// 0x1ba4 not in this level (LV3C)
	unsigned char		f_1ba5;			// 0x1ba5 fetching a bone (back to f_be4 at Jimmy)
	OMediaElement		*f_1ba8;		// 0x1ba8 an element kept at our head (never set: 0)
	int					f_1bac;			// 0x1bac next trick when waiting (0..4)
	float				f_1bb0;			// 0x1bb0 time left of the trick
	unsigned char		f_1bb4;			// 0x1bb4 0
	int					f_1bb8;			// 0x1bb8 -1
	C3DGrannyObj		*f_1bbc;		// 0x1bbc prop on his head ("headspinbase.grn")
	C3DGrannyObj		*f_1bc0;		// 0x1bc0 his Granny shadow ("goddshadow.grn" on "shadowbone")
};
