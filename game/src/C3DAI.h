/*  C3DAI ('3DAI') - a computer-controlled character: it patrols a chain of C3DPatrolPoints
    ("PatrolPoint", then each point's NextPatrolPoint), watches for its target ("TargetName",
    Jimmy by default) within "VisibleRange" and its field of view ("FOV"), and then attacks,
    follows, flees or searches according to its state. The base of C3DAIGranny ('3GRN'),
    C3DFish ('3FIS'), C3DAIJimmy, C3DAIOmtObj and the other AI characters.
    Reconstructed from Neutron2.exe: constructor 0x410b30 (factory '3DAI', 0x1b58 bytes),
    destructor 0x411060 (deleting thunks 0x411030 / 0x415940 / 0x415970 / 0x415980).

    Original layout (classes.json): as C3DPlayerObject (element vtable 0x4caf80, 144 slots:
    C3DPlayerObject's 106 + 106..143; OMediaAnim at +0x90, vtable 0x4caf30; listener +0x30,
    vtable 0x4caf6c; CLocalGameObject at +0xc0, vtable 0x4caab4, 286 slots), own members
    0x960..0x1b50, vtordisp 0x1b50, streamer 0x1b54. Derived classes put theirs from 0x1b50 on.

    States (f_96c, set through ev107; Process runs one case per frame):
      0  attack: chase the target (ev111 / ev108), fall back to AIState (6 scan, 11 wander) or
         2 when the target is no longer in reach, out of range -> 2/6
      1  wait (stand still)              2  patrol (ev137), seeing the target -> 0
      3  patrol only                     4  follow the target at offset f_978
      5  flee for f_c3c seconds, then FallbackAIState (or AIState)
      6  scan (stand, SIT), seeing the target -> 0
      7  wait at a patrol point for its WaitTime, then back to the previous state
      8  hidden                          9  frozen (no force)
      10 wander within WanderRange       11 wander, seeing the target -> 0
      12 run away from the target (ev131)
      13 walk the path found by FindBestPath (ev133) to a patrol point (AITrigger AIState 13)
      14 lost the target: look for it, seeing it -> flee (ev138)
      15 flee along a path, look back every 3 s       16 search: walk paths towards the
      target's nearest point every 7 s, seeing it -> 17   17 keep the target in view, losing it -> 16
      18 patrol, but only go on once touched by the target (f_1b4c)
    Path finding (states 13..17): ev136 lists the patrol points of type 1 in plain view (like
    C3DPatrolPoint::FindVisiblePoints), ev134 searches the network of their visible-point lists
    recursively (stack f_c7c, depth f_c78 < 100) for the shortest route to the goal, which ends
    up in f_112c (walked by ev109 with index f_1b2c). ev139/ev140 mark the type-1 points within
    f_1b44 (in front of a direction) as candidates (f_15dc, ev141 tests them); ev135 / ev143 pick
    the nearest / farthest of them.

    Animations: ev117 FLY (f_aec), ev118 RUN (f_ac4), ev119 SIT (f_b14), ev120 WALK (f_b3c),
    ev121 WAG (f_a9c), ev122 WALK (f_b64) - each only when not already playing and not while a
    cut scene animates us (f_c38). Subclasses rename them.

    Jimmy: C3DAI never calls into C3DJimmy. It sees him as its target (a CGameObject found by
    tag), as g_camera (Process stops while the camera is a frozen C3DJIMMY: CLocalGameObject
    f_4ab), and through CGameObject-family slots only.

    Element-family slots are `ev<n>` (`// ESLOT n`, n = vtable offset / 4) unless a log string
    names them; CGameObject-family slots keep their CGameObject names (`// SLOT n`). Members are
    f_<offset in the original object> (CGameObject-family code of the original sees them at
    offset - 0xc0).                                                                           */
#pragma once
#include "C3DPlayerObject.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"

class OMediaStreamOperators;
class OMedia3DPolygon;

class C3DAI : public C3DPlayerObject
{
	public:
	C3DAI();																// 0x410b30
	virtual ~C3DAI();														// 0x411060

	// One entry of the visible-point list (as C3DPatrolPoint::Visible).
	struct Visible { CGameObject *point; float dist; };

