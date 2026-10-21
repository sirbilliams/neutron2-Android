/*  CViewPort and the per-frame object passes (0x4ae580..0x4ae5e0). See CViewPort.h.
    The original uses QueryPerformanceCounter/Frequency; SDL's performance counter replaces them. */

#include "CViewPort.h"
#include "port/Companion.h"
#include "CGameObject.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "Physics.h"
#include "Sound.h"
#include <SDL.h>
#include <string.h>

namespace neg {
unsigned long long	g_perf_counter = 0;
unsigned long long	g_perf_frequency = 0;
int					g_frames_rendered = 0;
float				g_frame_time = 0;
int					g_logic_updates = 0;
}
using namespace neg;

// --------------------------------------------------------------------------- not reconstructed yet

static void UpdateTextureAnims(float dt)	{ (void)dt; NEG_TODO(0x4a28c0, "texture animations (list 0x56ad7c)"); }
static void UpdateFrameRate(float dt)		{ (void)dt; NEG_TODO(0x4b2d90, "frame rate average / debug overlay"); }
static void SetClearLayerViewMatrix(void)
{
	// 0x4b974e: rotation-only view matrix from the viewport's angles (-ax, -ay, -az) copied to
	// g_layer_clear's custom view matrix (+0x7c, 17 dwords).
	NEG_TODO(0x4b974e, "clear layer view matrix");
}

// --------------------------------------------------------------------------- the per-frame passes over g_objects

static void PreUpdateObjects(void)											// 0x4ae5b0
{
	unsigned int bits;						// the slot takes the frame time; its generated signature
	memcpy(&bits, &g_frame_time, 4);		// is (unsigned int), so it gets the float's bits
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		(*i)->v241(bits);					// vtable +0x3c4
}

static void UpdateObjects(void)												// 0x4ae580
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		(*i)->v4(g_frame_time);				// vtable +0x10 -> Process(dt)
}

static void PostUpdateObjects(void)											// 0x4ae5e0
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
	{
		(*i)->v247();						// vtable +0x3dc
		(*i)->v248();						// vtable +0x3e0
	}
}

// --------------------------------------------------------------------------- CViewPort

CViewPort::CViewPort(OMediaSupervisor *super) : OMediaViewPort(super)		// 0x4b94a0
{
	g_perf_frequency = SDL_GetPerformanceFrequency();
	g_perf_counter = SDL_GetPerformanceCounter();
	g_frame_time = 0;
}

CViewPort::~CViewPort() {}													// 0x4b95a0

void CViewPort::update_logic(float millisecs_elapsed)						// 0x4b95e0
{
	OMediaViewPort::update_logic(millisecs_elapsed);
	g_logic_updates++;
}

float CViewPort::GetFrameTime(void)											// 0x4b9620
{
	unsigned long long now = SDL_GetPerformanceCounter();
	double dt = (double)(long long)(now - g_perf_counter) / (double)(long long)g_perf_frequency;
	if (dt != 0.0) g_perf_counter = now;
	if (dt > 0.2) return 0.2f;
	return (float)dt;
}

void CViewPort::render_viewport(OMediaRendererInterface *rdr_i)				// 0x4b96a0
{
	g_frame_time = GetFrameTime();			// vtable +0xd4
	if (g_frame_time > 0.0f && g_frame_time < 1.0f)
	{
		if (GameIsRunning() && g_editor && !EditorIsEditing())
		{
			PhysicsProcess(g_frame_time);	// 0x4b4c60
			UpdateTextureAnims(g_frame_time);	// 0x4a28c0
		}
		UpdateInput();						// 0x4b35d0 joystick poll (Input.cpp)
		UpdateSounds();						// 0x4b8a30
		PreUpdateObjects();					// 0x4ae5b0
		UpdateObjects();					// 0x4ae580
		PostUpdateObjects();				// 0x4ae5e0
		CompanionTick(g_frame_time);		// port: the dual-screen companion snapshot (port/Companion.h; read-only)
		if (g_layer_clear) SetClearLayerViewMatrix();
		UpdateFrameRate(g_frame_time);		// 0x4b2d90
	}
	// else: "LASTDELTA=%f" (debug log, empty in the shipped build)
	OMediaViewPort::render_viewport(rdr_i);
	ClearText();							// 0x4a0f00: the text sprites are redrawn every frame
	g_frames_rendered++;
}

void neg::ResetFrameTimer(void)												// 0x4b9600
{
	g_perf_counter = SDL_GetPerformanceCounter();
	g_frame_time = 0;
}
