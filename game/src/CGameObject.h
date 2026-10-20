/*  CGameObject - the base of every game object (characters, pickups, triggers, menus).
    Reconstructed from Neutron2.exe: vtable 0x51b1a8 (265 slots), constructor 0x4aa960,
    destructor 0x4aaf60.

    Most of the class is a large set of attributes, each kept twice: the value the object was
    created/loaded with (f_00c..f_038 and friends) and the current value (f_134...), with a getter
    and a setter slot for each. Those slots are generated from the binary by
    tools/decomp/gen_class.py into gen/CGameObject_*.{inc,cpp}; the rest is written here by hand.

    Properties: InitObject() registers named fields ("PositionX", "ObjectID", ...) that the level
    files (.gam) and savegames read and write by name (ReadProperties / WriteProperties).

    Members named f_<offset> keep the original byte offset as their name until their meaning is
    known; runs of adjacent fields keep their original order (the property loader copies 4 bytes
    into 1-byte fields, exactly as the original did).                                         */
#pragma once
#include <list>
#include "OMediaClassStreamer.h"
#include "Globals.h"
#include "OMedia3DVector.h"
#include "Sound.h"				// StopObjectSounds, IsChannelPlaying

class OMediaStreamOperators;
class OMedia3DShape;
class OMedia3DPolygon;
class CGameObject;

// A registered property (0x4c bytes): see RegisterProperty 0x4aeee0.
struct CProperty
{
	int		flags;				// +0x00
	int		type;				// +0x04  PROP_*
	char	name[64];			// +0x08
	void	*data;				// +0x48  the field it edits
};

enum { PROP_NONE = 0, PROP_ALPHA = 1, PROP_BOOL = 2, PROP_FLOAT = 3, PROP_SPECIFIC = 4, PROP_5 = 5, PROP_LONG = 6 };

class CGameObject : public virtual OMediaClassStreamer		// virtual, as in the original (vbptr at +4)
{
	public:
	CGameObject();															// 0x4aa960
	virtual ~CGameObject();													// SLOT 0  0x4aaee0 / 0x4aaf60

