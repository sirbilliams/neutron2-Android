/*  C3DSpriteObject - see C3DSpriteObject.h. Reconstructed from Neutron2.exe 0x472870..0x473420. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSpriteObject.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object whose ObjectTag is `tag` (case-insensitive).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

static bool IsGemHolder(const char *tag)
{
	return strcasecmp(tag, "GEMH") == 0 || strcasecmp(tag, "LEFTGEM") == 0 || strcasecmp(tag, "RIGHTGEM") == 0;
}

// 0x437410 (thiscall on the gem Jimmy holds): put it down in `holder` (the sphinx puzzle:
// LEFTGEM / RIGHTGEM, the "PICKUP" animation, Jimmy's f_d38 cleared, SPHYNXRAY's colour).
static void PutGemDown(C3DObject *gem, CGameObject *jimmy, CGameObject *holder)
{
	(void)gem; (void)jimmy; (void)holder;
	NEG_TODO(0x437410, "Jimmy puts his gem in a gem holder");
}

// 0x437340: a gem taken out of `holder` (LEFTGEM / RIGHTGEM): SPHYNXRAY's colour.
static void GemTaken(CGameObject *holder)
{
	(void)holder;
	NEG_TODO(0x437340, "a gem taken out of LEFTGEM / RIGHTGEM");
}

// Jimmy turned to face this sprite: his rotation (own x, heading towards it, own z).
// (Inlined twice, in AddTouching and Process.)
static void FaceJimmy(C3DSpriteObject *self, CGameObject *jim)
{
	float p[3], q[3], rot[3];
	self->v195(p);											// +0x30c
	jim->v195(q);
	OMedia3DVector v(p[0] - q[0], p[1] - q[1], p[2] - q[2]);
	omt_Angle ax, ay;
	v.angles(ax, ay);
	self->v201(rot);										// +0x324
	jim->v203(rot[0], (float)(int)ay * 0.02197265625f, rot[2]);	// +0x32c (0x4cb1f8: 360 / 16384)
}

// The linked object (LinkedTag): found and placed (v260, ReadState).
static void LinkObject(C3DSpriteObject *self)
{
	if (strcasecmp(self->f_6a0, "none") == 0) return;
	self->f_704 = FindObjectByTag(self->f_6a0);
	if (!self->f_704) return;
	self->f_704->v197(self->f_694[0], self->f_694[1], self->f_694[2]);	// +0x314: position
	self->f_704->v203(self->f_688[0], self->f_688[1], self->f_688[2]);	// +0x32c: rotation
}

// --------------------------------------------------------------------------- construction

C3DSpriteObject::C3DSpriteObject()							// 0x472870
{
	SetName((char *)"C3DSPRITEOBJECT");
	C3DSpriteObject::InitObject();							// 0x472ae0 (called directly)
	SetObjectID('3SRO');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 50;
	m_sprite_size = 100;
	v42(0);
	v40(0);
	v52(1);
	v46(0);
	v34(1);
	v67(100.0f);
	f_66c = 1.0f;											// Red
	f_670 = 0.1f;											// Green
	f_674 = 0.1f;											// Blue
	f_668 = 0;												// SpecialCase
	f_678 = 0.8f;											// Opacity
	f_67c = 1;												// InitiallyVisible
	f_008 = 1;
	f_680 = 0;
	f_684 = 0;
	f_708 = 0;
	f_70c = 0;
	f_688[0] = f_688[1] = f_688[2] = 0;						// LinkedRot
	f_694[0] = f_694[1] = f_694[2] = -1.0f;					// LinkedOffset
	f_704 = 0;
	strcpy(f_6a0, "none");									// LinkedTag
	C3DSprite::InitPhysics();								// 0x49ca30
}

C3DSpriteObject::~C3DSpriteObject() {}						// 0x472aa0

namespace neg {
CGameObject *New_C3DSpriteObject(void) { return new C3DSpriteObject; }	// '3SRO' (0x718 bytes)
}

// --------------------------------------------------------------------------- slots

unsigned char C3DSpriteObject::ev55(float *t, float dt)	// 0x48fcb0
{
	if (!(*t > 0)) return 0;
	*t -= dt;
	if (*t > 0) return 0;
	*t = 0;
	return 1;
}

void C3DSpriteObject::InitObject(void)						// 0x472ae0
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("Red", &f_66c, PROP_FLOAT, 0);
	RegisterProperty("Green", &f_670, PROP_FLOAT, 0);
	RegisterProperty("Blue", &f_674, PROP_FLOAT, 0);
	RegisterProperty("SpecialCase", &f_668, PROP_LONG, 0);
	RegisterProperty("Opacity", &f_678, PROP_FLOAT, 0);
	RegisterProperty("InitiallyVisible", &f_67c, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("LinkedTag", f_6a0, PROP_ALPHA, 0);
	RegisterProperty("LinkedRotX", &f_688[0], PROP_FLOAT, 0);
	RegisterProperty("LinkedRotY", &f_688[1], PROP_FLOAT, 0);
	RegisterProperty("LinkedRotZ", &f_688[2], PROP_FLOAT, 0);
	RegisterProperty("LinkedOffsetX", &f_694[0], PROP_FLOAT, 0);
	RegisterProperty("LinkedOffsetY", &f_694[1], PROP_FLOAT, 0);
	RegisterProperty("LinkedOffsetZ", &f_694[2], PROP_FLOAT, 0);
}

void C3DSpriteObject::ResetObject(void)						// 0x473010
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	v176(0, 0, 0);											// +0x2c0
	v157(0, 0, 0);											// +0x274
}

void C3DSpriteObject::AddTouching(CGameObject *other)		// 0x473040
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (strcasecmp(other->m_tag, "JIM1") != 0 || f_708 != 0) return;
	if (!IsGemHolder(m_tag)) return;
	C3DJimmy *jim = Jimmy();
	CGameObject *held = f_704;
	if (!jim->f_d38)										// empty hands: take the gem out
	{
		if (held && held->NameContains((char *)"C3DGEM"))	// +0x18
		{
			if (strcasecmp(m_tag, "LEFTGEM") == 0 || strcasecmp(m_tag, "RIGHTGEM") == 0)
				GemTaken(this);								// 0x437340
			held->AddTouching(other);						// +0x40
			f_704 = 0;
		}
		return;
	}
	if (held || jim->f_ca8) return;
	if (g_jimmy->v133())									// +0x214: on the floor
	{
		FaceJimmy(this, g_jimmy);
		f_704 = jim->f_d38;
		PutGemDown(jim->f_d38, other, this);				// 0x437410
		return;
	}
	f_70c = other;											// put it down once he lands (Process)
	f_708 = this;
}

void C3DSpriteObject::Process(float dt)						// 0x472d60
{
	C3DSpriteType::Process(dt);								// 0x473700
	if (ev55(&f_680, dt)) HideAgain();						// element +0xdc, +0xd8
	if (ev55(&f_684, dt)) ShowAgain();						// element +0xdc, +0xd4
	if (f_708 && g_jimmy->v133())							// +0x214
	{
		C3DJimmy *jim = Jimmy();
		FaceJimmy(this, g_jimmy);
		f_704 = jim->f_d38;
		PutGemDown(jim->f_d38, f_70c, f_708);				// 0x437410
		f_708 = 0;
		f_70c = 0;
	}
	float p[3];
	v195(p);												// +0x30c
	const float y = p[1] + 10.0f;							// 0x4cb1cc
	const float z = p[2] - 30.0f;							// 0x4ca4b4
	if (f_694[0] == -1.0f) f_694[0] = p[0] + 10.0f;
	if (f_694[1] == -1.0f) f_694[1] = y;
	if (f_694[2] == -1.0f) f_694[2] = z;
	if (f_704)
	{
		f_704->v197(f_694[0], f_694[1], f_694[2]);			// +0x314
		f_704->v203(f_688[0], f_688[1], f_688[2]);			// +0x32c
	}
}

void C3DSpriteObject::v260(void)							// 0x472c60
{
	C3DSpriteType::v260();									// 0x473650
	if (f_67c == 0) HideAgain();							// element +0xd8
	else ShowAgain();										// element +0xd4
	f_680 = 0;
	f_684 = 0;
	set_diffuse(OMediaFARGBColor(f_678, f_66c, f_670, f_674));	// +0x90..+0x9c (r, g, b, a = Red, Green, Blue, Opacity)
	LinkObject(this);
}

void C3DSpriteObject::WriteState(OMediaStreamOperators &s)	// 0x464ee0 (empty)
{
	(void)s;
}

void C3DSpriteObject::ReadState(OMediaStreamOperators &s)	// 0x473340
{
	(void)s;
	f_704 = 0;
	LinkObject(this);
}