	// ---- element-family slots
	virtual void ev106(char *dst, char *src);								// ESLOT 106 0x411210 strcpy(dst, src)
	virtual void ev107(int state);											// ESLOT 107 0x412240 set the state (f_970 = old), its start actions
	virtual void ev108(void);												// ESLOT 108 0x412470 arrival test: f_9a0 < f_c14 -> f_998 + ev113
	virtual void ev109(void);												// ESLOT 109 0x413790 next destination (patrol point / path / PatrolPoint), then ev110
	virtual void ev110(void);												// ESLOT 110 0x413930 destination point and direction from f_b8c
	virtual void ev111(CGameObject *dest);									// ESLOT 111 0x413c50 velocity towards f_988 at f_968 (scaled by distance in state 4)
	virtual void ev112(float dt);											// ESLOT 112 0x413f50 turn towards the destination; animations on arrival
	virtual void ev113(void);												// ESLOT 113 0x414630 arrived: states 10/11 wander on (ev129 1400)
	virtual void ev114(void) {}												// ESLOT 114 0x463890 (empty) arrived, every frame
	virtual void ev115(void);												// ESLOT 115 0x414d00 destination = target + f_978 turned by its rotation
	virtual void ev116(void);												// ESLOT 116 0x414300 face the way the target faces, turned round
	virtual void ev117(void);												// ESLOT 117 0x4141e0 animation f_aec (FLY)
	virtual void ev118(void);												// ESLOT 118 0x414210 animation f_ac4 (RUN)
	virtual void ev119(void);												// ESLOT 119 0x414240 animation f_b14 (SIT)
	virtual void ev120(void);												// ESLOT 120 0x4142a0 animation f_b3c (WALK)
	virtual void ev121(void);												// ESLOT 121 0x414270 animation f_a9c (WAG)
	virtual void ev122(void);												// ESLOT 122 0x4142d0 animation f_b64 (WALK)
	virtual void ev123(CGameObject *o, float time);							// ESLOT 123 0x414f30 attack o for <time> s, then back to the saved state
	virtual void ev124(void) {}												// ESLOT 124 0x463890 (empty) ev123's time is over
	virtual void ev125(float dt);											// ESLOT 125 0x414fd0 stuck check every 5 s (skip a patrol point / back to PatrolPoint)
	virtual void ev126(void) {}												// ESLOT 126 0x463890 (empty)
	virtual void ev127(void);												// ESLOT 127 0x415440 not arrived (f_998, f_99c = 0)
	virtual void ev128(float unused);										// ESLOT 128 0x413e20 turn towards f_bb4
	virtual void ev129(float range, OMedia3DVector away);					// ESLOT 129 0x414660 wander: a random destination within range (+ away * 550)
	virtual void ev130(float unused, float height);							// ESLOT 130 0x415470 bumped the head: bounce down (sound "HitHead" when we are the camera)
	virtual unsigned char ev131(CGameObject *o);							// ESLOT 131 0x413a30 run away from o (within 1500); 0
	virtual unsigned char ev132(OMedia3DVector a, OMedia3DVector b);		// ESLOT 132 0x412670 nothing between a and b
	virtual unsigned char FindBestPath(CGameObject *goal);					// ESLOT 133 0x412e50 "FindBestPath": route to goal into f_112c
	virtual float ev134(CGameObject *from, CGameObject *to);				// ESLOT 134 0x412f50 recursive search: length of the route, -1 none
	virtual CGameObject *ev135(OMedia3DVector pos, unsigned char nearest);	// ESLOT 135 0x413640 nearest / farthest candidate point of our group
	virtual void ev136(int group);											// ESLOT 136 0x412530 our visible type-1 patrol points (f_1a8c)
	virtual void ev137(float dt);											// ESLOT 137 0x411d90 patrol: walk to f_b8c, wait there, go on to the next point
	virtual void ev138(unsigned char far);									// ESLOT 138 0x412be0 flee from the target along a path (state 15), else 5
	virtual void ev139(OMedia3DVector pos, OMedia3DVector dir, unsigned char cone);	// ESLOT 139 0x4126a0 candidates within f_1b44 of pos (in front of dir)
	virtual void ev140(OMedia3DVector pos, OMedia3DVector dir, unsigned char cone);	// ESLOT 140 0x412830 the same, dir not normalised, marks not cleared
	virtual unsigned char ev141(CGameObject *o);							// ESLOT 141 0x412990 o is a candidate (or there is no candidate list)
	virtual void ev142(unsigned char unused);								// ESLOT 142 0x4129d0 search: path to the candidate nearest the target -> 16, else 17
	virtual CGameObject *ev143(OMedia3DVector pos, unsigned char visible);	// ESLOT 143 0x4133d0 nearest type-1 point of our group (in view of pos)

