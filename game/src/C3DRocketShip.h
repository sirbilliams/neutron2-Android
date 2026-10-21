/*  C3DRocketShip ('3ROC') - Jimmy's rocket ship (the Strato XL): the C3DFlyingObject Jimmy flies
    with Goddard on board ("C3DROCKETSHIP"). It trails smoke when damaged, opens a parachute
    (Brake key), burns energy with the afterburner (Speed key) and shoots (Action key, through
    Jimmy) at Carl in the level 'LV5C'.
    Reconstructed from Neutron2.exe: constructor 0x46a290 (factory '3ROC', 0x1284 bytes),
    destructor 0x46a7a0 (deleting thunks 0x46a770 / 0x46cd40 / 0x46cd50 / 0x46cd60).

    Original layout (classes.json): as C3DFlyingObject (element vtable 0x4f5704, 112 slots:
    C3DFlyingObject's 110 + 110, 111; CLocalGameObject vtable 0x4f5238 at +0xc0), own members
    0x9f0..0x127c, vtordisp 0x127c, streamer 0x1280.

    What it owns (the constructor creates them, the destructor leaves them in the object list):
      f_9f8[20]  C3DPermanentSmoke puffs, laid behind the ship one per frame while damaged (f_1248)
      f_1230     the C3DFireStrato flame (its CGameObject), DEFAULT / AFTERBURN
      f_1270     the parachute, a C3DAIGranny (its CGameObject) with the chute*.grn animations
    The rider f_9f4 is Jimmy's element (tag "JIM1"; C3DJimmy's rocket gadget sets and clears it,
    ReadState finds it again), f_11f4 Goddard's ("C3DGODDARD"): both are moved with the ship by
    Process. Without a rider the ship only flies through C3DFlyingObject.

    Hits (v220 walls by material, AddTouching enemies): Jimmy is hurt (his ESLOT 134), f_11f8 starts
    f_1238 seconds of flashing red (f_11fc counts up) with a push down, f_1248 makes it smoke and
    the engine sputter. A wall crash may break a wing off ("LOSTWING", Goddard "FIX").

    Hooks into classes written elsewhere: the rocket reads and calls a few things of C3DJimmy,
    C3DGoddard, C3DAIOmtObj, C3DDino / C3DPtero and C3DMine. They are declared below and defined
    weak (doing nothing but a TODO log, neutral results) in C3DRocketShip.cpp; the classes' own
    files define the real ones. The parachute is made through the factory hook New_C3DAIGranny
    and its animation files are set through its registered properties (see the constructor).
    Jimmy's code uses f_9f4 and f_1230 directly (0x449df0: "Deactivating Rocket").

    Element-family slots are `ev<n>` (`// ESLOT n`); members are f_<offset in the original
    object> (CGameObject-family code sees them at offset - 0xc0).                            */
#pragma once
#include "C3DFlyingObject.h"

class C3DPermanentSmoke;
class OMediaStreamOperators;
class OMedia3DPolygon;

class C3DRocketShip : public C3DFlyingObject
{
	public:
	C3DRocketShip();														// 0x46a290
	virtual ~C3DRocketShip();												// 0x46a7a0

