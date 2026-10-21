/*  C3DAIGranny - see C3DAIGranny.h. Reconstructed from Neutron2.exe 0x415e00..0x416af0, 0x417f40. */
#include "C3DAIGranny.h"
#include "Game.h"
#include "CGameType.h"
#include "Sound.h"
#include "gen/globals.h"
#include <string.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- construction

C3DAIGranny::C3DAIGranny()									// 0x415e00
{
	SetName((char *)"C3DAIGRANNY");
	Log("C3DAIGranny()");
	f_6d0 = 1;												// Granny
	f_649 = 0;												// ev69 loads the animations later
	C3DAIGranny::InitObject();								// called directly (as shipped: the
															// loader's InitObject runs it again)
	C3DObject::SetObjectID('3GRN');
	f_9a8 = 1000.0f;										// VisibleRange
	f_968 = 160.0f;											// speed
	f_96c = 1;
	f_c08 = 1;												// AIState
	C3DObject::InitPhysics();
	C3DAI::ev106(f_ac4, (char *)"WALK");					// running is walking
	C3DAI::ev106(f_a9c, (char *)"STOP");					// so are waiting ...
	C3DAI::ev106(f_b14, (char *)"STOP");					// ... and sitting
	v50(0);
	v40(1);
	v42(1);
	v67(60.0f);												// radius
	strcpy(f_1b50, "nummeybase.grn");
	strcpy(f_1bb4, "nummeywalk.grn");
	strcpy(f_1c18, "nummeytalk.grn");
	strcpy(f_1c7c, "nummeystop.grn");
	strcpy(f_1ce0, "none");
	strcpy(f_1d44, "none");
	strcpy(f_1da8, "none");
	strcpy(f_1e0c, "none");
	strcpy(f_1e70, "none");
	strcpy(f_1ed4, "none");
	strcpy(f_1f38, "none");
	strcpy(f_1f9c, "none");
	strcpy(f_2000, "none");
	strcpy(f_2064, "none");
	strcpy(f_20c8, "none");
	f_212c = 0;
	f_2134 = 0;
	f_2130 = -1.0f;											// ResetToStop
	f_2138 = -1;											// PassThru
}

C3DAIGranny::~C3DAIGranny()									// 0x416220
{
	// (debug log "TEST")
}

namespace neg {
CGameObject *New_C3DAIGranny(void) { return new C3DAIGranny; }			// '3GRN' (0x2144 bytes)
}

// --------------------------------------------------------------------------- CGameObject family

void C3DAIGranny::InitObject(void)							// 0x4162c0
{
	Log("InitObject()");
	C3DAI::InitObject();
	RegisterProperty("BASEAnimation", f_1b50, PROP_ALPHA, 0);
	RegisterProperty("WALKAnimation", f_1bb4, PROP_ALPHA, 0);
	RegisterProperty("TALKAnimation", f_1c18, PROP_ALPHA, 0);
	RegisterProperty("STOPAnimation", f_1c7c, PROP_ALPHA, 0);
	RegisterProperty("RUNAnimation", f_1ce0, PROP_ALPHA, 0);
	RegisterProperty("FLYAnimation", f_1d44, PROP_ALPHA, 0);
	RegisterProperty("ANIM1Animation", f_1da8, PROP_ALPHA, 0);
	RegisterProperty("ANIM2Animation", f_1e0c, PROP_ALPHA, 0);
	RegisterProperty("ANIM3Animation", f_1e70, PROP_ALPHA, 0);
	RegisterProperty("ANIM4Animation", f_1ed4, PROP_ALPHA, 0);
	RegisterProperty("TouchAnimA", f_1f9c, PROP_ALPHA, 0);
	RegisterProperty("TouchAnimB", f_2000, PROP_ALPHA, 0);
	RegisterProperty("TouchSound", f_1f38, PROP_ALPHA, 0);
	RegisterProperty("Radius", &f_034, PROP_FLOAT, 0);
	RegisterProperty("ResetToStop", &f_2130, PROP_FLOAT, 0);
	RegisterProperty("PassThru", &f_2138, PROP_LONG, 0);
}

