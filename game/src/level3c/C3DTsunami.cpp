/*  C3DTsunami - see C3DTsunami.h. Reconstructed from Neutron2.exe 0x479a10..0x47a330. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DTsunami.h"
#include "../C3DJimmy.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>
#include <strings.h>
#include <string>

using namespace neg;

// The running level's ID (g_gametype +0x490: its ObjectID, e.g. 'LV3C').
static int LevelID(void)
{
	return static_cast<CGameObject *>(g_gametype)->m_object_id;
}

// 0x4aefb0: the first object of the global list whose ObjectTag matches (_stricmp).
static CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (*i && strcasecmp((*i)->m_tag, tag) == 0) return *i;
	return NULL;
}

// --------------------------------------------------------------------------- construction

C3DTsunami::C3DTsunami()									// 0x479a10
{
	SetName((char *)"C3DTSUNAMI");
	Log("C3DTsunami()");
	f_649 = 0;												// ev69 loads the animation later
	Log("InitObject()");
	C3DAI::InitObject();									// 0x4110e0 (called directly)
	// (then a call to the empty 0x463890)
	C3DObject::SetObjectID('3TSU');							// 0x49ab00
	f_9a8 = 500.0f;											// VisibleRange
	f_968 = 700.0f;											// speed
	f_96c = 3;												// patrol only
	f_c08 = 3;												// AIState
	f_978.x = 0;
	f_978.y = 0;
	f_978.z = 0;
	v94(2);
	v42(0);
	v40(0);
	v34(1);
	C3DObject::InitPhysics();								// 0x49acb0
	C3DAI::ev106(f_ac4, (char *)"none");					// 0x411210
	C3DAI::ev106(f_a9c, (char *)"none");
	C3DAI::ev106(f_b14, (char *)"none");
	strcpy(f_1b50, "tsunami.ase");
	strcpy(f_1bb4, "tsunami.png");
	f_bd4 = 1;												// hovering
	f_1c18 = -1;
	v52(1);
	f_1c1c = 0;
	// (then a call to the empty 0x463890)
}

C3DTsunami::~C3DTsunami() {}								// 0x479c50

namespace neg {
CGameObject *New_C3DTsunami(void) { return new C3DTsunami; }	// '3TSU' (0x1c28 bytes)
}

// --------------------------------------------------------------------------- slots

void C3DTsunami::ev69(void)									// 0x479ca0
{
	if (f_649) return;
	f_649 = 1;
	if (LevelID() == 'L5B1')
	{
		strcpy(f_1b50, "boulder.ase");
		strcpy(f_1bb4, "boulder.png");
		CreateAnim((char *)"TSUNAMI", f_1b50);				// +0xe0
		CreateAnim((char *)"STOP", (char *)"boulderStop.ase");
		CreateTexture(f_1bb4, 0);							// +0xfc
		ev58((char *)"TSUNAMI", 1);							// +0xe8
	}
	else
	{
		CreateAnim((char *)"TSUNAMI", f_1b50);
		CreateTexture(f_1bb4, 0);
		ev58((char *)"TSUNAMI", 1);
	}
	ApplyTexture(f_590, 0);									// +0x100
	v67(400.0f);											// CGameObject +0x10c: radius
	v64(0.1f);												// CGameObject +0x100
}

void C3DTsunami::v260(void)									// 0x479de0
{
	C3DAI::v260();											// 0x4143a0
	InitAnim();												// element +0x110
	if (LevelID() == 'LV3C')
	{
		ev71(0.85f);										// element +0x11c: alpha
		ev91(1);											// element +0x16c: second pass
	}
	f_658 = -1;												// HasCollision: leave as is
	f_1c1c = 3.0f;
}

void C3DTsunami::Process(float dt)							// 0x479e50
{
	if (g_gametype->IsPaused()) return;						// +0x114
	if (f_1c1c > 0)											// 0x4caa44
	{
		v176(0, 0, 0);										// +0x2c0: held in place
		f_1c1c = f_1c1c - dt;
		if (f_1c1c > 0) return;
		f_1c1c = 0;
	}
	C3DAI::Process(dt);										// 0x411240
	if (!g_camera) return;
	if (f_1c18 == -1)
	{
		if (f_96c == 3)
			f_1c18 = PlayGameSound3DNoDoppler(this, -1, std::string("waterspout"), true);	// 0x48f0f0
	}
	else if (f_96c == 1)
	{
		StopGameSound(f_1c18, false);						// 0x48f0d0
		f_1c18 = -1;
	}
	float a[4], b[4];
	v195(a);												// +0x30c
	g_camera->v195(b);
	const float d = sqrtf((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]));
	// (The original tests for L5B1 first; both branches do the same.)
	switch (g_difficulty)									// 0x53b5b0
	{
		case 0:
			if (d < 4000.0f) f_968 = (float)((double)f_1b3c * 0.15);	// 0x4d2ca8, 0x4d8b90 (a double)
			else f_968 = f_1b3c * 0.5f;						// 0x4caa5c
			break;
		case 1:
			if (d < 4000.0f) f_968 = f_1b3c * 0.3f;			// 0x4caa64
			else if (d <= 8000.0f) f_968 = f_1b3c;			// 0x50295c
			else f_968 = f_1b3c * 1.3f;						// 0x4ccef0
			break;
		case 2:
			if (d < 4000.0f) f_968 = f_1b3c * 0.45f;		// 0x502960
			else if (d > 8000.0f) f_968 = f_1b3c * 2.1f;	// 0x502958
			else f_968 = f_1b3c * 1.3f;
			break;
	}
}

void C3DTsunami::AddTouching(CGameObject *other)			// 0x47a190
{
	C3DAI::AddTouching(other);								// 0x415150
	if (!other->NameContains((char *)"C3DJIMMY") && !other->NameContains((char *)"C3DHOVERCYCLE")) return;
	if (!FindObjectByTag("JIM1")) return;
	C3DJimmy *j = g_jimmy ? dynamic_cast<C3DJimmy *>(g_jimmy) : NULL;	// (g_jimmy - 0xc0)
	if (!j) return;											// port: the original calls through a NULL g_jimmy
	j->ev134(-300.0f, -10, 5.0f, 1);						// element +0x218: hurt
}

void C3DTsunami::v277(void)									// 0x47a240
{
	C3DAnimated::v277();									// 0x41ce80
	if (f_1c18 != -1) StopGameSound(f_1c18, false);			// (keeps the channel: v278 restarts it)
}

void C3DTsunami::v278(void)									// 0x47a260
{
	C3DAnimated::v278();									// 0x41cea0
	if (f_1c18 != -1) f_1c18 = PlayGameSound3DNoDoppler(this, -1, std::string("waterspout"), true);
}

void C3DTsunami::WriteState(OMediaStreamOperators &s)		// 0x47a230
{
	C3DAI::WriteState(s);									// 0x415760
}

void C3DTsunami::ReadState(OMediaStreamOperators &s)		// 0x47a200
{
	C3DAI::ReadState(s);									// 0x4155e0
	f_1c1c = 3.0f;
	f_1c18 = -1;
}
