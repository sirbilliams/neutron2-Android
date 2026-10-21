/*  C3DArrow - see C3DArrow.h. Reconstructed from Neutron2.exe 0x420350..0x4207e0 (and 0x4368e0). */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DArrow.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../SaveGame.h"
#pragma GCC diagnostic pop
#include <math.h>
#include <string.h>

using namespace neg;

C3DArrow::C3DArrow()										// 0x420350
{
	SetName((char *)"C3DARROW");
	Log("C3DArrow()");
	C3DArrow::InitObject();									// 0x4205d0 inlined: InitObject() called directly
	SetObjectID('3ARR');									// 0x49b550
	strcpy(m_sprite_db, "sprites.omt");
	m_sprite_index = 33;
	m_sprite_size = 200;
	v42(0);
	v40(0);
	v52(1);
	m_time = 0;
	v67(150.0f);											// radius
	v46(0);
	v34(1);
	strcpy(m_required_task, "none");
	m_required_level = 0;
	m_exact_level = -1;
	f_008 = 1;												// element +0xd0
	C3DSprite::InitPhysics();								// 0x49ca30
	// (then a call to the empty 0x463890)
}

C3DArrow::~C3DArrow()										// 0x420590
{
}

namespace neg {
CGameObject *New_C3DArrow(void) { return new C3DArrow; }				// '3ARR' (0x6cc bytes)
}

void C3DArrow::InitObject(void)								// 0x4205d0
{
	Log("InitObject()");
	C3DSprite::InitObject();								// 0x49c950
	RegisterProperty("RequiredTask", m_required_task, PROP_ALPHA, 0);
	RegisterProperty("RequiredLevel", &m_required_level, PROP_LONG, 0);
	RegisterProperty("ExactLevel", &m_exact_level, PROP_LONG, 0);
	// (then a call to the empty 0x463890)
}

void C3DArrow::Process(float dt)							// 0x420640 (C3DSpriteType's is not called)
{
	if (g_gametype->IsPaused()) return;						// +0x114
	m_time += dt;
	// x87: the sines are taken of the float time times a float constant, scaled by doubles
	const float h = (float)(sin((double)(m_time * 10.0f)) * 50.0 + 200.0);	// 0x4cb1cc, 0x4ce650, 0x4ce648
	set_size(200.0f, h);									// +0xac / +0xb0
	double g = sin((double)(m_time * 7.0f)) * (double)0.7f;	// 0x4cb1fc, 0x4ce640
	if (g > 1.0) g = 1.0;
	else if (g < 0.0) g = 0.0;
	set_diffuse(OMediaFARGBColor(0.5f, 1.0f, (float)g, 0.0f));	// +0x90.. (r, g, b, a) = (1, g, 0, 0.5)
}

void C3DArrow::v266(void)									// 0x4368e0 (shared code)
{
	// (debug log, empty)
	SetTaskState(m_task_name, GetTaskState(m_task_name));	// 0x497980 / 0x497a40 (writes back what it read)
}

void C3DArrow::v267(int level)								// 0x420710
{
	CLocalGameObject::v267(level);							// 0x48ef30
	const int state = GetTaskState(m_required_task);		// 0x497a40
	if (m_exact_level == -1)
	{
		if (m_required_level <= state) { ShowAgain(); return; }	// element +0xd4
	}
	else if (state == m_exact_level) { ShowAgain(); return; }
	HideAgain();											// element +0xd8
}