	// ---- element-family slots
	virtual void ev108(int state);											// ESLOT 108 0x46c860 engine sound: 0 off, 1 "rocket fly", 2 "rocket blast" (sputters while damaged)
	virtual void ev110(void);												// ESLOT 110 0x46c140 (only counts f_1228; its two vectors are unused)
	virtual void ev111(float dt);											// ESLOT 111 0x46b7e0 Goddard's marker 300 in front of the camera, towards Carl

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x46aaf0 "Strato*.ase" animations, "strato.png"
	virtual void ResetObject(void);											// SLOT 10  0x46c840 (speed kept with a rider)
	virtual void InitPhysics(void);											// SLOT 11  0x43ba20 (= C3DGrannyObj's: unless f_4aa)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x46abc0 crashes into enemies
	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x4534f0 ceiling: vertical speed -120 (= C3DMiniSub's)
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x46c1b0 wall: hits by material
	virtual void Process(float dt);											// SLOT 240 0x46b140 riders, smoke, flame, hit flash
	virtual void v243(float dt);											// SLOT 243 0x46b960 afterburner, parachute, shooting
	virtual void v260(void);												// SLOT 260 0x46a840 level start: riders, MaxHeight by level
	virtual void v266(void);												// SLOT 266 0x46c830 GetTaskState("LIBBY") (result unused)
	virtual void v267(int level);											// SLOT 267 0x46ca70 + with a rider: shown, engine can't die
	virtual void v275(void);												// SLOT 275 0x46ca40 passed on to the rider
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x46cc40 position, rotation
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x46cac0 + finds the riders again (if it is the camera)

	// ---- data members (object offsets; 0xa48..0x11ef and 0x1200..0x1227 are never used)
	int					f_9f0;			// 0x9f0  0
	C3DAnimated			*f_9f4;			// 0x9f4  the rider: Jimmy's element (NULL: nobody on board)
	C3DPermanentSmoke	*f_9f8[20];		// 0x9f8  the smoke puffs (elements)
	short				f_11f0;			// 0x11f0 next smoke puff (0..19)
	C3DAnimated			*f_11f4;		// 0x11f4 Goddard's element (on board with Jimmy)
	unsigned char		f_11f8;			// 0x11f8 hit: flashing for f_1238 s, pushed down
	float				f_11fc;			// 0x11fc time since the hit
	int					f_1228;			// 0x1228 ev110 counter (0..9)
	float				f_122c;			// 0x122c time since the last shot
	CGameObject			*f_1230;		// 0x1230 the flame (a C3DFireStrato's CGameObject)
	unsigned char		f_1234;			// 0x1234 engine dead: top speed 10, pushed down (after 10 s in the air, unless f_1235)
	unsigned char		f_1235;			// 0x1235 the engine never dies (1)
	float				f_1238;			// 0x1238 length of the hit (s)
	int					f_123c;			// 0x123c engine sound channel (-1)
	int					f_1240;			// 0x1240 "needfood" channel (-1)
	int					f_1244;			// 0x1244 engine sound state (ev108's last)
	unsigned char		f_1248;			// 0x1248 damaged: smoke trail, red flash, sputtering engine
	CGameObject			*f_124c;		// 0x124c "C3DCARL" (level 'LV5C' only): the shots' target
	float				f_1250;			// 0x1250 Carl's marker point, in Carl's frame (0, 1300, 550)
	float				f_1254;			// 0x1254
	float				f_1258;			// 0x1258
	int					f_125c;			// 0x125c 0
	CGameObject			*f_1260;		// 0x1260 wing debris (never set: always NULL)
	CGameObject			*f_1264;		// 0x1264
	CGameObject			*f_1268;		// 0x1268
	CGameObject			*f_126c;		// 0x126c
	CGameObject			*f_1270;		// 0x1270 the parachute (a C3DAIGranny's CGameObject)
	unsigned char		f_1274;			// 0x1274 Brake held (the parachute toggles once per press)
	unsigned char		f_1275;			// 0x1275 afterburner on
	int					f_1278;			// 0x1278 0
};

namespace neg {
// Hooks (weak in C3DRocketShip.cpp). `jimmy` is the rider's CGameObject.
unsigned char JimmyHurtFlag(CGameObject *jimmy);			// C3DJimmy +0x4cb8 (byte): being hit (ESLOT 134 sets it)
void JimmySetHurtFlag(CGameObject *jimmy, unsigned char v);
void JimmyHurt(CGameObject *jimmy, float energy, int points, float time, unsigned char flag);	// C3DJimmy ESLOT 134 0x44bd90 (energy and points added)
void JimmyPain(CGameObject *jimmy);							// C3DJimmy ESLOT 130 0x44bfe0 "SPLASH" + "pain agh ahh"
void JimmyRocketAim(CGameObject *jimmy, float dt, OMedia3DVector pos);	// C3DJimmy ESLOT 146 0x446100 (with g_menu)
void JimmyRocketShoot(CGameObject *jimmy, float x, float y, float z, float side, float speed,
					  CGameObject *target, int a7, float a8);	// C3DJimmy ESLOT 148 0x44b090
int  JimmyAmmo(CGameObject *jimmy);							// C3DJimmy +0x4d94 (shots left; "needfood" at 0)
void JimmySetAmmo(CGameObject *jimmy, int n);
void JimmyGadgetShot(CGameObject *jimmy);					// C3DJimmy gadget [(short)+0xd50] of +0xcd0: +0x63c = 0, +0x640 = 1300, +0x644 = 550 (if any)
int  AIOmtObjState(CGameObject *o);							// C3DAIOmtObj element +0x1bbc (3: the train is running)
float AIAttackTime(CGameObject *o);							// C3DDino / C3DPtero element +0x1d58 (> 0: attacking)
void MineDetonate(CGameObject *o);							// 0x4521e0 on a C3DMine (sprite element +0x6ac = 2)
CGameObject *GoddardMarker(CGameObject *goddard);			// C3DGoddard element +0x1b80 (a C3DObject's CGameObject, or NULL)
}
