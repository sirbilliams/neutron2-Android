/*  The game types of the level 2 maps:
        CLevel02Game   'LEV2'  level2.gam   sky and terrain tint by the story stage (task "SCENE"):
                                            day blue sky, dusk sky, night sky (still); no fog set
        CLevel02AGame  'LV2A'  level2a.gam  no sky, level2a.omt tinted, dark grey fog and clear colour
        CLevel02BGame  'LV2B'  level2b.gam  as CLevel02Game, other tints
    0x68c bytes each (CJimmyGame's layout). See CLevel01DGame.h for what their constructors
    share. Reconstructed from Neutron2.exe 0x488950..0x489550 (and 0x48a070).            */
#pragma once
#include "../CJimmyGame.h"

class CLevel02Game : public CJimmyGame										// vtables 0x50c0c4 / 0x50c0b0 / 0x50bc34 / 0x50bc24
{
	public:
	CLevel02Game();															// 0x4890d0
	virtual ~CLevel02Game();												// 0x4891f0
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x489280
};

class CLevel02AGame : public CJimmyGame										// vtables 0x50b404 / 0x50b3f0 / 0x50af74 / 0x50af64
{
	public:
	CLevel02AGame();														// 0x488950
	virtual ~CLevel02AGame();												// 0x488a70
	virtual void InitGame(void);											// ESLOT 45 0x488ab0
};

class CLevel02BGame : public CJimmyGame										// vtables 0x50ba64 / 0x50ba50 / 0x50b5d4 / 0x50b5c4
{
	public:
	CLevel02BGame();														// 0x488c70
	virtual ~CLevel02BGame();												// 0x488d90
	virtual void InitGame(void);											// ESLOT 45 0x488dd0
};
