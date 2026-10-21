/*  C3DJimmy part b: the methods of Neutron2.exe 0x448b40..0x44e8f0 (see jimmy/C3DJimmy_b.inc).
    The gadgets: ActivateMenuItem (element slot 138) picks the gadget of the menu (0 / 8 none,
    1 rocket ship, 2 ?, 3 jetpack, 4 jeep (scooter), 5 VR tank, 6 water gun, 7 grappling hook,
    9 mini sub, 10 burp gun, 11 crane, 12 hover cycle, 13 lunar lander) and ev139 puts the
    previous one away. The guns shoot the 20 pooled water balloons (ev144 -> ev148, their look by
    ev152). The grappling hook (gadget 7) fires straight up (ev141), Jimmy then hangs on the rope
    (ev143) and swings / climbs it (ev140) until he lets go (ev142).

    The vehicles, Goddard, the rocket ship, the jetpack, the shadow and the water balloons are
    game classes that are not reconstructed yet: calls through their virtual slots are made, but
    their own members (offsets past their bases) are NEG_TODO lines at the call site, naming the
    member by its offset in that object's CGameObject part (the "cgo" offset) or element part.

    Element-family methods see members at their object offset (f_<offset>); the CGameObject-family
    methods of the original (v133, v220, v261, v270, v271, Read/WriteState) used offsets 0xc0
    lower, translated here.                                                                    */
#include "../C3DJimmy.h"
#include "../level3c/C3DHoverCycle.h"
#include "../level3d/C3DCraneBoom.h"
#include "../level3a/C3DMiniSub.h"
#include "../level5/C3DLunarLander.h"
#include "../C3DRocketShip.h"
#include "../C3DGrannyObj.h"
#include "../C3DPermanentSprite.h"
#include "../C3DShadow.h"
#include "../C3DTriggerType.h"
#include "../C2DInGameMenu.h"
#include "../CJimmyGame.h"
#include "../CGameType.h"
#include "../InterfaceElement.h"
#include "../CanvasWrapper.h"
#include "../Physics.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Todo.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaTrigo.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMediaStreamOperators.h"
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

static const float kDegToAngle = 45.511112213134766f;		// 0x4cb228: 16384 / 360
static const float kAngleToDeg = 0.02197265625f;			// 0x4cb1f8: 360 / 16384

// --------------------------------------------------------------------------- helpers (copies)

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// 0x465690: the "climb down" keys (no joystick button). As the key tests of C3DPlayer.cpp:
// 0x1f5 left mouse, 0x1f4 right mouse, else a key of g_keys (which clears the mouse movement).
static bool PlayerKeyClimbDown(void)
{
	for (const int *list = g_key_climb_down; *list; list++)
	{
		const int k = *list;
		if (k == 0x1f5) { if (g_mouse_down) return true; }
		else if (k == 0x1f4) { if (g_mouse_rdown) return true; }
		else if ((unsigned)k < 256 && g_keys[k])			// port: index checked (the original reads g_keys[k] for any k)
		{
			g_56aebe = 0;									// 0x4a39e0
			g_56aec0 = 0;
			return true;
		}
	}
	return false;
}

// 0x485730: add to (or, negative, take from) Jimmy's energy g_health, kept in 0..100. Under 19
// the "low neutron energy" warning plays once (g_545fd4), above 20 it is armed again.
static void AddEnergy(float d)
{
	g_health = g_health + d;
	if (g_health > 100.0f)
	{
		g_health = 100.0f;
		g_545fd4 = 0;
		return;
	}
	if (g_health < 0.0f)
	{
		g_health = 0.0f;
		return;
	}
	if (g_health < 19.0f && g_health != 0.0f)
	{
		if (!g_545fd4)
		{
			PlayGameSound(-1, std::string("low neutron energy"), false);
			g_545fd4 = 1;
		}
		return;
	}
	if (g_health > 20.0f) g_545fd4 = 0;
}

// 0x404f20 (menu helper, copied): the type of item <item> of menu page <page> (2 = pickups).
static short GetItemState(short page, short item)
{
	C2DInGameMenu::MenuPage *p = C2DInGameMenu::Page(page);
	if (p && item >= 0 && item < C2DInGameMenu::MENU_ITEMS) return p->items[item].type;
	return 0;
}

// 0x409fe0 (menu helper, copied): the "?" help icon of the current help entry.
static void ShowHelpIcon(C2DInGameMenu *m, bool show)
{
	if (!show || m->f_594 != 0 || g_help_index == 0) g_help_icons[g_help_index]->hide(true);
	else g_help_icons[g_help_index]->force_show();
}

// 0x40a030 (menu helper, copied): close the help text.
static void CloseHelp(C2DInGameMenu *m, bool keep_icon)
{
	m->f_598 = 0;
	g_elt_544908[g_help_index].Hide();
	if (!keep_icon) ShowHelpIcon(m, true);
	g_help_menu_elt->Hide();
	m->f_594 = 0;
	m->f_59c = 0;
}

// 0x40a0c0 (menu helper, copied): open the help text for <secs> (only while playing).
static void OpenHelp(C2DInGameMenu *m, float secs)
{
	if (m->f_594 != 0) return;
	if (m->f_598 == 0) m->f_598 = secs;
	if (secs == 0) return;
	if (g_game_mode == 5)
	{
		g_help_icons[g_help_index]->hide(true);
		g_elt_544908[g_help_index].Show();
		g_help_menu_elt->Show();
		m->f_594 = -130;
		m->f_59c = 1;
		g_545ef4 = 0;
	}
	else g_545ef4 = (int)secs;
}

