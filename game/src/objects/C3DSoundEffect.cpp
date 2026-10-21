/*  C3DSoundEffect - see C3DSoundEffect.h. Reconstructed from Neutron2.exe 0x470b50..0x471300
    (and C3DHelpSound's shared 0x43d610, 0x43dd60, 0x43e180..0x43e1d0). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DSoundEffect.h"
#include "../C3DPlayer.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaStreamOperators.h"
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

C3DSoundEffect::C3DSoundEffect()							// 0x470b50
{
	SetName((char *)"C3DSOUNDEFFECT");
	Log("C3DSoundEffect()");
	C3DSoundEffect::InitObject();							// 0x470de0 (called directly)
	SetObjectID('3SOU');									// 0x49b550
	m_sprite_index = 8;
	m_sprite_size = 50;
	strcpy(m_sprite_db, "icons.omt");
	m_sound_index = -1;
	v67(250.0f);											// radius
	v46(0);
	v42(0);
	v40(0);
	v52(1);
	m_times_to_trigger = -1;
	m_times_triggered = 0;
	f_008 = 1;
	C3DSprite::InitPhysics();								// 0x49ca30
	v34(1);
	m_is_ambient = 0;
	m_required_level = 0;
	m_channel = -1;
	strcpy(m_sound_name, "none");
	m_quake_time = 0;
	m_touch_activated = 1;
	m_is_voice = 1;
	// (then a call to the empty 0x463890)
}

C3DSoundEffect::~C3DSoundEffect()							// 0x470d50
{
	if (m_channel != -1) StopChannel(m_channel, false);		// 0x4b88e0
}

namespace neg {
CGameObject *New_C3DSoundEffect(void) { return new C3DSoundEffect; }	// '3SOU' (0x6f8 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DSoundEffect::InitObject(void)						// 0x470de0
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("SoundIndex", &m_sound_index, PROP_LONG, 0);
	RegisterProperty("SoundName", m_sound_name, PROP_ALPHA, 0);
	RegisterProperty("TimesToTrigger", &m_times_to_trigger, PROP_LONG, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("IsAmbient", &m_is_ambient, PROP_LONG, 0);
	RegisterProperty("RequiredLevel", &m_required_level, PROP_LONG, 0);
	RegisterProperty("QuakeTime", &m_quake_time, PROP_FLOAT, 0);
	RegisterProperty("TouchActivated", &m_touch_activated, PROP_LONG, 0);
	RegisterProperty("IsVoice", &m_is_voice, PROP_LONG, 0);
}

void C3DSoundEffect::AddTouching(CGameObject *other)		// 0x470ee0
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (m_touch_activated && other == g_camera) TriggerSound();	// element +0xdc
}

unsigned char C3DSoundEffect::v231(CGameObject *other)		// 0x43d610 (shared code)
{
	return g_camera == other;
}

void C3DSoundEffect::Process(float dt)						// 0x4710c0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSpriteType::Process(dt);								// 0x473700
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

void C3DSoundEffect::v260(void)								// 0x46ff00 (jmp 0x473650)
{
	C3DSpriteType::v260();
}

void C3DSoundEffect::v268(int a2)							// 0x43dd60 (shared code)
{
	(void)a2;
	TriggerSound();											// element +0xdc
}

void C3DSoundEffect::v277(void)								// 0x43e1b0 (shared code)
{
	// (debug log, empty)
	if (m_channel != -1) StopChannel(m_channel, false);		// 0x4b88e0 (the channel is kept: v278 restarts it)
}

void C3DSoundEffect::v278(void)								// 0x43e1d0 (shared code)
{
	// (debug log, empty)
	if (m_channel == -1 || strcasecmp(m_sound_name, "none") == 0) return;
	m_channel = PlayGameSound3DNoDoppler(this, -1, std::string(m_sound_name), true);	// 0x48f0f0
}

void C3DSoundEffect::WriteState(OMediaStreamOperators &s)	// 0x43e180 (shared code)
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	s << m_times_triggered;
}

void C3DSoundEffect::ReadState(OMediaStreamOperators &s)	// 0x471290
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	s >> m_times_triggered;
}

// --------------------------------------------------------------------------- element-family slots

void C3DSoundEffect::TriggerSound(void)						// 0x470f20
{
	if (m_is_ambient) return;
	if (m_times_to_trigger != -1 && m_times_to_trigger <= m_times_triggered) return;
	m_times_triggered++;
	if (m_sound_index != -1)
	{
		if (GetTaskState(m_task_name) < m_required_level) return;	// 0x497a40
		ErrorLog("ERROR: Sound index used for %s index number %d", m_tag, m_sound_index);	// 0x4b3b10
		PlayGameSound(-1, std::string("not available"), false);	// 0x48f420
	}
	else
	{
		if (strcasecmp(m_sound_name, "none") == 0) return;
		if (!m_is_voice) PlayGameSound(-1, std::string(m_sound_name), false);
		else if (!IsChannelPlaying(g_voice_channel))		// 0x4b89d0: one voice at a time
			g_voice_channel = PlayGameSound(-1, std::string(m_sound_name), false);
	}
	if (m_quake_time <= 0) return;
	CGameObject *jim = FindObjectByTag("JIM1");
	if (!jim) return;
	C3DPlayer *p = dynamic_cast<C3DPlayer *>(jim);			// (CGameObject +0xb38 / +0xb40)
	if (!p) return;											// port: the original does not check the type
	p->f_bf8 = m_quake_time;								// camera shake time
	p->f_c00 = 1.0f;										// and strength
}
