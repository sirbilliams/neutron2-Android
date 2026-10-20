/*  The game types of levels 6 and 7:
        CLevel06Game   'LEV6'  level6.gam   level6.omt unlit, dark blue fog from 2000 to 4000
        CLevel06AGame  'LV6A'  level6a.gam  level6a.omt unlit, the same fog
        CLevel07Game   'LEV7'  level7.gam   space sky, level7.omt unlit, black fog from 9000
    0x68c bytes each (CJimmyGame's layout). See CLevel01DGame.h for what their constructors
    share. They set the layers inline instead of calling SetFog, writing the world layer
    through both of its globals (0x56afc8 and 0x56afd4, g_layer_world here).
    Reconstructed from Neutron2.exe 0x48ce90..0x48da10.                                     */
#pragma once
#include "../CJimmyGame.h"

class CLevel06Game : public CJimmyGame										// vtables 0x5139e4 / 0x5139d0 / 0x513554 / 0x513544
{
	public:
	CLevel06Game();															// 0x48d250
	virtual ~CLevel06Game();												// 0x48d370
	virtual void InitGame(void);											// ESLOT 45 0x48d400
};

class CLevel06AGame : public CJimmyGame										// vtables 0x513384 / 0x513370 / 0x512ef4 / 0x512ee4
{
	public:
	CLevel06AGame();														// 0x48ce90
	virtual ~CLevel06AGame();												// 0x48cfb0
	virtual void InitGame(void);											// ESLOT 45 0x48d040
};

class CLevel07Game : public CJimmyGame										// vtables 0x514044 / 0x514030 / 0x513bb4 / 0x513ba4
{
	public:
	CLevel07Game();															// 0x48d610
	virtual ~CLevel07Game();												// 0x48d730
	virtual void InitGame(void);											// ESLOT 45 0x48d7c0
};
