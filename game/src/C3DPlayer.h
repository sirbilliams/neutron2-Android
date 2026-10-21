/*  C3DPlayer - the C3DPlayerObject the player drives: keyboard / mouse / joystick movement, the
    jump and the jetpack, climbing fences and ladders, the animation choice and the chase camera.
    The base of C3DJimmy ('3JIM'). No factory ID of its own (InitObject sets ObjectID '3PLA').
    Reconstructed from Neutron2.exe: constructor 0x45ff00, destructor 0x4603d0 (deleting thunks
    0x460350 / 0x464d00 / 0x464d10 / 0x464d20).

    Original layout: as C3DPlayerObject (element vtable 0x4f1d54, 127 slots: C3DPlayerObject's
    106 + 106..126; CLocalGameObject vtable 0x4f1888 at +0xc0), own members 0x960..0xcac,
    vtordisp 0xcac, streamer 0xcb0. C3DJimmy puts its members from 0xcac on.

    Control state f_a68 (Process / v243 pick the slots by it):
      state   Process (movement)        v243 (control + camera)
      0       ev108 walk                ev109 keyboard/mouse + chase camera
      1       ev112 jump / jetpack      ev110 vehicle-like steering + camera
      2       ev107 bounce              ev110
      3       (nothing)                 ev111 turn on the spot + camera
      4       ev108 (swimming)          ev109
      5       ev106 (glide)             ev109
    States 0, 1, 4 and 5 also pick RUN / STOP / LEFT / RIGHT / JUMP from the speed and keys.
    While f_b20 > 0 (climbing a fence or ladder: v220 -> ev114 "FENCE"/"LADDER") v243 runs the
    climb camera instead and ends the climb (ev67 "AnimEnded" ends the animation side).

    Speeds: f_96c forward speed (clamped to f_968), f_960 acceleration and f_964 braking per
    frame, f_978 turn rate (degrees / s). The camera wants to sit at v224(f_a6c) (ev113 for
    states 1-3) looking at v224(f_a7c), pulled in front of walls (ev95), kept within reach
    (ev96) and moved there smoothly (ev98, C3DPlayerObject).

    Element-family slots are `ev<n>` (`// ESLOT n`) unless a log string names them; members are
    f_<offset in the original object> (CGameObject-family code sees them at offset - 0xc0).

    Hooks into C3DJimmy: C3DPlayer calls a few things of Jimmy's (g_jimmy): its jetpack (the
    object at C3DJimmy +0xccc) and its ESLOT 138 "ActivateMenuItem". They are declared below and
    defined weak (doing nothing but a TODO log) in C3DPlayer.cpp; C3DJimmy.cpp defines the real
    ones. The input tests 0x465750..0x465ad0 are exported for C3DJimmy too.                  */
#pragma once
#include "C3DPlayerObject.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"

class OMedia3DPolygon;

class C3DPlayer : public C3DPlayerObject
{
	public:
	C3DPlayer();															// 0x45ff00
	virtual ~C3DPlayer();													// 0x4603d0

	// ---- element-family slots
	virtual void ev67(void);												// ESLOT 67  0x4639b0 "AnimEnded": FENCE / LADDER / FULLSTOP / SPLAT / HIT done
	virtual void ev106(float dt);											// ESLOT 106 0x460940 state 5 movement (turns twice as fast; jumps through Jimmy)
	virtual void ev107(float dt);											// ESLOT 107 0x461640 state 2 movement: bounce ("bubble boing")
	virtual void ev108(float dt);											// ESLOT 108 0x460ce0 state 0/4 movement: walk, side step f_b0c, held to the floor
	virtual void ev109(float dt);											// ESLOT 109 0x461b50 state 0/4/5 control (speed, turn, BACK) + chase camera
	virtual void ev110(float dt);											// ESLOT 110 0x463140 state 1/2 control (steer, mouse y = throttle) + camera
	virtual void ev111(float dt);											// ESLOT 111 0x462b70 state 3: turn only + camera
	virtual void ev112(float dt);											// ESLOT 112 0x461010 state 1 movement: jump, or fly (jetpack: gadget 3 or g_flag_545e54)
	virtual OMedia3DVector ev113(float x, float y, float z);				// ESLOT 113 0x435d20 position + (x,y,z) turned by (0, heading + f_978, roll), w = 1
	virtual void ev114(char *name);											// ESLOT 114 0x464260 play player animation <name> (rules: locks, FENCE/LADDER start a climb)
	virtual void ev115(void) {}												// ESLOT 115 0x463890 (empty) before RUN starts
	virtual void ev116(void) {}												// ESLOT 116 0x463890 (empty) an animation change / stop
	virtual unsigned char ev117(void);										// ESLOT 117 0x4638a0 state 0 and the floor more than 70 below v224(0,0,70)
	virtual void ev118(void) {}												// ESLOT 118 0x463890 (empty) bounced on the floor (ev107)
	virtual void ev119(OMedia3DVector plane) { (void)plane; }				// ESLOT 119 0x464250 (empty) touched a wall of material 120..129
	virtual void ev120(void) {}												// ESLOT 120 0x463890 (empty) touched a wall of material 110..119
	virtual void ev121(char *game, char *start, char *ret_game, char *ret_start, int picture);	// ESLOT 121 0x4648a0 "PlayerLoadLevel"
	virtual void ev122(void);												// ESLOT 122 0x460380 reset the camera offsets
	virtual void ev123(float k, OMedia3DVector off, OMedia3DVector look, float dt);	// ESLOT 123 0x462de0 scripted camera: towards ev113(off), looking at v224(look)
	virtual void ev124(float dt);											// ESLOT 124 0x4629c0 camera shake (f_bf8 seconds, f_c00 strength)
	virtual void ev125(char *start);										// ESLOT 125 0x464840 f_9c8 = start point
	virtual void ev126(char *game);											// ESLOT 126 0x464870 f_c25 = game file to return to

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x460470 "StartPoint", ObjectID '3PLA', InitPhysics
	virtual void ResetObject(void);											// SLOT 10  0x4604d0
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x463c20 wall contact: start climbing a fence (material 60..69) / ladder (180)
	virtual void Process(float dt);											// SLOT 240 0x460510
	virtual void v243(float dt);											// SLOT 243 0x4618d0 control + camera (by f_a68), or the climb camera
	virtual void RotateToDest(float dt);									// SLOT 246 0x462f90 rates 4.5 / 3.7 / 4.5
	virtual void v258(void);												// SLOT 258 0x435ea0 (CLocalGameObject's)
	virtual void v260(void);												// SLOT 260 0x4637d0 + f_c20 = "C3DHERMAN"
	virtual void v261(void);												// SLOT 261 0x4637f0 stop (speed 0, velocity 0, ev116)
	virtual void v262(CGameObject *other, float dist);						// SLOT 262 0x463830 C3DGODDARD stops us instead of pushing
	virtual void v271(void);												// SLOT 271 0x461b20 power-up over: f_968 = 600

