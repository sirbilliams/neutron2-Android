/*  CViewPort - the game's viewport (0x120 bytes in the original; OMediaViewPort + game code).
    Reconstructed from Neutron2.exe: constructor 0x4b94a0, destructor 0x4b95a0.

    It drives the game: each rendered frame, render_viewport (0x4b96a0) measures the frame time and,
    before drawing, runs the per-frame game passes over g_objects - v241(dt), v4(dt) (which calls
    Process(dt), or v242(dt) while editing) and v247/v248 - plus physics, texture animations, input
    and sounds. That is how every game object, the running game type included, is updated.       */
#pragma once
#include "OMediaViewPort.h"

class CViewPort : public OMediaViewPort
{
	public:
	CViewPort(OMediaSupervisor *super);									// 0x4b94a0
	virtual ~CViewPort();													// 0x4b95a0

	virtual void update_logic(float millisecs_elapsed);						// 0x4b95e0
	virtual void render_viewport(OMediaRendererInterface *rdr_i);			// 0x4b96a0
	virtual float GetFrameTime(void);										// 0x4b9620 (element vtable +0xd4)
};

namespace neg {
extern unsigned long long	g_perf_counter;		// 0x6f75b0  QueryPerformanceCounter at the last frame
extern unsigned long long	g_perf_frequency;	// 0x6f75b8
extern int					g_frames_rendered;	// 0x6f75c0
extern float				g_frame_time;		// 0x6f75c8  seconds, at most 0.2
extern int					g_logic_updates;	// 0x6f75cc

void ResetFrameTimer(void);						// 0x4b9600
}
