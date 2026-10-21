/*  C3DJetPack - see C3DJetPack.h. */
#include "C3DJetPack.h"
#include "C3DJetpackFire.h"
#include "C3DJimmy.h"
#include "InterfaceElement.h"				// g_flag_545e54
#include "Game.h"
#include "Sound.h"
#include "gen/globals.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include <string>

using namespace neg;

C3DJetPack::C3DJetPack()									// 0x442f10
{
	f_9f4 = -1;
	f_9f0 = new C3DJetpackFire;								// 0x938 bytes
	// (debug log of the new object: its CGameObject part, 0x938, "C3DJetpackFire")
	if (f_9f0)
	{
		f_9f0->ShowAgain();									// element +0xd8
		f_9f0->f_924 = g_jimmy;								// (NULL while Jimmy is being built)
	}
}

C3DJetPack::~C3DJetPack()									// 0x443090
{
	Log("~C3DFlyingObject()");								// (inlined base destructor's log)
}

// --------------------------------------------------------------------------- element slots

void C3DJetPack::ShowAgain(void)							// 0x4430e0
{
	f_9f0->ShowAgain();										// +0xd8
	C3DObject::ShowAgain();									// 0x49b100
}

void C3DJetPack::HideAgain(void)							// 0x443100
{
	f_9f0->HideAgain();										// +0xdc
	C3DObject::HideAgain();									// 0x49b120
}

// --------------------------------------------------------------------------- CGameObject slots

void C3DJetPack::Process(float dt)							// 0x443120
{
	(void)dt;
	if (g_menu && g_flag_545e54) UpdateFire();
}

void C3DJetPack::v260(void)									// 0x443310
{
	if (f_9f0)
	{
		f_9f0->link(g_world);								// +0x2c
		f_9f0->link_layer(g_layer_world);					// +0x34
		f_9f0->InitPhysics();								// CGameObject +0x2c
	}
	f_9f0->ev58((char *)"STOP", 1);							// +0xe8 (unchecked)
}

void C3DJetPack::v277(void)									// 0x443430
{
	if (f_9f4 != -1) StopGameSound(f_9f4, false);			// 0x48f0d0
}

void C3DJetPack::v278(void)									// 0x443450
{
	if (f_9f4 != -1) f_9f4 = PlayGameSound(-1, std::string("jetpack"), true);	// 0x48f420
}

// --------------------------------------------------------------------------- methods

void C3DJetPack::UpdateFire(void)							// 0x443140
{
	C3DJetpackFire *fire = f_9f0;
	if (!fire) return;
	C3DJimmy *j = Jimmy();									// (g_545fd0 - 0xc0)
	if (f_9f8 != 1 && j->f_a68 == 1) fire->ev58((char *)"OPEN", 0);	// +0xe8
	else if (f_9f8 == 1 && j->f_a68 != 1) fire->ev58((char *)"CLOSE", 0);
	if (j->f_a68 == 1)
	{
		float p[4], r[4];
		j->v195(p);											// +0x30c (three times: z, y, x)
		f_9f0->v197(p[0], p[1], p[2]);						// +0x314
		j->v201(r);											// 0x453130 (g_jimmy's element), +0x324
		f_9f0->v202(r[0], r[1], r[2], r[3]);				// +0x328
		f_9f0->ShowAgain();									// +0xd8
	}
	else f_9f0->HideAgain();								// +0xdc
	f_9f8 = j->f_a68;
}

void C3DJetPack::Off(void)									// 0x443370
{
	C3DJimmy *j = Jimmy();
	if (j->f_ccc->f_9f0) j->f_ccc->f_9f0->f_928 = 0;		// g_jimmy's jet pack
	f_9f0->f_928 = 0;
	if (f_9f4 != -1)
	{
		StopGameSound(f_9f4, false);						// 0x48f0d0
		f_9f4 = -1;
	}
}

void C3DJetPack::Sound(void)								// 0x4433f0
{
	if (f_9f4 == -1) f_9f4 = PlayGameSound(-1, std::string("jetpack"), true);	// 0x48f420
}