// CGameObject slot 226 (0x4ad990): (x, y, z) turned by the object's rotation, w = 0 (copy of
// C3DPlayer.cpp's; the generated declaration of v226 has the wrong signature).
static OMedia3DVector ObjectDir(CGameObject *o, float x, float y, float z)
{
	float rot[4];
	o->v201(rot);											// vtable +0x324
	const omt_Angle ax = (omt_Angle)((int)(rot[0] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle ay = (omt_Angle)((int)(rot[1] * kDegToAngle) & omc_MaxAngleMask);
	const omt_Angle az = (omt_Angle)((int)(rot[2] * kDegToAngle) & omc_MaxAngleMask);
	OMedia3DVector v(x, y, z, 0.0f);
	v.rotate(ax, ay, az);
	return v;
}

static OMedia3DVector Position(CGameObject *o)
{
	float p[4];
	o->v195(p);												// vtable +0x30c
	return OMedia3DVector(p[0], p[1], p[2], p[3]);
}

static float RotationY(CGameObject *o)
{
	float r[4];
	o->v201(r);												// vtable +0x324
	return r[1];
}

// The element (C3DObject family) of a game object found by tag, or NULL.
static C3DObject *Element(CGameObject *o)
{
	return o ? dynamic_cast<C3DObject *>(o) : NULL;
}

// Set up one of Jimmy's Granny props (the guns, the shades, the PDA): model file, the bone it
// sits on and the object it follows (us). Inline in the original each time.
static void SetupProp(C3DGrannyObj *g, const char *model, const char *bone, const char *attach_to, bool v36)
{
	strcpy(g->f_924, model);
	strcpy(g->f_ca8, bone);
	strcpy(g->f_c44, attach_to);
	g->v107(0);												// +0x1ac
	if (v36) g->v36(0);										// +0x90
	g->f_d70 = NULL;
	g->v52(0);												// +0xd0
	g->v46(0);												// +0xb8
	g->f_d7c = 0.5f;
}

// --------------------------------------------------------------------------- the guns

void C3DJimmy::ev144(void)											// 0x448b40
{
	ev116();
	if (!g_menu->IsCurrentGadget(10))
	{
		// the water gun: one balloon, faster on easy
		PlayGameSound(-1, std::string("wbgun"), false);
		float speed = 1500.0f;
		if (g_difficulty == 0) speed = 1800.0f;
		else if (g_difficulty == 1) speed = 1500.0f;
		else if (g_difficulty == 2) speed = 1400.0f;
		ev148(30.0f, 50.0f, 50.0f, f_4cd8, speed, NULL, 0.0f, 0.1f);
		ev58((char *)"SHOOT", 0);
		f_d54 = 0;
		f_4cd4 = 0;
		if (g_power_timers[5] <= 0.0f) AddEnergy(-1.0f);
		f_d20->HideAgain();
		return;
	}
	if (f_4cd0 < 20.0f)
	{
		// the burp gun is not charged yet
		PlayGameSound(-1, std::string("click shrink ray"), false);
		f_4cd4 = 0;
		f_d20->HideAgain();
		return;
	}
	// the burp gun: four big bubbles
	g_menu->ShowGadgetBar();
	PlayGameSound(-1, std::string("burpgun"), false);
	ev148(30.0f, 80.0f, 50.0f, f_4cd8, 800.0f, NULL, 0.0f, 90.0f);
	ev148(30.0f, 80.0f, 50.0f, f_4cd8, 800.0f, NULL, 0.1f, 85.0f);
	ev148(30.0f, 80.0f, 50.0f, f_4cd8, 800.0f, NULL, 0.2f, 80.0f);
	ev148(30.0f, 80.0f, 50.0f, f_4cd8, 800.0f, NULL, 0.3f, 75.0f);
	f_4cd0 = 0;
	ev58((char *)"SHOOT", 0);
	f_d54 = 0;
	f_4cd4 = 0;
	if (g_power_timers[5] <= 0.0f) AddEnergy(-1.0f);
	f_d20->HideAgain();
}

void C3DJimmy::ev148(float x, float y, float z, float pitch, float speed, CGameObject *target, float delay, float growth)	// 0x44b090
{
	CGameObject *self = this;
	const OMedia3DVector p = self->v224(x, y, z);				// +0x380 muzzle in the world
	C3DWaterBalloon *b = f_cd0[f_d50];
	if (!b) return;
	b->ShowAgain();												// sprite slot 53
	b->v136(1);
	b->v132(1);
	b->v196(p.x, p.y, p.z, p.w);								// +0x310
	b->f_660 = p;												// the muzzle
	OMedia3DVector d = ObjectDir(self, 0.0f, pitch * 0.022222223f, 1.0f);	// +0x388; 0x4df740
	const float k = 1.0f / d.quick_magnitude();
	d.x = k * d.x;
	d.y = k * d.y;
	d.z = k * d.z;
	b->v176(0, 0, 0);											// +0x2c0
	b->v157(0, 0, 0);											// +0x274
	b->v162(d.x * speed, d.y * speed, d.z * speed);				// +0x288
	b->f_62c = 0;
	b->f_634 = target;
	b->f_638 = speed;
	b->f_658 = growth;
	if (b->f_658 > 0.0f) b->set_size((float)b->m_sprite_size, (float)b->m_sprite_size);	// canvas = SpriteSize
	b->f_654 = 0;
	b->f_65c = delay;
	f_d50++;
	if (f_d50 >= 20) f_d50 = 0;
}

// The look of the 20 water balloons and of each one's trail sprite (C3DWaterBalloon +0x64c,
// that sprite's CGameObject part).
static void SetTrail(C3DWaterBalloon *b, int index, bool v136_off)
{
	C3DSprite *t = b->f_64c;
	if (!t) return;
	t->m_sprite_index = index;									// +0x4cc of its CGameObject part
	t->m_sprite_size = 100;
	t->InitSpriteProperties(100.0f);							// sprite slot 52
	if (v136_off) t->v136(0);
	t->HideAgain();												// sprite slot 54
}

void C3DJimmy::ev152(int kind)										// 0x44d9e0
{
	if (kind == 0)
	{
		for (int i = 0; i < 20; i++)
		{
			C3DWaterBalloon *b = f_cd0[i];
			if (!b) continue;
			b->m_sprite_index = 10;
			b->m_sprite_size = 45;
			b->set_flags(omelf_FaceViewport);					// +0x7c = 0x200
			b->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);	// +0xb4 = 6, +0xb8 = 7
			b->v258();											// reload the sprite
			b->HideAgain();
			SetTrail(b, 0x12, false);
		}
		return;
	}
	if (kind == 1)
	{
		for (int i = 0; i < 20; i++)
		{
			C3DWaterBalloon *b = f_cd0[i];
			if (!b) continue;
			unsigned int *seed = g_random_seed;
			*seed = *seed * 0x19660d + 0x7fff;
			const float r = (float)(((int)((*seed >> 16) * 1001) >> 16) & 0xffff) * 0.001f;	// 0x4cb218
			if (r < 0.1f) b->m_sprite_index = 0x15;
			else if (r < 0.2f) b->m_sprite_index = 0x16;
			else if (r < 0.4f) b->m_sprite_index = 0x17;
			else if (r < 0.5f) b->m_sprite_index = 0x18;
			else if (r < 0.6f) b->m_sprite_index = 0x19;
			else if (r < 0.8f) b->m_sprite_index = 0x18;
			else b->m_sprite_index = 0x17;
			b->m_sprite_size = 0x55;
			b->set_flags(omelf_SecondPass | omelf_DisableRotate | omelf_FaceViewport);	// +0x7c = 0x248
			b->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
			b->v258();
			b->HideAgain();
			SetTrail(b, 0x20, true);
		}
		return;
	}
	if (kind == 3)
	{
		for (int i = 0; i < 20; i++)
		{
			C3DWaterBalloon *b = f_cd0[i];
			if (!b) continue;
			b->m_sprite_index = 0x21;
			b->m_sprite_size = 0xf;
			b->set_flags(omelf_FaceViewport);
			b->v258();
			if (!g_windowed) b->set_blend(omblendfc_Src_Alpha, omblendfc_One);	// +0xb8 = 1
			else b->set_blend(omblendfc_Src_Alpha, omblendfc_Inv_Src_Alpha);
			b->HideAgain();
			SetTrail(b, 0x23, false);
		}
	}
}

// --------------------------------------------------------------------------- energy

unsigned char C3DJimmy::ev158(float dt)								// 0x448de0
{
	(void)dt;
	if (g_health > 0.0f || f_bec) return 0;
	if (f_4c10 != -1)
	{
		StopGameSound(f_4c10, false);
		f_4c10 = -1;
	}
	if (f_4c24 == -1) f_4c24 = PlayGameSound(-1, std::string("not available"), true);
	if (f_cbc) f_cbc->HideAgain();
	return 1;
}

void C3DJimmy::ev159(float dt)										// 0x449420
{
	if (f_d5c) ev160(dt * -0.05f);
}

void C3DJimmy::ev160(float d)										// 0x44bed0
{
	f_d60 = d + f_d60;
	if (f_d60 > 100.0f) f_d60 = 100.0f;
	if (f_d60 < 0.0f) f_d60 = 0.0f;
}

void C3DJimmy::ev119(OMedia3DVector plane)							// 0x44bc40
{
	if (f_4ce8 > 0.0f || f_4ca5) return;
	ev58((char *)"SHOCK", 1);
	f_4ca5 = 1;
	f_4c9c = 0;
	f_96c = 0;
	AddEnergy(-5.0f);
	AddScore(-5);
	PlayGameSound(-1, std::string("shock"), false);
	// pushed away from the wall
	const float k = 1.0f / plane.quick_magnitude();
	plane.x = k * plane.x;
	plane.y = k * plane.y;
	plane.z = k * plane.z;
	float p[4];
	v195(p);
	v196(plane.x * 50.0f + p[0], plane.y * 50.0f + p[1], plane.z * 50.0f + p[2], p[3]);	// 0x4ca4b0
}

void C3DJimmy::ev120(void)											// 0x44be70
{
	if (f_4ca4 || f_4da4 > 0.0f) return;
	ev147(0.5f);
	f_4ca4 = 1;
	f_4c9c = 0;
	AddEnergy(-5.0f);
	AddScore(-5);
}

void C3DJimmy::ev134(float energy, int points, float time, unsigned char shield)	// 0x44bd90
{
	if (f_4cb8 && energy > -100.0f) return;					// big hits (<= -100) always count
	if (f_8a7) return;
	if (g_game_mode == 3) return;
	if (f_4da4 > 0.0f && energy > -100.0f) return;
	float e = energy;
	if (shield)
	{
		if (GetItemState(2, 0x14)) ev147(time);				// pickup 20 taken
		else e = energy * 3.0f;
	}
	AddEnergy(e);
	f_4cb8 = 1;
	f_4c9c = 0;
	f_4cdc = time;
	AddScore(points);
}

// --------------------------------------------------------------------------- water

void C3DJimmy::EnterWater(void)										// 0x44bf20
{
	// (debug log "Calling EnterWater")
	if (f_4ca6 || f_4c0c != -1) return;
	PlayGameSound(-1, std::string("SPLASH"), false);
	f_4c0c = PlayGameSound(-1, std::string("water stream"), true);
}

void C3DJimmy::LeaveWater(void)										// 0x44bfa0
{
	// (debug log "Calling LeaveWater")
	if (f_4c0c != -1)
	{
		StopGameSound(f_4c0c, false);
		f_4c0c = -1;
	}
}

void C3DJimmy::ev130(void)											// 0x44bfe0
{
	PlayGameSound(-1, std::string("SPLASH"), false);
	PlayGameSound(-1, std::string("pain agh ahh"), false);
}

// --------------------------------------------------------------------------- footsteps

void C3DJimmy::ev115(void)											// 0x44b040
{
	if (f_4c20 != -1) return;
	CJimmyGame *game = dynamic_cast<CJimmyGame *>(g_gametype);
	ev153(game ? game->m_surface_sound_next : 0);			// g_gametype +0x560 (port: a CJimmyGame in every level)
}

void C3DJimmy::ev116(void)											// 0x44b060
{
	if (f_4c20 != -1)
	{
		StopGameSound(f_4c20, false);
		f_4c20 = -1;
	}
}

void C3DJimmy::ev153(int material)									// 0x44c080
{
	if (f_4c20 != -1)
	{
		StopGameSound(f_4c20, false);
		f_4c20 = -1;
	}
	if (g_game_mode != 5 || f_96c <= 0.0f) return;
	if (g_camera != static_cast<CGameObject *>(this)) return;
	const char *name;
	switch (material)
	{
		case 5:   name = "steps"; break;
		case 35:  name = "Lava"; break;
		case 123: name = "steps mud"; break;
		case 124: name = "steps concrete"; break;
		case 125: name = "steps leaves"; break;
		case 126: name = "steps concrete gritty"; break;
		case 127: name = "steps deck"; break;
		case 128: name = "steps dirt"; break;
		case 129: name = "steps metal"; break;
		default:  return;
	}
	f_4c20 = PlayGameSound(-1, std::string(name), true);
}

// --------------------------------------------------------------------------- places

void C3DJimmy::ev150(void)											// 0x44c2a0
{
	f_4c00 = g_gametype->m_object_id;						// +0x490
	float p[4];
	v195(p);
	f_4bf0 = OMedia3DPoint(p[0], p[1], p[2], p[3]);
}

void C3DJimmy::ev151(void)											// 0x44c2f0
{
	f_4c00 = g_gametype->m_object_id;
	float p[4];
	v195(p);
	f_4bf0 = OMedia3DPoint(p[0], p[1], p[2], p[3]);
	g_menu->ShowHudPicture(19, true, 200, 18);				// +0x4f4
}

void C3DJimmy::ev149(float spin, CGameObject *target, C3DObject *obj)	// 0x44e420
{
	if (!obj) return;
	if (!target)
	{
		obj->HideAgain();
		return;
	}
	const OMedia3DVector p = Position(target);
	const float vx = g_viewport->getx(), vy = g_viewport->gety(), vz = g_viewport->getz();	// 2002 +0x44
	OMedia3DVector d(p.x - vx, p.y - vy, p.z - vz, 0.0f);
	const float k = 1.0f / d.quick_magnitude();
	d.x = k * d.x * 300.0f;									// 0x4caa7c
	d.y = k * d.y * 300.0f;
	d.z = k * d.z * 300.0f;
	CGameObject *o = obj;
	o->v196(d.x + vx, d.y + vy, d.z + vz, 1.0f);			// +0x310
	o->AddRotation(0, 0, spin * -100.0f);					// +0x330
	obj->ShowAgain();
}

void C3DJimmy::ev161(unsigned char show, int which)					// 0x44e3b0
{
	if (which != 1) return;
	CGameObject *a = f_d5c->f_1b80;							// Goddard's arrow (a CGameObject part)
	C3DObject *o = a ? dynamic_cast<C3DObject *>(a) : NULL;
	if (!o) return;												// port: the original calls through NULL
	if (show) o->ShowAgain();
	else o->HideAgain();
}

// --------------------------------------------------------------------------- the grappling hook

unsigned char C3DJimmy::ev141(float dt)								// 0x449050
{
	if (!g_menu->IsCurrentGadget(7)) return 0;
	if (f_4db8)
	{
		// on the rope: reel in down to 250
		if (f_4e60 > 250.0f)
		{
			f_4e60 = f_4e60 - dt * 250.0f;
			return 1;
		}
		if (f_4c08 != -1)
		{
			StopGameSound(f_4c08, false);
			f_4c08 = -1;
		}
		return 1;
	}
	if (f_ccc->f_9f4 != -1)
	{
		StopGameSound(f_ccc->f_9f4, false);					// 0x48f0d0
		f_ccc->f_9f4 = -1;
	}
	CGameObject *self = this;
	const OMedia3DVector pos = Position(self);
	const OMedia3DVector up = self->v224(0.0f, 2400.0f, 2400.0f);	// +0x380
	if (!PhysicsLineBlocked(pos, up, false)) return 1;
	if (f_d5c)
	{
		f_d5c->HideAgain();
		f_d5c->v52(0);
		f_d5c->v132(0);
	}
	OMedia3DPoint &a = f_4dbc[f_4e78];
	a = OMedia3DPoint(g_phys_ray.end.x, g_phys_ray.end.y, g_phys_ray.end.z, g_phys_ray.end.w);	// 0x6311f8
	const float dx = a.x - pos.x, dy = a.y - pos.y, dz = a.z - pos.z;
	const float len = sqrtf(dx * dx + dy * dy + dz * dz) - 100.0f;
	f_4e60 = len;
	if (f_4e5c == 0.0f)
	{
		// the first rope: the hook's model is stretched to its length
		if (f_d20) f_d20->f_590->scale(1.0f, 1.0f, len * 0.0016666667f);	// shape vtable +0x34; 0x4d92c8
		f_4e5c = f_4e60;
	}
	f_4db8 = 1;
	f_b00 = 1;
	f_a8c = 0;
	JimmyJetpackFlame(self, 0.0f);							// jetpack +0x9f0 -> +0x928 = 0
	if (f_4c08 != -1)
	{
		StopGameSound(f_4c08, false);
		f_4c08 = -1;
	}
	if (f_4c08 == -1) f_4c08 = PlayGameSound(-1, std::string("ropeclimb"), true);
	f_4e64 = 0;
	f_4e68 = 0;
	{
		float vz[4], vx[4];
		v174(vz);											// (the original reads the velocity twice)
		v174(vx);
		v176(vx[0], 100.0f, vz[2]);
	}
	f_4e6c = 0;
	f_a6c.y = 400.0f;
	f_a6c.z = -650.0f;
	f_4e70 = (a.y - pos.y) - 200.0f;
	f_af0.x = 0.8f;
	f_af0.y = 1.0f;
	f_af0.z = 0.8f;
	if (f_d20) f_d20->ShowAgain();
	return 1;
}

void C3DJimmy::ev143(OMedia3DPoint anchor, OMedia3DVector pos, int swing, float len)	// 0x448e80
{
	if (f_4db8) return;
	f_4cf8 = swing;
	CGameObject *self = this;
	if (f_4cfc) ActivateMenuItem(self, self, 7, 0.0f);
	else
	{
		ActivateMenuItem(self, self, 7, 12.0f);
		f_4cfc = 1;
	}
	f_4db8 = 1;
	OMedia3DPoint &a = f_4dbc[f_4e78];
	a = anchor;
	f_4e74 = len;
	const float dx = a.x - pos.x, dy = a.y - pos.y, dz = a.z - pos.z;
	f_4e60 = sqrtf(dx * dx + dy * dy + dz * dz);
	if (f_4e60 > len - 200.0f) f_4e60 = len - 200.0f;
	f_4db8 = 1;
	f_b00 = 1;
	f_a8c = 0;
	JimmyJetpackFlame(self, 0.0f);
	f_4e64 = 0;
	f_4e68 = 0;
	f_4e6c = 0;
	f_a6c.y = 400.0f;
	f_a6c.z = -650.0f;
	f_af0.x = 0.8f;
	f_af0.y = 1.0f;
	f_af0.z = 0.8f;
	f_4e70 = (f_4dbc[f_4e78].y - pos.y) - 100.0f;
}

void C3DJimmy::ev142(void)											// 0x44b3a0
{
	if (!f_4db8) return;
	f_4db8 = 0;
	f_b00 = 0;
	g_5461cc = 0;
	f_af0.x = 1.6f;
	f_af0.z = 1.6f;
	f_4e64 = f_4e64 * 0.96f;								// 0x4e5620
	f_af0.y = 1.0f;
	f_a6c = OMedia3DPoint(0.0f, 50.0f, -350.0f, f_a6c.w);
	f_4e68 = f_4e68 * 0.96f;
	f_a7c = OMedia3DPoint(0.0f, 130.0f, 100.0f, f_a7c.w);
	v176(0, 0, 0);
	ev58((char *)"FALL", 1);
	if (f_4c08 != -1)
	{
		StopGameSound(f_4c08, false);
		f_4c08 = -1;
	}
	if (f_d20) f_d20->HideAgain();
}

unsigned char C3DJimmy::ev140(float dt)								// 0x44b490
{
	// the rope sound runs while Action (not Jump) is held on the rope
	if (PlayerKeyAction() && !PlayerKeyJump())
	{
		if (f_4db8)
		{
			if (f_4c08 == -1) f_4c08 = PlayGameSound(-1, std::string("ropeclimb"), true);
		}
		else if (f_4c08 != -1)
		{
			StopGameSound(f_4c08, false);
			f_4c08 = -1;
		}
	}
	else if (f_4db8 && f_4c08 != -1)
	{
		StopGameSound(f_4c08, false);
		f_4c08 = -1;
	}
	if (!g_menu->IsCurrentGadget(7)) return 0;
	CGameObject *self = this;
	if (f_4db8)
	{
		if (PlayerKeyGo() || f_c14 > 80.0f) v218(20.0f, RotationY(self), 0);		// +0x368
		if (PlayerKeyBack() || f_c14 < -80.0f) v218(-20.0f, RotationY(self), 0);
		OMedia3DVector pos = Position(self);
		pos.y = pos.y + 116.0f;								// 0x4e5624: the hands
		const OMedia3DPoint a = f_4dbc[f_4e78];
		ev114((char *)"SWING");
		OMedia3DVector d(a.x - pos.x, a.y - pos.y, a.z - pos.z, 0.0f);
		if (f_4cf8)
		{
			d.x = dt * f_4e64 * 30.0f + d.x;				// 0x4ca4b4
			d.z = dt * f_4e68 * 30.0f + d.z;
		}
		const float k = 1.0f / d.quick_magnitude();
		d.x = k * d.x * f_4e60;
		d.y = k * d.y * f_4e60;
		d.z = k * d.z * f_4e60;
		OMedia3DPoint np(a.x - d.x, a.y, a.z, a.w);
		np.y = a.y - (d.y + 116.0f);
		np.z = a.z - d.z;
		{
			float unused[4];
			v195(unused);									// (read and not used)
		}
		v196(np.x, np.y, np.z, np.w);						// +0x310
		if (f_4cf8)
		{
			v111(0);										// +0x1bc
			float vz[4], vx[4];
			v174(vz);
			v174(vx);
			v176(vx[0], 0, vz[2]);
		}
		else
		{
			v176(0, 0, 0);
			float az[4], ax[4];
			v155(az);										// +0x26c
			v155(ax);
			v157(ax[0], -g_gravity.y, az[2]);				// 0x63119c: hold against gravity
		}
		f_4e6c = dt + f_4e6c;
		if (f_4cf8)
		{
			const double tx = (double)d.x * dt * 0.2;		// 0x4d6c38 (x87: not rounded)
			const float tz = (float)((double)d.z * dt * 0.2);
			f_4e64 = (float)((double)f_4e64 - tx);
			f_4e68 = f_4e68 - tz;
		}
		if (f_d20)
		{
			// the hook between the anchor and the hands
			omt_Angle ax, ay;
			d.angles(ax, ay);
			f_d20->HideAgain();
			CGameObject *hook = f_d20;
			hook->v203((float)ax * kAngleToDeg, (float)ay * kAngleToDeg, 0);	// +0x32c
			hook->v196(np.x, np.y + 116.0f, np.z, np.w);
			// the shadow on the floor below us
			const OMedia3DVector p = Position(self);
			OMedia3DVector down(p.x, p.y - 2500.0f, p.z, p.w);	// 0x4e2bac
			if (!g_windowed)
			{
				if (PhysicsClosestHit(p, down, false))
				{
					const float hit_y = g_phys_ray.end.y;
					const OMedia3DVector q = Position(self);
					f_cc4->v197(q.x, hit_y + 30.0f, q.z);			// +0x314 of its CGameObject part
					if (!g_windowed) f_cc4->ShowAgain();
				}
				else f_cc4->HideAgain();
			}
		}
	}
	// Action climbs up, the climb-down keys let the rope out
	if (PlayerKeyAction() && !PlayerKeyJump())
	{
		if (f_4e60 <= 100.0f) return 0;
		if (ev77((char *)"CLIMBROPE")) ev58((char *)"CLIMBROPE", 1);
		if (f_4c08 == -1) f_4c08 = PlayGameSound(-1, std::string("ropeclimb"), true);
		f_4e60 = f_4e60 - dt * 100.0f;
		return 0;
	}
	if (PlayerKeyClimbDown())
	{
		if (f_4e74 - 200.0f <= f_4e60) return 0;
		if (ev77((char *)"CLIMBROPE")) ev58((char *)"CLIMBROPE", 1);
		if (f_4c08 == -1) f_4c08 = PlayGameSound(-1, std::string("ropeclimb"), true);
		f_4e60 = dt * 100.0f + f_4e60;
	}
	return 0;
}

void C3DJimmy::v220(float a, float b, float c, float d, OMedia3DPolygon *poly)	// 0x44bbe0
{
	if (g_menu->IsCurrentGadget(7) && f_4db8) return;			// no wall contact on the rope
	C3DPlayer::v220(a, b, c, d, poly);
}

// --------------------------------------------------------------------------- the gadgets

void C3DJimmy::ActivateMenuItem(CGameObject *o, CGameObject *who, short item, float value)	// 0x449df0
{
	(void)o; (void)who;
	CGameObject *self = this;
	self->Log("C3DJimmy::ActivateMenuItem()");				// +0x3f8
	if (g_cur_gadget == item && g_cur_gadget != 0)
	{
		// (debug log "ActivateMenuItem Already have the ItemNum= %d  Time= %d")
		return;
	}
	// (debug log "ActivateMenuItem ItemNum= %d  Time= %d")
	bool save = false;									// autosave after taking a vehicle / gun
	CGameObject *cam = NULL;							// becomes the camera (the object controlled)
	ev139();
	switch (item)
	{
		case 0:
		case 8:
			cam = self;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			f_4d7c = 0;
			break;

		case 1:
		{
			CGameObject *r = FindObjectByTag("C3DROCKETSHIP");
			if (!r) break;
			f_cc4->SetSize(150, 50);								// 0x46ec30
			Element(r)->ShowAgain();
			r->v52(1);
			r->v132(1);
			{
				const OMedia3DVector p = Position(self);
				r->v197(p.x, p.y, p.z);							// +0x314
				float rot[4];
				v201(rot);
				r->v203(rot[0], rot[1], rot[2]);				// +0x32c
			}
			r->f_00b = 0;
			if (C3DRocketShip *rs = dynamic_cast<C3DRocketShip *>(r)) rs->f_9f4 = this;	// 0x449fc0: the rider (port: checked)
			ev116();
			v136(0);
			ev58((char *)"DRIVE", 1);
			v120(1);
			if (f_d5c)
			{
				if (C3DRocketShip *rs = dynamic_cast<C3DRocketShip *>(r)) rs->f_11f4 = f_d5c;	// 0x44a006: Goddard on board
				f_d5c->v120(1);
				f_d5c->v132(0);
				f_d5c->f_1b60 = 1;								// riding along
				f_d5c->ev58((char *)"ROCK", 1);
				f_d5c->v136(0);
			}
			save = true;
			cam = r;
			break;
		}

		case 2:
			cam = self;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			f_4d7c = 0;
			f_4ccc = 0;
			f_4a9 = 1;
			f_4cd0 = 0;
			if (g_menu)
			{
				g_menu->GadgetBarEmpty();
				g_menu->ShowGadgetBar();
			}
			break;

		case 3:
			cam = self;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			f_4d7c = 0;
			g_flag_545e54 = 1;
			f_ccc->ShowAgain();
			break;

		case 4:
		{
			CGameObject *j = FindObjectByTag("C3DJEEP");
			if (!j) break;
			ev116();
			ev58((char *)"SCOOT", 1);
			v136(0);
			v132(0);
			f_4cfd = 1;
			if (f_d5c)
			{
				f_d5c->HideAgain();
				f_d5c->f_1b50->HideAgain();						// his shadow, sprite +0xd8
				f_d5c->v136(0);
				f_d5c->v132(0);
			}
			NEG_TODO(0x44a275, "C3DJeep +0x9c4 (cgo) = Jimmy (driver)");
			j->v136(1);
			j->v132(1);
			{
				const OMedia3DVector p = Position(f_d5c);
				j->v90(p.x, p.y, p.z);							// +0x168
				float rot[4];
				f_d5c->v201(rot);
				j->v92(rot[0], rot[1], rot[2]);					// +0x170
			}
			Element(j)->ShowAgain();
			NEG_TODO(0x44a3ad, "C3DJeep element slot 141 (73, -30, 3, 3, 1, 1, 1)");
			save = true;
			cam = j;
			break;
		}

		case 5:
		{
			CGameObject *t = FindObjectByTag("C3DVRTANK");
			if (!t) break;
			NEG_TODO(0x44a3d8, "C3DVRTank +0x9c4 (cgo) = Jimmy (driver)");
			t->v132(1);
			v261();
			ev58((char *)"DRIVE", 1);
			v136(0);
			v132(0);
			if (f_d5c)
			{
				f_d5c->v136(0);
				f_d5c->v132(0);
				f_d5c->ev107(7);								// element +0x1ac
				f_d5c->ev58((char *)"SIT", 1);
			}
			save = true;
			cam = t;
			break;
		}

		case 6:
			// the water gun in the right hand and the shades
			cam = self;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			f_4d7c = 0;
			if (!f_d28) f_d28 = new C3DGrannyObj();				// (debug log "C3DGrannyObj")
			if (f_d28)
			{
				SetupProp(f_d28, "newgunbase.grn", "Bip01 R Hand", m_tag, true);
				f_d28->v260();
			}
			if (!f_d30) f_d30 = new C3DGrannyObj();
			if (f_d30)
			{
				SetupProp(f_d30, "Shades.grn", "Bip01 Head", m_tag, false);
				f_d30->v260();
			}
			ev152(0);
			f_a7c.x = 0;
			f_a7c.y = 100.0f;
			f_a7c.z = 30.0f;
			f_a6c.x = 0;
			f_a6c.y = 300.0f;
			f_a6c.z = -350.0f;
			f_d20->ev71(0.5f);
			if (f_d28) f_d28->ShowAgain();
			save = true;
			break;

		case 7:
			// the grappling hook
			ev116();
			cam = self;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			v111(0);
			f_4d7c = 0;
			f_a6c.x = 0;
			f_a6c.y = 50.0f;
			f_a6c.z = -350.0f;
			f_a7c.x = 0;
			f_a7c.y = 150.0f;
			f_a7c.z = 80.0f;
			break;

		case 9:
		{
			CGameObject *m = FindObjectByTag("C3DMINISUB");
			if (!m) break;
			ev58((char *)"DRIVE", 1);
			v136(0);
			v132(0);
			v120(1);
			if (C3DMiniSub *sub = dynamic_cast<C3DMiniSub *>(m)) sub->f_9f4 = this;	// 0x44a912: the driver
			m->v132(1);
			if (f_d5c)
			{
				f_d5c->v120(1);
				f_d5c->v136(0);
				f_d5c->ev58((char *)"ROCK", 1);
				f_d5c->f_1b60 = 1;
				f_d5c->v132(0);
				f_d5c->ev121();									// element +0x1e4
				if (C3DMiniSub *sub = dynamic_cast<C3DMiniSub *>(m)) sub->f_9f8 = f_d5c;	// 0x44a9a8
			}
			save = true;
			cam = m;
			break;
		}

		case 10:
			// the burp gun and the shades
			cam = self;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			f_4d7c = 0;
			if (!f_d2c) f_d2c = new C3DGrannyObj();
			if (f_d2c)
			{
				SetupProp(f_d2c, "megaburpgun.grn", "Bip01 R Hand", m_tag, true);
				f_d2c->v260();
			}
			if (!f_d30) f_d30 = new C3DGrannyObj();
			if (f_d30)
			{
				SetupProp(f_d30, "Shades.grn", "Bip01 Head", m_tag, false);
				f_d30->v260();
			}
			ev152(3);
			f_a7c.x = 0;
			f_a7c.y = 160.0f;
			f_a7c.z = 30.0f;
			f_a6c.x = 0;
			f_a6c.y = 250.0f;
			f_a6c.z = -350.0f;
			f_d20->ev71(0.5f);
			g_menu->ShowGadgetBar();
			if (f_d2c) f_d2c->ShowAgain();
			save = true;
			break;

		case 11:
		{
			CGameObject *c = FindObjectByTag("C3DCRANEBOOM");
			if (!c) break;
			f_4d7c = 1;
			v261();
			ev58((char *)"DRIVE", 1);
			v136(0);
			g_5461cc = 0;
			PlayGameSound(-1, std::string("cranestart"), false);
			{
				C3DCraneBoom *b = dynamic_cast<C3DCraneBoom *>(c);
				if (b) b->m_driver = static_cast<CGameObject *>(this);	// 0x44adfb (port: checked)
			}
			cam = c;
			break;
		}

		case 12:
		{
			CGameObject *h = FindObjectByTag("C3DHOVERCYCLE");
			if (!h) break;
			C3DHoverCycle *hc = dynamic_cast<C3DHoverCycle *>(h);
			if (hc) hc->f_1ba4 = this;							// the rider
			h->v132(1);
			ev58((char *)"DRIVE", 1);
			v136(0);
			f_d5c->v136(0);
			if (hc && hc->f_1bb4 != -1)							// (sic: only when it is already playing)
				hc->f_1bb4 = PlayGameSound3D(this, -1, std::string("harley1"), true);	// 0x48f200
			save = true;
			cam = h;
			break;
		}

		case 13:
		{
			CGameObject *l = FindObjectByTag("C3DLUNARLANDER");
			if (!l) break;
			g_menu->HideGadgetBar();
			v136(1);
			v132(1);
			f_4d7c = 0;
			ev58((char *)"STOP", 1);
			v261();
			if (g_menu) g_menu->ShowGadgetBar();
			unsigned int *seed = g_random_seed;
			*seed = *seed * 0x19660d + 0x7fff;
			const float r = (float)(((int)((*seed >> 16) * 1001) >> 16) & 0xffff) * 0.001f;	// 0x4cb218
			l->v162(0, 750.0f, (r - 0.5f) * 250.0f);			// +0x288
			l->v203(0, 0, 0);
			PlayGameSound(-1, std::string("rocket blast"), false);
			C3DLunarLander *ll = dynamic_cast<C3DLunarLander *>(l);	// (l - 0xc0)
			if (ll)												// port: checked (the tag names a lander)
			{
				ll->m_landed = 0;								// 0x44afa3: +0x1aa4 (cgo)
				ll->m_fuel = 100.0f;							// +0x1aa8
			}
			cam = l;
			break;
		}

		default:
			// (debug log "Activating an invalid item with ID= %d")
			break;
	}
	if (cam) g_gametype->SetCamera(cam);					// element slot 73
	if (g_camera && g_menu && item < 14)
	{
		g_cur_gadget = item;
		if (!g_5461cd)
		{
			OpenHelp(g_menu, value);
			if (save && g_game_mode == 5) SaveTask("autosave.tsk");
		}
		else CloseHelp(g_menu, false);
		g_5461cd = 0;
	}
	// (debug log)
}

void C3DJimmy::ev139(void)											// 0x449450
{
	CGameObject *self = this;
	switch (g_menu->GetCurrentGadget())
	{
		case 0:
			v136(0);
			v132(0);
			f_4d7c = 1;
			break;

		case 1:
		{
			// (debug log "Deactivating Rocket")
			CGameObject *r = FindObjectByTag("C3DROCKETSHIP");
			if (!r) break;
			f_cc4->SetSize(70, 30);								// 0x46ec30
			Element(r)->HideAgain();
			r->v52(0);
			r->v132(0);
			C3DRocketShip *ours = static_cast<C3DRocketShip *>(f_cc8);
			ours->f_9f4 = NULL;									// 0x44952a: no rider
			if (C3DObject *f = Element(ours->f_1230)) f->HideAgain();	// 0x449536: its flame (element +0xdc); port: the original has no NULL test
			ev116();
			v136(1);
			v132(1);
			ev58((char *)"STOP", 1);
			v120(0);
			r->v120(1);
			C3DRocketShip *rs = dynamic_cast<C3DRocketShip *>(r);	// (r - 0xc0; port: checked)
			if (rs) rs->ev108(0);								// 0x4495b5: engine sound off
			if (f_d5c)
			{
				if (rs) rs->f_11f4 = NULL;						// 0x4495c7: Goddard off
				f_d5c->v120(0);
				f_d5c->f_1b60 = 0;
				f_d5c->v132(1);
				f_d5c->ev58((char *)"STOP", 1);
				f_d5c->v136(1);
				if (!g_windowed) f_d5c->f_1b50->ShowAgain();		// his shadow, sprite +0xd4
			}
			break;
		}

		case 2:
			ev65(0, 6, 7);
			f_4ccc = 20.0f;
			f_4a9 = 1;
			f_4cd0 = 0;
			if (g_menu) g_menu->HideGadgetBar();
			ev71(0);
			break;

		case 3:
			if (f_ccc)
			{
				g_flag_545e54 = 0;
				JimmyJetpackOff(self);							// 0x443370 on f_ccc
				f_ccc->HideAgain();
				f_968 = 600.0f;
			}
			break;

		case 4:
		{
			CGameObject *j = FindObjectByTag("C3DJEEP");
			if (!j) break;
			ev58((char *)"STOP", 1);
			v136(1);
			v132(1);
			if (!g_windowed) f_cc4->ShowAgain();
			{
				float p[4];
				v195(p);
				v197(p[0], p[1] - 40.0f, p[2]);					// 0x4d65c8: off the scooter
			}
			v157(0, 0, 0);
			v176(0, 0, 0);
			if (f_d5c)
			{
				f_d5c->v136(0);
				f_d5c->v132(0);
				f_d5c->ShowAgain();
				f_d5c->f_1b50->ShowAgain();						// his shadow, sprite +0xd4
				f_d5c->v136(1);
				f_d5c->v132(1);
			}
			f_d64 = 0.5f;
			NEG_TODO(0x449842, "C3DJeep +0x9c4 (cgo) = 0");
			j->v136(0);
			j->v132(0);
			Element(j)->HideAgain();
			NEG_TODO(0x449871, "C3DJeep element slot 140; +0xa78 (engine channel) stopped, -1");
			break;
		}

		case 5:
		{
			CGameObject *t = FindObjectByTag("C3DVRTANK");
			if (!t) break;
			NEG_TODO(0x4498b9, "C3DVRTank +0x9c4 (cgo) = 0");
			t->v132(0);
			v261();
			ev58((char *)"STOP", 1);
			v136(1);
			v132(1);
			if (f_d5c)
			{
				f_d5c->v136(1);
				f_d5c->v132(1);
				f_d5c->ev107(1);								// element +0x1ac
				f_d5c->ev58((char *)"SIT", 1);
			}
			NEG_TODO(0x44995c, "C3DVRTank +0x9b8 (cgo, channel): StopChannel if != -1");
			break;
		}

		case 6:
			if (f_d28)
			{
				f_d28->ev98(self, 0);
				f_d28->ev95(0);
			}
			if (f_d30)
			{
				f_d30->ev98(self, 0);
				f_d30->ev95(0);
			}
			f_ae8 = 0;
			f_d20->ev71(0);
			ev122();
			break;

		case 7:
			ev142();
			ev122();
			v111(1);
			break;

		case 8:
			break;

		case 9:
		{
			CGameObject *m = FindObjectByTag("C3DMINISUB");
			if (!m) break;
			ev58((char *)"STOP", 1);
			v136(1);
			v132(1);
			v120(0);
			if (C3DMiniSub *sub = dynamic_cast<C3DMiniSub *>(m)) sub->f_9f4 = NULL;	// 0x449a82
			m->v132(0);
			if (f_d5c)
			{
				f_d5c->v120(0);
				f_d5c->v136(1);
				f_d5c->ev58((char *)"SIT", 1);
				f_d5c->f_1b60 = 0;
				f_d5c->v132(1);
				if (C3DMiniSub *sub = dynamic_cast<C3DMiniSub *>(m)) sub->f_9f8 = NULL;	// 0x449b07
			}
			break;
		}

		case 10:
			if (f_d2c)
			{
				f_d2c->ev98(self, 0);
				f_d2c->ev95(0);
			}
			if (!f_d30)
			{
				f_d30 = new C3DGrannyObj();						// (debug log "C3DGrannyObj")
				if (f_d30) f_d30->v107(0);
			}
			if (f_d30)
			{
				f_d30->ev98(self, 0);
				f_d30->ev95(0);
			}
			f_ae8 = 0;
			ev122();
			f_d20->ev71(0);
			g_menu->HideGadgetBar();
			break;

		case 11:
		{
			CGameObject *c = FindObjectByTag("C3DCRANEBOOM");
			if (!c) break;
			f_4d7c = 0;
			ev58((char *)"STOP", 1);
			v136(1);
			v132(1);
			C3DCraneBoom *b = dynamic_cast<C3DCraneBoom *>(c);	// (port: checked)
			if (b)
			{
				StopGameSound(b->m_channel, false);			// 0x449c71 (0x48f0d0)
				b->m_driver = NULL;
			}
			break;
		}

		case 12:
		{
			CGameObject *h = FindObjectByTag("C3DHOVERCYCLE");
			if (!h) break;
			C3DHoverCycle *hc = dynamic_cast<C3DHoverCycle *>(h);
			if (hc) hc->f_1ba4 = NULL;
			h->v132(0);
			ev58((char *)"STOP", 1);
			v136(1);
			v132(1);
			f_d5c->v136(1);
			if (hc && hc->f_1bb4 != -1)
			{
				StopGameSound(hc->f_1bb4, false);				// 0x48f0d0
				hc->f_1bb4 = -1;
			}
			break;
		}

		case 13:
		{
			CGameObject *l = FindObjectByTag("C3DLUNARLANDER");
			if (!l) break;
			ev58((char *)"STOP", 1);
			v261();
			if (g_menu) g_menu->HideGadgetBar();
			break;
		}

		default:
			// (debug log "DeActivating an invalid item with ID= %d")
			break;
	}
	ShowHelpIcon(g_menu, false);
	CloseHelp(g_menu, true);
}

void C3DJimmy::ev137(int which)										// 0x44de40
{
	CGameObject *self = this;
	bool invent = false;
	switch (which)
	{
		case 1:
		case 4:
			ActivateMenuItem(self, self, 6, 12.0f);
			invent = which == 1;
			break;
		case 2:
		case 5:
			ActivateMenuItem(self, self, 10, 12.0f);
			invent = which == 2;
			break;
		case 3:
		{
			bool made = false;
			if (!f_d34)
			{
				f_d34 = new C3DGrannyObj();						// (debug log "C3DGrannyObj")
				made = true;
			}
			if (f_d34)
			{
				SetupProp(f_d34, "PDA.grn", "Bip01 R Hand", m_tag, true);
				if (made) f_d34->v260();
			}
			return;
		}
		default:
			return;
	}
	if (invent) ev58((char *)"INVENT", 1);
}

// --------------------------------------------------------------------------- animations

unsigned char C3DJimmy::ev162(char *name, char *anim)				// 0x44c390
{
	if (strcasecmp(name, anim) == 0 && !ev77(anim))
	{
		ev78(anim);
		return 1;
	}
	return 0;
}

// The gestures that stand Jimmy still (PICKUPGLASSES, INVENT, GET; inline in ev58): in the air
// they wait in f_4e80 for the floor; on the floor they turn him upright, stop him and lock the
// controls (g_5461ce, f_8a7) until the animation ends (ev67).
static void PlayGesture(C3DJimmy *j, char *name, char *anim, bool clear_4ca0)
{
	CGameObject *self = j;
	if (!self->v133())
	{
		strcpy(j->f_4e80, name);
		return;
	}
	j->ShowAgain();
	if (clear_4ca0) j->f_4ca0 = 0;
	strcpy(j->f_4e80, g_545f14);
	j->v203(0, RotationY(self), 0);
	j->v176(0, 0, 0);
	g_5461ce = 1;
	j->f_8a7 = 1;
	j->v132(0);
	j->C3DPlayer::v261();
	j->ev78(anim);
	j->ev86(anim, NULL, NULL, NULL, 0, 0.2f);
}

// The Granny props attached to someone ('3GRO' with f_d88) are dropped (inline in the original).
static void DropAttachedProps(void)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *g = *i;
		if (g && g->m_object_id == '3GRO')
		{
			C3DGrannyObj *prop = dynamic_cast<C3DGrannyObj *>(g);
			if (prop && prop->f_d88) prop->ev95(0);
		}
	}
}

