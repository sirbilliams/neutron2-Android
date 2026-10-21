/*  C3DTextTrigger - see C3DTextTrigger.h. Reconstructed from Neutron2.exe 0x4767d0..0x476cc0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DTextTrigger.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>

using namespace neg;

// The caption (C2DInGameMenu's TRIGTEXTSTRING teletype) restarts at `rate` with no text.
static void ClearCaption(float rate)
{
	g_teletype_rate = rate;									// 0x5324f4
	g_trig_text_len = 0;									// 0x545e4c
	g_trig_text_line = 0;									// 0x545e48
	strcpy(g_trig_text_group, g_empty_string);				// 0x545de4
}

// --------------------------------------------------------------------------- construction

C3DTextTrigger::C3DTextTrigger()							// 0x4767d0
{
	SetName((char *)"C3DTEXTTRIGGER");
	Log("C3DAITrigger()");									// (sic)
	C3DTriggerType::InitObject();							// (not ours: the properties are registered here)
	RegisterProperty("TextTrigName", m_text_trig_name, PROP_ALPHA, 0);
	RegisterProperty("DrawSpeed", &m_draw_speed, PROP_FLOAT, 0);
	RegisterProperty("EnableTrigger", &m_enable_trigger, PROP_LONG, 0);
	RegisterProperty("DisableTime", &m_disable_time, PROP_FLOAT, 0);
	SetObjectID('3TEX');
	m_sprite_index = 9;
	m_sprite_size = 50;
	strcpy(m_sprite_db, "icons.omt");
	v67(100.0f);
	v46(0);
	v52(1);
	v32(1);
	f_008 = 1;												// element +0xd0
	C3DSprite::InitPhysics();
	v34(1);
	m_draw_speed = 12.0f;
	strcpy(m_text_trig_name, "none");
	m_enable_trigger = 1;
	m_disable_time = -1.0f;
	m_time_left = 0;
	// (an empty debug log)
}

C3DTextTrigger::~C3DTextTrigger() {}						// 0x476a20

namespace neg {
CGameObject *New_C3DTextTrigger(void) { return new C3DTextTrigger; }	// '3TEX' (0x81c bytes)

void TextTriggerTrigger(CGameObject *o)
{
	C3DTextTrigger *t = o ? dynamic_cast<C3DTextTrigger *>(o) : NULL;	// (o - 0xc8)
	if (t) t->Trigger();
}
}

// --------------------------------------------------------------------------- slots

void C3DTextTrigger::InitObject(void)						// 0x476b50
{
	C3DTriggerType::InitObject();
	RegisterProperty("TextTrigName", m_text_trig_name, PROP_ALPHA, 0);
	RegisterProperty("DrawSpeed", &m_draw_speed, PROP_FLOAT, 0);
	RegisterProperty("EnableTrigger", &m_enable_trigger, PROP_LONG, 0);
	RegisterProperty("DisableTime", &m_disable_time, PROP_FLOAT, 0);
}

// 0x476a60: a skipped cut scene clears the caption (every frame while skipped); else the
// DisableTime countdown clears it when it runs out.
void C3DTextTrigger::Process(float dt)
{
	C3DSpriteType::Process(dt);								// 0x473700
	if (g_cutscene_skipped)
	{
		m_time_left = 0;
		ClearCaption(m_draw_speed);
	}
	if (strcasecmp(g_trig_text_group, g_empty_string) == 0) return;
	if (!(m_time_left > 0)) return;
	m_time_left -= dt;
	if (m_time_left > 0) return;
	ClearCaption(m_draw_speed);
}

// 0x476bd0
void C3DTextTrigger::Trigger(void)
{
	g_teletype_rate = m_draw_speed;
	g_trig_text_len = 0;
	g_trig_text_line = 0;
	if (m_enable_trigger == 0) strcpy(g_trig_text_group, g_empty_string);
	else
	{
		strcpy(g_trig_text_group, m_text_trig_name);
		if (m_disable_time > 0) m_time_left = m_disable_time;
	}
	CallNextTrigger(m_next_trigger, 0);						// element +0xdc
}
