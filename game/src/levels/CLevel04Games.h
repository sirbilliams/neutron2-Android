/*  The game types of the level 4 maps:
        CLevel04Game   'LEV4'  level4.gam   sky by the story stage (task "SCENE"), level4.omt lit,
                                            fog colour without fog, far plane 30000
        CLevel04AGame  'LV4A'  level4a.gam  night sky, level4a.omt unlit, green fog
        CLevel04BGame  'LV4B'  level4b.gam  level4b.omt unlit, orange fog; the game type has the
                                            task name "POWER" (0x694 bytes: two members of its own)
        CLevel04CGame  'LV4C'  level4c.gam  sky by stage, level4c.omt unlit, blue fog colour, no fog
        CLevel04DGame  'LV4D'  level4d.gam  night sky, level4d.omt unlit, green fog colour, no fog
    The others are 0x68c bytes (CJimmyGame's layout). See CLevel01DGame.h for what their
    constructors share. Reconstructed from Neutron2.exe 0x48a840..0x48bd50.

    LEV4, LV4C and LV4D set the layers inline instead of calling SetFog; they write the world
    layer through both of its globals (0x56afc8 and 0x56afd4, g_layer_world here).          */
#pragma once
#include "../CJimmyGame.h"

class CLevel04Game : public CJimmyGame										// vtables 0x5106e4 / 0x5106d0 / 0x510254 / 0x510244
{
	public:
	CLevel04Game();															// 0x48b8b0
	virtual ~CLevel04Game();												// 0x48b9d0
	virtual void InitGame(void);											// ESLOT 45 0x48ba60
};

class CLevel04AGame : public CJimmyGame										// vtables 0x50ed64 / 0x50ed50 / 0x50e8d4 / 0x50e8c4
{
	public:
	CLevel04AGame();														// 0x48a840
	virtual ~CLevel04AGame();												// 0x48a960
	virtual void InitGame(void);											// ESLOT 45 0x48a9f0
};

class CLevel04BGame : public CJimmyGame										// vtables 0x50f3c4 / 0x50f3b0 / 0x50ef34 / 0x50ef24
{
	public:
	CLevel04BGame();														// 0x48ac00
	virtual ~CLevel04BGame();												// 0x48ad50
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void Process(float dt);											// SLOT 240 0x487160
	virtual void v260(void);												// SLOT 260 0x48af40 after a level load
	virtual void v267(int a2);												// SLOT 267 0x48af30 its task ("POWER") changed state
	virtual void InitGame(void);											// ESLOT 45 0x48ade0

	int				f_684;				// 0x684  0 (constructor; not used by the class)
	unsigned char	f_688;				// 0x688  0 (constructor; not used by the class)
};

class CLevel04CGame : public CJimmyGame										// vtables 0x50fa24 / 0x50fa10 / 0x50f594 / 0x50f584
{
	public:
	CLevel04CGame();														// 0x48afe0
	virtual ~CLevel04CGame();												// 0x48b100
	virtual void InitGame(void);											// ESLOT 45 0x48b190
};

class CLevel04DGame : public CJimmyGame										// vtables 0x510084 / 0x510070 / 0x50fbf4 / 0x50fbe4
{
	public:
	CLevel04DGame();														// 0x48b490
	virtual ~CLevel04DGame();												// 0x48b5b0
	virtual void InitGame(void);											// ESLOT 45 0x48b640
};
