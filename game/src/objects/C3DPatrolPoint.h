/*  C3DPatrolPoint ('3PAT') - a waypoint of an AI character's patrol ("icons.omt" canvas 6,
    size 50): the AI walks to "NextPatrolPoint", plays "ActivateAnim" / "WaitAnim" for
    "WaitTime", may call "CallObjectTag" or play "SoundName" there. A point with PatrolType 1
    is a node of a free network instead: it lists the other type-1 points of its "GroupID" in
    plain view (no wall in between, at most 20) with their distances, and shows green in the
    editor when it has any, yellow when it has none.
    Reconstructed from Neutron2.exe: constructor 0x45c4c0, destructor 0x45c780 (only resets the
    vtables, then ~C3DSprite; deleting thunks 0x45cc20 / 0x45c750 / 0x45cc30). Vtables 0x4ef48c
    (element, 55 slots, all C3DSprite's) / 0x4ef478 (listener) / 0x4eeffc (game object) /
    0x4eefec (streamer). 0x97c bytes: C3DSpriteType's 0x668, own members 0x668..0x974,
    vtordisp 0x974, the streamer at 0x978.

    Members are named after their 2002 offset from the object start (element view; the
    CGameObject-family methods of the original see them 0xc8 lower). C3DAI reads them.       */
#pragma once
#include "../C3DSpriteType.h"

class C3DPatrolPoint : public C3DSpriteType
{
	public:
	C3DPatrolPoint();														// 0x45c4c0
	virtual ~C3DPatrolPoint();												// 0x45c780

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x45c7c0
	virtual void Process(float dt);											// SLOT 240 0x464ee0 (empty: patrol points do nothing per frame)
	virtual void v242(float t);												// SLOT 242 0x45c9b0 editor: holding A lists the visible points
	virtual void v260(void);												// SLOT 260 0x45c8d0 (re)load: find the visible points, colour
	virtual void v266(void);												// SLOT 266 0x45c990 find the visible points

	void FindVisiblePoints(int group);										// 0x45ca90

	struct Visible { CGameObject *point; float dist; };

	// FIELD 0x668 0x30c
	char		m_call_object_tag[100];	// 0x668  "CallObjectTag" (= "none")
	char		m_activate_anim[100];	// 0x6cc  "ActivateAnim" (= "none")
	char		m_next_patrol_point[100];	// 0x730  "NextPatrolPoint" (= "none")
	char		m_wait_anim[100];		// 0x794  "WaitAnim" (= "none")
	char		m_sound_name[100];		// 0x7f8  "SoundName" (= "none")
	char		m_attach_obj[100];		// 0x85c  "AttachObj" (= "none")
	float		m_wait_time;			// 0x8c0  "WaitTime" (= 0)
	int			m_patrol_type;			// 0x8c4  "PatrolType" (= 0; 1: a node of the GroupID network)
	Visible		m_visible_points[20];	// 0x8c8  FindVisiblePoints: the type-1 points of the group in view
	unsigned char f_968;				// 0x968  = 1
	int			m_group_id;				// 0x96c  "GroupID" (= 0)
	float		m_patrol_speed;			// 0x970  "PatrolSpeed" (= -1)
};
