/*  The game types of the Retroville levels other than level1d (CLevel01DGame.h):
        CLevel01Game   'LEV1'  level1.gam    winter sky, level1.omt unlit, grey fog
        CLevel01AGame  'LV1A'  level1a.gam   no sky, level1a.omt lit, blue-grey fog
        CLevel01BGame  'LV1B'  level1b.gam   no sky, level1b.omt unlit, black fog
        CLevel01CGame  'LV1C'  level1c.gam   as level1b with level1c.omt
        CLevel01EGame  'LV1E'  level1e.gam   level3.omt (sic) unlit, no sky and no fog set
        CLevel01FGame  'LV1F'  level1f.gam   space sky, level1f.omt lit, transparent black fog
        CLevel01_AGame 'L1_A'  level1_a.gam  winter sky (still), level1_A.omt tinted, fog off
        CLevel01_BGame 'L1_B'  level1_b.gam  dusk sky, level1_B.omt tinted, brown fog
        CLevel01_CGame 'L1_C'  level1_c.gam  night sky (still), level1_C.omt tinted blue, grey fog
    Each overrides InitGame (ESLOT 45) only, except as noted. All are 0x68c bytes (CJimmyGame's
    layout) except CLevel01Game (0x690: one int of its own). See CLevel01DGame.h for what their
    constructors share. Reconstructed from Neutron2.exe 0x486580..0x488940.                  */
#pragma once
#include "../CJimmyGame.h"

class CLevel01Game : public CJimmyGame										// vtables 0x50ada4 / 0x50ad90 / 0x50a914 / 0x50a904
{
	public:
	CLevel01Game();															// 0x488560
	virtual ~CLevel01Game();												// 0x488690
	virtual void Process(float dt);											// SLOT 240 0x487160
	virtual void InitGame(void);											// ESLOT 45 0x488720

	int			f_684;					// 0x684  0 (constructor; not used by the class)
};

class CLevel01AGame : public CJimmyGame										// vtables 0x508764 / 0x508750 / 0x5082d4 / 0x5082c4
{
	public:
	CLevel01AGame();														// 0x4871b0
	virtual ~CLevel01AGame();												// 0x4872d0
	virtual void InitGame(void);											// ESLOT 45 0x487360
};

class CLevel01BGame : public CJimmyGame										// vtables 0x508dc4 / 0x508db0 / 0x508934 / 0x508924
{
	public:
	CLevel01BGame();														// 0x4874f0
	virtual ~CLevel01BGame();												// 0x487610
	virtual void InitGame(void);											// ESLOT 45 0x4876a0
};

class CLevel01CGame : public CJimmyGame										// vtables 0x509424 / 0x509410 / 0x508f94 / 0x508f84
{
	public:
	CLevel01CGame();														// 0x487830
	virtual ~CLevel01CGame();												// 0x487950
	virtual void InitGame(void);											// ESLOT 45 0x4879e0
};

class CLevel01EGame : public CJimmyGame										// vtables 0x50a0e4 / 0x50a0d0 / 0x509c54 / 0x509c44
{
	public:
	CLevel01EGame();														// 0x487f60
	virtual ~CLevel01EGame();												// 0x488080
	virtual void InitGame(void);											// ESLOT 45 0x4880c0
};

class CLevel01FGame : public CJimmyGame										// vtables 0x50a744 / 0x50a730 / 0x50a2b4 / 0x50a2a4
{
	public:
	CLevel01FGame();														// 0x4881f0
	virtual ~CLevel01FGame();												// 0x488310
	virtual void InitGame(void);											// ESLOT 45 0x488350
};

class CLevel01_AGame : public CJimmyGame									// vtables 0x507444 / 0x507430 / 0x506fb4 / 0x506fa4
{
	public:
	CLevel01_AGame();														// 0x486580
	virtual ~CLevel01_AGame();												// 0x4866a0
	virtual void Process(float dt);											// SLOT 240 0x487160
	virtual void InitGame(void);											// ESLOT 45 0x486730
};

class CLevel01_BGame : public CJimmyGame									// vtables 0x507aa4 / 0x507a90 / 0x507614 / 0x507604
{
	public:
	CLevel01_BGame();														// 0x486990
	virtual ~CLevel01_BGame();												// 0x486ab0
	virtual void Process(float dt);											// SLOT 240 0x487160
	virtual void InitGame(void);											// ESLOT 45 0x486b40
};

class CLevel01_CGame : public CJimmyGame									// vtables 0x508104 / 0x5080f0 / 0x507c74 / 0x507c64
{
	public:
	CLevel01_CGame();														// 0x486d90
	virtual ~CLevel01_CGame();												// 0x486eb0
	virtual void Process(float dt);											// SLOT 240 0x487160
	virtual void InitGame(void);											// ESLOT 45 0x486f40
};
