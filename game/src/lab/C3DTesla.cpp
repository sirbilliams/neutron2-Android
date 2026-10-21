/*  C3DTesla - see C3DTesla.h. Reconstructed from Neutron2.exe 0x475ff0..0x4767d0. */
#include "C3DTesla.h"
#include "../C3DPlayer.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include "OMedia3DVector.h"
#include "OMediaStreamOperators.h"
#include <string>

using namespace neg;

C3DTesla::C3DTesla()										// 0x475ff0
{
	SetName((char *)"C3DTESLA");
	f_924 = 0;
	C3DTesla::InitObject();									// 0x4761e0 (called directly)
	C3DObject::SetObjectID('3TES');							// 0x49ab00
	v52(1);
	v46(0);
	v34(1);
	f_928 = 0;
	f_92c = 1;												// ItemActive
	f_930 = 1;
	f_938 = 0;												// SoundActive
	f_934 = -1;
	C3DObject::ShowAgain();									// 0x49b100
	C3DObject::InitPhysics();								// 0x49acb0
}

C3DTesla::~C3DTesla()										// 0x476140
{
	if (f_934 != -1) StopChannel(f_934, false);				// 0x4b88e0
}

namespace neg {
CGameObject *New_C3DTesla(void) { return new C3DTesla; }				// '3TES' (0x944 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DTesla::InitObject(void)								// 0x4761e0
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	RegisterProperty("ItemActive", &f_92c, PROP_LONG, 0);
	RegisterProperty("SoundActive", &f_938, PROP_LONG, 0);
	InitAnim();												// element +0x110
	CreateAnim((char *)"DEFAULT", (char *)"tesla.ase");		// +0xe0
	CreateTexture((char *)"tesla0000.png", 0);				// +0xfc
	ev65(0, omblendfc_Src_Alpha, omblendfc_One);			// +0x104: added onto the scene
	CreateTexture((char *)"tesla0001.png", 1);
	ev65(1, omblendfc_Src_Alpha, omblendfc_One);
	CreateTexture((char *)"tesla0002.png", 2);
	ev65(2, omblendfc_Src_Alpha, omblendfc_One);
	CreateTexture((char *)"tesla0003.png", 3);
	ev65(3, omblendfc_Src_Alpha, omblendfc_One);
	ApplyTexture(f_590, f_924);								// +0x100
	v67(80.0f);												// CGameObject +0x10c: radius
	ev58((char *)"DEFAULT", 1);								// +0xe8
	// (then a call to the empty 0x463890)
}

void C3DTesla::ResetObject(void)							// 0x482a70 (jmp 0x48ee80)
{
	CLocalGameObject::ResetObject();
}

void C3DTesla::AddTouching(CGameObject *other)				// 0x476320
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (!f_930) return;
	if (!other->NameContains((char *)"C3DJIMMY")) return;	// +0x18
	float a[4], b[4];
	other->v195(a);											// +0x30c
	v195(b);
	OMedia3DVector d(a[0] - b[0], a[1] - b[1], a[2] - b[2], 0);
	if (d.quick_magnitude() > 1.0f)
	{
		const float k = 1.0f / d.quick_magnitude();
		C3DPlayer *p = dynamic_cast<C3DPlayer *>(other);	// port: the original calls element slot 119 of whatever is named C3DJIMMY
		if (p) p->ev119(OMedia3DVector(k * d.x, d.y * k, d.z * k, 0));	// element +0x1dc: pushed away
	}
}

void C3DTesla::RemoveTouching(CGameObject *other)			// 0x47fca0
{
	CGameObject::RemoveTouching(other);						// 0x4aba00
}

void C3DTesla::Process(float dt)							// 0x476420
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAnimated::Process(dt);								// 0x41c120
	if (!f_930) return;
	f_928 = dt + f_928;
	if (!(f_928 > 0.1)) return;								// 0x4e9be8 (double)
	f_928 = 0;
	f_924++;
	if (f_924 > 3) f_924 = 0;
	ApplyTexture(f_590, f_924);								// element +0x100
	ev72(g_windowed ? 0.4f : 0.7f);							// +0x120: additive (0x56afab)
}

void C3DTesla::v260(void)									// 0x476640
{
	C3DAnimated::v260();									// 0x41cec0
	f_930 = f_92c;											// ItemActive
	ev72(0.8f);												// +0x120
}

void C3DTesla::v266(void)									// 0x476610
{
	// (debug log, empty)
	if (f_930) ev94();										// element +0x178
	else ev95();											// +0x17c
}

void C3DTesla::v268(int state)								// 0x4764f0
{
	if (state == 0) f_930 = 0;
	else if (state == 1)
	{
		f_930 = 1;
		ShowAgain();										// element +0xd8 (C3DObject's ShowAgain)
		ev94();
		return;
	}
	else
	{
		f_930 = f_930 == 0;
		if (f_930)
		{
			ShowAgain();
			ev94();
			return;
		}
	}
	HideAgain();											// +0xdc
	ev95();
}

void C3DTesla::v277(void)									// 0x476700
{
	C3DAnimated::v277();									// 0x41ce80
	if (f_934 != -1) StopChannel(f_934, false);				// 0x4b88e0
}

void C3DTesla::v278(void)									// 0x476720
{
	C3DAnimated::v278();									// 0x41cea0
	if (f_934 != -1 && f_938 != 0)							// (sic: only while the sound is still set)
		f_934 = PlayGameSound3DNoDoppler(this, -1, std::string("electric spark"), true);	// 0x48f0f0
}

void C3DTesla::WriteState(OMediaStreamOperators &s)			// 0x476590
{
	CLocalGameObject::WriteState(s);						// 0x48fbb0
	s << f_930;
}

void C3DTesla::ReadState(OMediaStreamOperators &s)			// 0x4765c0
{
	CLocalGameObject::ReadState(s);							// 0x48fbf0
	s >> f_930;
	if (f_930) ShowAgain();									// element +0xd8
	else HideAgain();
}

// --------------------------------------------------------------------------- element family

void C3DTesla::ev94(void)									// 0x476670
{
	if (f_934 == -1 && f_938 != 0)
		f_934 = PlayGameSound3DNoDoppler(this, -1, std::string("electric spark"), true);	// 0x48f0f0
}

void C3DTesla::ev95(void)									// 0x4766d0
{
	if (f_934 == -1) return;
	StopGameSound(f_934, false);							// 0x48f0d0
	f_934 = -1;
}
