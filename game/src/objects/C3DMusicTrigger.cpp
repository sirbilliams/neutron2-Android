/*  C3DMusicTrigger - see C3DMusicTrigger.h. Reconstructed from Neutron2.exe 0x458d60..0x459210. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DMusicTrigger.h"
#include "../CJimmyGame.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>

using namespace neg;

// g_gametype element +0x198: CJimmyGame::PlayLevelMusic (every level game type is one).
static void PlayLevelMusic(const char *db, int index)
{
	CJimmyGame *g = dynamic_cast<CJimmyGame *>(g_gametype);	// port: checked (the original calls the slot of any game type)
	if (g) g->PlayLevelMusic(db, index);
}

C3DMusicTrigger::C3DMusicTrigger()							// 0x458d60
{
	SetName((char *)"C3DMUSICTRIGGER");
	Log("C3DMusicTrigger()");
	C3DMusicTrigger::InitObject();							// (called directly, as in the original)
	SetObjectID('3MUS');									// element slot 48
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_music_db, "none");
	m_sprite_index = 11;
	m_sprite_size = 50;
	m_music_index[0] = 0;
	m_music_index[1] = -1;
	m_music_index[2] = -1;
	m_music_index[3] = -1;
	m_music_index[4] = -1;
	v67(150.0f);
	C3DSprite::InitPhysics();								// 0x49ca30
	m_touch_activated = 1;
	v52(0);
	v46(0);
	v34(1);
	f_008 = 1;												// element +0xd0
	m_required_level = -1;
	m_exact_level = -1;
	m_remove_level = -1;
	// (an empty debug log)
}

C3DMusicTrigger::~C3DMusicTrigger() {}						// 0x458f60

namespace neg {
CGameObject *New_C3DMusicTrigger(void) { return new C3DMusicTrigger; }	// '3MUS' (0x830 bytes)

void MusicTriggerTrigger(CGameObject *o)
{
	C3DMusicTrigger *m = o ? dynamic_cast<C3DMusicTrigger *>(o) : NULL;	// (o - 0xc8)
	if (m) m->Trigger();
}
}

void C3DMusicTrigger::InitObject(void)						// 0x458fa0
{
	C3DTriggerType::InitObject();
	RegisterProperty("MusicDatabase", m_music_db, PROP_ALPHA, 0);
	RegisterProperty("MusicIndex0", &m_music_index[0], PROP_LONG, 0);
	RegisterProperty("TouchActivated", &m_touch_activated, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("RequiredLevel", &m_required_level, PROP_LONG, 0);
	RegisterProperty("ExactLevel", &m_exact_level, PROP_LONG, 0);
	RegisterProperty("RemoveLevel", &m_remove_level, PROP_LONG, 0);
}

void C3DMusicTrigger::AddTouching(CGameObject *other)		// 0x4590a0
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (other == g_camera) Trigger();						// element +0xe0
}

unsigned char C3DMusicTrigger::v231(CGameObject *other)		// 0x45ce70
{
	if (g_camera == other) return 1;
	return other->m_object_id == '3JIM';
}

void C3DMusicTrigger::v260(void)							// 0x459060
{
	C3DSpriteType::v260();									// 0x473650
	if (m_touch_activated == 0) v136(0);					// +0x220
	if (m_touch_activated == 1) v136(1);
}

// 0x459130: not touch activated: ExactLevel, else RequiredLevel <= level < RemoveLevel (-1: no
// upper bound) fires it.
void C3DMusicTrigger::v267(int level)
{
	CLocalGameObject::v267(level);
	if (m_touch_activated != 0) return;
	if (m_exact_level == -1)
	{
		if (m_required_level == -1) return;
		if (level < m_required_level) return;
		if (m_remove_level != -1 && m_remove_level <= level) return;
	}
	else if (level != m_exact_level) return;
	Trigger();
}

void C3DMusicTrigger::v268(int a2)							// 0x459100
{
	(void)a2;
	PlayLevelMusic(m_music_db, m_music_index[0]);			// g_gametype element +0x198
}

void C3DMusicTrigger::Trigger(void)							// 0x4590d0
{
	if (g_cutscene_skipped) return;
	PlayLevelMusic(m_music_db, m_music_index[0]);
}