void C3DJimmy::ev58(char *name, unsigned char loop)				// 0x44c3e0
{
	(void)loop;
	char *const o = NULL;										// b..d: none
	const float b = 0.2f;										// 0x3e4ccccd: blend of every animation
	CGameObject *self = this;
	// (debug log "EnteringPlaying Animation %s")
	if (f_8a7) return;
	if (ev162(name, (char *)"RUN"))
	{
		if (g_menu->IsCurrentGadget(6)) ev86((char *)"FULLRUNGUN", o, o, o, 1, b);
		else if (g_menu->IsCurrentGadget(10)) ev86((char *)"FULLBURPRUN", o, o, o, 1, b);
		else if (f_d38) ev86((char *)"FULLRUNHOLD", o, o, o, 1, b);
		else ev86((char *)"FULLRUN", o, o, o, 1, b);
	}
	if (ev162(name, (char *)"SHOOT")) ev86((char *)"FULLGUNSHOOT", o, o, o, 0, b);
	if (strcasecmp(name, "STOP") == 0)
	{
		if (ev77((char *)"SHOOT")) return;					// the shot finishes first
		if (!ev77((char *)"STOP") && g_menu)
		{
			ev78((char *)"STOP");
			if (g_menu->IsCurrentGadget(6)) ev86((char *)"FULLSTOPHOLD", o, o, o, 1, b);
			else if (g_menu->IsCurrentGadget(10)) ev86((char *)"FULLBURPSTOP", o, o, o, 1, b);
			else if (f_d38) ev86((char *)"FULLSTOPHOLD", o, o, o, 1, b);
			else ev86((char *)"FULLSTOP", o, o, o, 1, b);
		}
	}
	if (ev162(name, (char *)"POINT")) ev86((char *)"FULLGUNPOINT", o, o, o, 1, b);
	if (ev162(name, (char *)"RIGHT"))
	{
		ev116();
		if (g_menu)
		{
			if (g_menu->IsCurrentGadget(6)) ev86((char *)"FULLRIGHTHOLDGUN", o, o, o, 1, b);
			else ev86((char *)"FULLRIGHT", o, o, o, 1, b);
		}
	}
	if (ev162(name, (char *)"LEFT"))
	{
		ev116();
		if (g_menu)
		{
			if (g_menu->IsCurrentGadget(6)) ev86((char *)"FULLLEFTHOLDGUN", o, o, o, 1, b);
			else ev86((char *)"FULLLEFT", o, o, o, 1, b);
		}
	}
	if (strcasecmp(GetAnim(), "PICKUPGLASSES") != 0 && strcasecmp(name, "PICKUPGLASSES") == 0)
		PlayGesture(this, name, (char *)"PICKUPGLASSES", false);
	if (ev162(name, (char *)"PICKUPFLOOR"))
	{
		ShowAgain();
		v176(0, 0, 0);
		g_5461ce = 1;
		f_8a7 = 1;
		v132(0);
		C3DPlayer::v261();
		ev86((char *)"PICKUPFLOOR", o, o, o, 0, b);
	}
	if (!ev77((char *)"GETITEM") && strcasecmp(name, "GETITEM") == 0 && g_menu && !g_menu->IsCurrentGadget(6))
	{
		if (!self->v133()) strcpy(f_4e80, name);
		else
		{
			ShowAgain();
			v176(0, 0, 0);
			strcpy(f_4e80, g_545f14);
			v203(0, RotationY(self), 0);
			g_5461ce = 1;
			f_8a7 = 1;
			v132(0);
			C3DPlayer::v261();
			ev78((char *)"GETITEM");
			ev86((char *)"GETITEM", o, o, o, 0, b);
			ActivateMenuItem(self, self, 6, 12.0f);			// the water gun comes out
		}
	}
	if (strcasecmp(name, "INVENT") == 0 && !ev77((char *)"INVENT"))
		PlayGesture(this, name, (char *)"INVENT", true);
	if (ev162(name, (char *)"NO")) ev86((char *)"NO", o, o, o, 0, b);
	if (ev162(name, (char *)"LOOKPDA"))
	{
		v176(0, 0, 0);
		g_5461ce = 1;
		v132(0);
		C3DPlayer::v261();
		ev86((char *)"LOOKPDA", o, o, o, 0, b);
	}
	if (ev162(name, (char *)"TALKPDA")) ev86((char *)"TALKPDA", o, o, o, 1, b);
	if (ev162(name, (char *)"GIVEFLURP")) ev86((char *)"GIVEFLURP", o, o, o, 1, b);
	if (ev162(name, (char *)"JUMPJOY")) ev86((char *)"JUMPJOY", o, o, o, 1, b);
	if (ev162(name, (char *)"GETPDA"))
	{
		v176(0, 0, 0);
		DropAttachedProps();
		if (g_camera)
		{
			g_5461ce = 1;
			f_8a7 = 1;
			v132(0);
			C3DPlayer::v261();
		}
		ev86((char *)"GETPDA", o, o, o, 0, b);
		ev137(3);
	}
	if (ev162(name, (char *)"DRIVE")) ev86((char *)"DRIVE", o, o, o, 1, b);
	if (ev162(name, (char *)"PUTAWAY"))
	{
		ShowAgain();
		v176(0, 0, 0);
		g_5461ce = 1;
		f_8a7 = 1;
		v132(0);
		C3DPlayer::v261();
		ev86((char *)"PUTAWAY", o, o, o, 0, b);
	}
	if (ev162(name, (char *)"SITWAVE")) ev86((char *)"SITWAVE", o, o, o, 1, b);
	if (ev162(name, (char *)"PANIC")) ev86((char *)"PANIC", o, o, o, 1, b);
	if (ev162(name, (char *)"AFTERBURN")) ev86((char *)"AFTERBURN", o, o, o, 1, b);
	if (ev162(name, (char *)"SCOOT")) ev86((char *)"SCOOT", o, o, o, 1, b);
	if (ev162(name, (char *)"SCOOTSTOP")) ev86((char *)"SCOOTSTOP", o, o, o, 1, b);
	if (ev162(name, (char *)"SLIDE")) ev86((char *)"SLIDE", o, o, o, 1, b);
	if (ev162(name, (char *)"SWIM")) ev86((char *)"SWIM", o, o, o, 1, b);
	if (ev162(name, (char *)"FLOAT")) ev86((char *)"FLOAT", o, o, o, 1, b);
	if (ev162(name, (char *)"PICKUP"))
	{
		ShowAgain();
		v176(0, 0, 0);
		g_5461ce = 1;
		f_8a7 = 1;
		ev86((char *)"PICKUP", o, o, o, 0, b);
	}
	if (strcasecmp(name, "GET") == 0 && !ev77((char *)"GET"))
		PlayGesture(this, name, (char *)"GET", false);
	if (ev162(name, (char *)"JUMP") && g_menu)
	{
		if (g_menu->IsCurrentGadget(6)) ev86((char *)"JUMPGUN", o, o, o, 0, b);
		else ev86((char *)"JUMP", o, o, o, 0, b);
	}
	if (ev162(name, (char *)"FALL")) ev86((char *)"FALL", o, o, o, 1, b);
	if (ev162(name, (char *)"LEVER")) ev86((char *)"LEVER", o, o, o, 1, b);
	if (f_a68 != 4 && ev162(name, (char *)"BACK")) ev86((char *)"FULLBACK", o, o, o, 1, b);
	if (ev162(name, (char *)"TALK")) ev86((char *)"TALK", o, o, o, 1, b);
	if (ev162(name, (char *)"SHOCK"))
	{
		g_5461ce = 1;
		f_8a7 = 1;
		v132(0);
		C3DPlayer::v261();
		ev86((char *)"SHOCK", o, o, o, 0, b);
	}
	if (ev162(name, (char *)"FLY")) ev86((char *)"FLY", o, o, o, 1, b);
	if (ev162(name, (char *)"LADDER"))
	{
		ev86((char *)"LADDER", o, o, o, 0, b);
		f_4da0 = 0;
	}
	if (ev162(name, (char *)"FENCE"))
	{
		ev86((char *)"FENCE", o, o, o, 0, b);
		f_4da0 = 0;
	}
	if (ev162(name, (char *)"SWING")) ev86((char *)"SWING", o, o, o, 1, b);
	if (ev162(name, (char *)"TALKBODY")) ev86((char *)"TALKBODY", o, o, o, 1, b);
	if (ev162(name, (char *)"CLIMBROPE")) ev86((char *)"CLIMBROPE", o, o, o, 1, b);
	if (ev162(name, (char *)"TALKHERMAN")) ev86((char *)"TALKHERMAN", o, o, o, 1, b);
	if (ev162(name, (char *)"WORRYIDLE")) ev86((char *)"WORRYIDLE", o, o, o, 1, b);
	if (ev162(name, (char *)"WORRYTALK")) ev86((char *)"WORRYTALK", o, o, o, 1, b);
	if (ev162(name, (char *)"PROUD")) ev86((char *)"PROUD", o, o, o, 1, b);
	if (ev162(name, (char *)"EXCITETALK")) ev86((char *)"EXCITETALK", o, o, o, 1, b);
	if (ev162(name, (char *)"EXCITEIDLE")) ev86((char *)"EXCITEIDLE", o, o, o, 1, b);
	if (ev162(name, (char *)"ARGGH")) ev86((char *)"ARGGH", o, o, o, 1, b);
	if (ev162(name, (char *)"YAWN")) ev86((char *)"YAWN", o, o, o, 1, b);
	if (ev162(name, (char *)"ORDER")) ev86((char *)"ORDER", o, o, o, 1, b);
	if (ev162(name, (char *)"IDLE")) ev86((char *)"IDLE", o, o, o, 1, b);
	if (ev162(name, (char *)"HANGTALK")) ev86((char *)"HANGTALK", o, o, o, 1, b);
	// (debug log "Exiting Playing Animation %s")
}

