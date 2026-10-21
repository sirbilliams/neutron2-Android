/*  C3DGrannyObj ('3GRO') - a Granny-animated prop: a pickup that plays an animation or is taken
    when touched, or an object that attaches itself to a bone of another Granny object (the guns
    in Jimmy's hand: AttachTo "JIM1", GrannyYouBone "Bip01 R Hand", BASEAnimation
    "newgunbase.grn").
    Reconstructed from Neutron2.exe: constructor 0x43b5f0 (factory '3GRO', 0xdc0 bytes),
    destructor 0x43b9d0 (deleting thunks 0x43b9a0 / 0x43cd60 / 0x43cd70 / 0x43cd80).

    Original layout: as C3DAnimated (element vtable 0x4dfc30, 99 slots: C3DAnimated's 94 +
    94..98; CLocalGameObject vtable 0x4df764 at +0xc0), own members 0x924..0xdb8, vtordisp 0xdb8,
    streamer 0xdbc.

    Life: the constructor sets everything up (InitObject + InitPhysics run there already).
    v260 (properties loaded) -> ev69 loads the model and the BASE/STOP/ANIM1..4/ANIMMyTouch
    animations once and plays "STOP". If AttachTo names an object, Process waits DelayAttach
    seconds, then attaches (ev97: the object's ev88 puts our sequence on its GrannyYouBone).
    Touching it (AddTouching): AnimateWhenTouched 0 -> taken (Points, SoundName, PIC_NUMBER menu
    item); otherwise, if the toucher is the AttachTo object, it turns towards it and ev96 plays
    ANIMMyTouch on us and ANIMYouTouch on it.                                                */
#pragma once
#include "C3DAnimated.h"

class OMediaStreamOperators;

class C3DGrannyObj : public C3DAnimated
{
	public:
	C3DGrannyObj();															// 0x43b5f0
	virtual ~C3DGrannyObj();												// 0x43b9d0

	// ---- element-family slots
	virtual void ev67(void) {}												// ESLOT 67 0x456d10 (empty)
	virtual void ev69(void);												// ESLOT 69 0x43c750 load the model and animations (once)
	virtual void ev94(void);												// ESLOT 94 0x43c950 "GETWATERGUN": Jimmy gets the water gun
	virtual void ev95(unsigned char attached);								// ESLOT 95 0x43ba30 attached flag (shown, scale 1) / hidden
	virtual void ev96(CGameObject *other);									// ESLOT 96 0x43c620 touched by the AttachTo object: play the touch animations
	virtual void ev97(CGameObject *target);									// ESLOT 97 0x43be90 attach to target's GrannyYouBone
	virtual void ev98(CGameObject *target, unsigned char a);				// ESLOT 98 0x43bf90 detach from target (v136(a))

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x43ba70 properties
	virtual void InitPhysics(void);											// SLOT 11  0x43ba20 (unless f_4aa)
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x43c430 taken / touched
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x43d610 collides with the camera only
	virtual void Process(float dt);											// SLOT 240 0x43c040
	virtual void v241(unsigned int a);										// SLOT 241 0x43bda0 follow the object we are attached to
	virtual void v260(void);												// SLOT 260 0x43bcd0 properties loaded
	virtual void v264(void);												// SLOT 264 0x43c310 drop the attachment (level restart)
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x43cbd0
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x43c9f0

	// ---- data members (object offsets)
	char				f_924[100];		// 0x924  "BASEAnimation" (model file, "newgunbase.grn")
	char				f_988[100];		// 0x988  "STOPAnimation"  (file of the animation "STOP", "none")
	char				f_9ec[100];		// 0x9ec  "ANIM1Animation" ("ANIM1")
	char				f_a50[100];		// 0xa50  "ANIM2Animation" ("ANIM2")
	char				f_ab4[100];		// 0xab4  "ANIM3Animation" ("ANIM3")
	char				f_b18[100];		// 0xb18  "ANIM4Animation" ("ANIM4")
	char				f_b7c[100];		// 0xb7c  "ANIMMyTouch" (file of our "ANIMMyTouch" animation)
	char				f_be0[100];		// 0xbe0  "ANIMYouTouch" (animation the toucher plays, "GET")
	char				f_c44[100];		// 0xc44  "AttachTo" (object tag, "JIM1")
	char				f_ca8[100];		// 0xca8  "GrannyYouBone" ("Bip01 R Hand")
	char				f_d0c[100];		// 0xd0c  "SoundName" ("none")
	CGameObject			*f_d70;			// 0xd70  the object we are attached to
	int					f_d74;			// 0xd74  value given with PIC_NUMBER to the menu (0)
	int					f_d78;			// 0xd78  "PIC_NUMBER" (menu page 2 item, -1)
	float				f_d7c;			// 0xd7c  attach delay left (-1: none)
	float				f_d80;			// 0xd80  "DelayAttach" (0.1)
	float				f_d84;			// 0xd84  "ResetToStop" (play STOP after that many seconds)
	unsigned char		f_d88;			// 0xd88  attached
	int					f_d8c;			// 0xd8c  game type id when attached (g_gametype +0x490)
	CGameObject			*f_d90;			// 0xd90  arrow sprite (a C3DSprite's game object; set elsewhere)
	unsigned char		f_d94;			// 0xd94  not taken yet (saved)
	int					f_d98;			// 0xd98  "GlowAndScale" (1)
	int					f_d9c;			// 0xd9c  "Points" (50)
	float				f_da0;			// 0xda0  "SlideSpeedX" (texture scroll, 0)
	float				f_da4;			// 0xda4  "DefaultScale" (2.0)
	float				f_da8;			// 0xda8  "DetachTime" (-1)
	float				f_dac;			// 0xdac  detach time left (-1)
	int					f_db0;			// 0xdb0  "AnimateWhenTouched" (1; 0: a pickup)
	int					f_db4;			// 0xdb4  "ShowArrow" (0)
};
