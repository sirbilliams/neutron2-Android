/*  C3DButton - see C3DButton.h. Reconstructed from Neutron2.exe 0x4238e0..0x4244c0, 0x47d9a0,
    0x47da00. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DButton.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DMaterial.h"
#include "OMediaStreamOperators.h"
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

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'L1_A').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// --------------------------------------------------------------------------- construction

C3DButton::C3DButton()										// 0x4238e0
{
	SetName((char *)"C3DBUTTON");
	Log("C3DBUTTON()");
	if (LevelID() == 'LEV6' || LevelID() == 'LV6A')			// the rocket ship
	{
		strcpy(f_9a4, "buttondownship.ase");
		strcpy(f_a08, "buttonupship.ase");
	}
	else
	{
		strcpy(f_9a4, "buttondown.ase");
		strcpy(f_a08, "buttonup.ase");
	}
	strcpy(f_a6c, "switch.png");
	f_649 = 0;												// ev69 loads the animations later
	C3DButton::InitObject();								// 0x423fe0 (called directly)
	C3DObject::SetObjectID('3BUT');							// 0x49ab00
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
	f_b34 = 0;
	f_b38 = -1;												// Toggle
	f_b3c = 0;
	f_b40 = 4.0f;											// ResetTime
	strcpy(f_ad0, "none");									// AvailSoundName
	v34(1);
	f_b44 = 150.0f;											// Radius
	// (then a call to the empty 0x463890)
}

C3DButton::~C3DButton()										// 0x423bc0
{
}

namespace neg {
CGameObject *New_C3DButton(void) { return new C3DButton; }				// '3BUT' (0xb50 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DButton::InitObject(void)							// 0x423fe0
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("Red", &f_924, PROP_FLOAT, 0);
	RegisterProperty("Green", &f_928, PROP_FLOAT, 0);
	RegisterProperty("Blue", &f_92c, PROP_FLOAT, 0);
	RegisterProperty("ButtonAvailable", &f_934, PROP_LONG, 0);
	RegisterProperty("NASound", &f_938, PROP_LONG, 0);
	RegisterProperty("AvailSound", &f_93c, PROP_LONG, 0);
	RegisterProperty("AvailSoundName", f_ad0, PROP_ALPHA, 0);
	RegisterProperty("ActivateButton", f_940, PROP_ALPHA, 0);
	RegisterProperty("NewTaskState", &f_b34, PROP_LONG, 0);
	RegisterProperty("Toggle", &f_b38, PROP_LONG, 0);
	RegisterProperty("Down.ase", f_9a4, PROP_ALPHA, 0);
	RegisterProperty("Up.ase", f_a08, PROP_ALPHA, 0);
	RegisterProperty("UpDown.Png", f_a6c, PROP_ALPHA, 0);
	RegisterProperty("ResetTime", &f_b40, PROP_FLOAT, 0);
	RegisterProperty("Radius", &f_b44, PROP_FLOAT, 0);
	// (then a call to the empty 0x463890)
}

void C3DButton::ResetObject(void)							// 0x423e60
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	f_930 = f_934;											// ButtonAvailable
	OMedia3DMaterial *m = f_5d4[0];
	if (f_934 != 0)
	{
		if (m)
		{
			ev58((char *)"UP", 1);							// element +0xe8
			if (HasColour()) m->set_diffuse(OMediaFARGBColor(1.0f, f_924, f_928, f_92c));	// +0x4c..
		}
	}
	else if (m)
	{
		if (HasColour()) m->set_diffuse(OMediaFARGBColor(1.0f, f_924 * 0.5f, f_928 * 0.5f, f_92c * 0.5f));	// dimmed
		ev58((char *)"DOWN", 1);
	}
	v67(150.0f);											// +0x10c: radius
	v152(150.0f);											// +0x260
}

void C3DButton::AddTouching(CGameObject *other)				// 0x47d9a0
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (GetTaskState(m_task_name) < f_64c) return;			// 0x497a40: RequiredLevel reached
	if (!g_camera) return;
	if (other != g_camera && !other->NameContains((char *)"C3DWATERBALLOON")) return;
	ev95();													// element +0x17c
}

void C3DButton::v268(int state)								// 0x47da00
{
	(void)state;
	ev95();													// element +0x17c
}

void C3DButton::Process(float dt)							// 0x424190
{
	C3DAnimated::Process(dt);								// 0x41c120
	float p[4];
	v88(p);													// +0x160: the start position
	v196(p[0], p[1], p[2], p[3]);							// +0x310: held there
	if (f_b3c > 0)
	{
		f_b3c -= dt;
		if (f_b3c <= 0)
		{
			ResetObject();									// +0x28
			f_b3c = 0;
		}
	}
	if (strcasecmp(ev66(), "UP") != 0) return;				// +0x108
	if (!HasColour()) return;
	const double k = (double)g_545ddc * 0.5f + 0.5;			// 0x40b270: the pulse; 0x4cb210 (double)
	OMedia3DMaterial *m = f_5d4[0];
	if (m) m->set_diffuse(OMediaFARGBColor(1.0f, (float)(k * f_924), (float)(k * f_928), (float)(k * f_92c)));	// port: the original does not check
}

void C3DButton::v258(void)									// 0x4242c0
{
	CLocalGameObject::v258();								// 0x48eff0
	v67(300.0f);
	v152(300.0f);
}

void C3DButton::v260(void)									// 0x4242f0
{
	C3DAnimated::v260();									// 0x41cec0
	InitAnim();												// element +0x110
	v67(f_b44);												// Radius
}

void C3DButton::WriteState(OMediaStreamOperators &s)		// 0x424410
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	s << f_b3c;
	s << ev66();											// +0x108: the animation playing
}

void C3DButton::ReadState(OMediaStreamOperators &s)			// 0x4243c0
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	s >> f_b3c;
	char anim[100];											// (a stack buffer in the original)
	s >> anim;
	ev58(anim, 1);											// element +0xe8
}

// --------------------------------------------------------------------------- element family

void C3DButton::ev69(void)									// 0x424320
{
	if (f_649) return;
	f_649 = 1;
	CreateAnim((char *)"DOWN", f_9a4);						// +0xe0
	CreateAnim((char *)"UP", f_a08);
	CreateTexture(f_a6c, 0);								// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(300.0f);											// CGameObject +0x10c
	ev58((char *)"UP", 1);									// +0xe8
}

void C3DButton::ev95(void)									// 0x423c10
{
	if (!f_930) return;										// not available
	if (f_b3c > 0) return;									// just pressed
	OMedia3DMaterial *m = f_5d4[0];
	if (strcasecmp(GetAnim(), "UP") != 0)					// +0xf4: down -> up
	{
		ev58((char *)"UP", 1);
		if (HasColour() && m) m->set_diffuse(OMediaFARGBColor(1.0f, f_924, f_928, f_92c));	// port: the original does not check m
		CGameObject *t = FindObjectByTag(f_940);			// ActivateButton
		if (CLocalGameObject *l = dynamic_cast<CLocalGameObject *>(t)) l->v268(f_b38);	// +0x430: Toggle
		return;
	}
	f_b3c = f_b40;											// ResetTime
	if (strcasecmp(f_ad0, "none") != 0)
	{
		if (PlayGameSound(-1, std::string(f_ad0), false) == -1)	// 0x48f420
			PlayGameSound(-1, std::string("not available"), false);
	}
	else if (f_93c != -1)
		PlayGameSound(-1, std::string("not available"), false);
	ev58((char *)"DOWN", 1);
	if (HasColour() && m) m->set_diffuse(OMediaFARGBColor(1.0f, 0.2f, 0.2f, 0.2f));	// dark
	CGameObject *t = FindObjectByTag(f_940);
	if (!t) return;
	if (CLocalGameObject *l = dynamic_cast<CLocalGameObject *>(t)) l->v268(f_b38);	// +0x430
	if (!t->NameContains((char *)"C3DBUTTON")) return;		// +0x18
	C3DButton *b = dynamic_cast<C3DButton *>(t);			// port: the original writes +0x930 of whatever is named C3DBUTTON
	if (b) b->f_930 = 1;									// the other button becomes available
	f_930 = 0;
}