	// ---- not virtual
	unsigned char CanSeeTarget(void);										// 0x414a60 target in range, in the field of view and not behind a wall

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4110e0 properties
	virtual void ResetObject(void);											// SLOT 10  0x4110b0
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x415150
	virtual void RemoveTouching(CGameObject *other);						// SLOT 17  0x415110
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x4144d0 wall contact: wanderers turn away
	virtual void Process(float dt);											// SLOT 240 0x411240
	virtual void v242(float t);												// SLOT 242 0x47e3c0 (C3DAnimated's)
	virtual void RotateToDest(float dt);									// SLOT 246 0x42d280 rates f_b9c (1, 3, 2)
	virtual void v258(void);												// SLOT 258 0x414380 back to AIState, then v260
	virtual void v260(void);												// SLOT 260 0x4143a0 properties loaded: target, state, speed
	virtual void v261(void);												// SLOT 261 0x415450 stop
	virtual void v265(float a, float b, float c, float d);					// SLOT 265 0x414540 "Hitwall": hover up / wander away
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x415760
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x4155e0

	// ---- data members (object offsets)
	float				f_960;			// 0x960  time (+= dt)
	CGameObject			*f_964;			// 0x964  target (TargetName; ev123 swaps it for a while)
	float				f_968;			// 0x968  speed (200; InitialSpeed, a patrol point's PatrolSpeed, AITrigger AISpeed)
	int					f_96c;			// 0x96c  state (see above; 1)
	int					f_970;			// 0x970  previous state (-1)
	int					f_974;			// 0x974  target's ObjectID ('3JIM')
	OMedia3DVector		f_978;			// 0x978  where to be relative to the target (state 4, ev115)
	OMedia3DVector		f_988;			// 0x988  destination - position
	unsigned char		f_998;			// 0x998  arrived (ev108)
	float				f_99c;			// 0x99c  time since arriving (ev112: SIT after 7 s, else WAG)
	float				f_9a0;			// 0x9a0  distance to the destination
	float				f_9a4;			// 0x9a4  that distance at the last stuck check (ev125)
	float				f_9a8;			// 0x9a8  "VisibleRange" (2500)
	char				f_9ac[100];		// 0x9ac  "PatrolPoint" ("none"): the first point (AITrigger AIPatrol)
	char				f_a10[100];		// 0xa10  "TargetName" ("JIM1")
	char				f_a74[40];		// 0xa74  ("none") an animation name subclasses use
	char				f_a9c[40];		// 0xa9c  "WAG" animation (ev121)
	char				f_ac4[40];		// 0xac4  "RUN" animation (ev118)
	char				f_aec[40];		// 0xaec  "FLY" animation (ev117)
	char				f_b14[40];		// 0xb14  "SIT" animation (ev119)
	char				f_b3c[40];		// 0xb3c  "WALK" animation (ev120)
	char				f_b64[40];		// 0xb64  "WALK" animation (ev122)
	CGameObject			*f_b8c;			// 0xb8c  destination: a patrol point, a path point or an object
	CGameObject			*f_b94;			// 0xb94  the patrol point we left last
	float				f_b98;			// 0xb98  "FOV" in degrees (90; > 358: all round)
	OMedia3DPoint		f_b9c;			// 0xb9c  turn rates x, y, z (1, 3, 2) (RotateToDest)
	float				f_bac;			// 0xbac  time (+= dt)
	unsigned char		f_bb0;			// 0xbb0  keep f_bb4 as it is (ev110 / ev115)
	OMedia3DPoint		f_bb4;			// 0xbb4  destination point
	OMedia3DPoint		f_bc4;			// 0xbc4  (0, 0, 0)
	unsigned char		f_bd4;			// 0xbd4  hovering: no gravity (CanHover, AITrigger AIHover)
	unsigned char		f_bd5;			// 0xbd5  0
	float				f_bd8;			// 0xbd8  wait time left at a patrol point (state 7)
	unsigned char		f_bdc;			// 0xbdc  don't turn until the speed is over 1 (ev112)
	int					f_be0;			// 0xbe0  state saved by ev123 (1)
	OMedia3DVector		f_be4;			// 0xbe4  f_978 saved by ev123
	CGameObject			*f_bf4;			// 0xbf4  target saved by ev123
	float				f_bf8;			// 0xbf8  ev123 time left
	float				f_bfc;			// 0xbfc  stuck check timer (ev125)
	float				f_c00;			// 0xc00  > 0: the target is seen even when it is hidden (counts down)
	unsigned char		f_c04;			// 0xc04  0
	int					f_c08;			// 0xc08  "AIState" (1): the state to start in / go back to
	int					f_c0c;			// 0xc0c  "FallbackAIState" (-1): after fleeing
	float				f_c10;			// 0xc10  flee distance (1500): state 5 ends beyond it
	float				f_c14;			// 0xc14  arrival distance (110; 440 for patrol points with f_c18)
	unsigned char		f_c18;			// 0xc18  move the way we face (ev111)
	float				f_c1c;			// 0xc1c  > 0: state 5 keeps going (v261); wanderers flee a seen target
	unsigned char		f_c20;			// 0xc20  wander started
	float				f_c24;			// 0xc24  wander time (new destination after 11 s)
	unsigned char		f_c28;			// 0xc28  0
	float				f_c2c;			// 0xc2c  "WanderRange" (1500)
	float				f_c30;			// 0xc30  last yaw step (RotateToDest)
	unsigned char		f_c34;			// 0xc34  sees through walls (CanSeeTarget)
	int					f_c38;			// 0xc38  animations disabled (a cut scene animates us)
	float				f_c3c;			// 0xc3c  flee time left (state 5; ev138 10 s)
	unsigned char		f_c40;			// 0xc40  state locked (ev107 ignored)
	int					f_c44;			// 0xc44  0
	unsigned char		f_c48;			// 0xc48  1
	OMedia3DPoint		f_c4c;			// 0xc4c  position held while touched (f_c5c)
	unsigned char		f_c5c;			// 0xc5c  touched by the camera or Goddard: held at f_c4c
	unsigned char		f_c5d;			// 0xc5d  is held when touched
	float				f_c60;			// 0xc60  150
	unsigned char		f_c64;			// 0xc64  bank into turns while airborne
	OMedia3DPoint		f_c68;			// 0xc68  where ev131 runs to
	int					f_c78;			// 0xc78  ev134 recursion depth
	CGameObject			*f_c7c[300];	// 0xc7c  ev134 recursion stack (the route so far)
	CGameObject			*f_112c[300];	// 0x112c the route FindBestPath found
	CGameObject			*f_15dc[300];	// 0x15dc candidate points (ev139 / ev140)
	Visible				f_1a8c[20];		// 0x1a8c our visible type-1 patrol points (ev136)
	int					f_1b2c;			// 0x1b2c next index in f_112c
	float				f_1b30;			// 0x1b30 look-around timer (states 15 / 16)
	unsigned char		f_1b34;			// 0x1b34 f_15dc is valid
	CGameObject			*f_1b38;		// 0x1b38 point the search heads for
	float				f_1b3c;			// 0x1b3c "InitialSpeed" (200)
	int					f_1b40;			// 0x1b40 "CanHover" (0)
	float				f_1b44;			// 0x1b44 candidate radius (3500; ev138 4000 / 9000)
	int					f_1b48;			// 0x1b48 "GroupID" (0)
	unsigned char		f_1b4c;			// 0x1b4c touched by the target (state 18 goes on)
};
