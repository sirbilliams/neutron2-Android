/*  C3DWaterBalloon ('3WBL') - a shot of Jimmy's gadgets (the water balloon, and the other guns'
    shots, which C3DJimmy ev152 restyles): sprite 20 of "retainedsprites.omt" with physics that
    spins, may wait f_65c seconds before it appears, grows and fades (f_658 > 0), homes on a
    target (f_634, at the offset f_63c.., by skill level within 1700 / 1200 / 700 units) and
    pops on a ceiling or a floor (v219 / v222), on bots and Tesla coils ("SPLASH" +
    "balloon pop", or "vrzap" in the VR levels). Its trail sprite f_64c (sprite 11) is left
    where it popped and fades out. C3DJimmy's constructor makes 20 (f_cd0[]).
    Reconstructed from Neutron2.exe: factory '3WBL', constructor 0x47c780 (0x678 bytes),
    destructor 0x47cb10 (only resets the vtables, then ~C3DPermanentSprite; deleting thunks
    0x47d3c0 / 0x47cae0 / 0x47d3d0).

    Original layout: as C3DSprite (element vtable 0x504874, 56 slots: C3DSprite's 55 + 55;
    listener 0x504860 at +0x30; CLocalGameObject vtable 0x5043e4 at +0xc8), own members
    0x600..0x670, vtordisp 0x670, streamer 0x674.                                              */
#pragma once
#include "C3DPermanentSprite.h"
#include "OMedia3DVector.h"

class OMedia3DPolygon;

class C3DWaterBalloon : public C3DPermanentSprite
{
	public:
	C3DWaterBalloon();														// 0x47c780
	virtual ~C3DWaterBalloon();												// 0x47cb10

	// ---- element-family game slots
	virtual void Pop(void);													// ESLOT 55 0x47d240 the pop sound, no physics, hidden

	// ---- CGameObject-family overrides
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x47d130 pops on C3DBOT / C3DTESLA
	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x47cb50 ceiling: pop, the trail stays there
	virtual void v222(void);												// SLOT 222 0x47d1a0 floor: the same
	virtual void Process(float dt);											// SLOT 240 0x47cbf0 delay, spin, grow / fade, homing
	virtual void v260(void);												// SLOT 260 0x47d310 sprite, physics, no target, hidden

	// ---- data members (object offsets)
	// 0x600..0x61f never used
	float				f_620;			// 0x620  1 (not read)
	int					f_624;			// 0x624  0 (not read)
	short				f_628;			// 0x628  0 (not read)
	float				f_62c;			// 0x62c  time before the homing starts (0); when it runs out: v136(1)
	int					f_630;			// 0x630  0 (not read)
	CGameObject			*f_634;			// 0x634  target to home on (NULL: none; v260 clears it)
	float				f_638;			// 0x638  homing speed
	float				f_63c;			// 0x63c  offset of the aim point from the target (x, y, z)
	float				f_640;			// 0x640
	float				f_644;			// 0x644
	// 0x648 never used
	C3DPermanentSprite	*f_64c;			// 0x64c  the trail sprite (the original keeps its CGameObject part)
	float				f_650;			// 0x650  the trail's fade time left (1 when it is left behind)
	float				f_654;			// 0x654  time since the shot
	float				f_658;			// 0x658  growth, units / s (> 0: grows and fades out in 5 s)
	float				f_65c;			// 0x65c  delay before it appears
	OMedia3DVector		f_660;			// 0x660  the muzzle it was shot from (not read)
};
