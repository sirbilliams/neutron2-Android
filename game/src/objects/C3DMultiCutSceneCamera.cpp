/*  C3DMultiCutSceneCamera - see C3DMultiCutSceneCamera.h. Reconstructed from Neutron2.exe
    0x456de0..0x458d60. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DMultiCutSceneCamera.h"
#include "../CGameType.h"
#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"
#include "../C3DAI.h"
#include "../C3DAnimated.h"
#include "../DatabaseCache.h"
#include "../InterfaceElement.h"
#include "../Sound.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "../port/Widescreen.h"
#include "OMediaViewPort.h"
#include "OMediaWindow.h"
#include "OMediaRect.h"
#include "OMediaSDLRetarget.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <string>
#include <vector>

using namespace neg;

// --------------------------------------------------------------------------- small helpers

// 0x4aefb0: the object whose tag ("ObjectTag") is `tag`, case-insensitive.
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

using neg::CutsceneLetterbox;				// 0x40c0f0 (on g_menu): cut-scene letterbox in/out
using neg::PlayerKeyCutsceneBypass;		// 0x465ad0: the player skips the cut scene

// C3DAnimated element slot 58 (+0xe8) of an animated target: play animation `name`.
static void TargetSetAnimation(CGameObject *target, const char *name, bool loop)
{
	C3DAnimated *a = target ? dynamic_cast<C3DAnimated *>(target) : NULL;	// port: only a C3DAnimated has the slot
	if (a) a->ev58((char *)name, loop);
}

// C3DAI element +0x96c = 9: the character is frozen (held by the cut scene).
static void AIFreeze(CGameObject *ai)
{
	C3DAI *a = ai ? dynamic_cast<C3DAI *>(ai) : NULL;		// port: a name with "C3DAI" that is not one is left alone
	if (a) a->f_96c = 9;
}

// C3DAI element +0xc38: its own animations are off while a cut scene animates it.
static void AISetAnimationsDisabled(CGameObject *ai, int off)
{
	C3DAI *a = ai ? dynamic_cast<C3DAI *>(ai) : NULL;
	if (a) a->f_c38 = off;
}

// C3DAI element slot 126 (+0x1f8): the cut scene lets go of it.
static void AIRelease(CGameObject *ai)
{
	C3DAI *a = ai ? dynamic_cast<C3DAI *>(ai) : NULL;
	if (a) a->ev126();
}

// C3DJimmy element slot 138 (+0x228)(this, jimmy, 0, 0): put the gadget in hand away.
static void JimmyDeactivateInventory(CGameObject *jimmy, CGameObject *caller)
{
	C3DJimmy *j = jimmy ? dynamic_cast<C3DJimmy *>(jimmy) : NULL;
	if (j) j->ev138(caller, jimmy, 0, 0.0f);
}

// --------------------------------------------------------------------------- the camera types' constants

// UpdateView's camera offsets are the original's constants. They are read from the player's own
// Neutron2.exe the first time they are needed (the .rdata floats it multiplies by, and the
// immediates of its `mov [esi+...], imm32` instructions), not copied into this source.
// port: the original has them in its code.
namespace {

struct ExeImage
{
	std::vector<unsigned char> f;
	struct Section { unsigned int va, vsize, raw, rawsize; };
	std::vector<Section> secs;

	static unsigned int rd32(const std::vector<unsigned char> &f, size_t o)
	{
		return o + 4 <= f.size() ? (unsigned int)f[o] | (unsigned int)f[o + 1] << 8 | (unsigned int)f[o + 2] << 16 | (unsigned int)f[o + 3] << 24 : 0;
	}
	bool Load(void)
	{
		FILE *fp = fopen(omf_SDLResolvePath("Neutron2.exe").c_str(), "rb");
		if (!fp) return false;
		unsigned char buf[65536];
		size_t n;
		while ((n = fread(buf, 1, sizeof buf, fp)) > 0) f.insert(f.end(), buf, buf + n);
		fclose(fp);
		unsigned int pe = rd32(f, 0x3c);
		if (rd32(f, pe) != 0x4550) return false;
		unsigned int nsec = rd32(f, pe + 4) >> 16, opt = rd32(f, pe + 0x14) & 0xffff;
		for (unsigned int i = 0; i < nsec; i++)
		{
			size_t h = pe + 0x18 + opt + i * 0x28;
			Section s = { rd32(f, h + 12), rd32(f, h + 8), rd32(f, h + 20), rd32(f, h + 16) };
			secs.push_back(s);
		}
		return true;
	}
	void Read(unsigned int va, void *out, unsigned int n) const
	{
		memset(out, 0, n);
		for (size_t i = 0; i < secs.size(); i++)
		{
			unsigned int lo = 0x400000 + secs[i].va;
			if (va < lo || va + n > lo + secs[i].rawsize) continue;
			size_t o = secs[i].raw + (va - lo);
			if (o + n <= f.size()) memcpy(out, &f[o], n);
			return;
		}
	}
	float F(unsigned int va) const	{ float v; Read(va, &v, 4); return v; }
	double D(unsigned int va) const	{ double v; Read(va, &v, 8); return v; }
};

// Named after the camera type that uses them; `_imm` values are instruction immediates.
struct CameraConsts
{
	float c0_x, c0_y, c0_z;							// default (types 0, 11, 18+): 0x4582b4..
	float c1_x, c1_y, c1_start, c1_rate;			// 1: zoom in
	float c2_x, c2_y, c2_start, c2_rate;			// 2
	float c3_x, c3_y, c4_x, c4_y, c34_start, c34_rate;	// 3, 4
	float near_min, near_clamp;						// 1..4: no nearer than
	float c5_x, c5_y, c6_x, c6_y, c7_x, c7_y, c567_start, c57_rate, c6_rate;	// 5..7: from behind
	float far_max, c57_clamp;						// 5..7: no nearer than (negative z)
	double c8_freq, c8_radius;						// 8: spiral
	float c8_rise, c8_base;
	float c9_y, c16_y, c17_y;						// 9, 16, 17: circling heights
	float c12_x, c12_v;								// 12
	float c13_y, c13_z;								// 10, 13
	float c14_x, c14_y, c14_z, c15_x, c15_y, c15_z;	// 14, 15: from the side
	float look_base;								// LookatVOffset - this
};

const CameraConsts &Consts(void)
{
	static CameraConsts k;
	static bool loaded = false;
	if (loaded) return k;
	loaded = true;
	ExeImage x;
	if (!x.Load()) { memset(&k, 0, sizeof k); return k; }
	k.c0_x = x.F(0x4582ba); k.c0_y = x.F(0x4582c4); k.c0_z = x.F(0x4582ce);
	k.c1_x = x.F(0x45805b); k.c1_y = x.F(0x458065); k.c1_start = x.F(0x4caa7c); k.c1_rate = x.F(0x4dce54);
	k.c2_x = x.F(0x458081); k.c2_y = x.F(0x45808b); k.c2_start = x.F(0x4cb200); k.c2_rate = x.F(0x4e560c);
	k.c3_x = x.F(0x45809d); k.c3_y = x.F(0x4580a7); k.c4_x = x.F(0x4580b3); k.c4_y = x.F(0x4580bd);
	k.c34_start = x.F(0x4ccf18); k.c34_rate = x.F(0x4ebc18);
	k.near_min = x.F(0x4d2cb0); k.near_clamp = x.F(0x4580ee);
	k.c5_x = x.F(0x458107); k.c5_y = x.F(0x458111); k.c6_x = x.F(0x45814c); k.c6_y = x.F(0x458156);
	k.c7_x = x.F(0x458198); k.c7_y = x.F(0x4581a2);
	k.c567_start = x.F(0x4dce34); k.c57_rate = x.F(0x4ebc18); k.c6_rate = x.F(0x4e560c);
	k.far_max = x.F(0x4df748); k.c57_clamp = x.F(0x458138);
	k.c8_freq = x.D(0x4d2cc0); k.c8_radius = x.D(0x4ebc10); k.c8_rise = x.F(0x4cb1e4); k.c8_base = x.F(0x4ca4b0);
	k.c9_y = x.F(0x458225); k.c16_y = x.F(0x45825c); k.c17_y = x.F(0x458290);
	k.c12_x = x.F(0x457dc0); k.c12_v = x.F(0x457db6);
	k.c13_y = x.F(0x457ff2); k.c13_z = x.F(0x457ffc);
	k.c14_x = x.F(0x45800b); k.c14_y = x.F(0x458015); k.c14_z = x.F(0x45801f);
	k.c15_x = x.F(0x45802e); k.c15_y = x.F(0x458038); k.c15_z = x.F(0x458042);
	k.look_base = x.F(0x4d8b84);
	return k;
}

}	// namespace

// --------------------------------------------------------------------------- the small viewport

// 0x458500 and inline in UpdateView case 10: a picture-in-picture viewport in the bottom right
// (a quarter of the screen, at 450 x 286 in 640x480), with the frame 0x545c80 around it.
// `plus_one`: the inline copy rounds the width up by one as well.
static void CreateAltViewport(bool plus_one)
{
	OMediaViewPort *vp = new OMediaViewPort(g_window);		// 0x120 bytes; g_window's supervisor (+0x14), NULL without a window
	g_cutscene_element = vp;
	if (!g_cutscene_element) return;
	const float sx = (float)(short)g_screen_w * (1.0f / 640.0f);	// 0x4ca4bc
	const float sy = (float)(short)g_screen_h * (1.0f / 480.0f);	// 0x4ca4b8
	g_elt_545c80.Show();									// 0x482520
	vp->link(g_world);										// +0x2c
	vp->link_layer(g_layer_world);							// +0x34 (0x56afc8)
	vp->link_window(g_window);								// +0xb0
	const int right = plus_one ? (int)((float)(short)g_screen_w * 0.25f + 1.0f)	// 0x4d340c, 0x4caa6c
							   : (int)((float)(short)g_screen_w * 0.25f);
	const int bottom = (int)((float)(short)g_screen_h * 0.25f + 1.0f);
	OMediaRect r(0, 0, right, bottom);
	vp->setbounds(&r);										// +0xac
	OMediaRect b;
	vp->getbounds(&b);										// 2002 +0xe4
	const int dx = -b.left - (int)(sx * -450.0f)
				 + (int)WideMargin();			// 0x4d7840; port: + widescreen margin
	const int dy = (int)(sy * 286.0f) - b.top;				// 0x4d7844
	b.left += dx; b.top += dy;
	b.right += dx; b.bottom += dy;
	vp->setbounds(&b);
}

// 0x4826f0 + the listener vtable's deleting destructor: close the small viewport.
static void CloseAltViewport(void)
{
	g_elt_545c80.Hide();
	if (g_cutscene_element) delete g_cutscene_element;
	g_cutscene_element = NULL;
}

// --------------------------------------------------------------------------- construction

C3DMultiCutSceneCamera::C3DMultiCutSceneCamera()			// 0x456de0
{
	f_1310 = 0;
	SetName((char *)"C3DMULTICUTSCENE");
	Log("C3DMultiCutSceneCamera()");
	C3DMultiCutSceneCamera::InitObject();					// (called directly, as in the original)
	SetObjectID('3MCA');
	strcpy(m_sprite_db, "icons.omt");
	strcpy(m_next_trigger, "none");
	strcpy(m_sound_db, "none");
	m_sprite_index = 7;
	m_sprite_size = 50;
	m_channel = -1;
	m_cam_offset[0] = 0;
	m_cam_offset[1] = 0;
	m_cam_offset[2] = 0;
	m_shot = 0;
	m_active = 0;
	v52(0);
	v46(0);
	v32(0);
	v232();
	m_time = 0;
	v67(150.0f);
	m_target = NULL;
	m_held_ai = NULL;
	m_lookat = 0;
	m_cur_camera_type = 0;
	f_008 = 1;												// element +0xd0
	m_deactivate_inv = 1;
	strcpy(m_fixed_target, "none");
	m_fixed_target_obj = NULL;
	m_letterbox = 1;
	m_bypass_off = 0;
	m_preview = 0;
	m_camera_speed = 2.0f;
	m_camera_dist = 500.0f;
	m_in_window = 0;
	// (then a call to the empty 0x463890)
}

C3DMultiCutSceneCamera::~C3DMultiCutSceneCamera() {}			// 0x457070

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DMultiCutSceneCamera::InitObject(void)				// 0x4570b0
{
	C3DTriggerType::InitObject();							// 0x479000
	strcpy(m_sound_db, "none");
	RegisterProperty("SoundDatabase", m_sound_db, PROP_ALPHA, 0);
	strcpy(m_target_deact_anim, "STOP");
	RegisterProperty("TargetDeactAnim", m_target_deact_anim, PROP_ALPHA, 0);
	char name[16];
	for (int i = 0; i < kShots; i++)
	{
		sprintf(name, "%s%d", "CameraTarget", i);
		strcpy(m_camera_target[i], "none");
		RegisterProperty(name, m_camera_target[i], PROP_ALPHA, 0);
		sprintf(name, "%s%d", "TargetAnim", i);
		strcpy(m_target_anim[i], "TALK");
		RegisterProperty(name, m_target_anim[i], PROP_ALPHA, 0);
		sprintf(name, "%s%d", "LookatVOffset", i);
		m_lookat_voffset[i] = 100.0f;
		RegisterProperty(name, &m_lookat_voffset[i], PROP_FLOAT, 0);
		sprintf(name, "%s%d", "SoundIndex", i);
		m_sound_index[i] = -1;
		RegisterProperty(name, &m_sound_index[i], PROP_LONG, 0);
		m_camera_type[i] = 0;
		sprintf(name, "%s%d", "CameraType", i);
		RegisterProperty(name, &m_camera_type[i], PROP_LONG, 0);
		strcpy(m_sound_names[i], "none");
		sprintf(name, "%s%d", "SoundNames", i);
		RegisterProperty(name, m_sound_names[i], PROP_ALPHA, 0);
	}
	strcpy(m_player_controlled, "NULL");
	RegisterProperty("PlayerControlled", m_player_controlled, PROP_ALPHA, 0);
	RegisterProperty("DeactivateInv",    &m_deactivate_inv,   PROP_LONG,  0);
	RegisterProperty("FixedTarget",      m_fixed_target,      PROP_ALPHA, 0);
	RegisterProperty("LetterBox",        &m_letterbox,        PROP_LONG,  0);
	RegisterProperty("BypassOff",        &m_bypass_off,       PROP_LONG,  0);
	RegisterProperty("CameraSpeed",      &m_camera_speed,     PROP_FLOAT, 0);
	RegisterProperty("CameraDist",       &m_camera_dist,      PROP_FLOAT, 0);
}

// 0x457400: while a shot runs, move the camera. When its clip has ended the target gets
// TargetDeactAnim and the next shot starts; after the last one the scene ends. When the player
// skips, the clip stops and the scene ends at once.
void C3DMultiCutSceneCamera::Process(float dt)
{
	C3DSpriteType::Process(dt);								// 0x473700
	if (!m_active) return;
	// (debug text of the tag at 520, 5 - compiled out)
	const float t = m_time;									// QUIRK: UpdateView below gets the time before this frame's dt
	m_time += dt;
	if (!IsChannelPlaying(m_channel))
	{
		StopChannel(m_channel, true);
		if (m_target && m_target->NameContains((char *)"C3DANIMATED") && strcasecmp(m_target_deact_anim, "none") != 0)
		{
			// (debug log "Setting %s to animation %s for %s")
			TargetSetAnimation(m_target, m_target_deact_anim, true);
		}
		m_active = 0;
		if (NextShot())										// element +0xec
		{
			m_active = 1;
			m_lookat = m_lookat_voffset[m_shot];
			m_cur_camera_type = m_camera_type[m_shot];
			m_shot++;
			return;
		}
		EndCutScene();										// element +0xf0
		m_shot = 0;
		return;
	}

	if (!PlayerKeyCutsceneBypass() && !g_cutscene_skipped)
	{
		UpdateView(t, m_cur_camera_type);					// element +0xf8
		return;
	}

	// skipped
	if (g_cutscene_element) CloseAltViewport();
	FindObjectByTag("JIM1");								// (result unused)
	g_cutscene_skipped = 1;
	StopChannel(m_channel, true);
	if (m_target)
	{
		if (m_target->NameContains((char *)"C3DAI")) AISetAnimationsDisabled(m_target, 0);
		if (m_target->NameContains((char *)"C3DANIMATED") && strcasecmp(m_target_deact_anim, "none") != 0)
		{
			// (debug log "Setting %s to animation %s for %s")
			TargetSetAnimation(m_target, m_target_deact_anim, true);
		}
	}
	m_active = 0;
	EndCutScene();											// element +0xf0
	m_shot = 0;
}

// 0x4586f0: in the editor, while the preview (slot 61) runs, keys 0..7 pick the shot (its
// target starts TargetAnim) and ` ends it; the camera follows UpdateView.
void C3DMultiCutSceneCamera::v242(float t)
{
	C3DTriggerType::v242(t);								// 0x478e10
	if (!m_preview) return;
	bool picked = false;
	for (int k = 0; k < kShots; k++)						// 0x56add0.. ('0'..'7')
	{
		if (g_keys['0' + k])
		{
			m_preview_shot = k;
			m_preview_time = 0;
			picked = true;
		}
	}
	if (g_keys[0xc0]) m_preview = 0;						// 0x56ae60 VK_OEM_3 (`)
	if (picked)
	{
		m_target = FindObjectByTag(m_camera_target[m_preview_shot]);
		if (m_target) TargetSetAnimation(m_target, m_target_anim[m_preview_shot], true);	// (no NameContains check)
		m_lookat = m_lookat_voffset[m_preview_shot];
		m_cur_camera_type = m_camera_type[m_preview_shot];
	}
	m_preview_time = t + m_preview_time;
	UpdateView(m_preview_time, m_cur_camera_type);			// element +0xf8
}

// --------------------------------------------------------------------------- element-family slots

// 0x457760: fire the scene: take the view (letterbox, gadget away, the camera from the player),
// find shot 0's target, hold the C3DAI it is (or CameraTarget1's), tell ToggleObject, swap the
// sound database for SoundDatabase and start shot 0.
void C3DMultiCutSceneCamera::TriggerCamera(unsigned char in_window)
{
	// (debug log "MulticutsceneCamera is calling TriggerCamera for %s")
	m_shot = 0;
	if (m_bypass_off) g_cutscene_skipped = 0;
	m_in_window = in_window;
	if (in_window) OpenAltViewport();						// element +0xfc
	if (m_letterbox == 1) CutsceneLetterbox(true);
	if (m_deactivate_inv && FindObjectByTag("JIM1"))
	{
		JimmyDeactivateInventory(g_jimmy, this);
		g_gametype->SetCamera(NULL);
	}
	// (debug log "CALLING MultiCutSceneCamera %s")
	if (strcasecmp(m_camera_target[m_shot], "none") == 0) return;
	m_target = FindObjectByTag(m_camera_target[m_shot]);
	m_held_ai = NULL;
	if (!m_target)
		ErrorLog("ERROR: Cannot find CameraTargetObj %s in %s", m_camera_target[m_shot], m_tag);
	else if (m_target->NameContains((char *)"C3DAI"))
	{
		AIFreeze(m_target);
		m_held_ai = m_target;
		if (m_target->NameContains((char *)"C3DANIMATED") && strcasecmp(m_target_anim[m_shot], "none") != 0)
		{
			// (debug log "Setting %s to animation %s for %s")
			TargetSetAnimation(m_target, m_target_anim[m_shot], true);
		}
	}
	// QUIRK: the AI to release at the end is looked up from the second shot's target as well.
	CGameObject *o = FindObjectByTag(m_camera_target[m_shot + 1]);
	if (o && o->NameContains((char *)"C3DAI")) m_held_ai = o;
	if (strcasecmp(m_toggle_object, "none") != 0)
	{
		CGameObject *t = FindObjectByTag(m_toggle_object);
		if (t) static_cast<CLocalGameObject *>(t)->v268(m_toggle);	// +0x430
	}
	if (!m_target) return;
	m_target->v261();										// +0x414
	if (m_target->NameContains((char *)"C3DAI"))
	{
		AIFreeze(m_target);
		m_held_ai = m_target;
	}
	if (g_camera)
	{
		g_camera->v261();									// +0x414
		float a[4];
		g_camera->v201(a);									// +0x324: current angles
		g_camera->v202(0, a[1], 0, a[3]);					// +0x328: only the heading is kept
		g_camera->f_274 = 0;
		g_camera->f_278 = a[1];
		g_camera->f_27c = 0;
		g_camera->f_280 = a[3];
		static_cast<CLocalGameObject *>(g_camera)->v275();	// +0x44c
		g_gametype->SetCamera(NULL);
	}
	if (g_active_sound_db)
	{
		CloseDatabase("SoundEffects.omt");
		g_active_sound_db = NULL;
		g_sfx_db_loaded = 0;								// 0x54a4b8
	}
	if (m_letterbox == 1) g_music_volume = 0;
	g_active_sound_db = GetDatabase(m_sound_db);
	if (!g_active_sound_db)
	{
		// port: the original's second argument is an uninitialised stack slot; the tag here.
		ErrorLog("ERROR: Invalid SoundDatabase %s for %s", m_sound_db, m_tag);
		return;
	}
	if (!NextShot()) return;								// element +0xec
	// (debug log "Camera is active")
	m_active = 1;
	m_lookat = m_lookat_voffset[m_shot];
	m_cur_camera_type = m_camera_type[m_shot];
	m_shot++;
}

void C3DMultiCutSceneCamera::TriggerCallback(CGameObject *caller, unsigned char flag)	// 0x457be0
{
	(void)caller;
	// (debug log "Calling TC from MCSC::TriggerCallback in %s")
	TriggerCamera(flag);									// element +0xe0
}

// 0x457c10: put the view at `pos` looking at `look` (no roll): the small viewport for camera
// type 10 or a scene shown in the window, else the main one. Also remembered as the view to
// restore after a fade.
void C3DMultiCutSceneCamera::SetView(OMedia3DVector pos, OMedia3DVector look)
{
	OMediaElement *v;
	if (m_cur_camera_type == 10 || m_in_window) v = g_cutscene_element ? g_cutscene_element : g_viewport;
	else v = g_viewport;
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
	if (m_cur_camera_type == 10 || m_in_window) v = g_cutscene_element ? g_cutscene_element : g_viewport;
	else v = g_viewport;
	v->set_anglex(ax);										// +0x50
	v->set_angley(ay);										// +0x52
	v->set_anglez(0);										// +0x54
	g_view_restore_ax = (float)ax;							// 0x54a490
	g_view_restore_ay = (float)ay;							// 0x54a494
}

// 0x4588c0 ("GetNextSoundChannel"): start shot m_shot: find its target (an AI is held), play
// its clip (SoundIndex, or SoundNames with chunk 0 as a fallback) and its TargetAnim. The shot
// time restarts unless this shot and the one before both circle the same way (9, 16, 17).
bool C3DMultiCutSceneCamera::NextShot(void)
{
	if (m_shot >= kShots) return false;						// "GNSC Exit 1"
	// (debug log "GNSC %s  %d  %s")
	m_target = FindObjectByTag(m_camera_target[m_shot]);
	if (!m_target) return false;							// "GNSC Exit 5"
	if (m_target->NameContains((char *)"C3DAI"))
	{
		AIFreeze(m_target);
		m_held_ai = m_target;
	}
	if (!g_active_sound_db) return false;					// "GNSC Exit 4"
	// (debug log "Playing Sound %s in MCSC::GetNextSoundChannel for %s")
	if (m_sound_index[m_shot] != -1)
	{
		m_channel = PlaySoundIndex(-1, m_sound_index[m_shot], false);	// 0x4b8240
		SetChannelVolume((short)m_channel, (short)g_sfx_volume);			// 0x4b8e30
		if (m_channel == -1)
		{
			ErrorLog("ERROR: Invalid channel of %d for %s", m_sound_index[m_shot], m_tag);
			return false;
		}
	}
	else
	{
		m_channel = PlaySound(-1, std::string(m_sound_names[m_shot]), false);	// 0x4b8380
		SetChannelVolume((short)m_channel, (short)g_sfx_volume);
		if (m_channel == -1)
		{
			m_channel = PlaySoundIndex(-1, 0, false);		// chunk 0 as a fallback
			SetChannelVolume((short)m_channel, (short)g_sfx_volume);
			if (m_channel == -1) return false;				// "GNSC Exit 3"
		}
	}
	if (m_target->NameContains((char *)"C3DANIMATED") && strcasecmp(m_target_anim[m_shot], "none") != 0)
	{
		// (debug log "Setting %s to animation %s for %s")
		TargetSetAnimation(m_target, m_target_anim[m_shot], true);
	}
	if (m_shot > 0)
	{
		const int prev = m_camera_type[m_shot - 1] & 0x7f;
		const int cur = m_camera_type[m_shot] & 0x7f;
		// (debug logs "MEBTEMP %d  %d  %d", "A %d", "B")
		if ((cur == 9 || cur == 16 || cur == 17) && prev == cur) return true;
	}
	// (debug log "Calling CameraActiveTime=0;")
	m_time = 0;
	// (debug log "GNSC Got sound channel %d")
	return true;
}

// 0x4576c0: the scene is over: the camera goes to PlayerControlled (unless "NULL"), the held AI
// is let go, SoundDatabase is closed and NextTrigger fires.
void C3DMultiCutSceneCamera::EndCutScene(void)
{
	if (strcasecmp(m_player_controlled, "NULL") != 0)
	{
		CGameObject *o = FindObjectByTag(m_player_controlled);
		if (o) g_gametype->SetCamera(o);					// +0x124
	}
	if (m_held_ai && m_held_ai->NameContains((char *)"C3DAI")) AIRelease(m_held_ai);	// element +0x1f8
	CloseDatabase(m_sound_db);								// 0x4a4730
	CallNextTrigger(m_next_trigger, 0);						// element +0xdc
}

void C3DMultiCutSceneCamera::StartPreview(void)				// 0x4588a0
{
	m_preview = 1;
	m_preview_shot = 0;
	m_preview_time = 0;
}

// 0x457d80: the camera at time t of a shot of `camera_type`. Each type gives a point in the
// target's frame (m_cam_offset; with +128 FixedTarget's frame); the camera is put there,
// looking at the target raised by LookatVOffset - 60.
//   1..4: zooming in to a nearest distance; 5..7: from behind; 8: a rising spiral;
//   9, 16, 17: circling at CameraDist, CameraSpeed radians per unit of time, at three heights;
//   10: in the small viewport; 11: the small viewport closed; 12..15: fixed.
void C3DMultiCutSceneCamera::UpdateView(float t, int camera_type)
{
	const CameraConsts &k = Consts();
	// (the x87 code keeps t * rate and the subtraction in extended precision: doubles here)
	switch (camera_type & 0x7f)
	{
		case 1:
			m_cam_offset[0] = k.c1_x;
			m_cam_offset[1] = k.c1_y;
			m_cam_offset[2] = (float)((double)k.c1_start - (double)t * k.c1_rate);
			if (m_cam_offset[2] < k.near_min) m_cam_offset[2] = k.near_clamp;
			break;
		case 2:
			m_cam_offset[0] = k.c2_x;
			m_cam_offset[1] = k.c2_y;
			m_cam_offset[2] = (float)((double)k.c2_start - (double)t * k.c2_rate);
			if (m_cam_offset[2] < k.near_min) m_cam_offset[2] = k.near_clamp;
			break;
		case 3:
		case 4:
			m_cam_offset[0] = (camera_type & 0x7f) == 3 ? k.c3_x : k.c4_x;
			m_cam_offset[1] = (camera_type & 0x7f) == 3 ? k.c3_y : k.c4_y;
			m_cam_offset[2] = (float)((double)k.c34_start - (double)t * k.c34_rate);
			if (m_cam_offset[2] < k.near_min) m_cam_offset[2] = k.near_clamp;
			break;
		case 5:
			m_cam_offset[0] = k.c5_x;
			m_cam_offset[1] = k.c5_y;
			m_cam_offset[2] = (float)((double)k.c567_start - (double)t * k.c57_rate);
			if (!(m_cam_offset[2] <= k.far_max)) m_cam_offset[2] = k.c57_clamp;
			break;
		case 6:
			m_cam_offset[1] = k.c6_y;
			m_cam_offset[0] = k.c6_x;
			m_cam_offset[2] = (float)((double)k.c567_start - (double)t * k.c6_rate);
			if (!(m_cam_offset[2] <= k.far_max)) m_cam_offset[2] = k.c6_x;	// (the same register as x)
			break;
		case 7:
			m_cam_offset[0] = k.c7_x;
			m_cam_offset[1] = k.c7_y;
			m_cam_offset[2] = (float)((double)k.c567_start - (double)t * k.c57_rate);
			if (!(m_cam_offset[2] <= k.far_max)) m_cam_offset[2] = k.c57_clamp;
			break;
		case 8:
		{
			const double a = (double)t * k.c8_freq;
			m_cam_offset[0] = (float)(sin(a) * k.c8_radius);
			m_cam_offset[1] = (float)((double)t * k.c8_rise + k.c8_base);
			m_cam_offset[2] = (float)(cos(a) * k.c8_radius);
			break;
		}
		case 9:
		case 16:
		case 17:
		{
			const int ct = camera_type & 0x7f;
			m_cam_offset[1] = ct == 9 ? k.c9_y : ct == 16 ? k.c16_y : k.c17_y;
			const double a = (double)t * m_camera_speed;
			m_cam_offset[0] = (float)(sin(a) * m_camera_dist);
			m_cam_offset[2] = (float)(cos(a) * m_camera_dist);
			break;
		}
		case 10:
			OpenAltViewport();								// element +0xfc
			if (!g_cutscene_element) CreateAltViewport(true);	// (inline copy; only when that failed)
			m_cam_offset[0] = 0;
			m_cam_offset[1] = k.c13_y;
			m_cam_offset[2] = k.c13_z;
			break;
		case 11:
			if (g_cutscene_element || m_in_window) CloseAltViewport();
			m_cam_offset[0] = 0;
			m_cam_offset[1] = k.c0_y;
			m_cam_offset[2] = k.c0_z;
			break;
		case 12:
			m_cam_offset[0] = k.c12_x;
			m_cam_offset[1] = k.c12_v;
			m_cam_offset[2] = k.c12_v;
			break;
		case 13:
			m_cam_offset[0] = 0;
			m_cam_offset[1] = k.c13_y;
			m_cam_offset[2] = k.c13_z;
			break;
		case 14:
			m_cam_offset[0] = k.c14_x;
			m_cam_offset[1] = k.c14_y;
			m_cam_offset[2] = k.c14_z;
			break;
		case 15:
			m_cam_offset[0] = k.c15_x;
			m_cam_offset[1] = k.c15_y;
			m_cam_offset[2] = k.c15_z;
			break;
		default:											// 0 and 18..127
			m_cam_offset[0] = k.c0_x;
			m_cam_offset[1] = k.c0_y;
			m_cam_offset[2] = k.c0_z;
			break;
	}

	if (!m_target) return;
	if (!m_fixed_target_obj && strcasecmp(m_fixed_target, "none") != 0)
		m_fixed_target_obj = FindObjectByTag(m_fixed_target);
	CGameObject *frame = (m_fixed_target_obj && (camera_type & 0x80)) ? m_fixed_target_obj : m_target;
	OMedia3DVector p = frame->v224(m_cam_offset[0], m_cam_offset[1], m_cam_offset[2]);	// +0x380
	OMedia3DVector eye;
	eye.x = p.x; eye.y = p.y; eye.z = p.z; eye.w = 1.0f;
	OMedia3DVector pos;
	frame->v195(pos.xyzw());								// +0x30c
	OMedia3DVector look;
	look.x = pos.x;
	look.y = (float)(((double)m_lookat - k.look_base) + pos.y);	// 0x4d8b84 (x87)
	look.z = pos.z;
	look.w = pos.w;
	SetView(eye, look);										// element +0xe8
}

void C3DMultiCutSceneCamera::OpenAltViewport(void)			// 0x458500
{
	if (g_cutscene_element) return;
	CreateAltViewport(false);
}

namespace neg {
CGameObject *New_C3DMultiCutSceneCamera(void)	{ return new C3DMultiCutSceneCamera; }	// factory '3MCA' (0x1354 bytes)
}
