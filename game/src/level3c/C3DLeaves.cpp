/*  C3DLeaves - see C3DLeaves.h. Reconstructed from Neutron2.exe 0x44f0b0..0x44f720. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in OMedia3DShape.h)
#include "C3DLeaves.h"
#include "../CGameType.h"
#include "../DatabaseCache.h"
#include "../Sound.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMediaCanvas.h"
#pragma GCC diagnostic pop
#include <string.h>

using namespace neg;

// The original's random number (0x19660d LCG, 0..1000) * 0.001 * 30: 0..30 s.
static float RandomWait(void)
{
	*g_random_seed = *g_random_seed * 0x19660d + 0x7fff;
	const int r = (int)(((*g_random_seed >> 16) * 1001) >> 16) & 0xffff;
	return (float)r * 0.001f * 30.0f;						// 0x4cb218, 0x4ca4b4
}

// --------------------------------------------------------------------------- construction

C3DLeaves::C3DLeaves()										// 0x44f0b0
{
	SetName((char *)"C3DLEAVES");
	Log("C3DLeaves()");
	C3DSprite::InitObject();								// 0x49c950 (called directly)
	SetObjectID('3LEA');
	OMediaDataBase *db = GetDatabase("sprites.omt");		// 0x4a4180
	static const int ids[5] = { 0x31, 0x2e, 0x2f, 0x30, 0x2d };
	for (int i = 0; i < 5; i++)
	{
		m_frames[i] = LoadCanvasID(db, ids[i]);				// 0x4b2860
		m_frames[i]->set_filtering(omtfc_Linear, omtfc_Linear);	// +0x60, +0x64 = 1, +0x24 vtable +0x10 (no NULL check)
	}
	m_frame = 0;
	set_canvas(m_frames[0]);
	m_time = 0;
	C3DSprite::InitSpriteProperties(200.0f);				// 0x49ce10 (called directly)
	m_landed = 0;
	m_falling = 0;
	v67(250.0f);
	v42(1);
	v40(1);
	v46(0);
	v52(1);
	v34(1);
	m_fall_time = 0;
	m_wait = RandomWait();
	C3DSprite::InitPhysics();								// 0x49ca30
	// (then a call to the empty 0x463890)
}

C3DLeaves::~C3DLeaves() {}									// 0x44f330

namespace neg {
CGameObject *New_C3DLeaves(void) { return new C3DLeaves; }	// '3LEA' (0x6c8 bytes)
}

// --------------------------------------------------------------------------- slots

// Start falling: 7 s, physics on, collisions on, not hidden, no velocity / acceleration.
// (Inline twice in the original: AddTouching and Process.)
void C3DLeaves::AddTouching(CGameObject *other)				// 0x44f370
{
	CGameObject::AddTouching(other);						// 0x4ab970
	if (m_falling) return;
	if (!other->NameContains((char *)"C3DJIMMY") && !other->NameContains((char *)"C3DBASEBALL")) return;
	m_fall_time = 7.0f;
	m_channel = PlayGameSound(-1, std::string("leaves"), false);	// 0x48f420
	m_falling = 1;
	v132(1);
	v120(1);
	v136(0);
	v157(0, 0, 0);
	v176(0, 0, 0);
}

void C3DLeaves::Process(float dt)							// 0x44f450
{
	if (g_gametype->IsPaused()) return;						// +0x114
	C3DSprite::Process(dt);									// 0x49cfa0
	m_time = dt + m_time;
	m_wait = m_wait - dt;
	if (m_fall_time > 0)									// 0x4caa44
	{
		m_fall_time = m_fall_time - dt;
		float v[4];
		v155(v);
		const float vz = v[2];
		v155(v);
		v157(v[0], -g_gravity.y, vz);						// gravity cancelled (0x63119c)
		v176(0, -150.0f, 0);
		if (!(m_fall_time > 0))
		{
			// landed for good: back to the start, wait again
			m_wait = RandomWait();
			float p[4];
			v88(p);											// the initial position
			v196(p[0], p[1], p[2], p[3]);
			m_falling = 0;
			m_landed = 0;
			m_fall_time = 0;
			v136(1);
			v132(0);
			m_frame = 0;
			m_time = 0;
		}
	}
	else if (m_wait < 0)
	{
		m_fall_time = 7.0f;									// (no sound this way)
		m_falling = 1;
		v132(1);
		v120(1);
		v136(0);
		v157(0, 0, 0);
		v176(0, 0, 0);
	}
	if (m_landed) return;
	if (v133()) m_landed = 1;
	if (m_time > 0.07f && m_falling)						// 0x4e6950
	{
		m_time = 0;
		m_frame++;
		if (m_frame > 4) m_frame = 0;
		set_canvas(m_frames[m_frame]);						// element +0xac
	}
}

void C3DLeaves::v260(void)									// 0x44f430
{
	set_canvas(m_frames[0]);
}
