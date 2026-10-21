/*  C3DVortex - see C3DVortex.h. Reconstructed from Neutron2.exe 0x47a6f0..0x47aae0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DVortex.h"
#include "C3DMiniSub.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#include "OMedia3DVector.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- small helpers

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// The 10 trail bubbles a C3DMiniSub keeps at +0xa04..+0xa28 (game object views).
static CGameObject **SubObjects(CGameObject *sub)
{
	C3DMiniSub *m = dynamic_cast<C3DMiniSub *>(sub);		// port: the original assumes it is one
	return m ? m->f_a04 : NULL;
}

// --------------------------------------------------------------------------- construction

C3DVortex::C3DVortex()										// 0x47a6f0
{
	SetName((char *)"C3DVORTEX");
	Log("C3DVortex()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	SetObjectID('3VOR');									// element +0xc0
	strcpy(m_sprite_db, "icons.omt");
	m_sprite_index = 1;
	m_sprite_size = 150;
	v42(0);
	v40(0);
	v52(0);
	v46(0);
	m_sub = NULL;
	f_008 = 1;												// element +0xd0
	// (then calls the empty 0x463890)
}

C3DVortex::~C3DVortex() {}									// 0x47a870

// --------------------------------------------------------------------------- CGameObject-family slots

// 0x47a8b0
void C3DVortex::v260(void)
{
	C3DSprite::v260();										// 0x49cfd0
	m_sub = FindObjectByTag("C3DMINISUB");
	HideAgain();											// element +0xd8
}

// 0x47a8e0: within 5000 of the sub, set its impulse towards the vortex, stronger the closer.
void C3DVortex::Process(float dt)
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	if (!m_sub) return;

	float me[4], it[4];
	v195(me);												// +0x30c position
	m_sub->v195(it);
	OMedia3DVector d(me[0] - it[0], me[1] - it[1], me[2] - it[2], 0);
	float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
	if (!(dist < 5000.0f)) return;
	float pull = 5000.0f - dist;
	float inv = 1.0f / d.quick_magnitude();
	d.x *= inv; d.y *= inv; d.z *= inv;
	d.x = d.x * pull * 0.1f;
	d.y = d.y * pull * 0.008f;
	d.z = d.z * pull * 0.1f;
	m_sub->v161(d.x, d.y, d.z, d.w);						// +0x284 impulse
	d.x *= 0.1f;
	d.y *= 0.02f;
	d.z *= 0.1f;
	CGameObject **objs = SubObjects(m_sub);					// (m_sub re-read, as an object - 0xc0)
	if (!objs) return;
	for (int i = 0; i < 10; ++i)							// +0xa04..+0xa28
		if (objs[i]) objs[i]->v161(d.x, d.y, d.z, d.w);		// (the trail bubbles)
}

namespace neg {
CGameObject *New_C3DVortex(void)	{ return new C3DVortex; }		// factory '3VOR' (0x60c bytes)
}
