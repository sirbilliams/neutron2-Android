/*  C3DHelpSound - see C3DHelpSound.h. Reconstructed from Neutron2.exe 0x43d610, 0x43d720..0x43e2a0
    (and 0x43dd60). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DHelpSound.h"
#include "../C3DTriggerType.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop
#include <string>
#include <string.h>
#include <strings.h>

using namespace neg;

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DHelpSound::C3DHelpSound()								// 0x43d720
{
	SetName((char *)"C3DHELPSOUND");
	Log("C3DHelpSound()");
	C3DHelpSound::InitObject();								// 0x43da30 (called directly)
	SetObjectID('3HEL');									// 0x49b550
	m_sprite_size = 170;
	strcpy(m_sprite_db, "sprites.omt");
	m_sound_index = -1;
	m_sprite_index = 303;
	v67(250.0f);											// radius
	v46(0);
	v42(0);
	v40(0);
	v34(1);
	v52(1);
	m_times_to_trigger = 1;
	m_times_triggered = 0;
	C3DSprite::InitPhysics();								// 0x49ca30
	v34(1);
	m_is_ambient = 0;
	m_channel = -1;
	m_required_level = 0;
	strcpy(m_sound_name, "none");
	m_touch_activated = 1;
	strcpy(m_next_trigger, "none");
	strcpy(m_player_control, "none");
	strcpy(m_disable_help, "none");
	m_alt_viewport = 0;
	// (then a call to the empty 0x463890)
}

C3DHelpSound::~C3DHelpSound()								// 0x43d9a0
{
	if (m_channel != -1) StopChannel(m_channel, false);		// 0x4b88e0
}

namespace neg {
CGameObject *New_C3DHelpSound(void) { return new C3DHelpSound; }		// '3HEL' (0x820 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DHelpSound::InitObject(void)							// 0x43da30
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("SoundIndex", &m_sound_index, PROP_LONG, 0);
	RegisterProperty("SoundName", m_sound_name, PROP_ALPHA, 0);
	RegisterProperty("TimesToTrigger", &m_times_to_trigger, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("IsAmbient", &m_is_ambient, PROP_LONG, 0);
	RegisterProperty("RequiredLevel", &m_required_level, PROP_LONG, 0);
	RegisterProperty("TouchActivated", &m_touch_activated, PROP_LONG, 0);
	RegisterProperty("NextTrigger", m_next_trigger, PROP_ALPHA, 0);
	RegisterProperty("AltViewport", &m_alt_viewport, PROP_LONG, 0);
	RegisterProperty("PlayerControl", m_player_control, PROP_ALPHA, 0);
	RegisterProperty("DisableHelp", m_disable_help, PROP_ALPHA, 0);
}

void C3DHelpSound::AddTouching(CGameObject *other)			// 0x43db60
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (!m_touch_activated || other != g_camera) return;
	if (m_times_to_trigger != -1 && m_times_to_trigger <= m_times_triggered) return;
	if (strcasecmp(m_next_trigger, "none") == 0)
	{
		if (g_game_mode != 5) return;						// playing only
		TriggerHelp();										// element +0xdc
		m_times_triggered++;								// (TriggerHelp counted it already: twice per touch, as shipped)
		if (m_times_to_trigger != -1 && m_times_to_trigger <= m_times_triggered)
		{
			HideAgain();									// element +0xd8
			v136(0);										// +0x220
		}
		return;
	}
	if (g_game_mode != 5) return;
	TriggerHelp();
	m_times_triggered++;
	CGameObject *t = FindObjectByTag(m_next_trigger);
	if (!t || !t->NameContains((char *)"C3DTRIGGERTYPE")) return;	// +0x18
	if (g_cutscene_element) return;							// 0x545f44: a cut scene is on
	if (m_times_to_trigger != -1 && m_times_to_trigger <= m_times_triggered)
	{
		HideAgain();
		v136(0);
	}
	PlayGameSound(-1, std::string("bring"), false);			// 0x48f420
	if (strcasecmp(m_disable_help, "none") != 0)
	{
		CGameObject *h = FindObjectByTag(m_disable_help);
		if (h && h->NameContains((char *)"C3DHELPSOUND"))
		{
			C3DSprite *s = dynamic_cast<C3DSprite *>(h);	// (h - 0xc8)
			if (s) s->HideAgain();							// element +0xd8
			h->v136(0);										// +0x220
		}
	}
	C3DTriggerType *tt = dynamic_cast<C3DTriggerType *>(t);	// (t - 0xc8)
	if (!tt) return;										// port: checked (NameContains said it is one)
	tt->CallNextTrigger(m_next_trigger, m_alt_viewport != 0);	// element +0xdc
}

unsigned char C3DHelpSound::v231(CGameObject *other)		// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DHelpSound::Process(float dt)						// 0x43dea0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
	AddRotation(0, g_enemy_time_scale * dt * 140.0f, 0);	// +0x330: spin (0x540594, 0x4e0f48)
	if (!m_is_ambient) return;
	if (!g_sfx_db_loaded)									// 0x54a4b8
	{
		if (m_channel != -1)
		{
			StopChannel(m_channel, false);					// 0x4b88e0
			m_channel = -1;
		}
		else if (strcasecmp(m_sound_name, "none") != 0)
			StopChannel(m_channel, false);					// (sic: channel -1)
		return;
	}
	if (IsChannelPlaying(m_channel) || m_channel != -1) return;	// 0x4b89d0
	if (GetTaskState(m_task_name) < m_required_level) return;	// 0x497a40
	if (m_sound_index != -1)
	{
		ErrorLog("ERROR: Sound index used for %s index number %d", m_tag, m_sound_index);	// 0x4b3b10
		m_channel = PlayGameSound3DNoDoppler(this, -1, std::string("not available"), true);	// 0x48f0f0
		return;
	}
	if (strcasecmp(m_sound_name, "none") == 0) return;
	m_channel = PlayGameSound3DNoDoppler(this, -1, std::string(m_sound_name), true);
	if (m_channel == -1)
		PlayGameSound3DNoDoppler(this, -1, std::string("not available"), true);	// (its channel is not kept)
}

void C3DHelpSound::v260(void)								// 0x43e080
{
	C3DSpriteType::v260();									// 0x473650
	if (g_windowed)											// 0x56afab
	{
		set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);	// +0xb4 = 6, +0xb8 = 7
		set_flags(omelf_SecondPass);						// +0x7c = 8
		return;
	}
	set_diffuse(OMediaFARGBColor(0.7f, 1.0f, 1.0f, 1.0f));	// +0x90.. (r, g, b, a) = (1, 1, 1, 0.7)
	set_blend(omblendfc_Src_Alpha, omblendfc_One);			// +0xb4 = 6, +0xb8 = 1
	set_flags(omelf_SecondPass);							// +0x7c = 8
}

void C3DHelpSound::v268(int a2)								// 0x43dd60 (shared code)
{
	(void)a2;
	TriggerHelp();											// element +0xdc
}

void C3DHelpSound::v277(void)								// 0x43e1b0
{
	// (debug log, empty)
	if (m_channel != -1) StopChannel(m_channel, false);		// 0x4b88e0 (the channel is kept: v278 restarts it)
}

void C3DHelpSound::v278(void)								// 0x43e1d0
{
	// (debug log, empty)
	if (m_channel == -1 || strcasecmp(m_sound_name, "none") == 0) return;
	m_channel = PlayGameSound3DNoDoppler(this, -1, std::string(m_sound_name), true);	// 0x48f0f0
}

void C3DHelpSound::WriteState(OMediaStreamOperators &s)		// 0x43e180
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	s << m_times_triggered;
}

void C3DHelpSound::ReadState(OMediaStreamOperators &s)		// 0x43e110
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	s >> m_times_triggered;
	if (m_times_to_trigger == -1) return;
	if (m_times_to_trigger <= m_times_triggered)			// used up
	{
		HideAgain();										// element +0xd8
		v136(0);											// +0x220
		return;
	}
	ShowAgain();											// element +0xd4
	v136(1);
}

// --------------------------------------------------------------------------- element-family slots

void C3DHelpSound::TriggerHelp(void)						// 0x43dd80
{
	if (m_is_ambient) return;
	if (m_times_to_trigger != -1 && m_times_to_trigger <= m_times_triggered) return;
	m_times_triggered++;
	if (m_sound_index != -1)
	{
		if (GetTaskState(m_task_name) < m_required_level) return;	// 0x497a40
		ErrorLog("ERROR: Sound index used for %s index number %d", m_tag, m_sound_index);	// 0x4b3b10
		PlayGameSound(-1, std::string("not available"), false);	// 0x48f420
		return;
	}
	if (strcasecmp(m_sound_name, "none") == 0) return;
	PlayGameSound(-1, std::string("bring"), false);
	if (IsChannelPlaying(g_voice_channel)) return;			// 0x4b89d0: one help voice at a time
	g_voice_channel = PlayGameSound(-1, std::string(m_sound_name), false);
}
