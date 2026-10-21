/*  C3DFogTrigger - see C3DFogTrigger.h. Reconstructed from Neutron2.exe 0x435fd0..0x436540
    (and 0x43dd60, 0x45ce70). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFogTrigger.h"
#include "../CJimmyGame.h"
#include "../CGameType.h"
#include "../Sound.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "../levels/LevelHelpers.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// 0x5367a0 (= -1): the fog sound's channel, one for all fog triggers.
#define g_fog_sound_channel g_5367a0

// --------------------------------------------------------------------------- construction

C3DFogTrigger::C3DFogTrigger()								// 0x435fd0
{
	SetName((char *)"C3DFOGTRIGGER");
	Log("C3DFogTrigger()");
	C3DFogTrigger::InitObject();							// (called directly, as in the original)
	SetObjectID('3FOG');
	m_sprite_index = 12;
	m_sprite_size = 100;
	strcpy(m_sprite_db, "icons.omt");
	v67(350.0f);
	v46(0);
	v42(0);
	v40(0);
	v52(1);
	f_008 = 1;												// element +0xd0
	C3DSprite::InitPhysics();								// 0x49ca30
	m_clip_dist = 12000.0f;
	m_start_fog = 1000.0f;
	m_end_fog = 10000.0f;
	m_red = 1.0f;
	m_green = 1.0f;
	m_blue = 1.0f;
	m_enable_fog = 1;
	strcpy(m_fog_sound, "none");
	m_lighting_val = 1.0f;
	m_change_lighting = 0;
	v34(1);
	// (then a call to the empty 0x463890)
}

C3DFogTrigger::~C3DFogTrigger()								// 0x4361e0
{
	if (g_fog_sound_channel != -1)
	{
		StopChannel(g_fog_sound_channel, false);			// 0x4b88e0
		g_fog_sound_channel = -1;
	}
}

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DFogTrigger::InitObject(void)						// 0x436280
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("Red",            &m_red,             PROP_FLOAT, 0);
	RegisterProperty("Green",          &m_green,           PROP_FLOAT, 0);
	RegisterProperty("Blue",           &m_blue,            PROP_FLOAT, 0);
	RegisterProperty("Clipdist",       &m_clip_dist,       PROP_FLOAT, 0);
	RegisterProperty("StartFog",       &m_start_fog,       PROP_FLOAT, 0);
	RegisterProperty("EndFog",         &m_end_fog,         PROP_FLOAT, 0);
	RegisterProperty("EnableFog",      &m_enable_fog,      PROP_LONG,  0);
	RegisterProperty("FogSound",       m_fog_sound,        PROP_ALPHA, 0);
	RegisterProperty("LightingVal",    &m_lighting_val,    PROP_FLOAT, 0);
	RegisterProperty("ChangeLighting", &m_change_lighting, PROP_LONG,  0);
}

void C3DFogTrigger::AddTouching(CGameObject *other)			// 0x4364c0
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (g_camera && g_camera == other) TriggerFog();		// element +0xdc
}

unsigned char C3DFogTrigger::v231(CGameObject *other)		// 0x45ce70 (shared code)
{
	if (g_camera == other) return 1;
	return other->m_object_id == '3JIM';
}

void C3DFogTrigger::v268(int a2)							// 0x43dd60 (shared code)
{
	(void)a2;
	TriggerFog();											// element +0xdc
}

// --------------------------------------------------------------------------- element-family slots

// 0x436390: the level's fog to this trigger's (alpha 0), the terrain lighting, the fog sound.
void C3DFogTrigger::TriggerFog(void)
{
	if (!m_enable_fog)
	{
		levels::SetLayerFog(g_layer_world, false);			// (0x56afd4) 0x4b1860(layer, 0x800)
		return;
	}
	// (debug log "Calling triggerfog")
	CJimmyGame *game = static_cast<CJimmyGame *>(g_gametype);
	game->SetFog(m_red, m_green, m_blue, 0.0f, m_start_fog, m_end_fog, m_clip_dist, true);	// element +0x194
	if (m_change_lighting) game->SetTerrainGrey(m_lighting_val);	// element +0x178
	if (g_fog_sound_channel != -1)
	{
		StopChannel(g_fog_sound_channel, false);			// 0x4b88e0
		g_fog_sound_channel = -1;
	}
	if (strcasecmp(m_fog_sound, "none") != 0 && g_fog_sound_channel == -1)
		g_fog_sound_channel = PlayGameSound(-1, std::string(m_fog_sound), true);	// 0x48f420
}

namespace neg {
CGameObject *New_C3DFogTrigger(void)	{ return new C3DFogTrigger; }	// factory '3FOG' (0x6f8 bytes)
}
