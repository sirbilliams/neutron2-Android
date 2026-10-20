/*  CDefaultGame - the game type that LoadNewLevel creates for every level id (0x480230); the main
    menu runs on it. It adds nothing to CGameType but resetting the "static objects created" flag,
    so the next CreateStaticObjects() spawns Jimmy ('3JIM') again.
    Reconstructed from Neutron2.exe: constructor inline in 0x480230 (0x55c bytes), destructor
    0x4801f0, vtables 0x50679c (element) / 0x506788 / 0x50630c (game object) / 0x5062fc.

    The level's real game type (CJimmyGame, CLevelXXGame) is not made here: CEditor's LoadGame reads
    the level .gam file, whose first object id is the game type, creates it through the object
    factory and sets g_gametype to it.                                                         */
#pragma once
#include "CGameType.h"
#include <list>

class CWayPoint;

class CDefaultGame : public CGameType
{
	public:
	CDefaultGame();															// inline in 0x480230
	virtual ~CDefaultGame();												// 0x4801f0 (deleting: 0x480370 / 0x480380 / 0x4801c0)
};

namespace neg {
extern bool					 g_static_objects_created;	// 0x54a3d8  CreateStaticObjects ran for this level
extern std::list<CWayPoint *> g_waypoints;				// 0x6f75dc  every CWayPoint (its constructor 0x4b99d0 appends)

CGameType *CreateGameType(unsigned int level);			// 0x480230  (level is not used)
void CreateStaticObjects(void);							// 0x480320  "CreateStaticObjects, spawning 3JIM"
void ClearWayPoints(void);								// 0x4b9b60
}