	// ---- properties and identity (hand-written)
	virtual void ReadProperties(OMediaStreamOperators &s);					// SLOT 1  0x4aea70
	virtual void WriteProperties(OMediaStreamOperators &s);				// SLOT 2  0x4ae7d0
	virtual void v4(float t);												// SLOT 4  0x48f880
	virtual void SetName(char *tag);										// SLOT 5  0x4ab1b0
	virtual bool NameContains(char *s);										// SLOT 6  0x4ab260
	virtual void InitObject(void);											// SLOT 7  0x4ab290
	virtual void ResetObject(void);											// SLOT 10 0x4ab420
	// touches, from PhysicsCollideObjects: the argument is the other object
	virtual void v3(CGameObject *other);									// SLOT 3  0x4ab1a0 (= AddTouching)
	virtual bool IsTouching(CGameObject *other);							// SLOT 15 0x4ab940
	virtual void AddTouching(CGameObject *other);							// SLOT 16 0x4ab970 touch starts
	virtual void RemoveTouching(CGameObject *other);						// SLOT 17 0x4aba00 touch ends
	virtual void v18(CGameObject *other);									// SLOT 18 0x4abaa0 still touching
	virtual void Collide(CGameObject *other, float dist);					// SLOT 19 0x4abad0 spheres overlap (dist between centres)
	virtual void v21(CGameObject *other);									// SLOT 21 0x4abd40
	virtual void AddRotation(float x, float y, float z);					// SLOT 204 0x4ad310
	virtual void RequestEnd(void);											// SLOT 217 0x4ad4c0
	// terrain contact at the end of a sweep (PhysicsMoveAgainstTerrain): the polygon's normal
	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x4ad520 ceiling
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);	// SLOT 220 0x48ef40 wall/floor
	virtual OMedia3DVector v224(float x, float y, float z);				// SLOT 224 0x4ad800 local point -> world (rotation v201, position)
	virtual unsigned char v231(CGameObject *other);							// SLOT 231 0x410490 may collide with other (1)
	// floor polygon/shape found by PhysicsSnapToGround (initial value f_10c/f_114, current f_234/f_23c)
	virtual OMedia3DPolygon *v95(void);										// SLOT 95  0x4ac620
	virtual void v96(OMedia3DPolygon *poly);								// SLOT 96  0x4ac630
	virtual OMedia3DShape *v99(void);										// SLOT 99  0x4ac680
	virtual void v100(OMedia3DShape *shape);								// SLOT 100 0x4ac690
	virtual OMedia3DPolygon *v205(void);									// SLOT 205 0x4ad400
	virtual void v206(OMedia3DPolygon *poly);								// SLOT 206 0x4ad410
	virtual OMedia3DShape *v209(void);										// SLOT 209 0x4ad440
	virtual void v210(OMedia3DShape *shape);								// SLOT 210 0x4ad450
	virtual void Link(CGameObject *other);									// SLOT 233 0x4ade00
	virtual void Unlink(CGameObject *other);								// SLOT 235 0x4adeb0
	virtual void DisableCollisionID(int id);								// SLOT 236 0x4adf60
	virtual void EnableCollisionID(int id);									// SLOT 237 0x4adfe0
	virtual void Process(float dt);											// SLOT 240 0x4ae0b0
	virtual void CheckSounds(float dt);										// SLOT 245 0x4ae110
	virtual void RotateToDest(float dt);									// SLOT 246 0x4ae180
	virtual void Log(const char *fmt, ...);									// SLOT 254 0x4ae780
	virtual void RegisterProperty(const char *name, void *data, int type, int flags);	// SLOT 255 0x4aeee0
	virtual CProperty *FindProperty(const char *name);						// SLOT 256 0x4aedf0
	virtual void CopyPropertiesTo(CGameObject *dst);						// SLOT 257 0x4ae9a0
	virtual void DeleteAllProperties(void);									// SLOT 259 0x4af3f0
	virtual void v262(CGameObject *other, float dist);						// SLOT 262 0x4af030 (TODO body)
	virtual void v221(float a, float b, float c, float d);					// SLOT 221 0x4ad5e0 (TODO body)
	virtual void v23(CGameObject *o) { (void)o; }							// SLOT 23 0x464ee0 (empty)
	virtual void v24(CGameObject *o) { (void)o; }							// SLOT 24 0x464ee0 (empty)
	virtual void v242(float t) { (void)t; }									// SLOT 242 0x464ee0 (empty)

	// ---- generated slots (accessors, forwarders) and the slots still to write
#include "gen/CGameObject_slots.inc"

	// ---- data members
	// FIELD 0x128 12
	std::list<void *>			f_128;			// 0x128  (element type not known yet)
	// FIELD 0x250 12
	std::list<CGameObject *>	m_linked;		// 0x250  objects linked both ways (Link/Unlink)
	// FIELD 0x25c 12
	std::list<CGameObject *>	m_touching;		// 0x25c  objects currently touching
	// FIELD 0x268 12
	std::list<CProperty *>		m_properties;	// 0x268
	// FIELD 0x2a0 256
	char						m_name[256];	// 0x2a0  class name, then "_<tag>" (SetName)
	// FIELD 0x3a0 100
	char						m_tag[100];		// 0x3a0  "ObjectTag" property
	// FIELD 0x404 4
	int							m_object_id;	// 0x404  "ObjectID" property (4cc, e.g. '3JIM')
	// FIELD 0x420 8
	CGameObject					*m_debug_cube;	// 0x420  created on demand by v251 ('3CUB')
	CGameObject					*m_debug_sphere;// 0x424  created on demand by v253 ('3SPH')
	// FIELD 0x428 4
	int							m_num_properties;	// 0x428

	// TYPE 0x414 OMedia3DShape *
	// TYPE 0x10c OMedia3DPolygon *
	// TYPE 0x114 OMedia3DShape *
	// TYPE 0x234 OMedia3DPolygon *
	// TYPE 0x23c OMedia3DShape *
	// TYPE 0x294 float
	// TYPE 0x298 float
#include "gen/CGameObject_fields.inc"
};

// The global object list (0x56af34 head / 0x56af38 count).
extern std::list<CGameObject *> g_objects;

namespace neg {
void ErrorLog(const char *fmt, ...);										// 0x4b3b10  dat\debug.log
CGameObject *CreateObject(int id);											// 0x4912d0
void PhysicsRemoveObject(CGameObject *o);									// 0x4b4070
}
