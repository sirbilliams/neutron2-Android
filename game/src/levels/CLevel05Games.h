/*  The game types of the level 5 maps:
        CLevel05Game   'LEV5'  level5.gam    level5.omt unlit, brown fog
        CLevel05AGame  'LV5A'  level5a.gam   level5a.omt unlit, sand fog
        CLevel05BGame  'LV5B'  level5b.gam   level5b.omt unlit, black fog
        CLevel05B1Game 'L5B1'  level5b1.gam  level5b1.omt unlit, black fog
        CLevel05CGame  'LV5C'  level5c.gam   downtown sky, level5c.omt unlit, light blue fog
        CLevel05FGame  'LV5F'  level5f.gam   the same InitGame as LV5C (one function in the exe)
    0x68c bytes each (CJimmyGame's layout). See CLevel01DGame.h for what their constructors
    share. Reconstructed from Neutron2.exe 0x48bd60..0x48ce80.                           */
#pragma once
#include "../CJimmyGame.h"

class CLevel05Game : public CJimmyGame										// vtables 0x512d24 / 0x512d10 / 0x512894 / 0x512884
{
	public:
	CLevel05Game();															// 0x48cb50
	virtual ~CLevel05Game();												// 0x48cc70
	virtual void InitGame(void);											// ESLOT 45 0x48cd00
};

class CLevel05AGame : public CJimmyGame										// vtables 0x510d44 / 0x510d30 / 0x5108b4 / 0x5108a4
{
	public:
	CLevel05AGame();														// 0x48bd60
	virtual ~CLevel05AGame();												// 0x48be80
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x48bf10
};

class CLevel05BGame : public CJimmyGame										// vtables 0x511a04 / 0x5119f0 / 0x511574 / 0x511564
{
	public:
	CLevel05BGame();														// 0x48c390
	virtual ~CLevel05BGame();												// 0x48c4b0
	virtual void InitGame(void);											// ESLOT 45 0x48c4f0
};

class CLevel05B1Game : public CJimmyGame									// vtables 0x5113a4 / 0x511390 / 0x510f14 / 0x510f04
{
	public:
	CLevel05B1Game();														// 0x48c0a0
	virtual ~CLevel05B1Game();												// 0x48c1c0
	virtual void InitGame(void);											// ESLOT 45 0x48c200
};

class CLevel05CGame : public CJimmyGame										// vtables 0x512064 / 0x512050 / 0x511bd4 / 0x511bc4
{
	public:
	CLevel05CGame();														// 0x48c680
	virtual ~CLevel05CGame();												// 0x48c7a0
	virtual void InitGame(void);											// ESLOT 45 0x48c7e0
};

class CLevel05FGame : public CJimmyGame										// vtables 0x5126c4 / 0x5126b0 / 0x512234 / 0x512224
{
	public:
	CLevel05FGame();														// 0x48c9b0
	virtual ~CLevel05FGame();												// 0x48cad0
	virtual void InitGame(void);											// ESLOT 45 0x48c7e0 (CLevel05CGame's code)
};
