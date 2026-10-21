#include <SDL.h> // TEMPDBG
/*  C3DMovingTarget - see C3DMovingTarget.h. Reconstructed from Neutron2.exe 0x456320..0x456de0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DMovingTarget.h"
#include "../C3DTriggerType.h"
#include "../CGameType.h"
#include "../InterfaceElement.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMedia3DVector.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// The hit counter of target group `id` (0..2; others have none).
static int *HitCounter(int id)
{
	if (id == 0) return &g_545fdc;
	if (id == 1) return &g_545fe0;
	if (id == 2) return &g_545fe4;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DMovingTarget::C3DMovingTarget()							// 0x456320
{
	SetName((char *)"C3DMOVINGTARGET");
	Log("C3DMovingTarget()");
	C3DSprite::InitObject();								// 0x49c950 (it has no InitObject of its own)
	f_68c = 100.0f;											// Speed
	f_7d0 = 100.0f;
	f_6f8 = 5;												// HitsRequired
	f_700 = 15.0f;
	f_6fc = 10;												// NumPoints
	RegisterProperty("StartObj",       f_704, PROP_ALPHA, 0);	// 0x4aeee0
	RegisterProperty("DestObj",        f_768, PROP_ALPHA, 0);
	RegisterProperty("Speed",          &f_68c, PROP_FLOAT, 0);
	RegisterProperty("ActivateObject", f_694, PROP_ALPHA, 0);
	RegisterProperty("HitsRequired",   &f_6f8, PROP_LONG, 0);
	RegisterProperty("NumPoints",      &f_6fc, PROP_LONG, 0);
	RegisterProperty("TargetID",       &f_7cc, PROP_LONG, 0);
	SetObjectID('3TAR');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	strcpy(f_694, "none");
	m_sprite_index = 98;
	m_sprite_size = 100;
	v42(0);
	v40(0);
	v67(250.0f);											// radius
	v46(1);
	v34(1);
	strcpy(f_704, "none");
	strcpy(f_768, "none");
	C3DSprite::InitPhysics();								// 0x49ca30
	f_668 = 0;
	v111(0);
	f_67c[0] = f_67c[1] = f_67c[2] = 0;
	f_66c[0] = f_66c[1] = f_66c[2] = 0;
	f_690 = 0;
	f_7cc = 0;
	// (then a call to the empty 0x463890)
}

C3DMovingTarget::~C3DMovingTarget() {}						// 0x456630

namespace neg {
CGameObject *New_C3DMovingTarget(void) { return new C3DMovingTarget; }	// '3TAR' (0x7dc bytes)
}

// --------------------------------------------------------------------------- slots

void C3DMovingTarget::AddTouching(CGameObject *other)		// 0x456670
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (!other->NameContains((char *)"C3DWATERBALLOON") && !other->NameContains((char *)"C3DENERGYPROJECTILE")) return;
	SDL_Log("TEMPDBG target hit %s", f_694);
	HideAgain();											// element +0xd8
	v136(0);												// +0x220
	other->v136(0);
	// port: the original calls element +0xd8 of the object 0xc8 before `other` (a sprite's
	// HideAgain); both projectiles are sprites.
	C3DSprite *shot = dynamic_cast<C3DSprite *>(other);
	if (shot) shot->HideAgain();
	other->NameContains((char *)"C3DENERGYPROJECTILE");	// (both branches play the same sound)
	PlayGameSound(-1, std::string("vrzap"), false);		// 0x48f420
	AddScore(f_6fc);										// 0x482b50
	bool done = false;
	int *count = HitCounter(f_7cc);
	if (count && ++*count >= f_6f8) done = true;
	PlayGameSound(-1, std::string("pickup blip3"), false);	// (also the same in both branches)
	if (!done) return;
	*count = 0;
	CGameObject *act = FindObjectByTag(f_694);
	if (!act) return;
	if (act->NameContains((char *)"C3DAITRIGGER"))
	{
		static_cast<C3DTriggerType *>(act)->CallNextTrigger(f_694, 0);	// element +0xdc
		return;
	}
	float p[4];
	act->v195(p);											// +0x30c
	act->v196(p[0], p[1] - 10000.0f, p[2], p[3]);			// +0x310, 0x4eb658: out of the way
}

unsigned char C3DMovingTarget::v231(CGameObject *other)		// 0x456a10
{
	const int id = other->m_object_id;
	return id == '3EPR' || id == '3WBL';
}

void C3DMovingTarget::Process(float dt)						// 0x456a40
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	float p[4];
	v195(p);												// +0x30c
	OMedia3DVector d(f_67c[0] - p[0], f_67c[1] - p[1], f_67c[2] - p[2], 0);
	const float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
	const float inv = 1.0f / d.quick_magnitude();
	v176(d.x * inv * f_7d0, d.y * inv * f_7d0, d.z * inv * f_7d0);	// +0x2c0: the velocity
	if (dist < 100.0f) v196(f_66c[0], f_66c[1], f_66c[2], f_66c[3]);	// 0x4d2cb0: back to the start
}

// The speed for the difficulty (g_53b5b0): x 0.6 / 0.9 / 1.2.
static float DifficultySpeed(float speed)
{
	if (g_53b5b0 == 0) return speed * 0.6f;					// 0x4e5628
	if (g_53b5b0 == 1) return speed * 0.9f;					// 0x4e4158
	if (g_53b5b0 == 2) return speed * 1.2f;					// 0x4eb65c
	return -1.0f;											// port: (not reached) left unchanged
}

void C3DMovingTarget::v260(void)							// 0x4568f0
{
	C3DSprite::v260();										// 0x49cfd0
	CGameObject *o = FindObjectByTag(f_704);
	if (o) o->v195(f_66c);									// +0x30c
	o = FindObjectByTag(f_768);
	if (o) o->v195(f_67c);
	ShowAgain();											// element +0xd4
	g_545fdc = 0;
	g_545fe0 = 0;
	g_545fe4 = 0;
	const float s = DifficultySpeed(f_68c);
	if (s >= 0) f_7d0 = s;
}

void C3DMovingTarget::v278(void)							// 0x456d20
{
	// (debug log, empty)
	const float s = DifficultySpeed(f_68c);
	if (s >= 0) f_7d0 = s;
}

void C3DMovingTarget::WriteState(OMediaStreamOperators &s)	// 0x456b80
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	s << g_545fdc;
	s << g_545fe0;
	s << g_545fe4;
	float p[4];
	v195(p);												// +0x30c
	s << p[0];
	s << p[1];
	s << p[2];
	bool active = v135() != 0;								// +0x21c
	s << active;
}

void C3DMovingTarget::ReadState(OMediaStreamOperators &s)	// 0x456c30
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	s >> g_545fdc;
	s >> g_545fe0;
	s >> g_545fe4;
	float p[4];
	p[3] = 1.0f;
	s >> p[0];
	s >> p[1];
	s >> p[2];
	v196(p[0], p[1], p[2], p[3]);							// +0x310
	bool active;
	s >> active;
	v136(active);											// +0x220
	if (v135()) ShowAgain();								// element +0xd4
	else HideAgain();										// element +0xd8
}
