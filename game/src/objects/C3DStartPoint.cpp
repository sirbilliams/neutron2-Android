/*  C3DStartPoint - see C3DStartPoint.h. Reconstructed from Neutron2.exe 0x473750..0x473da0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DStartPoint.h"
#include "../C3DTriggerType.h"
#include "../C3DPlayer.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
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

// --------------------------------------------------------------------------- the player

// C3DPlayer (a C3DObject: element = object - 0xc0) members at their 2002 element offsets:
// +0x9c8 the start point Jimmy is to enter the level at, +0xb88 a flag that the level was
// entered by "RETURN" (CLoadLevel) - a start trigger is then not played again.
// (callers check NameContains("C3DPLAYER") first; port: dynamic_cast)
static C3DPlayer *Player(CGameObject *o)	{ return o ? dynamic_cast<C3DPlayer *>(o) : NULL; }
static const char *PlayerStartPoint(CGameObject *player)
{
	C3DPlayer *p = Player(player);
	return p ? p->f_9c8 : "";
}
static unsigned char PlayerReturnFlag(CGameObject *player)
{
	C3DPlayer *p = Player(player);
	return p ? p->f_b88 : 0;
}
static void PlayerClearReturnFlag(CGameObject *player)
{
	if (C3DPlayer *p = Player(player)) p->f_b88 = 0;
}

// --------------------------------------------------------------------------- construction

C3DStartPoint::C3DStartPoint()								// 0x473750
{
	m_viewport_pos[3] = 1.0f;								// 0x60c
	m_viewport_rot[3] = 1.0f;								// 0x61c
	SetName((char *)"C3DSTARTPOINT");
	Log("C3DStartPoint()");
	C3DStartPoint::InitObject();							// (called directly: the properties are
															// registered here and again by the loader)
	SetObjectID('STRT');									// element +0xc0
	strcpy(m_sprite_db, "Icons.omt");
	m_sprite_index = 5;
	m_sprite_size = 100;
	InitSpriteProperties(200.0f);
	v67(250.0f);
	v46(0);
	v32(0);
	v52(0);
	strcpy(m_music_db, "none");
	m_music_index = 0;
	m_viewport_pos[0] = 0; m_viewport_pos[1] = 0; m_viewport_pos[2] = 0;
	m_viewport_rot[0] = 0; m_viewport_rot[1] = 0; m_viewport_rot[2] = 0;
	f_008 = 1;												// element +0xd0
	strcpy(m_start_trigger, "none");
	m_trigger_armed = 0;
	C3DSprite::InitPhysics();
	f_6ed = 0;
}

C3DStartPoint::~C3DStartPoint() {}							// 0x473980

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DStartPoint::InitObject(void)						// 0x4739c0
{
	Log("InitObject()");
	C3DSprite::InitObject();
	RegisterProperty("ViewportPx",    &m_viewport_pos[0], PROP_FLOAT, 0);
	RegisterProperty("ViewportPy",    &m_viewport_pos[1], PROP_FLOAT, 0);
	RegisterProperty("ViewportPz",    &m_viewport_pos[2], PROP_FLOAT, 0);
	RegisterProperty("ViewportRx",    &m_viewport_rot[0], PROP_FLOAT, 0);
	RegisterProperty("ViewportRy",    &m_viewport_rot[1], PROP_FLOAT, 0);
	RegisterProperty("ViewportRz",    &m_viewport_rot[2], PROP_FLOAT, 0);
	RegisterProperty("MusicDatabase", m_music_db,         PROP_ALPHA, 0);
	RegisterProperty("MusicIndex",    &m_music_index,     PROP_LONG,  0);
	RegisterProperty("StartTrigger",  m_start_trigger,    PROP_ALPHA, 0);
}

void C3DStartPoint::Process(float dt)						// 0x473700
{
	C3DSprite::Process(dt);
}

void C3DStartPoint::v258(void)								// 0x473ad0
{
	C3DSprite::v258();										// 0x4735a0
	m_viewport_pos[0] = g_viewport->getx();					// element +0x44
	m_viewport_pos[1] = g_viewport->gety();
	m_viewport_pos[2] = g_viewport->getz();
}

// 0x473b10: one frame after Jimmy arrived here, take the camera from him and fire StartTrigger.
void C3DStartPoint::v266(void)
{
	if (!m_trigger_armed) return;
	m_trigger_armed = 0;
	if (strcasecmp(m_start_trigger, "none") == 0) return;
	// (debug logs "StartPoint with %s", "StartPoint found with StartTrigger %s for %s")
	CGameObject *o = FindObjectByTag(m_start_trigger);
	if (!o)
	{
		ErrorLog("ERROR: StartTrigger not found");
		return;
	}
	if (!o->NameContains((char *)"C3DTRIGGERTYPE")) return;
	g_gametype->SetCamera(NULL);							// element +0x124
	static_cast<C3DTriggerType *>(o)->CallNextTrigger(m_start_trigger, 0);	// element +0xdc
}

// 0x473bd0: on (re)load, unless a savegame is being restored: when the camera is Jimmy and he
// enters the level at this start point, arm StartTrigger (and take the camera from him) - but
// not when he came back by a "RETURN" exit, which only clears that flag.
void C3DStartPoint::v260(void)
{
	C3DSprite::v260();										// 0x49cfd0
	if (g_task_file[0]) return;								// 0x54e71c

	if (strcasecmp(m_start_trigger, "none") != 0)
	{
		// (debug log "StartPoint with %s")
		CGameObject *o = FindObjectByTag(m_start_trigger);
		if (o)
		{
			if (!g_camera) return;
			if (g_camera->NameContains((char *)"C3DPLAYER") && o->NameContains((char *)"C3DTRIGGERTYPE")
				&& strcasecmp(PlayerStartPoint(g_camera), m_tag) == 0)
			{
				if (!PlayerReturnFlag(g_camera))
				{
					g_gametype->SetCamera(NULL);
					m_trigger_armed = 1;
				}
				else
					PlayerClearReturnFlag(g_camera);
			}
		}
	}

	if (g_camera && g_camera->NameContains((char *)"C3DPLAYER")
		&& strcasecmp(PlayerStartPoint(g_camera), m_tag) == 0 && PlayerReturnFlag(g_camera))
		PlayerClearReturnFlag(g_camera);
}

void C3DStartPoint::v270(int a2, int a3)					// 0x473d50
{
	CLocalGameObject::v270(a2, a3);
}

namespace neg {
CGameObject *New_C3DStartPoint(void)	{ return new C3DStartPoint; }		// factory 'STRT' (0x6f8 bytes)
}