	// ---- data members (object offsets; the gaps are never used)
	float				f_960;			// 0x960  acceleration per frame (30)
	float				f_964;			// 0x964  braking per frame (50)
	float				f_968;			// 0x968  top speed (600; 1000 with power-up 0 (g_power_timers[0]), 1200 flying)
	float				f_96c;			// 0x96c  forward speed (v154 gets it every frame)
	float				f_970;			// 0x970  35
	float				f_974;			// 0x974  0
	float				f_978;			// 0x978  turn rate, degrees / s (+ left)
	float				f_97c;			// 0x97c  turn rate the movement used (side slip)
	float				f_980;			// 0x980  heading at ResetObject
	int					f_984;			// 0x984  1
	unsigned char		f_988;			// 0x988  driving forward this frame
	unsigned char		f_989;			// 0x989  braking this frame (ev110)
	unsigned char		f_98a;			// 0x98a  turning by keys (f_978 eases back to 0 after)
	unsigned char		f_98b;			// 0x98b  0
	int					f_9b4;			// 0x9b4  0
	OMedia3DPoint		f_9b8;			// 0x9b8  camera position this frame (ev109 cur)
	char				f_9c8[80];		// 0x9c8  "StartPoint" ("none")
	char				f_a18[80];		// 0xa18  start point to return to ("none")
	short				f_a68;			// 0xa68  control state (see above)
	OMedia3DPoint		f_a6c;			// 0xa6c  camera offset (0, 200, -300) (ev122)
	OMedia3DPoint		f_a7c;			// 0xa7c  camera look offset (0, 80, 0)
	short				f_a8c;			// 0xa8c  jump: 1 in the air after a jump (5: C3DJimmy), 0 landed
	float				f_a90;			// 0xa90  pitch when accelerating (ev110: -2; -10 in state 2)
	float				f_a94;			// 0xa94  pitch when braking (2; 10)
	unsigned char		f_a98;			// 0xa98  0
	unsigned char		f_a99;			// 0xa99  1; 0 once PlayerLoadLevel ran
	int					f_a9c;			// 0xa9c  0
	OMedia3DPoint		f_aa0;			// 0xaa0  (0, 0, 0)
	OMedia3DVector		f_ab0;			// 0xab0
	float				f_ac0;			// 0xac0  0
	unsigned char		f_ac4;			// 0xac4  0
	short				f_ac6;			// 0xac6  turn animation: -1 LEFT, 1 RIGHT, 0 none
	int					f_ac8;			// 0xac8  0
	float				f_acc;			// 0xacc  0
	float				f_ad0;			// 0xad0  time since the last climb started (+= dt; v220 needs > 0.5)
	OMedia3DPoint		f_ad4;			// 0xad4  where a climb ends (v224(0, 260, 70) fence, (0, 630, 75) ladder)
	float				f_ae4;			// 0xae4  mouse x, summed
	float				f_ae8;			// 0xae8  camera height / pitch adjust (10; Action + Go/Back or mouse y with gadget 6/10)
	unsigned char		f_aec;			// 0xaec  going backwards this frame (ev109)
	unsigned char		f_aed;			// 0xaed  camera frozen (no FALL, no shake / scripted moves)
	OMedia3DVector		f_af0;			// 0xaf0  (1.4, 1, 1.4) (ev122); its w (0xafc) is the camera axis w
	unsigned char		f_b00;			// 0xb00  Back does not reverse
	float				f_b04;			// 0xb04  climb animation time (1.55 fence, 2 ladder)
	int					f_b08;			// 0xb08  0
	float				f_b0c;			// 0xb0c  side slip of ev108 (-250 left, 250 right, 0)
	float				f_b10;			// 0xb10  0 (jetpack start)
	float				f_b14;			// 0xb14  LEFT turn animation time left
	float				f_b18;			// 0xb18  RIGHT turn animation time left
	float				f_b1c;			// 0xb1c  8
	float				f_b20;			// 0xb20  climb camera time left (1.5 fence, 2.2 ladder)
	char				f_b24[100];		// 0xb24  start point in the next game ("none")
	unsigned char		f_b88;			// 0xb88  0
	char				f_b89[80];		// 0xb89  game file of the next level ("level1b.gam")
	unsigned char		f_bd9;			// 0xbd9  keep f_bdc (PlayerLoadLevel does not store the position)
	OMedia3DPoint		f_bdc;			// 0xbdc  position when PlayerLoadLevel ran
	unsigned char		f_bec;			// 0xbec  0
	float				f_bf0;			// 0xbf0  Right held (s)
	float				f_bf4;			// 0xbf4  Left held (s)
	float				f_bf8;			// 0xbf8  camera shake time left
	float				f_bfc;			// 0xbfc  time to the next shake step (0.1)
	float				f_c00;			// 0xc00  camera shake strength (1)
	float				f_c04;			// 0xc04  dazed: no jumps / runs, mouse x reversed, longer camera reach
	float				f_c08;			// 0xc08  mouse turns slowed to a third (> 0)
	unsigned char		f_c0c;			// 0xc0c  0
	float				f_c10;			// 0xc10  climb camera rise speed (120; 150 fence, 350 ladder)
	float				f_c14;			// 0xc14  mouse y / dt (ev110 throttle)
	int					f_c18;			// 0xc18  PlayerLoadLevel: 0 plain, 1 check promotion, 2 back from it
	int					f_c1c;			// 0xc1c  camera mode 0..3 (2, 3: 100 higher; 0: reach limit; 1: wider axis)
	CGameObject			*f_c20;			// 0xc20  the "C3DHERMAN" object (raises the camera within 900)
	unsigned char		f_c24;			// 0xc24  on the floor (v133) this frame
	char				f_c25[100];		// 0xc25  game file to return to ("level1b.gam")
	OMedia3DPoint		f_c8c;			// 0xc8c  climb camera target (rises at f_c10)
	unsigned char		f_c9c;			// 0xc9c  other control set (mouse turns half, no mouse go, gadget 6/10 Action stops)
	int					f_ca0;			// 0xca0  0
	float				f_ca4;			// 0xca4  > 0: no braking when nothing is pressed
	unsigned char		f_ca8;			// 0xca8  first-person camera this frame (too close to a wall)
};

