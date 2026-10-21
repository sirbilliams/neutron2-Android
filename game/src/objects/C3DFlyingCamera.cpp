/*  C3DFlyingCamera - see C3DFlyingCamera.h. Reconstructed from Neutron2.exe 0x433720..0x4346b0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DFlyingCamera.h"
#include "../C3DTriggerType.h"
#include "../C3DJimmy.h"
#include "../C3DPlayer.h"
#include "../C2DInGameMenu.h"
#include "../CGameType.h"
#include "../CJimmyGame.h"
#include "../DatabaseCache.h"
#include "../Physics.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include "OMediaViewPort.h"
#include <math.h>
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// Cancel gravity: the acceleration's y becomes -gravity (x and z as they are). The original
// reads the acceleration twice (slot 155: z first, then x).
static void CancelGravity(C3DFlyingCamera *c)
{
	float a[4];
	c->v155(a);												// +0x26c
	const float z = a[2];
	c->v155(a);
	c->v157(a[0], -g_gravity.y, z);							// +0x274 (0x63119c)
}

// The view: from where we are, looking at CameraTarget (looked up once), or with "none" at our
// local point ViewDir. Shared code of Process (0x433d6f) and v283 (0x434290), compiled twice in
// the original. Returns without moving the view when CameraTarget names no object.
static void LookFromCamera(C3DFlyingCamera *c)
{
	OMedia3DVector target;
	if (strcasecmp(c->f_1b50, "none") == 0)
		target = c->v224(c->f_1bb8.x, c->f_1bb8.y, c->f_1bb8.z);	// +0x380
	else
	{
		if (!c->f_1bb4)
		{
			c->f_1bb4 = FindObjectByTag(c->f_1b50);
			if (!c->f_1bb4) return;
		}
		c->f_1bb4->v195(target.xyzw());						// +0x30c
	}
	OMedia3DVector pos;
	c->v195(pos.xyzw());
	c->ev97(pos, target);									// element +0x184: the viewport at pos, looking at target
}

// --------------------------------------------------------------------------- construction

C3DFlyingCamera::C3DFlyingCamera()							// 0x433720
{
	f_1bb8.w = 0;											// 0x1bc4
	SetName((char *)"C3DFLYINGCAMERA");
	Log("C3DFlyingCamera()");
	C3DFlyingCamera::InitObject();							// called directly (as shipped: the
															// loader's InitObject runs it again)
	SetObjectID('3FLY');
	f_96c = 3;												// patrolling only
	f_c08 = 3;
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 200.0f;											// Speed
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 300.0f;
	f_b9c.x = 1.0f;											// turn rates
	f_b9c.y = 1.0f;
	f_b9c.z = 1.0f;
	f_1bb8.x = 0;											// ViewDir
	f_1bb8.y = 20.0f;
	f_1bb8.z = 100.0f;
	C3DObject::InitPhysics();								// 0x49acb0
	v52(0);
	v42(0);
	v40(0);
	C3DAI::ev106(f_ac4, (char *)"WALK");
	C3DAI::ev106(f_a9c, (char *)"STOP");
	C3DAI::ev106(f_b14, (char *)"STOP");
	f_6d0 = 2;												// animation system 2: neither morphs nor Granny
	m_db = GetDatabase("camera.omt");						// 0x4a4180 (element +0x574)
	v34(1);
	f_648 = 0;												// animations off
	set_shape(Load3DShape(m_db, 0));						// 0x4b2c80, element slot 43
	f_c18 = 1;												// move the way we face
	strcpy(f_1b50, "none");
	strcpy(f_1c94, "none");
	f_1c2c = -1;
	strcpy(f_1c30, "none");
	f_008 = 0;												// element +0xc8
	f_1bb4 = NULL;
	f_1cfc = 0;
	strcpy(f_1bc8, "none");
	f_1d00 = -1;
	f_c48 = 0;
	f_1d04 = 1;
	f_1d08 = 0;
	f_1d0c = 0;
	f_658 = 0;												// HasCollision
	// (an empty debug log)
}

C3DFlyingCamera::~C3DFlyingCamera()							// 0x433a50
{
}

namespace neg {
CGameObject *New_C3DFlyingCamera(void) { return new C3DFlyingCamera; }	// '3FLY' (0x1d18 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DFlyingCamera::InitObject(void)						// 0x433ad0
{
	C3DAI::InitObject();
	RegisterProperty("Speed", &f_968, PROP_FLOAT, 0);		// (C3DAI::v260 replaces it with InitialSpeed)
	RegisterProperty("SoundDatabase", f_1c94, PROP_ALPHA, 0);
	RegisterProperty("SoundIndex", &f_1c2c, PROP_LONG, 0);
	RegisterProperty("SoundName", f_1c30, PROP_ALPHA, 0);
	RegisterProperty("CameraTarget", f_1b50, PROP_ALPHA, 0);
	RegisterProperty("TurnSpeedX", &f_b9c.x, PROP_FLOAT, 0);
	RegisterProperty("TurnSpeedY", &f_b9c.y, PROP_FLOAT, 0);
	RegisterProperty("TurnSpeedZ", &f_b9c.z, PROP_FLOAT, 0);
	RegisterProperty("ViewDirX", &f_1bb8.x, PROP_FLOAT, 0);
	RegisterProperty("ViewDirY", &f_1bb8.y, PROP_FLOAT, 0);
	RegisterProperty("ViewDirZ", &f_1bb8.z, PROP_FLOAT, 0);
	RegisterProperty("NextTrigger", f_1bc8, PROP_ALPHA, 0);
	RegisterProperty("LetterBox", &f_1d04, PROP_LONG, 0);
	RegisterProperty("BypassOff", &f_1d08, PROP_LONG, 0);
}

void C3DFlyingCamera::ResetObject(void)						// 0x433aa0
{
	C3DAI::ResetObject();
	f_1bb4 = NULL;
	f_1cfc = 0;
	v34(1);													// +0x88
}

unsigned char C3DFlyingCamera::v231(CGameObject *other)		// 0x434470
{
	(void)other;
	return 0;
}

// 0x433c50: patrol only while flying; a skip (the bypass key, or a cut scene already skipped)
// ends the flight at once. Always hidden, gravity cancelled. While flying, the end of the clip
// ends the flight; otherwise the view follows us.
void C3DFlyingCamera::Process(float dt)
{
	if (f_1cfc) C3DAI::Process(dt);
	else C3DAnimated::Process(dt);							// 0x41c120
	if ((PlayerKeyCutsceneBypass() || g_cutscene_skipped) && f_1cfc)	// 0x465ad0
	{
		g_cutscene_skipped = 1;
		if (IsChannelPlaying(f_1d00))
		{
			StopChannel(f_1d00, true);
			f_1cfc = 0;
			CallNextTrigger();								// element +0x24c
		}
		// (sic: a clip that has already ended leaves f_1cfc set; the test below ends the flight)
	}
	HideAgain();											// element +0xdc
	CancelGravity(this);
	if (!f_1cfc) return;
	if (f_1d00 != -1 && !IsChannelPlaying(f_1d00))
	{
		StopChannel(f_1d00, true);
		f_1cfc = 0;
		CallNextTrigger();
		return;
	}
	LookFromCamera(this);
}

void C3DFlyingCamera::v242(float t)							// 0x434180
{
	C3DAI::v242(t);											// 0x47e3c0
	if (f_1d0c && g_keys[0xc0])								// 0x56ae60 VK_OEM_3 (`): the preview ends
	{
		f_1d0c = 0;
		float v[4];
		v88(v);	v196(v[0], v[1], v[2], v[3]);				// back to where we were placed
		v91(v);	v202(v[0], v[1], v[2], v[3]);
	}
}

// 0x434220: in the editor (the argument is the frame time), Ctrl+L starts a preview of the
// flight from where we were placed; while it runs physics and the AI run here and the view
// follows us.
void C3DFlyingCamera::v283(unsigned int dt_bits)
{
	CLocalGameObject::v283(dt_bits);						// (CGameObject's, empty)
	float dt;
	memcpy(&dt, &dt_bits, sizeof dt);
	if (f_1d0c)
	{
		PhysicsProcess(f_294);								// 0x4b4c60 (+0x294: the frame time v4 kept)
		C3DAI::Process(dt);
		CancelGravity(this);
		LookFromCamera(this);
		return;
	}
	if (!g_keys['L'] || !g_keys[0x11]) return;				// 0x56adec, 0x56adb1 VK_CONTROL
	float v[4];
	v88(v);	v196(v[0], v[1], v[2], v[3]);
	f_1d0c = 1;
	f_1bb4 = NULL;
}

// --------------------------------------------------------------------------- element family

// 0x433f50: fired. Unless the cut scene is being skipped: back to where we were placed, the
// sound effects database swapped for SoundDatabase, and the clip started (the flight runs
// until it ends). Skipping goes straight on to NextTrigger.
void C3DFlyingCamera::TriggerCamera(void)
{
	if (f_1d08) g_cutscene_skipped = 0;						// BypassOff
	if (f_1cfc) return;
	if (f_1d04 == 1) CutsceneLetterbox(true);				// 0x40c0f0 (g_menu)
	if (PlayerKeyCutsceneBypass() || g_cutscene_skipped)
	{
		CallNextTrigger();									// +0x24c
		return;
	}
	float v[4];
	v88(v);													// CGameObject +0x160: the placed position
	v196(v[0], v[1], v[2], v[3]);							// +0x310
	f_1cfc = 1;
	if (g_active_sound_db)
	{
		CloseDatabase("SoundEffects.omt");					// 0x4a4730
		g_active_sound_db = NULL;
		g_sfx_db_loaded = 0;								// 0x54a4b8
	}
	if (f_1d04 == 1) g_music_volume = 0;					// 0x540c54
	g_active_sound_db = GetDatabase(f_1c94);
	if (!g_active_sound_db) return;							// (sic: flies on with no clip, f_1d00 as it was)
	if (StartSound())										// +0x244
	{
		f_1cfc = 1;
		return;
	}
	// (sic: the message names the database; f_1cfc stays set and f_1d00 is -1, so the flight
	// never ends by itself)
	ErrorLog("ERROR: Invalid SoundDatabase %s for %s", f_1c94, m_tag);
}

// 0x434090: SoundIndex, or SoundName when it is -1; chunk 0 when that does not play. 1 when
// a channel plays.
unsigned char C3DFlyingCamera::StartSound(void)
{
	if (f_1c2c != -1)
	{
		f_1d00 = PlaySoundIndex(-1, f_1c2c, false);			// 0x4b8240
		SetChannelVolume((short)f_1d00, (short)g_sfx_volume);	// 0x4b8e30, 0x540c58
		if (f_1d00 != -1) return 1;
	}
	else
	{
		f_1d00 = PlaySound(-1, std::string(f_1c30), false);	// 0x4b8380
		SetChannelVolume((short)f_1d00, (short)g_sfx_volume);
		if (f_1d00 != -1) return 1;
	}
	f_1d00 = PlaySoundIndex(-1, 0, false);
	SetChannelVolume((short)f_1d00, (short)g_sfx_volume);
	return f_1d00 != -1;
}

// 0x434480: NextTrigger "DEFAULT": the camera goes back to Jimmy (the view jumps to behind him
// when it is more than 1000 away), his level music and the sound effects come back, and he is
// unfrozen. Unlike C3DTriggerType's "DEFAULT" the letterbox stays.
void C3DFlyingCamera::ResetToDefault(void)
{
	if (strcasecmp(f_1bc8, "DEFAULT") != 0) return;
	// (debug log "Reseting to DEFAULT in C3DFlyingCamera", twice)
	g_545ef8 = 0;
	CGameObject *jim = FindObjectByTag("JIM1");
	if (g_camera == jim)
	{
		g_active_sound_db = GetDatabase("SoundEffects.omt");
		if (g_active_sound_db) g_sfx_db_loaded = 1;
		CLocalGameObject *l = static_cast<CLocalGameObject *>(jim);
		if (l && l->f_4ab) l->f_4ab = 0;					// port: l checked (both NULL: the original crashes)
		return;
	}
	if (jim)
	{
		g_gametype->SetCamera(jim);							// element +0x124
		const OMedia3DVector p = jim->v224(0, 500.0f, -500.0f);	// +0x380: behind and above him
		const float dx = g_viewport->getx() - p.x;			// +0x44..0x4c
		const float dy = g_viewport->gety() - p.y;
		const float dz = g_viewport->getz() - p.z;
		if (sqrtf(dz * dz + dy * dy + dx * dx) > 1000.0f)	// 0x4cb1d8
			g_viewport->place(p.x, p.y, p.z);
		jim = FindObjectByTag("JIM1");
		C3DJimmy *j = jim ? dynamic_cast<C3DJimmy *>(jim) : NULL;	// (JIM1 - 0xc0)
		if (j) static_cast<CJimmyGame *>(g_gametype)->PlayLevelMusic(j->f_4c28, j->f_4c18);	// element +0x198
		// port: the original reads the music of whatever JIM1 is (NULL - 0xc0 without one)
	}
	g_music_volume = g_6f7518;
	g_active_sound_db = GetDatabase("SoundEffects.omt");
	if (g_active_sound_db) g_sfx_db_loaded = 1;
	CLocalGameObject *l = static_cast<CLocalGameObject *>(jim);
	if (l && l->f_4ab) l->f_4ab = 0;						// port: l checked (no JIM1: the original crashes)
}

// 0x433e90: fire NextTrigger: "none" nothing, "DEFAULT" ResetToDefault, a C3DFlyingCamera its
// TriggerCamera, a C3DTriggerType its CallNextTrigger with its own name (so it fires itself).
void C3DFlyingCamera::CallNextTrigger(void)
{
	if (strcasecmp(f_1bc8, "none") == 0) return;
	if (strcasecmp(f_1bc8, "DEFAULT") == 0)
	{
		ResetToDefault();									// +0x248
		return;
	}
	CGameObject *o = FindObjectByTag(f_1bc8);
	if (!o) return;
	if (o->NameContains((char *)"C3DFLYINGCAMERA"))
	{
		// (debug log "Calling TC from CallNextTrigger in %s")
		C3DFlyingCamera *c = dynamic_cast<C3DFlyingCamera *>(o);	// (o - 0xc0)
		if (c) c->TriggerCamera();							// element +0x240
		return;
	}
	if (o->NameContains((char *)"C3DTRIGGERTYPE"))
	{
		C3DTriggerType *t = dynamic_cast<C3DTriggerType *>(o);	// (o - 0xc8)
		if (t) t->CallNextTrigger(f_1bc8, 0);				// element +0xdc
	}
}

// For the callers that know the object only by its name (C3DTriggerType::CallNextTrigger, the
// menu's fade): its element slot 144 (+0x240 on o - 0xc0).
void neg::FlyingCameraTrigger(CGameObject *o)
{
	C3DFlyingCamera *c = o ? dynamic_cast<C3DFlyingCamera *>(o) : NULL;	// port: only a C3DFlyingCamera has the slot
	if (c) c->TriggerCamera();
}
