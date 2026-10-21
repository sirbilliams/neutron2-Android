/*  C3DAITrigger - see C3DAITrigger.h. Reconstructed from Neutron2.exe 0x418000..0x41a5e0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DAITrigger.h"
#include "../C3DObject.h"
#include "../C3DJimmy.h"
#include "../C2DInGameMenu.h"
#include "../CGameType.h"
#include "../SaveGame.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaStreamOperators.h"
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

// 0x404dd0: set the type of item `item` of menu page `page`. The pickups come in pairs (10/21
// and 31/38): giving one also gives (and counts) the other.
static void SetItemState(short page, short item, short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p) p->items[item].type = state;
	if (page != C2DInGameMenu::MENU_PICKUPS) return;
	C2DInGameMenu::MenuPage *pk = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
	if (item == 10)
	{
		if (!pk || !pk->items[21].type) { SetItemState(2, 21, 1); g_pickup_counts[2 * 21]++; }
	}
	else if (item == 21)
	{
		if (!pk || !pk->items[10].type) { SetItemState(2, 10, 1); g_pickup_counts[2 * 10]++; }
	}
	else if (item == 38)
	{
		if (!pk || !pk->items[31].type) { SetItemState(2, 31, 1); g_pickup_counts[2 * 31]++; }
	}
	else if (item == 31)
	{
		if (!pk || !pk->items[38].type) { SetItemState(2, 38, 1); g_pickup_counts[2 * 38]++; }
	}
}

// 0x404ef0: item `item` of page `page` disabled.
static void ClearItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p;
	if (item >= 0 && item < C2DInGameMenu::MENU_ITEMS && (p = C2DInGameMenu::Page(page)) != NULL)
		p->items[item].type = 0;
}

// The C3DObject (element) of a game object: the original subtracts 0xc0 without looking.
static C3DObject *Obj(CGameObject *o)		{ return o ? dynamic_cast<C3DObject *>(o) : NULL; }

// --------------------------------------------------------------------------- not reconstructed yet

// C3DObject-family element slots of classes not reconstructed yet (C3DAnimated, C3DAI,
// C3DGrannyObj, C3DJimmy, C3DLibby...), and their members, at their 2002 element offsets.
static void ObjSetAnimation(CGameObject *o, const char *name, bool loop)	// +0xe8 (slot 58)
{
	(void)o; (void)name; (void)loop; NEG_TODO(0x418c6e, "C3DAnimated element slot 58 (set animation)");
}
static void ObjSetLit(CGameObject *o, float lit)							// +0x120 (slot 72)
{
	(void)o; (void)lit; NEG_TODO(0x419221, "C3DObject element slot 72 (lit)");
}
static void ObjFadeIn(CGameObject *o, float t)								// +0x12c (slot 75)
{
	(void)o; (void)t; NEG_TODO(0x41930f, "C3DObject element slot 75 (fade in)");
}
static void ObjFadeOut(CGameObject *o, float t)							// +0x130 (slot 76)
{
	(void)o; (void)t; NEG_TODO(0x419350, "C3DObject element slot 76 (fade out)");
}
static void AISetState(CGameObject *ai, int state)							// C3DAI +0x1ac (slot 107)
{
	(void)ai; (void)state; NEG_TODO(0x4195ce, "C3DAI element slot 107 (set state)");
}
static int AIInitialState(CGameObject *ai)									// C3DAI +0xc08
{
	(void)ai; NEG_TODO(0x418d09, "C3DAI +0xc08 (initial state)"); return 0;
}
static void AISetHover(CGameObject *ai, unsigned char on)					// C3DAI +0xbd4
{
	(void)ai; (void)on; NEG_TODO(0x419528, "C3DAI +0xbd4 (hover)");
}
static void AISetSpeed(CGameObject *ai, float speed)						// C3DAI +0x968
{
	(void)ai; (void)speed; NEG_TODO(0x4195f4, "C3DAI +0x968 (speed)");
}
static void AISetPatrolTarget(CGameObject *ai, CGameObject *p)				// C3DAI +0x214 (slot 133), AIState 13
{
	(void)ai; (void)p; NEG_TODO(0x419642, "C3DAI element slot 133 (patrol target)");
}
static void AISetPatrolPoint(CGameObject *ai, const char *name)			// C3DAI +0x9ac = name, +0xb8c = 0
{
	(void)ai; (void)name; NEG_TODO(0x419689, "C3DAI +0x9ac / +0xb8c (patrol point)");
}
static int GrannyObjFlag(CGameObject *o)									// C3DGrannyObj +0x6d0
{
	(void)o; NEG_TODO(0x419143, "C3DGrannyObj +0x6d0"); return 0;
}
static void GrannyAttach(CGameObject *holder, CGameObject *o, float detach_time)	// +0x180 (slot 96), +0xdac
{
	(void)holder; (void)o; (void)detach_time; NEG_TODO(0x41916b, "C3DGrannyObj element slot 96 (attach) / +0xdac");
}
static void GrannyDetach(CGameObject *holder, CGameObject *o)				// +0x188 (slot 98)(o, 0)
{
	(void)holder; (void)o; NEG_TODO(0x419202, "C3DGrannyObj element slot 98 (detach)");
}
// C3DJimmy element slot 138 (+0x228)(caller, other, item, t): hand Jimmy gadget `item` (0: put
// it away; 1 rocket, 2 invisibility, 3 jet pack, 4 scooter, 6 water gun, 9 sub, 11 crane,
// 13 lunar lander), `t` = 13.
static C3DJimmy *AsJimmy(CGameObject *o)	{ return o ? dynamic_cast<C3DJimmy *>(o) : NULL; }
static void JimmyUseItem(CGameObject *jimmy, CGameObject *caller, CGameObject *other, int item, float t)
{
	if (C3DJimmy *j = AsJimmy(jimmy)) j->ev138(caller, other, item, t);	// (the original calls through a NULL jimmy)
}
static void JimmySlot105(CGameObject *jimmy)								// +0x1a4: back to the checkpoint
{
	C3DPlayerObject *p = jimmy ? dynamic_cast<C3DPlayerObject *>(jimmy) : NULL;
	if (p) p->ev105();
}
static void JimmySlot116(CGameObject *o)									// +0x1d0 of the camera
{
	// (the original calls element slot 116 of whatever the camera is; only C3DPlayers have one)
	C3DPlayer *p = o ? dynamic_cast<C3DPlayer *>(o) : NULL;
	if (p) p->ev116();
}
static CGameObject *JimmyJetpack(CGameObject *jimmy)						// +0xccc (element pointer)
{
	C3DJimmy *j = AsJimmy(jimmy);
	return j && j->f_ccc ? static_cast<CGameObject *>(j->f_ccc) : NULL;
}
static CGameObject *JimmyShownTarget(CGameObject *jimmy)					// +0x4ef0: the target Goddard points at
{
	C3DJimmy *j = AsJimmy(jimmy);
	return j ? j->f_4ef0 : NULL;
}
static void JimmySetShownTarget(CGameObject *jimmy, CGameObject *t)
{
	if (C3DJimmy *j = AsJimmy(jimmy)) j->f_4ef0 = t;
}
static void JimmySetStartPoints(CGameObject *jimmy, const char *name)		// +0x9c8, +0xa18, +0xb24
{
	C3DPlayer *p = jimmy ? dynamic_cast<C3DPlayer *>(jimmy) : NULL;
	if (!p) return;
	strcpy(p->f_9c8, name);
	strcpy(p->f_a18, name);
	strcpy(p->f_b24, name);
}
static CGameObject *GoddardPointer(CGameObject *goddard)					// C3DGoddard +0x1b84 (object)
{
	(void)goddard; NEG_TODO(0x418fb0, "C3DGoddard +0x1b84 (pointer object)"); return NULL;
}
static void LibbySlot(CGameObject *libby, bool digitize)					// C3DLibby +0x24c / +0x250 (slots 147/148)
{
	(void)libby; (void)digitize; NEG_TODO(0x41a039, "C3DLibby element slot 147/148 (digitize)");
}
static void MinisubReset(CGameObject *sub)									// C3DMiniSub +0x5a0, +0x934, +0x938 = 0
{
	(void)sub; NEG_TODO(0x419dba, "C3DMiniSub +0x5a0 / +0x934 / +0x938 = 0");
}
// C2DInGameMenu helpers private to the menu module.
static void MenuOpenHelp(float secs)		{ (void)secs; NEG_TODO(0x40a0c0, "C2DInGameMenu open help"); }
static void MenuRestartGame(void)			{ NEG_TODO(0x410540, "C2DInGameMenu 0x410540 (score box, restart)"); }

// --------------------------------------------------------------------------- construction

C3DAITrigger::C3DAITrigger()								// 0x418000
{
	SetName((char *)"C3DAITRIGGER");
	Log("C3DAITrigger()");
	m_obj_fade_time = 1.0f;
	C3DAITrigger::InitObject();								// (called directly, as in the original)
	SetObjectID('3AIT');
	m_sprite_index = 9;
	m_sprite_size = 50;
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_activate_by, "JIM1");
	strcpy(m_activate_by_anim, "none");
	m_activate_state[0] = 0;
	m_activate_state[1] = -1;
	m_activate_state[2] = -1;
	m_activate_state[3] = -1;
	m_activate_state[4] = -1;
	for (int i = 0; i < 5; i++) strcpy(m_activate_object[i], "none");
	strcpy(m_ai_attach_obj, "none");
	strcpy(m_ai_detach_obj, "none");
	strcpy(m_is_a, "none");
	strcpy(m_ai_target, "none");
	strcpy(m_ai_anim, "none");
	strcpy(m_ai_new_pos, "none");
	m_ai_new_rot_y = -1.0f;
	m_ai_hide_obj = -1;
	m_ai_collision = -1;
	strcpy(m_next_trigger, "none");
	strcpy(m_ai_patrol, "none");
	strcpy(m_player_controlled, "none");
	v67(250.0f);
	v46(0);
	v52(1);
	v32(1);
	m_times_to_trigger = -1;
	m_times_triggered = 0;
	f_008 = 1;												// element +0xd0
	C3DSprite::InitPhysics();
	m_touch_activated = 0;
	m_ai_state = 2;
	v34(1);
	m_ai_speed = -1;
	f_db0 = 0;
	m_fade_in = 0;
	m_fade_out = 0;
	m_ai_anim_loop = 1;
	m_ai_hover = -1;
	m_ai_moveable = -1;
	m_detach_time = -1.0f;
	m_show_target = -1;
	m_ai_lit = -1;
}

C3DAITrigger::~C3DAITrigger() {}							// 0x418510

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DAITrigger::InitObject(void)							// 0x418590
{
	C3DTriggerType::InitObject();
	RegisterProperty("Radius",           &f_034,               PROP_FLOAT, 0);
	RegisterProperty("ActivateBy",       m_activate_by,        PROP_ALPHA, 0);
	RegisterProperty("IsA",              m_is_a,               PROP_ALPHA, 0);
	RegisterProperty("ActivateByAnim",   m_activate_by_anim,   PROP_ALPHA, 0);
	RegisterProperty("ActivateState0",   &m_activate_state[0], PROP_LONG,  0);
	RegisterProperty("ActivateObject0",  m_activate_object[0], PROP_ALPHA, 0);
	RegisterProperty("ActivateState1",   &m_activate_state[1], PROP_LONG,  0);
	RegisterProperty("ActivateObject1",  m_activate_object[1], PROP_ALPHA, 0);
	RegisterProperty("ActivateState2",   &m_activate_state[2], PROP_LONG,  0);
	RegisterProperty("ActivateObject2",  m_activate_object[2], PROP_ALPHA, 0);
	RegisterProperty("ActivateState3",   &m_activate_state[3], PROP_LONG,  0);
	RegisterProperty("ActivateObject3",  m_activate_object[3], PROP_ALPHA, 0);
	RegisterProperty("ActivateState4",   &m_activate_state[4], PROP_LONG,  0);
	RegisterProperty("ActivateObject4",  m_activate_object[4], PROP_ALPHA, 0);
	RegisterProperty("AITarget",         m_ai_target,          PROP_ALPHA, 0);
	RegisterProperty("AIAnim",           m_ai_anim,            PROP_ALPHA, 0);
	RegisterProperty("AIAnimLoop",       &m_ai_anim_loop,      PROP_LONG,  0);
	RegisterProperty("AIState",          &m_ai_state,          PROP_LONG,  0);
	RegisterProperty("AISpeed",          &m_ai_speed,          PROP_LONG,  0);
	RegisterProperty("AIPatrol",         m_ai_patrol,          PROP_ALPHA, 0);
	RegisterProperty("AINewPos",         m_ai_new_pos,         PROP_ALPHA, 0);
	RegisterProperty("AINewRotY",        &m_ai_new_rot_y,      PROP_FLOAT, 0);
	RegisterProperty("AIHideObj",        &m_ai_hide_obj,       PROP_LONG,  0);
	RegisterProperty("AILit",            &m_ai_lit,            PROP_LONG,  0);
	RegisterProperty("AICollision",      &m_ai_collision,      PROP_LONG,  0);
	RegisterProperty("AIHover",          &m_ai_hover,          PROP_LONG,  0);
	RegisterProperty("AIMoveable",       &m_ai_moveable,       PROP_LONG,  0);
	RegisterProperty("AIAttachObj",      m_ai_attach_obj,      PROP_ALPHA, 0);
	RegisterProperty("AIDetachObj",      m_ai_detach_obj,      PROP_ALPHA, 0);
	RegisterProperty("DetachTime",       &m_detach_time,       PROP_FLOAT, 0);
	RegisterProperty("ShowTarget",       &m_show_target,       PROP_LONG,  0);
	RegisterProperty("FadeIn",           &m_fade_in,           PROP_LONG,  0);
	RegisterProperty("FadeOut",          &m_fade_out,          PROP_LONG,  0);
	RegisterProperty("ObjFadeTime",      &m_obj_fade_time,     PROP_FLOAT, 0);
	RegisterProperty("PlayerControlled", m_player_controlled,  PROP_ALPHA, 0);
	RegisterProperty("TimesToTrigger",   &m_times_to_trigger,  PROP_LONG,  0);
	RegisterProperty("TouchActivated",   &m_touch_activated,   PROP_LONG,  0);
}

void C3DAITrigger::ResetObject(void)						// 0x418550
{
	CLocalGameObject::ResetObject();
	if (m_touch_activated) v234();							// +0x3a8
	else v232();											// +0x3a0
	m_times_triggered = 0;
}

void C3DAITrigger::AddTouching(CGameObject *other)			// 0x418960
{
	C3DSpriteType::AddTouching(other);						// 0x4736f0
	if (!m_touch_activated) return;
	// (debug log "%s touched by %s")
	Trigger(other, 0);										// element +0xe4
}

void C3DAITrigger::v23(CGameObject *o)						// 0x41a500
{
	(void)o;
	Trigger(NULL, 0);
}

void C3DAITrigger::v268(int a2)								// 0x41a500 (the same function)
{
	(void)a2;
	Trigger(NULL, 0);
}

// 0x4189b0: the trigger stays where it was placed.
void C3DAITrigger::Process(float dt)
{
	C3DSpriteType::Process(dt);								// 0x473700
	float p[4];
	v88(p);													// +0x160
	v196(p[0], p[1], p[2], p[3]);							// +0x310
}

void C3DAITrigger::v242(float t)							// 0x418a00
{
	C3DTriggerType::v242(t);								// 0x478e10
	if (m_touch_activated)
		set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 0.2f, 0.2f));	// +0x90.. (r, g, b, a) = 1, 0.2, 0.2, 1: red
	else
		set_diffuse(OMediaFARGBColor(0.9f, 1.0f, 1.0f, 1.0f));	// 1, 1, 1, 0.9
}

void C3DAITrigger::v267(int a2)								// 0x419790
{
	CLocalGameObject::v267(a2);
	GetTaskState(m_task_name);								// (result unused)
}

void C3DAITrigger::WriteState(OMediaStreamOperators &s)		// 0x41a550
{
	CLocalGameObject::WriteState(s);
	s << m_times_triggered;
}

void C3DAITrigger::ReadState(OMediaStreamOperators &s)		// 0x41a520
{
	CLocalGameObject::ReadState(s);
	s >> m_times_triggered;
}

// --------------------------------------------------------------------------- element-family slots

// 0x418ab0: the first ActivateObject<n> that exists and whose ActivateState<n> (not -1) is the
// state of this trigger's task.
CGameObject *C3DAITrigger::FindActiveObject(void)
{
	const int state = GetTaskState(m_task_name);			// 0x497a40
	for (int i = 0; i < 5; i++)
	{
		CGameObject *o = FindObjectByTag(m_activate_object[i]);
		if (o && m_activate_state[i] != -1 && m_activate_state[i] == state) return o;
	}
	return NULL;
}

// 0x41a4d0: true unless all three gadget worlds are done (0x545f00 = 7, 0x545f04 = 11,
// 0x545f08 = 13; set by SetLevelItems).
bool C3DAITrigger::NotAllGadgetLevelsDone(void)
{
	bool r = false;
	if (g_545f00 != 7) r = true;
	if (g_545f04 != 11) r = true;
	if (g_545f08 != 13) r = true;
	return r;
}

// 0x418b80: fire. `flag` set means a trigger chain asked for it (it is passed on).
void C3DAITrigger::Trigger(CGameObject *caller, unsigned char flag)
{
	bool go = false;
	// (debug log "CALLING AITrigger %s with %s=%d ", m_tag, m_task_name, GetTaskState(m_task_name))
	GetTaskState(m_task_name);
	if (flag) go = true;

	CGameObject *by = FindObjectByTag(m_activate_by);
	if (by)
	{
		if (by == caller)
		{
			// (debug log "%s == %s  continuing %s")
			go = true;
			if (by->NameContains((char *)"C3DANIMATED") && strcasecmp(m_activate_by_anim, "none") != 0)
				ObjSetAnimation(by, m_activate_by_anim, true);	// (debug logs "A4", "Setting %s to animation %s for %s" x2)
		}
		// else (debug log "No Match found for %s not calling trigger")
	}
	else if (strcasecmp(m_activate_by, "none") == 0)
	{
		go = true;											// (debug log "Activate by none, continuing %s")
	}

	if (strcasecmp(m_is_a, "none") != 0)
	{
		// every AI of kind IsA goes to AIState (-1: back to its own initial state)
		if (!go) return;
		for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		{
			CGameObject *o = *it;
			if (!o || !o->NameContains((char *)"C3DAI") || !o->NameContains(m_is_a)) continue;
			if (m_ai_state == -1) AISetState(o, AIInitialState(o));
			else AISetState(o, m_ai_state);
		}
		CGameObject *n = FindActiveObject();				// element +0xe0
		SpecialAction(caller);								// element +0xe8
		if (f_db0)
		{
			CallNextTrigger("default", 0);
			f_db0 = 0;
			return;
		}
		if (n) { CallNextTrigger(n->m_tag, 0); return; }	// (debug logs "16.5", "\tCalling Next Trigger %s for %s")
		CallNextTrigger(m_next_trigger, 0);
		return;
	}

	if (!go) return;
	if (!flag)
	{
		if (m_times_to_trigger != -1 && ++m_times_triggered > m_times_to_trigger) return;
		if (strcasecmp(m_player_controlled, "none") != 0 && strcasecmp(m_player_controlled, "NULL") == 0)
		{
			if (g_camera)
			{
				g_camera->v261();							// +0x414
				if (g_camera->NameContains((char *)"C3DJIMMY"))
				{
					// (debug log "Setting Jim1 animation STOP")
					ObjSetAnimation(g_camera, "STOP", true);
					JimmySlot116(g_camera);
				}
			}
			g_gametype->SetCamera(NULL);
		}
	}

	CGameObject *t;
	if (strcasecmp(m_ai_target, "none") != 0 && (t = FindObjectByTag(m_ai_target)) != NULL)
	{
		// ShowTarget: Goddard points at the target (1) or stops pointing (0)
		if (m_show_target == 1 && g_jimmy && strcasecmp(m_ai_target, "none") != 0)
			JimmySetShownTarget(g_jimmy, FindObjectByTag(m_ai_target));
		if (m_show_target == 0 && g_jimmy && JimmyShownTarget(g_jimmy))
		{
			CGameObject *g = FindObjectByTag("C3DGODDARD");
			if (g)
			{
				C3DObject *p = Obj(GoddardPointer(g));
				if (p) p->HideAgain();						// element +0xdc
			}
			JimmySetShownTarget(g_jimmy, NULL);
		}

		if (m_ai_collision != -1)
		{
			if (m_ai_collision == 0) t->v232();				// +0x3a0
			else { t->v234(); t->v136(1); t->v132(1); }		// +0x3a8, +0x220, +0x210
		}
		if (m_ai_moveable != -1) t->v132(m_ai_moveable != 0);	// +0x210

		if (t->NameContains((char *)"C3DSPRITE"))
		{
			C3DSprite *s = dynamic_cast<C3DSprite *>(t);		// (element = object - 0xc8)
			if (m_ai_hide_obj != -1 && s)
			{
				if (m_ai_hide_obj == 0) s->ShowAgain();		// element +0xd4
				else s->HideAgain();						// element +0xd8
			}
			CGameObject *p = FindObjectByTag(m_ai_new_pos);
			if (p)
			{
				// (debug log "\tSetting position of %s to %s for %s")
				float v[4];
				p->v195(v);
				t->v196(v[0], v[1], v[2], v[3]);
			}
		}

		if (t->NameContains((char *)"C3DANIMATED"))
		{
			if (strcasecmp(m_ai_attach_obj, "none") != 0)
			{
				CGameObject *a = FindObjectByTag(m_ai_attach_obj);
				// port: the original calls through a NULL object when AIAttachObj does not exist.
				if (a && a->NameContains((char *)"C3DGRANNYOBJ") && GrannyObjFlag(a) == 1 && GrannyObjFlag(t) == 1)
					GrannyAttach(a, t, m_detach_time);		// +0x180, then a +0xdac = DetachTime
			}
			if (strcasecmp(m_ai_detach_obj, "none") != 0)
			{
				CGameObject *d = FindObjectByTag(m_ai_detach_obj);
				// port: as above.
				if (d && d->NameContains((char *)"C3DGRANNYOBJ") && GrannyObjFlag(d) == 1 && GrannyObjFlag(t) == 1)
					GrannyDetach(d, t);						// +0x188 (t, 0)
			}
			if (m_ai_lit != -1) ObjSetLit(t, m_ai_lit == 0 ? 0.0f : 1.0f);	// +0x120
			if (m_ai_new_rot_y != -1.0f)
			{
				// (debug log "\tSetting Rotation of %s to %f for %s")
				float a[4], b[4];
				t->v201(a);									// +0x324: current angles
				const float az = a[2], ry = m_ai_new_rot_y;
				t->v201(b);
				t->v203(b[0], ry, az);						// +0x32c
				t->f_274 = b[0];
				t->f_278 = ry;
				t->f_27c = az;
				t->f_280 = 1.0f;
			}
			if (m_fade_in == 1) ObjFadeIn(t, m_obj_fade_time);	// +0x12c (debug log "\tFadeIn called for %s integer time:%d")
			if (m_fade_out == 1) ObjFadeOut(t, m_obj_fade_time);	// +0x130 (debug log "\tFadeOut called ...")
			if (m_ai_hide_obj != -1)
			{
				C3DObject *o = Obj(t);
				if (m_ai_hide_obj == 0) { if (o) o->ShowAgain(); t->v132(1); }	// "\tshowing object %s " (+0xd8, +0x210)
				else { if (o) o->HideAgain(); t->v132(0); }					// "\thiding object %s " (+0xdc)
			}
			if (strcasecmp(m_ai_anim, "none") != 0)
				ObjSetAnimation(t, m_ai_anim, m_ai_anim_loop != 0);	// (debug log "\tSetting sequence of %s to %s for %s")
			CGameObject *p = FindObjectByTag(m_ai_new_pos);
			if (p)
			{
				float v[4];
				p->v195(v);
				t->v196(v[0], v[1], v[2], v[3]);
			}
		}

		if (!t->NameContains((char *)"C3DAITRIGGER") && t->NameContains((char *)"C3DAI"))
		{
			if (m_ai_hover == 0) AISetHover(t, 0);			// "Setting hover to false for %s"
			else if (m_ai_hover == 1) AISetHover(t, 1);
			if (m_ai_hide_obj != -1)
			{
				C3DObject *o = Obj(t);
				if (m_ai_hide_obj == 0) { if (o) o->ShowAgain(); t->v132(1); }
				else { if (o) o->HideAgain(); t->v132(0); }
			}
			// (debug log "setting %s to %d")
			AISetState(t, m_ai_state);						// +0x1ac
			if (m_ai_speed != -1) AISetSpeed(t, (float)m_ai_speed);	// "\tChanging speed of %s to %d for %s"
			if (strcasecmp(m_ai_patrol, "none") != 0)
			{
				if (m_ai_state == 13)
				{
					CGameObject *p = FindObjectByTag(m_ai_patrol);
					if (p) AISetPatrolTarget(t, p);			// +0x214
				}
				else
					AISetPatrolPoint(t, m_ai_patrol);		// "\tSetting partolpoint to %s"
			}
		}
	}

	if (strcasecmp(m_toggle_object, "none") != 0)
	{
		CGameObject *o = FindObjectByTag(m_toggle_object);
		if (o) static_cast<CLocalGameObject *>(o)->v268(m_toggle);	// +0x430 (debug log "Calling ToggleObject %s")
	}

	CGameObject *n = FindActiveObject();					// element +0xe0
	SpecialAction(caller);									// element +0xe8
	if (f_db0)
	{
		CallNextTrigger("default", 0);
		f_db0 = 0;
		return;
	}
	if (n) { CallNextTrigger(n->m_tag, flag); return; }
	CallNextTrigger(m_next_trigger, flag);
}

// 0x4197b0: the actions built into special trigger tags.
void C3DAITrigger::SpecialAction(CGameObject *caller)
{
	const int scene = GetTaskState("SCENE");
	const char *tag = m_tag;
	CGameObject *jim;

	// ---- first group (falls through to the second)
	if (strcasecmp(tag, "RELOADCHECKPOINT") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL) JimmySlot105(jim);	// +0x1a4
	}
	else if (strcasecmp(tag, "MAINMENU") == 0)
	{
		if (g_menu)
		{
			g_gametype->PauseGame(true);					// +0x110
			g_menu->ActivateActiveMenu(3);					// +0x494
			g_menu->f_590 = 3;
		}
	}
	else if (strcasecmp(tag, "PICKUPWATERGUN") == 0)
	{
		if (scene == 50) SetTaskState("SCENE", 60);			// 0x497980
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();									// +0x414
			ObjSetAnimation(jim, "STOP", true);
			JimmyUseItem(jim, this, g_jimmy, 6, 13.0f);
		}
	}
	else if (strcasecmp(tag, "ACTIVATEWATERGUNNOANIM") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			JimmyUseItem(jim, this, g_jimmy, 6, 13.0f);
		}
	}
	else if (strcasecmp(tag, "ACTIVATEWATERGUN") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			ObjSetAnimation(jim, "GETITEM", true);
			JimmyUseItem(jim, this, g_jimmy, 6, 13.0f);
		}
	}
	else if (strcasecmp(tag, "CUTACTIVATEWATERGUNNOANIM") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			JimmyUseItem(jim, this, g_jimmy, 6, 13.0f);
		}
	}
	else if (strcasecmp(tag, "CUTACTIVATEWATERGUN") == 0)	// (shares ACTIVATEWATERGUN's code)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			ObjSetAnimation(jim, "GETITEM", true);
			JimmyUseItem(jim, this, g_jimmy, 6, 13.0f);
		}
	}
	else if (strcasecmp(tag, "ACTIVATEJETPACK") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			ObjSetAnimation(jim, "STOP", true);
			JimmyUseItem(jim, this, JimmyJetpack(g_jimmy), 3, 13.0f);	// (g_jimmy +0xccc, as an object)
		}
	}
	else if (strcasecmp(tag, "ACTIVATESCOOTER") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			ObjSetAnimation(jim, "STOP", true);
			JimmyUseItem(jim, this, NULL, 4, 13.0f);
		}
	}
	else if (strcasecmp(tag, "ACTIVATEINVIS") == 0)
	{
		if ((jim = FindObjectByTag("JIM1")) != NULL)
		{
			jim->v261();
			ObjSetAnimation(jim, "STOP", true);
			JimmyUseItem(jim, this, g_jimmy, 2, 13.0f);
		}
	}
	else if (strcasecmp(tag, "CRANEBOOM") == 0)
	{
		CGameObject *h = FindObjectByTag("CRANECUTXXX");
		if (h && h->NameContains((char *)"C3DHELPSOUND")) h->AddTouching(caller);	// +0x40
		JimmyUseItem(g_jimmy, this, NULL, 11, 13.0f);		// (on g_jimmy, NULL or not)
	}

	// ---- second group
	if (strcasecmp(tag, "HELPME") == 0) { MenuOpenHelp(20.0f); return; }	// g_menu 0x40a0c0
	if (strcasecmp(tag, "LUNARLANDER") == 0) { JimmyUseItem(g_jimmy, this, NULL, 13, 13.0f); return; }
	if (strcasecmp(tag, "ENTERSUB") == 0) { JimmyUseItem(g_jimmy, this, NULL, 9, 13.0f); return; }
	if (strcasecmp(tag, "LEAVESUB") == 0)
	{
		CGameObject *sub = FindObjectByTag("C3DMINISUB");
		if (sub)
		{
			float v[4];
			v195(v);
			sub->v196(v[0], v[1], v[2], v[3]);				// the sub comes to the trigger
			sub->v132(0);									// +0x210
			sub->v136(0);									// +0x220 (in between: +0x5a0 = 0)
			MinisubReset(sub);								// +0x5a0, +0x934, +0x938 = 0
		}
		JimmyUseItem(g_jimmy, this, g_jimmy, 0, 0.0f);
		return;
	}
	if (strcasecmp(tag, "STARTROCKET") == 0)
	{
		if (GetTaskState("CARLTASK") == 130 && g_jimmy) JimmyUseItem(g_jimmy, this, NULL, 1, 13.0f);
		return;
	}
	if (strcasecmp(tag, "ACTIVATEROCKET") == 0)
	{
		if (FindObjectByTag("JIM1")) JimmyUseItem(g_jimmy, this, NULL, 1, 13.0f);
		return;
	}
	if (strcasecmp(tag, "DEACTIVATEITEM") == 0)
	{
		if (g_jimmy) JimmyUseItem(g_jimmy, this, g_jimmy, 0, 0.0f);	// (debug log "IM HERE MEBTEMP")
		return;
	}
	if (strcasecmp(tag, "HARDTRIG") == 0)
	{
		const int s = GetTaskState("CARLTASK");
		if (s == 100) SetTaskState("CARLTASK", 110);
		else if (s == 120) SetTaskState("CARLTASK", 130);
		return;
	}
	if (strcasecmp(tag, "SOMETHING") == 0)
	{
		const int s = GetTaskState("CARLTASK");
		if (s == 100) SetTaskState("CARLTASK", 120);
		else if (s == 110) SetTaskState("CARLTASK", 130);
		return;
	}
	if (strcasecmp(tag, "DIGITIZE") == 0)
	{
		CGameObject *l = FindObjectByTag("C3DLIBBY");
		if (l) LibbySlot(l, true);							// +0x24c
		return;
	}
	if (strcasecmp(tag, "UNDIGITIZE") == 0)
	{
		CGameObject *l = FindObjectByTag("C3DLIBBY");
		if (l) LibbySlot(l, false);							// +0x250
		return;
	}
	if (!g_menu) return;
	if (strcasecmp(tag, "MUSEUM_DONE") == 0)		{ SetLevelItems(-1); return; }	// 0x40a860
	if (strcasecmp(tag, "VRTANK_DONE") == 0)		{ SetLevelItems(2); return; }
	if (strcasecmp(tag, "LAB_DONE") == 0)			{ SetLevelItems(3); return; }
	if (strcasecmp(tag, "HALLOWEEN_DONE") == 0)		{ SetLevelItems(4); return; }
	if (strcasecmp(tag, "WINTER_DONE") == 0)		{ SetLevelItems(5); return; }
	if (strcasecmp(tag, "ID4_DONE") == 0)
	{
		JimmyUseItem(g_jimmy, this, this, 0, 0.0f);
		SetLevelItems(6);
		return;
	}

	bool museum;											// after a gadget world: back to the museum,
	if (strcasecmp(tag, "NEIGHBORHOOD_DONE") == 0)			// or to Flurp once all three are done
	{
		SetLevelItems(7);
		museum = NotAllGadgetLevelsDone();					// element +0xec
	}
	else if (strcasecmp(tag, "PYRAMID_DONE") == 0)	{ SetLevelItems(9); return; }
	else if (strcasecmp(tag, "TOMB_DONE") == 0)		{ SetLevelItems(10); return; }
	else if (strcasecmp(tag, "TOMBB_DONE") == 0)
	{
		SetLevelItems(11);
		museum = NotAllGadgetLevelsDone();
	}
	else if (strcasecmp(tag, "CARL_DONE") == 0)
	{
		SetLevelItems(13);
		museum = NotAllGadgetLevelsDone();
	}
	else
	{
		if (strcasecmp(tag, "FLURP_DONE") == 0)
		{
			ClearItemState(3, 30);							// 0x404ef0
			SetItemState(3, 19, 1);							// 0x404dd0
			return;
		}
		if (strcasecmp(tag, "DINOS_DONE") == 0)
		{
			ClearItemState(3, 19);
			return;
		}
		if (strcasecmp(tag, "RESTARTGAME") == 0)
		{
			JimmySetStartPoints(g_jimmy, "PHONEBOOTH");		// +0x9c8, +0xa18, +0xb24
			SetGameMode(5);									// 0x485360
			g_gametype->PauseGame(true);					// +0x110
			g_545f11 = 1;
			MenuRestartGame();								// g_menu 0x410540
		}
		return;
	}

	if (museum)
	{
		if (FindObjectByTag("GOTOMUSEUM")) CallNextTrigger("GOTOMUSEUM", 0);
		return;
	}
	SetLevelItems(14);
	if (FindObjectByTag("GOTOFLURP")) CallNextTrigger("GOTOFLURP", 0);
}

namespace neg {
CGameObject *New_C3DAITrigger(void)	{ return new C3DAITrigger; }			// factory '3AIT' (0xde0 bytes)
}