namespace neg {
// Input tests (0 or the joystick, then the key list: 0x1f5 left mouse, 0x1f4 right mouse, else
// a key of g_keys; a key also clears the mouse movement 0x56aebe/0x56aec0).
bool PlayerKeyAction(void);					// 0x465750 joystick button 0, g_key_action
bool PlayerKeyTurnAround(void);				// 0x4657c0 joystick button 8, g_key_turnaround
bool PlayerKeySpeed(void);					// 0x465830 joystick button 1, g_key_speed
bool PlayerKeyJump(void);					// 0x4658a0 joystick button 1, g_key_jump
bool PlayerKeyGo(void);						// 0x465910 joystick y > 0.5, g_key_go
bool PlayerKeyBack(void);					// 0x465980 joystick y < -0.5, g_key_back
bool PlayerKeyLeft(void);					// 0x4659f0 joystick x < -0.5, g_key_turn_left
bool PlayerKeyRight(void);					// 0x465a60 joystick x > 0.5, g_key_turn_right
bool PlayerKeyCutsceneBypass(void);			// 0x465ad0 joystick button 0, g_key_cutscene_bypass

// C3DJimmy hooks (weak here, see above). `jimmy` is g_jimmy (or the object the caller found).
void JimmyJetpackOff(CGameObject *jimmy);					// 0x443370 on C3DJimmy +0xccc: flames (+0x9f0 -> +0x928) 0, stop the sound (+0x9f4)
void JimmyJetpackSound(CGameObject *jimmy, int a);		// 0x4433f0 on C3DJimmy +0xccc: loop "jetpack" unless playing (a unused)
void JimmyJetpackFlame(CGameObject *jimmy, float size);	// C3DJimmy +0xccc -> +0x9f0 (if set) -> +0x928 = size
void JimmyActivateMenuItem(CGameObject *jimmy, CGameObject *o, CGameObject *who, int item, float value);	// C3DJimmy ESLOT 138 0x449df0 "C3DJimmy::ActivateMenuItem()"
}
