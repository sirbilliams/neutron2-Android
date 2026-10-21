/*  C3DCheckPoint - see C3DCheckPoint.h. Reconstructed from Neutron2.exe 0x426d50..0x427600. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCheckPoint.h"
#include "C3DPatrolPoint.h"
#include "../C3DJimmy.h"
#include "../C3DGoddard.h"
#include "../SaveGame.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaStreamOperators.h"
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

C3DCheckPoint::C3DCheckPoint()								// 0x426d50
{
	SetName((char *)"C3DCHECKPOINT");
	Log("C3DCheckPoint()");
	C3DCheckPoint::InitObject();							// (called directly, as in the original)
	SetObjectID('3CHK');									// element slot 48
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 206;
	m_sprite_size = 300;
	v42(0);
	v40(0);
	v52(1);
	v107(1);
	v34(1);
	v46(0);
	f_008 = 1;												// element +0xd0
	v67((float)m_sprite_size);
	m_checked = 0;
	m_always_autosave = 0;
	strcpy(m_send_goddard_to, "none");
	m_checkpoint_num = -1;
	C3DSprite::InitPhysics();								// 0x49ca30
	f_6e0 = 0;
	m_reset_when_damaged = -1;
	m_fall_speed_damage = -1.0f;
	m_camera_type = -1;
	f_674 = 1;
	// (an empty debug log)
}

C3DCheckPoint::~C3DCheckPoint() {}							// 0x426f60

namespace neg {
CGameObject *New_C3DCheckPoint(void) { return new C3DCheckPoint; }	// '3CHK' (0x6f8 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DCheckPoint::InitObject(void)						// 0x427290
{
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("SendGoddardTo", m_send_goddard_to, PROP_ALPHA, 0);
	RegisterProperty("CheckpointNum", &m_checkpoint_num, PROP_LONG, 0);
	RegisterProperty("ResetWhenDamaged", &m_reset_when_damaged, PROP_LONG, 0);
	RegisterProperty("FallSpeedDamage", &m_fall_speed_damage, PROP_FLOAT, 0);
	RegisterProperty("CameraType", &m_camera_type, PROP_LONG, 0);
	RegisterProperty("AlwaysAutoSave", &m_always_autosave, PROP_LONG, 0);
}

void C3DCheckPoint::AddTouching(CGameObject *other)			// 0x426fb0
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (other != g_camera) return;
	if (g_game_mode == 5)									// playing
	{
		Touched(other);										// element +0xe0
		return;
	}
	g_checkpoint_pending = 1;								// 0x545f3c pending until game mode 5 (v270)
	g_checkpoint_toucher = other;							// 0x545f40
}

unsigned char C3DCheckPoint::v231(CGameObject *other)		// 0x43d610
{
	return g_camera == other;
}

void C3DCheckPoint::Process(float dt)						// 0x426fa0
{
	C3DSpriteType::Process(dt);								// 0x473700
}

void C3DCheckPoint::v260(void)								// 0x427470
{
	C3DSpriteType::v260();									// 0x473650
	v67((float)m_sprite_size);								// +0x10c
	m_checked = 0;
	g_checkpoint_num = 0;									// 0x545f38
}

void C3DCheckPoint::v270(int prev, int mode)				// 0x427560 (no base call)
{
	if (mode == 5 && g_checkpoint_pending && prev != 2)
	{
		Touched(g_checkpoint_toucher);
		g_checkpoint_pending = 0;
	}
}

void C3DCheckPoint::WriteState(OMediaStreamOperators &s)	// 0x427510
{
	CLocalGameObject::WriteState(s);
	bool checked = m_checked != 0;
	s << checked;
	if (m_always_autosave != 1) m_always_autosave = 0;
	s << m_always_autosave;
}

// 0x4274a0: Jimmy's checkpoint sends Goddard again.
void C3DCheckPoint::ReadState(OMediaStreamOperators &s)
{
	CLocalGameObject::ReadState(s);
	bool checked;
	s >> checked;
	m_checked = checked;
	CGameObject *jim = FindObjectByTag("JIM1");
	C3DJimmy *j = jim ? dynamic_cast<C3DJimmy *>(jim) : NULL;	// (JIM1 game object +0x4b44)
	if (j && j->f_4c04 == static_cast<CGameObject *>(this)) SendGoddard();	// element +0xdc
	s >> m_always_autosave;
}

// --------------------------------------------------------------------------- element family

// 0x427000: checked by `who` (the camera). The first time (or every time with AlwaysAutoSave):
// Jimmy's checkpoint state from ours, then the autosave with `who` on the checkpoint.
void C3DCheckPoint::Touched(CGameObject *who)
{
	// (debug log "Point checked for number %d")
	if (m_checked && !m_always_autosave) return;
	CGameObject *jim = FindObjectByTag("JIM1");
	C3DJimmy *j = jim ? dynamic_cast<C3DJimmy *>(jim) : NULL;	// port: the original writes into any JIM1
	if (j)
	{
		j->f_4c04 = static_cast<CGameObject *>(this);		// game object +0x4b44
		float r[4];
		jim->v201(r);										// +0x324: his rotation
		j->f_4d80.x = r[0]; j->f_4d80.y = r[1]; j->f_4d80.z = r[2]; j->f_4d80.w = r[3];
		if (m_camera_type != -1) j->f_c1c = m_camera_type;
		float pos[4], rot[4], p[4];
		who->v195(pos);										// +0x30c: where who is
		v195(p);
		who->v196(p[0], p[1], p[2], p[3]);					// +0x310: on the checkpoint
		who->v201(rot);
		if (g_cur_gadget != 1)								// 0x545d9c
		{
			v201(p);
			who->v202(p[0], p[1], p[2], p[3]);				// +0x328: facing its way
		}
		if (m_reset_when_damaged != -1)
		{
			j->f_4dac = m_reset_when_damaged;
			// (debug log "MEBTEMP ResetWhenDamaged= %d")
		}
		if (m_fall_speed_damage != -1.0f)
		{
			// (debug logs "MEBTEMP 1 / 2 FallSpeedDamage= %d")
			j->f_4db0 = m_fall_speed_damage;
		}
		SaveTask("autosave.tsk");							// 0x4983f0
		who->v202(rot[0], rot[1], rot[2], rot[3]);			// back as it was
		who->v196(pos[0], pos[1], pos[2], pos[3]);
		j->f_4c98 = 0;										// game object +0x4bd8
		j->f_4c94 = 0;										// +0x4bd4
		g_5461cc = 0;
		if (m_checkpoint_num != -1) g_checkpoint_num = m_checkpoint_num;
	}
	if (!m_checked)
	{
		SendGoddard();										// element +0xdc
		f_674 = 0;
	}
	m_checked = 1;
}

// 0x427340: SendGoddardTo "WAIT": Goddard waits (state 1); "FOLLOW": follows (4); a patrol point
// of the path network (PatrolType 1): walks there by path (13), or follows when there is no
// path; another object: patrols from there (3).
void C3DCheckPoint::SendGoddard(void)
{
	CGameObject *go = FindObjectByTag("C3DGODDARD");
	C3DGoddard *g = go ? dynamic_cast<C3DGoddard *>(go) : NULL;	// (- 0xc0)
	if (!g) return;
	if (strcasecmp(m_send_goddard_to, "none") == 0) return;
	if (strcasecmp(m_send_goddard_to, "WAIT") == 0)
	{
		g->ev107(1);										// element +0x1ac
		return;
	}
	if (strcasecmp(m_send_goddard_to, "FOLLOW") == 0)
	{
		g->ev107(4);
		return;
	}
	CGameObject *to = FindObjectByTag(m_send_goddard_to);
	if (!to) return;
	C3DPatrolPoint *pp = dynamic_cast<C3DPatrolPoint *>(to);	// port: the original reads +0x7fc of any object
	if (pp && pp->m_patrol_type == 1)
	{
		if (!g->FindBestPath(to)) g->ev107(4);				// element +0x214
		else g->ev107(13);
		g->f_1b34 = 0;
		return;
	}
	g->ev107(3);
	strcpy(g->f_9ac, m_send_goddard_to);					// PatrolPoint
	g->f_b8c = NULL;
}
