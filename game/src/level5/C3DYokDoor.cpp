/*  C3DYokDoor - see C3DYokDoor.h. Reconstructed from Neutron2.exe 0x47f800..0x4801c0. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DYokDoor.h"
#include "../objects/C3DAITrigger.h"
#include "../objects/C3DCutSceneCamera.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DShape.h"
#include "OMediaStreamOperators.h"
#pragma GCC diagnostic pop
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DYokDoor::C3DYokDoor()									// 0x47f800
{
	SetName((char *)"C3DYOKDOOR");
	Log("C3DYokDoor()");
	C3DYokDoor::InitObject();								// 0x47fac0 (called directly)
	C3DObject::SetObjectID('3DOR');							// 0x49ab00
	v52(1);
	v42(0);
	v40(0);
	v94(2);
	v46(0);
	MakeShapeUnLit(get_shape());							// 0x4b2520
	f_648 = 0;												// animations off
	f_924 = 1;												// ItemClosed
	f_928 = 1;
	strcpy(f_92c, "none");									// Next
	f_9f8 = 0;
	f_9fc = 5.0f;											// DoorSpeed
	f_a00 = 3.0f;											// OpenTime
	f_a04 = 0;
	f_a08 = 0;
	f_a0c = 500.0f;											// OpenAmount
	strcpy(f_990, "objects.omt");							// OmtDatabase
	f_9f4 = 0;												// OmtIndex
	f_a10 = -1;
	f_6d0 = 2;												// no animation system
	v34(0);
	// (then a call to the empty 0x463890)
}

C3DYokDoor::~C3DYokDoor() {}								// 0x47fa10

namespace neg {
CGameObject *New_C3DYokDoor(void) { return new C3DYokDoor; }		// '3DOR' (0xa1c bytes)
}

// --------------------------------------------------------------------------- slots

void C3DYokDoor::InitObject(void)							// 0x47fac0
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("ItemClosed",  &f_924, PROP_LONG,  0);
	RegisterProperty("Next",        f_92c,  PROP_ALPHA, 0);
	RegisterProperty("DoorSpeed",   &f_9fc, PROP_FLOAT, 0);
	RegisterProperty("OpenTime",    &f_a00, PROP_FLOAT, 0);
	RegisterProperty("OpenAmount",  &f_a0c, PROP_FLOAT, 0);
	RegisterProperty("OmtDatabase", f_990,  PROP_ALPHA, 0);
	RegisterProperty("OmtIndex",    &f_9f4, PROP_LONG,  0);
	f_648 = 0;												// animations off
	InitPhysics();											// +0x2c
	// (then a call to the empty 0x463890)
}

void C3DYokDoor::ResetObject(void)							// 0x47fa60
{
	CLocalGameObject::ResetObject();						// 0x48ee80
	f_928 = f_924;
	if (f_924 == 0)
	{
		float p[4];
		v88(p);												// the initial position
		v197(p[0], p[1] + f_a0c, p[2]);						// +0x314: open
	}
}

void C3DYokDoor::AddTouching(CGameObject *other)			// 0x47fc70
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (other->NameContains((char *)"C3DYOKIAN")) v268(0);	// +0x430
}

unsigned char C3DYokDoor::v231(CGameObject *other)			// 0x47fc20
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3JIM' || id == '3ENE' || id == '3GOD' || id == '3WBL';
}

// Moving: y = start + OpenAmount * (the fraction of the way open). Stopped open: close after
// OpenTime.
void C3DYokDoor::Process(float dt)							// 0x47fcb0
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	if (f_9f8 > 0)											// 0x4caa44
	{
		f_9f8 = f_9f8 - dt;
		if (f_9f8 <= 0)
		{
			f_9f8 = 0;
			StopGameSound(f_a10, false);					// 0x48f0d0
			f_a10 = -1;
			PlayGameSound3DNoDoppler(this, -1, std::string("door closing"), false);	// 0x48f0f0
		}
		const float t = f_9f8 / f_9fc;
		float p[4];
		if (f_928)
		{
			v88(p);
			v197(p[0], p[1] + t * f_a0c, p[2]);				// +0x314
			return;
		}
		f_a04 = 0;
		v88(p);
		v197(p[0], p[1] + (1.0f - t) * f_a0c, p[2]);		// 0x4caa6c
		return;
	}
	if (f_928) return;
	f_a04 = dt + f_a04;
	if (f_a04 > f_a00)
	{
		f_a04 = 0;
		v268(1);
	}
}

void C3DYokDoor::v260(void)									// 0x47fbb0
{
	C3DAnimated::v260();									// 0x41cec0
	m_db = GetDatabase(f_990);								// 0x4a4180
	set_shape(Load3DShape(m_db, f_9f4));					// element +0xac (0x4b2c80)
	MakeShapeUnLit(get_shape());							// element +0xb0, 0x4b2520
	ev51();													// element +0xcc
}

void C3DYokDoor::v268(int state)							// 0x47fe70
{
	if (f_9f8 > 0)
	{
		if (f_928 == state) return;
		f_9f8 = f_9fc - f_9f8;								// turn back from where it is
	}
	else
	{
		if (f_928 == state) return;
		f_9f8 = f_9fc;
		if (f_a10 == -1) f_a10 = PlayGameSound3DNoDoppler(this, -1, std::string("Door opening loop"), true);	// 0x48f0f0
	}
	if (strcasecmp(f_92c, "none") != 0)						// 0x4c4520 _stricmp
	{
		CGameObject *next = FindObjectByTag(f_92c);
		if (next)
		{
			if (next->NameContains((char *)"C3DAITRIGGER"))
				static_cast<C3DAITrigger *>(next)->Trigger(this, 0);				// element +0xe4
			else if (next->NameContains((char *)"C3DCUTSCENECAMERA"))
			{
				// ("Calling TC from NextState in %s": compiled-out log)
				static_cast<C3DCutSceneCamera *>(next)->TriggerCamera(0);			// element +0xe0
			}
		}
	}
	if (state == 0) f_928 = 0;
	else if (state == 1) f_928 = 1;
	else f_928 = f_928 == 0;
}

void C3DYokDoor::v270(int a, int msg)						// 0x47fff0
{
	(void)a;
	if (msg == 3 && f_a10 != -1) StopGameSound(f_a10, false);	// 0x48f0d0 (the channel is kept)
}

void C3DYokDoor::WriteState(OMediaStreamOperators &s)		// 0x480010
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	s << f_a0c;
	s << f_928;
	s << f_9f8;
}

void C3DYokDoor::ReadState(OMediaStreamOperators &s)		// 0x480050
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	s >> f_a0c;
	s >> f_928;
	s >> f_9f8;
	const float t = f_9f8 / f_9fc;
	float p[4];
	if (f_928)
	{
		v88(p);
		v197(p[0], t * f_a0c + p[1], p[2]);					// +0x314
		return;
	}
	f_a04 = 0;
	v88(p);
	v197(p[0], (1.0f - t) * f_a0c + p[1], p[2]);			// 0x4caa6c
}
