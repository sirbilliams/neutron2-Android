/*  C3DAITrigger ('3AIT') - the level scripts' work horse: when fired (by a trigger chain, by
    being touched with "TouchActivated", or by a TriggerCallback) and allowed ("ActivateBy" is
    the caller or "none"), it changes "AITarget" - collision, movement, visibility, lighting,
    fading, position ("AINewPos"), heading ("AINewRotY"), animation ("AIAnim"), what a granny
    object carries ("AIAttachObj" / "AIDetachObj") and, for an AI, its state, speed, hover and
    patrol - or, with "IsA", sets the state of every AI of that kind. Triggers with special
    tags also run a built-in action (SpecialAction: gadgets, level-done bookkeeping, restarting
    the game...). Then it fires NextTrigger, or the first "ActivateObject<n>" whose
    "ActivateState<n>" equals the state of its task ("TaskName").
    Reconstructed from Neutron2.exe: constructor 0x418000, destructor 0x418510 (only resets the
    vtables, then ~C3DTriggerType; deleting thunks 0x41a5a0 / 0x4184e0 / 0x41a5b0). Vtables
    0x4cd3c4 (element, 60 slots: C3DTriggerType's 56 + 56..59) / 0x4cd3b0 (listener) /
    0x4ccf34 (game object) / 0x4ccf24 (streamer). 0xde0 bytes: C3DTriggerType's 0x7a0, own
    members 0x7a0..0xdd8, vtordisp 0xdd8, the streamer at 0xddc.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower).                          */
#pragma once
#include "../C3DTriggerType.h"

class C3DAITrigger : public C3DTriggerType
{
	public:
	C3DAITrigger();															// 0x418000
	virtual ~C3DAITrigger();												// 0x418510

	// ---- element-family slots it adds
	virtual CGameObject *FindActiveObject(void);							// ESLOT 56 0x418ab0 (+0xe0) the ActivateObject<n> for the task's state
	virtual void Trigger(CGameObject *caller, unsigned char flag);			// ESLOT 57 0x418b80 (+0xe4) fire
	virtual void SpecialAction(CGameObject *caller);						// ESLOT 58 0x4197b0 (+0xe8) the built-in action of a special tag
	virtual bool NotAllGadgetLevelsDone(void);								// ESLOT 59 0x41a4d0 (+0xec)

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x418590
	virtual void ResetObject(void);											// SLOT 10  0x418550
	virtual void AddTouching(CGameObject *other);							// SLOT 16  0x418960 touch starts
	virtual void v23(CGameObject *o);										// SLOT 23  0x41a500 TriggerCallback: fire
	virtual void Process(float dt);											// SLOT 240 0x4189b0
	virtual void v242(float t);												// SLOT 242 0x418a00 editor colour
	virtual void v267(int a2);												// SLOT 267 0x419790
	virtual void v268(int a2);												// SLOT 268 0x41a500 ToggleObject: fire
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x41a550 savegame
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x41a520 savegame

	// FIELD 0x7a0 0x638
	char		m_activate_by[100];		// 0x7a0  "ActivateBy" (= "JIM1"; "none": anyone)
	char		m_ai_target[100];		// 0x804  "AITarget" (= "none")
	int			m_activate_state[5];	// 0x868  "ActivateState0..4" (= 0, -1, -1, -1, -1)
	char		m_activate_object[5][100];	// 0x87c  "ActivateObject0..4" (= "none")
	char		m_activate_by_anim[100];	// 0xa70  "ActivateByAnim" (= "none")
	char		m_ai_anim[100];			// 0xad4  "AIAnim" (= "none")
	char		m_player_controlled[100];	// 0xb38  "PlayerControlled" (= "none"; "NULL": take the camera away)
	char		m_ai_patrol[100];		// 0xb9c  "AIPatrol" (= "none")
	char		m_ai_new_pos[100];		// 0xc00  "AINewPos" (= "none")
	char		m_ai_attach_obj[100];	// 0xc64  "AIAttachObj" (= "none")
	char		m_ai_detach_obj[100];	// 0xcc8  "AIDetachObj" (= "none")
	int			m_times_to_trigger;		// 0xd2c  "TimesToTrigger" (= -1: always)
	int			m_times_triggered;		// 0xd30  (saved in the savegame)
	int			m_touch_activated;		// 0xd34  "TouchActivated" (= 0)
	int			m_ai_state;				// 0xd38  "AIState" (= 2)
	int			m_ai_speed;				// 0xd3c  "AISpeed" (= -1)
	float		m_ai_new_rot_y;			// 0xd40  "AINewRotY" (= -1)
	int			m_ai_hide_obj;			// 0xd44  "AIHideObj" (= -1)
	int			m_ai_collision;			// 0xd48  "AICollision" (= -1)
	char		m_is_a[100];			// 0xd4c  "IsA" (= "none")
	unsigned char f_db0;				// 0xdb0  (= 0) fire "default" instead of the next trigger (once)
	int			m_fade_in;				// 0xdb4  "FadeIn" (= 0)
	int			m_fade_out;				// 0xdb8  "FadeOut" (= 0)
	float		m_obj_fade_time;		// 0xdbc  "ObjFadeTime" (= 1)
	int			m_ai_anim_loop;			// 0xdc0  "AIAnimLoop" (= 1)
	int			m_ai_hover;				// 0xdc4  "AIHover" (= -1)
	int			m_ai_moveable;			// 0xdc8  "AIMoveable" (= -1)
	float		m_detach_time;			// 0xdcc  "DetachTime" (= -1)
	int			m_show_target;			// 0xdd0  "ShowTarget" (= -1)
	int			m_ai_lit;				// 0xdd4  "AILit" (= -1)
};