void C3DAIGranny::UnInitObject(void)						// 0x4162b0 (a jump to C3DAnimated's)
{
	C3DAnimated::UnInitObject();
}

void C3DAIGranny::AddTouching(CGameObject *other)			// 0x416590
{
	C3DAI::AddTouching(other);
	if (other != g_camera) return;
	if (strcasecmp(f_1f38, "none") != 0)
		PlayGameSound(-1, std::string(f_1f38), false);		// 0x48f420
	if (strcasecmp(f_1f9c, "none") == 0) return;
	strcpy(f_2064, f_aec);									// keep our FLY / RUN animation names
	strcpy(f_20c8, f_ac4);
	strcpy(f_aec, "TOUCHANIMA");							// and play TouchAnimA as both meanwhile
	strcpy(f_ac4, "TOUCHANIMA");
	ev58((char *)"TOUCHANIMA", 0);
	if (f_2130 > 0) f_2134 = f_2130;						// ResetToStop
}

unsigned char C3DAIGranny::v231(CGameObject *other)			// 0x417f40
{
	if (g_camera == other) return 1;
	const int id = other->m_object_id;
	return id == '3GOD' || id == '3JIM' || id == '3WBL';	// 0x33474f44, 0x334a494d, 0x3357424c
}

void C3DAIGranny::Process(float dt)							// 0x416510
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DAI::Process(dt);
	if (f_2134 > 0)
	{
		f_2134 -= dt;
		if (f_2134 <= 0)
		{
			f_2134 = 0;
			ev58((char *)"STOP", 1);
		}
	}
}

void C3DAIGranny::v260(void)								// 0x416480
{
	C3DAI::v260();
	InitAnim();												// element +0x110 (logs an error for granny objects)
	ev70(1);												// element +0x118
	if (f_65c == 0) HideAgain();							// InitiallyVisible
	else ShowAgain();
	if (f_2138 == 1) v34(1);								// PassThru
	if (f_658 == 0)											// HasCollision
	{
		v232();
		v136(0);
	}
}

// --------------------------------------------------------------------------- element family

void C3DAIGranny::ev67(void)								// 0x4169d0
{
	// (debug log, empty)
	if (strcasecmp(ev66(), "TOUCHANIMA") != 0) return;
	strcpy(f_aec, f_2064);									// our own animation names back
	strcpy(f_ac4, f_20c8);
	if (strcasecmp(f_2000, "none") != 0) ev58((char *)"TOUCHANIMB", 1);
}

void C3DAIGranny::ev69(void)								// 0x4166e0
{
	if (f_649) return;
	f_649 = 1;
	if (strcasecmp(f_1b50, "none") != 0) InitModel(std::string(f_1b50));	// element +0x144
	if (strcasecmp(f_1c18, "none") != 0) CreateAnim((char *)"TALK", f_1c18);	// +0xe0
	if (strcasecmp(f_1ce0, "none") != 0) CreateAnim((char *)"RUN", f_1ce0);
	if (strcasecmp(f_1bb4, "none") != 0) CreateAnim((char *)"WALK", f_1bb4);
	if (strcasecmp(f_1d44, "none") != 0) CreateAnim((char *)"FLY", f_1d44);
	if (strcasecmp(f_1da8, "none") != 0) CreateAnim((char *)"ANIM1", f_1da8);
	if (strcasecmp(f_1e0c, "none") != 0) CreateAnim((char *)"ANIM2", f_1e0c);
	if (strcasecmp(f_1e70, "none") != 0) CreateAnim((char *)"ANIM3", f_1e70);
	if (strcasecmp(f_1ed4, "none") != 0) CreateAnim((char *)"ANIM4", f_1ed4);
	if (strcasecmp(f_1f9c, "none") != 0) CreateAnim((char *)"TOUCHANIMA", f_1f9c);
	if (strcasecmp(f_2000, "none") != 0) CreateAnim((char *)"TOUCHANIMB", f_2000);
	if (strcasecmp(f_1c7c, "none") != 0) CreateAnim((char *)"STOP", f_1c7c);
	ev58((char *)"STOP", 1);
	v64(1.265f);
}
