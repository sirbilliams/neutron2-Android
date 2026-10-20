/*  CLevel01DGame ('LV1D') - the game type of level1d.gam, Retroville by night: the first level a
    new game loads. Sky "skylevel1d", terrain level1d.omt unlit, black fog from 120000.
    Reconstructed from Neutron2.exe: constructor 0x487b70, destructor 0x487c90 (deleting
    thunks 0x487f40 / 0x487c60 / 0x487f50), vtables 0x509a84 (element) / 0x509a70 / 0x5095f4
    (game object) / 0x5095e4. 0x68c bytes: CJimmyGame's layout, no members of its own.

    The constructor of every level game type does the same: name, InitObject (which runs
    InitGame through the vtable), the level's 4cc and the pickup index base (NextPickupIndex).
    It sets the base after InitGame, so its value wins over CJimmyGame::InitGame's 100.      */
#pragma once
#include "../CJimmyGame.h"

class CLevel01DGame : public CJimmyGame
{
	public:
	CLevel01DGame();														// 0x487b70
	virtual ~CLevel01DGame();												// 0x487c90

	virtual void UnInitObject(void);										// SLOT 8   0x487d20
	virtual void InitGame(void);											// ESLOT 45 0x487d50
};
