/*  C3DWaterButton - see C3DWaterButton.h. Reconstructed from Neutron2.exe 0x47d3e0..0x47e050
    (16 and 268 are C3DButton's shared 0x47d9a0 / 0x47da00). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DWaterButton.h"
#include "../level3a/C3DWaterPlane.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DMaterial.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// --------------------------------------------------------------------------- helpers

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// The water plane (tagged "C3DWATERPLANE") and its level.
void C3DWaterButton::FindWater(void)
{
	f_ae4 = dynamic_cast<C3DWaterPlane *>(FindObjectByTag("C3DWATERPLANE"));	// 0x4aefb0
	if (!f_ae4) return;										// port: the original does not check
	float p[4];
	f_ae4->v195(p);											// +0x30c
	f_ae8 = p[1];
}

// --------------------------------------------------------------------------- construction

C3DWaterButton::C3DWaterButton()							// 0x47d3e0
{
	SetName((char *)"C3DWATERBUTTON");
	Log("C3DWATERBUTTON()");
	strcpy(f_9a4, "arrowbuttonin.ase");
	strcpy(f_a08, "arrowbutton.ase");
	strcpy(f_a6c, "arrow1.png");
	f_649 = 0;												// ev69 loads the animations later
	C3DWaterButton::InitObject();							// 0x47d810 (called directly)
	C3DObject::SetObjectID('3WAB');							// 0x49ab00
	v64(0.05f);
	v66(1000.0f);
	v46(0);
	v52(1);
	C3DObject::InitPhysics();								// 0x49acb0
	f_928 = 0.1f;
	f_92c = 0.1f;
	f_924 = 1.0f;
	f_930 = 0;
	f_934 = 0;												// ButtonAvailable
	f_938 = -1;
	f_93c = -1;
	strcpy(f_940, "none");									// ActivateButton
	f_ad0 = 0;
	f_ad4 = -1;												// Toggle
	f_ad8 = 0;
	f_adc = 1;												// UpLevels
	f_ae0 = 200.0f;											// LevelAmount
	v34(1);
	f_ae4 = NULL;											// port: set by v260 in the original
	f_ae8 = 0;
	f_aec = 0;
	// (then a call to the empty 0x463890)
}

C3DWaterButton::~C3DWaterButton()							// 0x47d640
{
}

namespace neg {
CGameObject *New_C3DWaterButton(void) { return new C3DWaterButton; }	// '3WAB' (0xaf8 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DWaterButton::InitObject(void)						// 0x47d810
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("Red", &f_924, PROP_FLOAT, 0);
	RegisterProperty("Green", &f_928, PROP_FLOAT, 0);
	RegisterProperty("Blue", &f_92c, PROP_FLOAT, 0);
	RegisterProperty("ButtonAvailable", &f_934, PROP_LONG, 0);
	RegisterProperty("NASound", &f_938, PROP_LONG, 0);
	RegisterProperty("AvailSound", &f_93c, PROP_LONG, 0);
	RegisterProperty("ActivateButton", f_940, PROP_ALPHA, 0);
	RegisterProperty("NewTaskState", &f_ad0, PROP_LONG, 0);
	RegisterProperty("Toggle", &f_ad4, PROP_LONG, 0);
	RegisterProperty("Down.ase", f_9a4, PROP_ALPHA, 0);
	RegisterProperty("Up.ase", f_a08, PROP_ALPHA, 0);
	RegisterProperty("UpDown.Png", f_a6c, PROP_ALPHA, 0);
	RegisterProperty("UpLevels", &f_adc, PROP_LONG, 0);
	RegisterProperty("LevelAmount", &f_ae0, PROP_FLOAT, 0);
	// (then a call to the empty 0x463890)
}

void C3DWaterButton::ResetObject(void)						// 0x47d690
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	f_930 = f_934;											// ButtonAvailable
	OMedia3DMaterial *m = f_5d4[0];
	if (f_934 == 0)
	{
		if (m)
		{
			if (HasColour()) m->set_diffuse(OMediaFARGBColor(1.0f, f_924 * 0.5f, f_928 * 0.5f, f_92c * 0.5f));	// dimmed
			ev58((char *)"DOWN", 1);						// element +0xe8
		}
	}
	else if (m)
	{
		ev58((char *)"UP", 1);
		if (HasColour()) m->set_diffuse(OMediaFARGBColor(1.0f, f_924, f_928, f_92c));	// +0x4c..
	}
	v67(100.0f);											// +0x10c: radius
	v152(100.0f);											// +0x260
}

void C3DWaterButton::AddTouching(CGameObject *other)		// 0x47d9a0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (GetTaskState(m_task_name) < f_64c) return;			// 0x497a40: RequiredLevel reached
	if (!g_camera) return;
	if (other != g_camera && !other->NameContains((char *)"C3DWATERBALLOON")) return;
	ev95();													// element +0x17c
}

void C3DWaterButton::v268(int state)						// 0x47da00
{
	(void)state;
	ev95();													// element +0x17c
}

// Under the water: black and not available. Above it: available, the reset timer, the red
// pulse while UP.
void C3DWaterButton::Process(float dt)						// 0x47dce0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	FindWater();
	if (!f_ae4) return;										// port: (the original does not check)
	OMedia3DMaterial *m = f_5d4[0];
	if (gety() <= f_ae8)									// element +0x48
	{
		f_930 = 0;
		if (m) m->set_diffuse(OMediaFARGBColor(1.0f, 0, 0, 0));	// port: the original does not check m
		return;
	}
	f_930 = 1;
	if (f_ad8 > 0)
	{
		f_ad8 -= dt;
		if (f_ad8 <= 0)
		{
			ResetObject();									// +0x28
			f_ad8 = 0;
		}
	}
	C3DAnimated::Process(dt);								// 0x41c120
	if (strcasecmp(ev66(), "UP") != 0) return;				// +0x108
	if (!HasColour()) return;
	const double k = (double)g_545ddc * 0.5f + 0.5;			// 0x40b270: the pulse; 0x4cb210 (double)
	if (m) m->set_diffuse(OMediaFARGBColor(1.0f, (float)k, 0.2f, 0.3f));	// port: the original does not check
}

void C3DWaterButton::v258(void)								// 0x47de90
{
	CLocalGameObject::v258();								// 0x48eff0
	v67(100.0f);
	v152(100.0f);
}

void C3DWaterButton::v260(void)								// 0x47dec0
{
	C3DAnimated::v260();									// 0x41cec0
	FindWater();
	float p[4];
	v195(p);												// +0x30c
	f_aec = (float)f_adc * f_ae0 + (float)((double)p[1] - 70.0);	// 0x4f4478 (double)
	InitAnim();												// element +0x110
}

// --------------------------------------------------------------------------- element family

void C3DWaterButton::ev69(void)								// 0x47df40
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"DOWN", f_9a4);						// +0xe0
	CreateAnim((char *)"UP", f_a08);
	CreateTexture(f_a6c, 0);								// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(100.0f);											// CGameObject +0x10c
	ev58((char *)"UP", 1);									// +0xe8
}

void C3DWaterButton::ev95(void)								// 0x47da20
{
	if (!f_930) return;										// not available
	if (f_ad8 > 0) return;									// just pressed
	OMedia3DMaterial *m = f_5d4[0];
	if (strcasecmp(GetAnim(), "UP") == 0)					// +0xf4
	{
		if (f_ae4)
		{
			if (f_ae4->f_928 != -99999.0f) return;			// CGameObject +0x868: the water is moving
			if (f_aec == f_ae8)								// already there
			{
				f_930 = 0;
				return;
			}
			f_ad8 = 4.0f;
			if (f_93c != -1) PlayGameSoundIndex(-1, f_93c, false);	// 0x48f030
			ev58((char *)"DOWN", 1);						// +0xe8
			if (HasColour() && m) m->set_diffuse(OMediaFARGBColor(1.0f, 0.2f, 0.2f, 0.2f));	// dark; port: the original does not check m
			float p[4], w[4];
			v195(p);										// +0x30c
			f_ae4->v195(w);
			if (p[1] <= w[1])
			{
				PlayGameSound(-1, std::string("waterbutton1"), false);	// 0x48f420
				return;
			}
			PlayGameSound(-1, std::string("click shrink ray"), false);
			PlayGameSound(-1, std::string("waterstart"), false);
			f_ae4->ev95(f_aec, 100.0f);						// element +0x17c: the water to there
		}
	}
	else
	{
		ev58((char *)"UP", 1);
		if (HasColour() && m) m->set_diffuse(OMediaFARGBColor(1.0f, f_924, f_928, f_92c));
	}
	CGameObject *t = FindObjectByTag(f_940);				// ActivateButton
	if (CLocalGameObject *l = dynamic_cast<CLocalGameObject *>(t)) l->v268(f_ad4);	// +0x430: Toggle
}
