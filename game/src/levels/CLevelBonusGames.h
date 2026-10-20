/*  The game types of the bonus maps:
        CLevelPromotion 'LPRO'  promotion.gam  the promotion screen: Bonus01.omt unlit, black fog
        CLevelVR01      'VR01'  vr01.gam       virtual reality 1: sky "skyvr01", vr01.omt unlit
        CLevelVR02      'VR02'  vr02.gam       virtual reality 2: vr02.omt unlit, blue fog (inline)
    0x68c bytes each (CJimmyGame's layout). See CLevel01DGame.h for what their constructors
    share. Reconstructed from Neutron2.exe 0x48da20..0x48e350.                           */
#pragma once
#include "../CJimmyGame.h"

class CLevelPromotion : public CJimmyGame									// vtables 0x5146a4 / 0x514690 / 0x514214 / 0x514204
{
	public:
	CLevelPromotion();														// 0x48da20
	virtual ~CLevelPromotion();												// 0x48db40
	virtual void InitGame(void);											// ESLOT 45 0x48db80
};

class CLevelVR01 : public CJimmyGame										// vtables 0x514d04 / 0x514cf0 / 0x514874 / 0x514864
{
	public:
	CLevelVR01();															// 0x48dcc0
	virtual ~CLevelVR01();													// 0x48dde0
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x48de20
};

class CLevelVR02 : public CJimmyGame										// vtables 0x515364 / 0x515350 / 0x514ed4 / 0x514ec4
{
	public:
	CLevelVR02();															// 0x48dff0
	virtual ~CLevelVR02();													// 0x48e110
	virtual void UnInitObject(void);										// SLOT 8   0x48a070
	virtual void InitGame(void);											// ESLOT 45 0x48e150
};