void C3DJimmy::ev67(void)											// 0x44e0b0
{
	C3DPlayer::ev67();
	if (ev77((char *)"PICKUP"))
	{
		f_8a7 = 0;
		g_5461ce = 0;
		ev58((char *)"STOP", 0);
	}
	if (ev77((char *)"PUTAWAY"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
		if (f_d28) f_d28->ev95(0);
		if (f_d2c) f_d2c->ev95(0);
		DropAttachedProps();
		if (!g_camera) ev58((char *)"TALK", 1);
	}
	if (ev77((char *)"GET"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
	}
	if (ev77((char *)"SHOCK"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
	}
	if (ev77((char *)"GETPDA")) ev58((char *)"LOOKPDA", 0);
	if (ev77((char *)"LOOKPDA"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
	}
	if (ev77((char *)"INVENT"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
	}
	if (ev77((char *)"GETITEM"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
	}
	if (ev77((char *)"PICKUPFLOOR"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
		DropAttachedProps();
		if (f_4ca0) ev137(f_4ca0);
	}
	if (ev77((char *)"PICKUPGLASSES"))
	{
		g_5461ce = 0;
		f_8a7 = 0;
		v132(1);
		if (f_4ca0 == 1) ev137(1);
	}
	if (ev77((char *)"SHOOT")) ev86((char *)"STOP", NULL, NULL, NULL, 1, 0.2f);
}

// --------------------------------------------------------------------------- CGameObject family

unsigned char C3DJimmy::v133(void)									// 0x44d650
{
	if (f_4d90) return 1;
	return CLocalGameObject::v133();						// 0x48f010
}

void C3DJimmy::v261(void)											// 0x44c360
{
	C3DPlayer::v261();
	ev116();
	ev114((char *)"STOP");
}

void C3DJimmy::v271(void)											// 0x44e850
{
	C3DPlayer::v271();
	if (g_power_timers[5] <= 0.0f && f_d20) f_d20->HideAgain();
}

void C3DJimmy::v270(int old_mode, int new_mode)						// 0x44e590 "C3DJimmy::ChangeState"
{
	C3DAnimated::v270(old_mode, new_mode);
	// (debug log "C3DJimmy::ChangeState %d  %d")
	CGameObject *self = this;
	if (new_mode == 3)
	{
		// a cut scene starts
		ev116();
		if (g_power_timers[6] > 0.0f)
		{
			g_power_timers[6] = 0;							// the stopwatch
			CJimmyGame *game = dynamic_cast<CJimmyGame *>(g_gametype);
			if (game) game->StopwatchEnd();					// element slot 107
		}
		v203(0, RotationY(self), 0);
		v218(0, RotationY(self), 0);
		if (f_934 == -1 && strcasecmp("STOP", GetAnim()) == 0)
		{
			strcpy(f_938, "STOP");
			f_934 = 1;
			ev58((char *)"IDLE", 1);
		}
		f_4cb8 = 0;
		// the first polygon's material back to white (2002: shape +0x88 first polygon, +4 material)
		OMedia3DShape *sh = get_shape();
		if (sh && !sh->get_polygons()->empty())				// port: the original tests the polygon pointer
		{
			OMedia3DMaterial *m = sh->get_polygons()->front().get_material();
			if (m) m->set_diffuse(OMediaFARGBColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}
	if (old_mode == 3)
	{
		if (new_mode == 5)
		{
			// back to playing after a cut scene
			CJimmyGame *game = dynamic_cast<CJimmyGame *>(g_gametype);
			if (game) game->PlayLevelMusic(f_4c28, f_4c18);	// element slot 102
			ShowAgain();
			f_891 = 0;
			f_890 = 0;
			f_898 = 0;
			ShowAgain();
			ev71(0);
			ev91(0);
			if (f_d34)
			{
				ev88((char *)"none", f_d34);
				f_d34->HideAgain();
				f_d34->v136(0);
			}
		}
		if (f_934 != -1)
		{
			ev58(f_938, 1);
			f_934 = -1;
		}
		return;
	}
	if (old_mode == 1)
	{
		CGameObject *o = FindObjectByTag("POSTLOAD");
		if (o && o->NameContains((char *)"C3DAITRIGGER"))
		{
			C3DTriggerType *t = dynamic_cast<C3DTriggerType *>(o);
			if (t) t->CallNextTrigger("POSTLOAD", 0);		// sprite element slot 55
		}
	}
}

void C3DJimmy::WriteState(OMediaStreamOperators &s)					// 0x44d860
{
	C3DPlayerObject::WriteState(s);
	float v[4], a, b, c;
	v195(v); a = v[0];
	v195(v); b = v[1];
	v195(v); c = v[2];
	s << a << b << c;
	v201(v); a = v[0];
	v201(v); b = v[1];
	v201(v); c = v[2];
	s << a << b << c;
	s << f_4da4;
	int gadget = g_cur_gadget;
	s << gadget;
	// (debug log "MEBTEMP Saving ResetWhenDamaged=%d")
	s << f_4dac;
	// (debug log "MEBTEMP Saving FallSpeedDamage=%d")
	s << f_4db0;
	int n450 = g_54a450, n454 = g_54a454;
	s << n450;
	s << n454;
	bool no_physics = f_4d7c != 0;
	s << no_physics;
	s << f_c1c;
	s << f_4d58;
	s << f_4d94;
}

void C3DJimmy::ReadState(OMediaStreamOperators &s)					// 0x44d670
{
	C3DPlayerObject::ReadState(s);
	float a, b, c;
	s >> a >> b >> c;
	v197(a, b, c);											// position
	s >> a >> b >> c;
	v203(a, b, c);											// rotation
	{
		float r[4];
		v201(r);
		f_274 = r[0];
		f_278 = r[1];
		f_27c = r[2];
		f_280 = r[3];
	}
	s >> f_4da4;
	int gadget;
	s >> gadget;
	// (debug log "MEBTEMP calling LoadInfo with %d")
	CGameObject *self = this;
	ActivateMenuItem(self, self, (short)gadget, 12.0f);
	s >> f_4dac;
	// (debug log "MEBTEMP Loading ResetWhenDamaged=%d")
	s >> f_4db0;
	// (debug log "MEBTEMP Loading FallSpeedDamage=%d")
	int n450, n454;
	s >> n450;
	g_54a450 = n450;
	s >> n454;
	g_54a454 = n454;
	bool no_physics;
	s >> no_physics;
	f_4d7c = no_physics;
	s >> f_c1c;
	s >> f_4d58;
	if (g_camera == self && f_4d7c)
	{
		ErrorLog("ERROR: bNoPhysics was true but Player Controlled was Jimmy, reseting");
		f_4d7c = 0;
	}
	s >> f_4d94;
	f_d38 = NULL;
}
