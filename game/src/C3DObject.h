/*  C3DObject - the base of every 3D game object: an OMT morph-animated shape element that is
    also a CLocalGameObject. Reconstructed from Neutron2.exe: constructor 0x49a920, destructor
    0x49aa50. Original layout: OMedia3DMorphAnim at +0 (element vtable, 56 slots: OMT's 49 +
    game slots 49..55), its OMediaAnim at +0x90, CLocalGameObject at +0xc0, own members from
    +0x588, size 0x590 + the virtual-base streamer.

    This is also the template for the other multi-base classes: element-family game slots are
    `ev<n>` marked `// ESLOT n`, CGameObject-family slots keep their names and `// SLOT n`.  */
#pragma once
#include "OMedia3DMorphAnim.h"
#include "CLocalGameObject.h"

class C3DObject : public OMedia3DMorphAnim, public CLocalGameObject
{
	public:
	C3DObject();															// 0x49a920
	virtual ~C3DObject();													// SLOT 0  0x49aa50

	// ---- OMT element virtuals overridden
	virtual void set_shape(OMedia3DShape *shape);							// element slot 43  0x49ab10

	// ---- element-family game slots
	virtual void SetObjectID(int id);										// ESLOT 49 0x49ab00
	virtual void FitToShape(void);											// ESLOT 50 0x49ab50 (TODO body)
	virtual void ev51(void);												// ESLOT 51 0x49afd0 (TODO body)
	virtual void ev52(void);												// ESLOT 52 0x49b030 (TODO body)
	virtual void ev53(void);												// ESLOT 53 0x49b090 (TODO body)
	virtual void ShowAgain(void);											// ESLOT 54 0x49b100
	virtual void HideAgain(void);											// ESLOT 55 0x49b120

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7  0x49ac20
	virtual void UnInitObject(void);										// SLOT 8  0x49ac70
	virtual void InitPhysics(void);											// SLOT 11 0x49acb0
	virtual void UnInitPhysics(void);										// SLOT 12 0x49ad60
	virtual void Process(float dt);											// SLOT 240 0x49ae00 (TODO body)
	virtual void v243(float a);												// SLOT 243 0x49af30 (TODO body)
	virtual void v247(void);												// SLOT 247 0x49ad90 SyncElementPosition
	void SyncElementPosition(void) { v247(); }
	virtual void v248(void);												// SLOT 248 0x49add0 (TODO body)
	virtual void v249(short x, short y, short z);							// SLOT 249 0x49ade0 SetElementAngles
	void SetElementAngles(short x, short y, short z) { v249(x, y, z); }

#include "gen/C3DObject_slots.inc"
#include "gen/C3DObject_fields.inc"
};
