/*  C3DCutSceneCamera - see C3DCutSceneCamera.h. Reconstructed from Neutron2.exe 0x429f30..0x42b7a0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DCutSceneCamera.h"
#include "../CGameType.h"
#include "../CJimmyGame.h"
#include "../DatabaseCache.h"
#include "../InterfaceElement.h"
#include "../Sound.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaViewPort.h"
#include "OMediaWindow.h"
#include "OMediaRect.h"
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <math.h>

using namespace neg;

// --------------------------------------------------------------------------- small helpers

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// 0x44c030: set (or clear) the cut-scene freeze flag (+0x4ab) of every object.
static void FreezeAllObjects(bool freeze)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it) static_cast<CLocalGameObject *>(*it)->f_4ab = freeze;
}

// 0x4a39e0
static void ClearMouseDelta(void)
{
	g_56aebe = 0;
	g_56aec0 = 0;
}

// 0x465ad0: the player skips the cut scene: joystick button 0, or a key of the "cut scene
// bypass" binding (500 = right mouse button, 501 = left).
static bool CutsceneBypass(void)
{
	if (g_joy_button0) return true;
	for (const int *k = g_key_cutscene_bypass; *k; k++)
	{
		if (*k == 501) { if (g_mouse_down) return true; }
		else if (*k == 500) { if (g_mouse_rdown) return true; }
		else if (g_keys[*k]) { ClearMouseDelta(); return true; }
	}
	return false;
}

// --------------------------------------------------------------------------- not reconstructed yet

// C2DInGameMenu 0x40c0f0 (on g_menu): cut-scene letterbox in/out.
static void CutsceneLetterbox(bool on)				{ (void)on; NEG_TODO(0x40c0f0, "cut scene letterbox"); }

// C3DObject element slot 58 (+0xe8) of an animated target (C3DAnimated...): play animation
// `name`, looping or not.
static void TargetSetAnimation(CGameObject *target, const char *name, bool loop)
{
	(void)target; (void)name; (void)loop;
	NEG_TODO(0x42a6a3, "C3DAnimated element slot 58 (set animation)");
}

// C3DAI element +0xc38: its own animations are off while a cut scene animates it.
static void AISetAnimationsDisabled(CGameObject *ai, int off)
{
	(void)ai; (void)off;
	NEG_TODO(0x42b3c6, "C3DAI +0xc38 (animations disabled by a cut scene)");
}

// C3DJimmy element slot 138 (+0x228)(this, jimmy, 0, 0): put the gadget in hand away.
static void JimmyDeactivateInventory(CGameObject *jimmy, CGameObject *caller)
{
	(void)jimmy; (void)caller;
	NEG_TODO(0x42aca8, "C3DJimmy element slot 138 (deactivate inventory)");
}

// --------------------------------------------------------------------------- construction

C3DCutSceneCamera::C3DCutSceneCamera()						// 0x429f30
{
	m_targ_offset[3] = 0;									// 0xadc
	SetName((char *)"C3DCUTSCENECAMERA");
	Log("C3DCutSceneCamera()");
	C3DCutSceneCamera::InitObject();						// (called directly, as in the original)
	SetObjectID('3CAM');
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_camera_target, "none");
	strcpy(m_next_trigger, "none");
	strcpy(m_sound_db, "none");
	strcpy(m_face_object, "none");
	strcpy(m_target_act_anim, "none");
	strcpy(m_target_deact_anim, "none");
	strcpy(m_sound_name, "none");
	m_sound_index = -1;
	m_sprite_index = 10;
	m_sprite_size = 50;
	m_channel = -1;
	m_targ_offset[0] = 0;
	m_targ_offset[1] = 100.0f;
	m_targ_offset[2] = 200.0f;
	m_view_from_camera = 1;
	m_active = 0;
	v32(0);
	m_look_voffset = 0;
	v67(150.0f);
	v52(0);
	v46(0);
	v232();
	m_target = NULL;
	f_008 = 1;												// element +0xd0
	m_camera_type = 0;
	m_zoom_speed = 15.0f;
	m_loop_act_anim = -1;
	m_max_dist = 600.0f;
	m_min_dist = 200.0f;
	m_initial_dist = 300.0f;
	m_look_voffset = 100.0f;
	m_deactivate_inv = 1;
	m_alt_viewport = -1;
	g_cutscene_element = NULL;								// 0x545f44 (sic: every camera created forgets it)
	m_letterbox = 1;
	m_bypass_off = 0;
	m_preview = 0;
	m_in_window = 0;
}

C3DCutSceneCamera::~C3DCutSceneCamera() {}					// 0x42a2b0

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DCutSceneCamera::InitObject(void)					// 0x42a2f0
{
	C3DTriggerType::InitObject();
	RegisterProperty("CameraTarget",    m_camera_target,     PROP_ALPHA, 0);
	RegisterProperty("SoundDatabase",   m_sound_db,          PROP_ALPHA, 0);
	RegisterProperty("SoundIndex",      &m_sound_index,      PROP_LONG,  0);
	RegisterProperty("SoundName",       m_sound_name,        PROP_ALPHA, 0);
	RegisterProperty("FaceObject",      m_face_object,       PROP_ALPHA, 0);
	RegisterProperty("ViewFromCamera",  &m_view_from_camera, PROP_LONG,  0);
	RegisterProperty("TargOffsetX",     &m_targ_offset[0],   PROP_FLOAT, 0);
	RegisterProperty("TargOffsetY",     &m_targ_offset[1],   PROP_FLOAT, 0);
	RegisterProperty("TargOffsetZ",     &m_targ_offset[2],   PROP_FLOAT, 0);
	RegisterProperty("TargetActAnim",   m_target_act_anim,   PROP_ALPHA, 0);
	RegisterProperty("LoopActAnim",     &m_loop_act_anim,    PROP_LONG,  0);
	RegisterProperty("TargetDeactAnim", m_target_deact_anim, PROP_ALPHA, 0);
	RegisterProperty("LookVoffset",     &m_look_voffset,     PROP_FLOAT, 0);
	RegisterProperty("CameraType",      &m_camera_type,      PROP_LONG,  0);
	RegisterProperty("ZoomSpeed",       &m_zoom_speed,       PROP_FLOAT, 0);
	RegisterProperty("MaxDist",         &m_max_dist,         PROP_FLOAT, 0);
	RegisterProperty("MinDist",         &m_min_dist,         PROP_FLOAT, 0);
	RegisterProperty("InitialDist",     &m_initial_dist,     PROP_FLOAT, 0);
	strcpy(m_player_controlled, "none");					// (here, not in the constructor)
	RegisterProperty("PlayerControlled", m_player_controlled, PROP_ALPHA, 0);
	RegisterProperty("DeactivateInv",   &m_deactivate_inv,   PROP_LONG,  0);
	RegisterProperty("AltViewport",     &m_alt_viewport,     PROP_LONG,  0);
	RegisterProperty("LetterBox",       &m_letterbox,        PROP_LONG,  0);
	RegisterProperty("BypassOff",       &m_bypass_off,       PROP_LONG,  0);
}

void C3DCutSceneCamera::v23(CGameObject *o)					// 0x42b430
{
	(void)o;
	// (debug log "Calling TC from TriggerCallback in %s")
	TriggerCamera(0);										// element +0xe0
}

// 0x42a580: while the shot runs, move the camera; when the clip has ended, put the target's
// animation back and fire NextTrigger; when the player skips, stop the clip and fire it at once.
void C3DCutSceneCamera::Process(float dt)
{
	C3DSpriteType::Process(dt);								// 0x473700
	if (!m_active) return;
	// (debug text of the tag at 520, 5 - compiled out)
	m_time += dt;
	if (!IsChannelPlaying(m_channel))
	{
		// (debug log "Sound not playing in Cutscenecamera tick")
		FreezeAllObjects(false);
		StopChannel(m_channel, true);
		if (m_target->NameContains((char *)"C3DANIMATED") && strcasecmp(m_target_deact_anim, "none") != 0)
		{
			// (debug log "Setting %s to animation %s for %s")
			if (m_target->NameContains((char *)"C3DAI")) AISetAnimationsDisabled(m_target, 0);
			TargetSetAnimation(m_target, m_target_deact_anim, true);
		}
		m_active = 0;
		if (strcasecmp(m_next_trigger, "none") == 0) return;
		CloseDatabase(m_sound_db);							// 0x4a4730
		CallNextTrigger(m_next_trigger, m_in_window);		// element +0xdc
		return;
	}

	if (!CutsceneBypass() && !g_cutscene_skipped)
	{
		UpdateView(m_time);									// element +0xe8
		return;
	}

	// skipped
	if (g_cutscene_element)
	{
		g_elt_545c80.Hide();								// 0x4826f0
		if (g_cutscene_element) delete g_cutscene_element;	// listener vtable +8: deleting destructor
		g_cutscene_element = NULL;
	}
	FindObjectByTag("JIM1");								// (result unused)
	g_cutscene_skipped = 1;
	StopChannel(m_channel, true);
	m_active = 0;
	CloseDatabase(m_sound_db);
	CallNextTrigger(m_next_trigger, m_in_window);			// (also for "none": CallNextTrigger ignores it)
	if (m_target->NameContains((char *)"C3DAI")) AISetAnimationsDisabled(m_target, 0);
}

void C3DCutSceneCamera::v242(float t)						// 0x42b710
{
	C3DTriggerType::v242(t);								// 0x478e10
	if (g_keys[0xc0]) m_preview = 0;						// 0x56ae60 VK_OEM_3 (`)
}

// 0x42b5d0: in the editor (the argument is the frame time), Ctrl+L starts a preview of the shot
// (with TargetActAnim on the target); while it runs the camera follows UpdateView.
void C3DCutSceneCamera::v283(unsigned int dt_bits)
{
	CLocalGameObject::v283(dt_bits);						// (CGameObject's, empty)
	float dt;
	memcpy(&dt, &dt_bits, sizeof dt);
	if (m_preview)
	{
		if (!m_target) return;
		m_preview_time = dt + m_preview_time;
		UpdateView(m_preview_time);							// element +0xe8
		return;
	}
	if (!g_keys['L'] || !g_keys[0x11]) return;				// 0x56adec, 0x56adb1 VK_CONTROL
	m_preview = 1;
	m_preview_time = 0;
	if (strcasecmp(m_camera_target, "none") == 0) return;
	m_target = FindObjectByTag(m_camera_target);
	if (!m_target) return;
	if (strcasecmp(m_target_act_anim, "none") == 0) return;
	TargetSetAnimation(m_target, m_target_act_anim, m_loop_act_anim != 0);
}

// --------------------------------------------------------------------------- element-family slots

// 0x42abd0: fire the shot. Unless it is to be shown in the small viewport, the player's view is
// taken (letterbox, gadget put away, the camera handed to PlayerControlled or to nobody); then
// the small viewport is opened or closed, ToggleObject is told, the world is frozen and the clip
// starts, with TargetActAnim on the target.
void C3DCutSceneCamera::TriggerCamera(unsigned char in_window)
{
	// (debug log "C3DCutSceneCamera TriggerCamera is %s")
	if (m_active) return;
	if (m_bypass_off) g_cutscene_skipped = 0;
	m_in_window = in_window;

	if (!in_window)
	{
		if (m_letterbox == 1) CutsceneLetterbox(true);
		if (m_deactivate_inv && FindObjectByTag("JIM1"))
		{
			JimmyDeactivateInventory(g_jimmy, this);
			g_gametype->SetCamera(NULL);
		}
		if (strcasecmp(m_player_controlled, "none") != 0)
		{
			if (strcasecmp(m_player_controlled, "NULL") != 0)
			{
				CGameObject *o = FindObjectByTag(m_player_controlled);
				if (o) g_gametype->SetCamera(o);
			}
			else
			{
				if (g_camera)
				{
					// (debug log "There is a pco %s")
					static_cast<CLocalGameObject *>(g_camera)->v275();	// +0x44c
					g_camera->v261();								// +0x414
					float a[4];
					g_camera->v201(a);								// +0x324: current angles
					// only the heading is kept
					g_camera->f_274 = 0;
					g_camera->f_278 = a[1];
					g_camera->f_27c = 0;
					g_camera->f_280 = a[3];
					g_camera->v202(0, a[1], 0, a[3]);				// +0x328
				}
				g_gametype->SetCamera(NULL);
			}
		}
	}

	if ((m_alt_viewport == 1 || m_in_window) && !g_cutscene_element)
	{
		OMediaViewPort *vp = new OMediaViewPort(g_window);	// 0x120 bytes; g_window's supervisor (+0x14), NULL without a window
		g_cutscene_element = vp;
		if (g_cutscene_element)
		{
			if (g_camera) g_camera->v261();					// +0x414
			g_elt_545c80.Show();							// 0x482520: the frame around it
			const float sx = (float)(short)g_screen_w * (1.0f / 640.0f);	// 0x4ca4bc
			float sy = (float)(short)g_screen_h * (1.0f / 480.0f);		// 0x4ca4b8
			const float fy = (m_letterbox && !m_in_window) ? sy * 204.0f : sy * 268.0f;	// 0x4d7850 / 0x4d784c
			g_elt_545c80.SetPos(sx * 425.0f, fy);			// 0x4d7848
			vp->link(g_world);								// +0x2c
			vp->link_layer(g_layer_world);					// +0x34 (0x56afc8)
			vp->link_window(g_window);						// +0xb0
			const int bottom = (int)((float)(short)g_screen_h * 0.25f + 1.0f);	// 0x4d340c
			const int right = (int)((float)(short)g_screen_w * 0.25f);
			OMediaRect r(0, 0, right, bottom);
			vp->setbounds(&r);								// +0xac
			// then moved to the bottom right: 450 x 286 (letterbox) or 350 (no letterbox) in 640x480
			OMediaRect b;
			vp->getbounds(&b);								// 2002 +0xe4
			const int top = (m_letterbox && !m_in_window) ? (int)(sy * 286.0f) : (int)(sy * 350.0f);	// 0x4d7844 / 0x4d783c
			const int dy = top - b.top;
			const int dx = -b.left - (int)(sx * -450.0f);	// 0x4d7840
			b.left += dx; b.right += dx;
			b.top += dy; b.bottom += dy;
			vp->setbounds(&b);
		}
	}

	if (m_alt_viewport == 0 && g_cutscene_element)
	{
		g_elt_545c80.Hide();
		if (g_cutscene_element) delete g_cutscene_element;
		g_cutscene_element = NULL;
	}

	if (strcasecmp(m_toggle_object, "none") != 0)
	{
		CGameObject *o = FindObjectByTag(m_toggle_object);
		if (o) static_cast<CLocalGameObject *>(o)->v268(m_toggle);	// +0x430
	}

	if (strcasecmp(m_camera_target, "none") == 0) return;
	FreezeAllObjects(true);
	m_target = FindObjectByTag(m_camera_target);
	if (!m_target) return;
	static_cast<CJimmyGame *>(g_gametype)->StopLevelMusicKeepName();	// element +0x1a0 (a CJimmyGame slot)
	FindObjectByTag("JIM1");								// (result unused)
	if (m_letterbox == 1) g_music_volume = 0;
	if (g_active_sound_db)
	{
		CloseDatabase("SoundEffects.omt");
		g_active_sound_db = NULL;
		g_sfx_db_loaded = 0;								// 0x54a4b8
	}
	g_active_sound_db = GetDatabase(m_sound_db);
	if (!g_active_sound_db)
	{
		ErrorLog("ERROR: Invalid SoundDatabase %s for %s", m_sound_db, m_tag);
		return;
	}

	if (m_sound_index == -1)
	{
		// (debug log "Playing Sound %s in TriggerCamera")
		m_channel = PlaySound(-1, std::string(m_sound_name), false);	// 0x4b8380
		if (m_channel == -1)
		{
			m_channel = PlaySoundIndex(m_channel, 0, false);	// 0x4b8240: chunk 0 as a fallback
			if (m_channel == -1)
			{
				// QUIRK: %d is given the SoundDatabase string, as shipped (prints its address).
				ErrorLog("ERROR: Invalid SoundIndex %d for %s", (int)(intptr_t)m_sound_db, m_tag);
				return;
			}
		}
	}
	else
		m_channel = PlaySoundIndex(-1, m_sound_index, false);
	SetChannelVolume((short)m_channel, (short)g_sfx_volume);	// 0x4b8e30, 0x540c58
	if (m_channel == -1)
	{
		ErrorLog("ERROR: Invalid SoundIndex %d for %s", (int)(intptr_t)m_sound_db, m_tag);	// (sic, as above)
		return;
	}

	m_active = 1;
	m_time = 0;
	if (!m_target->NameContains((char *)"C3DANIMATED")) return;
	if (strcasecmp(m_target_act_anim, "none") == 0) return;
	// (debug log "Setting %s to animation %s for %s", twice)
	TargetSetAnimation(m_target, m_target_act_anim, m_loop_act_anim != 0);
	if (!m_target->NameContains((char *)"C3DAI")) return;
	// (debug log "Disabling animations for %s", twice)
	AISetAnimationsDisabled(m_target, 1);
}

// 0x42b460: put the view at `pos` looking at `look` (no roll): the cut-scene viewport when the
// shot uses it, else the main one. Also remembered as the view to restore after a fade.
void C3DCutSceneCamera::SetView(OMedia3DVector pos, OMedia3DVector look)
{
	OMediaElement *v;
	if (m_alt_viewport == 0 && !m_in_window) v = g_viewport;
	else v = g_cutscene_element ? g_cutscene_element : g_viewport;
	v->setx(pos.x);											// +0x44
	v->sety(pos.y);
	v->setz(pos.z);
	g_view_restore_x = pos.x;								// 0x54a480..
	g_view_restore_y = pos.y;
	g_view_restore_z = pos.z;
	memcpy(&g_54a48c, &pos.w, 4);
	OMedia3DVector dir;
	dir.x = look.x - pos.x;
	dir.y = look.y - pos.y;
	dir.z = look.z - pos.z;
	dir.w = 0;
	omt_Angle ax, ay;
	dir.angles(ax, ay);
	if (m_alt_viewport == 0 && !m_in_window) v = g_viewport;
	else v = g_cutscene_element ? g_cutscene_element : g_viewport;
	v->set_anglex(ax);										// +0x50
	v->set_angley(ay);										// +0x52
	v->set_anglez(0);										// +0x54
	g_view_restore_ax = (float)ax;							// 0x54a490
	g_view_restore_ay = (float)ay;							// 0x54a494
}

// 0x42a7f0: where the camera is at time t of the shot.
//   CameraType 2: at the sprite, looking at the target.
//   else, ViewFromCamera 0: at the target's local point (TargOffsetX, TargOffsetY, d), d zooming
//     from InitialDist by ZoomSpeed per unit of time, clamped to MinDist..MaxDist (and kept in
//     TargOffsetZ), looking at the target raised by LookVoffset.
//   else: looking at the target's local point TargOffset, from the sprite's side of it, at
//     distance InitialDist - t * ZoomSpeed rescaled into MinDist..MaxDist.
void C3DCutSceneCamera::UpdateView(float t)
{
	if (m_camera_type == 2)
	{
		OMedia3DVector me, target;
		v195(me.xyzw());									// +0x30c
		m_target->v195(target.xyzw());
		SetView(me, target);								// element +0xe4
		return;
	}

	if (m_view_from_camera == 0)
	{
		m_targ_offset[2] = m_initial_dist - t * m_zoom_speed;
		if (!(m_targ_offset[2] > m_min_dist)) m_targ_offset[2] = m_min_dist;
		if (!(m_targ_offset[2] < m_max_dist)) m_targ_offset[2] = m_max_dist;
		OMedia3DVector p = m_target->v224(m_targ_offset[0], m_targ_offset[1], m_targ_offset[2]);	// +0x380
		OMedia3DVector eye;
		eye.x = p.x; eye.y = p.y; eye.z = p.z; eye.w = 1.0f;
		OMedia3DVector look;
		m_target->v195(look.xyzw());
		look.y += m_look_voffset;
		SetView(eye, look);
		return;
	}

	OMedia3DVector me;
	v195(me.xyzw());
	OMedia3DVector p = m_target->v224(m_targ_offset[0], m_targ_offset[1], m_targ_offset[2]);
	OMedia3DVector d;
	d.x = me.x - p.x; d.y = me.y - p.y; d.z = me.z - p.z; d.w = 0;
	// (x87: the z component of each rescale stays in extended precision until it is stored)
	float inv = 1.0f / d.quick_magnitude();
	d.x = inv * d.x; d.y = inv * d.y;
	const float s = m_initial_dist - t * m_zoom_speed;
	d.x = s * d.x; d.y = s * d.y; d.z = s * (inv * d.z);
	const float len = sqrtf(d.z * d.z + d.y * d.y + d.x * d.x);
	if (len > m_max_dist)
	{
		inv = 1.0f / d.quick_magnitude();
		d.x = inv * d.x; d.y = inv * d.y; const float z = inv * d.z;
		d.x = d.x * m_max_dist; d.y = d.y * m_max_dist; d.z = z * m_max_dist;
	}
	if (len < m_min_dist)									// (the length before the MaxDist rescale)
	{
		inv = 1.0f / d.quick_magnitude();
		d.x = inv * d.x; d.y = inv * d.y; const float z = inv * d.z;
		d.x = d.x * m_min_dist; d.y = d.y * m_min_dist; d.z = z * m_min_dist;
	}
	OMedia3DVector eye;
	eye.x = p.x + d.x; eye.y = p.y + d.y; eye.z = p.z + d.z; eye.w = me.w;
	SetView(eye, p);
}

namespace neg {
CGameObject *New_C3DCutSceneCamera(void)	{ return new C3DCutSceneCamera; }	// factory '3CAM' (0xb30 bytes)
}
