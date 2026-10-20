/*  The game types of the level 3 maps:
        CLevel03Game   'LEV3'  level3.gam   level3.omt unlit, blue fog
        CLevel03AGame  'LV3A'  level3a.gam  level3A.omt lit, a blue sky that is hidden, light blue fog
        CLevel03BGame  'LV3B'  level3b.gam  level3B.omt lit, blue fog
        CLevel03CGame  'LV3C'  level3c.gam  level3c.omt unlit, sky "skylevel5", grey fog
        CLevel03DGame  'LV3D'  level3d.gam  level3D.omt unlit, grey fog
        CLevel03EGame  'LV3E'  level3e.gam  level3.omt (sic) unlit, blue fog as level 3
    0x68c bytes each (CJimmyGame's layout). See CLevel01DGame.h for what their constructors
    share. Reconstructed from Neutron2.exe 0x489560..0x48a830.                            */
#pragma once
#include "../CJimmyGame.h"

class CLevel03Game : public CJimmyGame										// vtables 0x50e704 / 0x50e6f0 / 0x50e274 / 0x50e264
{
	public:
	CLevel03Game();															// 0x48a500
	virtual ~CLevel03Game();												// 0x48a620
	virtual void InitGame(void);											// ESLOT 45 0x48a6b0
};

class CLevel03AGame : public CJimmyGame										// vtables 0x50c724 / 0x50c710 / 0x50c294 / 0x50c284
{
	public:
	CLevel03AGame();														// 0x489560
	virtual ~CLevel03AGame();												// 0x489680
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x4896c0
};

class CLevel03BGame : public CJimmyGame										// vtables 0x50cd84 / 0x50cd70 / 0x50c8f4 / 0x50c8e4
{
	public:
	CLevel03BGame();														// 0x4898e0
	virtual ~CLevel03BGame();												// 0x489a00
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x489a40
};

class CLevel03CGame : public CJimmyGame										// vtables 0x50d3e4 / 0x50d3d0 / 0x50cf54 / 0x50cf44
{
	public:
	CLevel03CGame();														// 0x489bd0
	virtual ~CLevel03CGame();												// 0x489cf0
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x489d30
};

class CLevel03DGame : public CJimmyGame										// vtables 0x50da44 / 0x50da30 / 0x50d5b4 / 0x50d5a4
{
	public:
	CLevel03DGame();														// 0x489f10
	virtual ~CLevel03DGame();												// 0x48a030
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x48a080
};

class CLevel03EGame : public CJimmyGame										// vtables 0x50e0a4 / 0x50e090 / 0x50dc14 / 0x50dc04
{
	public:
	CLevel03EGame();														// 0x48a210
	virtual ~CLevel03EGame();												// 0x48a330
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x48a370
};
