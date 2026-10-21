/*  C3DFadeObject - see C3DFadeObject.h. Reconstructed from Neutron2.exe 0x431180..0x4314d0
    and the fade starter 0x406910. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFadeObject.h"
#include "C3DFlyingCamera.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

void neg::StartTriggerFade(int mode, float t, float hold, float in_time, const char *trigger)	// 0x406910
{
	if (g_fade_setting == 0)								// fades off: the trigger at once
	{
		g_fade_done = true;
		if (!trigger || strcasecmp(trigger, g_empty_string) == 0) return;
		CGameObject *o = FindObjectByTag(trigger);
		if (!o) return;
		if (o->NameContains((char *)"C3DTRIGGERTYPE"))
		{
			C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(o);	// (o - 0xc8)
			if (tt) tt->CallNextTrigger(trigger, 0);		// element +0xdc
			return;
		}
		if (o->NameContains((char *)"C3DFLYINGCAMERA")) FlyingCameraTrigger(o);	// element +0x240
		return;
	}
	if (mode == 6) mode = 4;
	g_fade_duration = t;
	if (t < 0.1f) g_fade_duration = 0.1f;					// 0x4caa68
	g_fade_hold_time = hold;
	g_fade_in_time = in_time;
	g_fade_done = false;
	g_fading = true;
	g_fade_time = g_fade_duration;
	g_fade_state = mode;
	strcpy(g_fade_trigger, trigger);						// (no length check, as shipped)
}

// --------------------------------------------------------------------------- construction

C3DFadeObject::C3DFadeObject()								// 0x431180
{
	SetName((char *)"C3DFADEOBJECT");
	Log("C3DFadeObject()");
	C3DTriggerType::InitObject();							// (not ours: the two properties are
	RegisterProperty("FadeInTime", &m_fade_in_time, PROP_FLOAT, 0);	//  registered here instead)
	RegisterProperty("StayBlack", &m_stay_black, PROP_FLOAT, 0);
	SetObjectID('3FAD');
	m_sprite_index = 9;
	m_sprite_size = 50;
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_next_trigger, "none");
	v67(50.0f);
	v46(0);
	v52(1);
	v32(1);
	f_008 = 1;												// element +0xd0
	C3DSprite::InitPhysics();
	v34(1);
	m_fade_type = 4;
	m_fade_time = 1.0f;
	m_fade_in_time = 1.0f;
	m_stay_black = 0.5f;
	// (an empty debug log)
}

C3DFadeObject::~C3DFadeObject() {}							// 0x431390

namespace neg {
CGameObject *New_C3DFadeObject(void) { return new C3DFadeObject; }	// '3FAD' (0x7b0 bytes)

void FadeObjectTrigger(CGameObject *o)
{
	C3DFadeObject *f = o ? dynamic_cast<C3DFadeObject *>(o) : NULL;	// (o - 0xc8)
	if (f) f->Trigger();
}
}

// --------------------------------------------------------------------------- slots

void C3DFadeObject::InitObject(void)						// 0x4313e0
{
	C3DTriggerType::InitObject();
	RegisterProperty("FadeInTime", &m_fade_in_time, PROP_FLOAT, 0);
	RegisterProperty("StayBlack", &m_stay_black, PROP_FLOAT, 0);
}

void C3DFadeObject::Trigger(void)							// 0x431420
{
	if (g_cutscene_skipped)
	{
		CallNextTrigger(m_next_trigger, 0);					// element +0xdc
		return;
	}
	StartTriggerFade(m_fade_type, m_fade_time, m_stay_black, m_fade_in_time, m_next_trigger);
}
