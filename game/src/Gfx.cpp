/*  CGfx - display, world, layers, viewport.  Reconstructed from Neutron2.exe.
    0x4b0e80 CGfx::CGfx            0x4b1270 InitDisplay/InitWorld   0x4b1710 InitLayers
    0x4b1560 InitViewPort          0x4b18a0 SetDisplayMode                                   */

#include "Game.h"
#include "OMediaDataBase.h"
#include "OMedia3DShape.h"
#include "OMedia3DMaterial.h"
#include "OMediaCanvas.h"
#include "OMediaSound.h"
#include "OMediaFullscreenWindow.h"
#include "OMediaEngineFactory.h"
#include "OMediaVideoEngine.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include "OMediaViewPort.h"
#include "CViewPort.h"
#include "port/Widescreen.h"
#include "OMediaTimeCounter.h"
#include <SDL.h>
#include <strings.h>

using namespace neg;

// --------------------------------------------------------------------------- 0x4b0e80

CGfx::CGfx()
{
	OMediaDataBase::set_cache_size(32000000);
	omd_REGISTERCLASS(OMedia3DShape);			// '3DSh'
	omd_REGISTERCLASS(OMedia3DMaterial);		// '3DMa'
	omd_REGISTERCLASS(OMediaCanvas);			// 'Canv'
	omd_REGISTERCLASS(OMediaSound);				// 'Wave'

	// 0x56afec / 0x56aff0 cleared; 0x56afd8 = new uint seeded from the clock
	static unsigned int seed;
	g_random_seed = &seed;
	seed = OMediaTimeCounter::lowlevel_get_millisecs();

	InitDisplay();								// 0x4b1270
}

CGfx::~CGfx() {}

// --------------------------------------------------------------------------- 0x4b1270

void CGfx::InitDisplay(void)
{
	SDL_Log("InitDisplay()");
	WidescreenSetup();							// port: the logical window's width (port/Widescreen.h)
	g_window = new OMediaFullscreenWindow((OMediaSupervisor*)g_app, false);

	// The original logs every engine the factory offers, then asks for ommeic_Best (6).
	OMediaEngineFactory *factory = OMediaEngineFactory::get_factory();
	g_video = factory->create_video_engine(ommeic_Best, g_window);
	if (g_video) g_window->link_video_engine(g_video);

	InitWorld();
}

void CGfx::InitWorld(void)
{
	SDL_Log("InitWorld()");
	g_world = new OMediaWorld;

	// Five layers, linked in draw order (0x4b1270 creates them, 0x4b1710 configures them).
	OMediaLayer **order[] = { &g_layer_clear, &g_layer_back2d, &g_layer_world, &g_layer_hud2d, &g_layer_top2d };
	for (int i = 0; i < 5; i++)
	{
		*order[i] = new OMediaLayer;
		(*order[i])->link(g_world);
	}
	InitLayers();
}

// --------------------------------------------------------------------------- 0x4b1710

void CGfx::InitLayers(void)
{
	SDL_Log("InitLayers()");
	OMediaFARGBColor black(1.0f, 0.0f, 0.0f, 0.0f);

	// L0: clears colour + z-buffer (0x287 = ClearZBuffer|ClearColor|Visible|DisablePicking|CustomViewMatrix)
	if (g_layer_clear)
	{
		g_layer_clear->set_flags(omlayerf_ClearZBuffer | omlayerf_ClearColor | omlayerf_Visible |
								 omlayerf_DisablePicking | omlayerf_CustomViewMatrix);
		g_layer_clear->set_clear_color(black);					// +0x10c..0x118 = (0,0,0,1)
		g_layer_clear->set_far_clip(1000000.0f);				// +0xc0 = 1e6
	}

	// L1, L3, L4: 2D overlays (0x184 = Visible|DisablePicking|DisableViewportTransform), ortho, near 0
	OMediaLayer *flat[] = { g_layer_back2d, g_layer_hud2d, g_layer_top2d };
	for (int i = 0; i < 3; i++)
	{
		if (!flat[i]) continue;
		flat[i]->set_flags(omlayerf_Visible | omlayerf_DisablePicking | omlayerf_DisableViewportTransform);
		flat[i]->set_projection_type(omlptc_Ortho);
		flat[i]->set_near_clip(0.0f);
	}

	// L2: the 3D world (0x9c = Visible|ZWrite|ZTest|DisablePicking)
	if (g_layer_world)
	{
		g_layer_world->set_flags(omlayerf_Visible | omlayerf_EnableZBufferWrite | omlayerf_EnableZBufferTest |
								 omlayerf_DisablePicking);
		if (!g_windowed)		// second (transparency) pass sorted back to front when fullscreen
			g_layer_world->set_flags_pass2(g_layer_world->get_flags_pass2() | omlayerf_SortElementBack2Front);
		g_layer_world->set_clear_color(black);
		g_layer_world->set_far_clip(g_world_far_clip);
	}
}

// --------------------------------------------------------------------------- 0x4b1560

void CGfx::InitViewPort(void)
{
	SDL_Log("InitViewPort()");
	if (g_viewport) return;
	g_viewport = new CViewPort(g_window);					// 0x4b94a0 (0x120 bytes)
	g_viewport->link(g_world);
	g_viewport->link_window(g_window);
	OMediaRect r; r.set(0, 0, 0, 0);
	g_viewport->setbounds(&r);
	g_viewport->set_bounds_mode(omcpbc_Right, omvpbmc_RightRelative);
	g_viewport->set_bounds_mode(omcpbc_Bottom, omvpbmc_BottomRelative);
}

// --------------------------------------------------------------------------- 0x4b18a0

void CGfx::SetDisplayMode(int w, int h, int depth, bool windowed)
{
	SDL_Log("SetDisplayMode(%d,%d,%d,%d)", w, h, depth, (int)windowed);
	UnInitAlpha();				// 0x4a0e10
	UnInitCaptions();			// 0x4b16a0 (inline in the original)
	UnInitFont();
	// UnInitViewPort() is a no-op in the shipped build.

	if (!g_video) return;
	g_video->unlink();
	OMediaVideoCard *card = &g_video->get_video_cards()->front();		// FindBestVideoCard(): first card
	g_video->link(card);

	// Find the mode. The original requires w x h x depth; on SDL/GLES there is one mode per
	// logical size and colour depth is meaningless, so match on size only.
	OMediaVideoMode *mode = NULL;
	for (omt_VideoModeList::iterator m = card->modes.begin(); m != card->modes.end(); m++)
		if ((*m).width == w && (*m).height == h) mode = &(*m);
	if (!mode && !card->modes.empty()) mode = &card->modes.front();
	if (mode)
	{
		g_screen_depth = depth; g_screen_h = h; g_screen_w = w;
		g_video->set_mode(mode);

		// FindRenderer(): renderer names in order of preference (0x5406e0); windowed skips the HAL ones.
		static const char *prefs[] = { "DIRECT3D T&L HAL", "DIRECT3D HAL", "OPENGL", "RGB EMULATION", "OPEN MEDIA TOOLKIT" };
		OMediaRendererDef *chosen = NULL;
		for (int p = windowed ? 4 : 0; p < 5 && !chosen; p++)
			for (omt_RendererDefList::iterator r = g_video->get_renderer_list()->begin(); r != g_video->get_renderer_list()->end(); r++)
				if (strcasecmp((*r).name.c_str(), prefs[p]) == 0) { chosen = &(*r); break; }
		// SDL build: also accept our GL renderer for the windowed path, and fall back to the first one.
		if (!chosen)
			for (omt_RendererDefList::iterator r = g_video->get_renderer_list()->begin(); r != g_video->get_renderer_list()->end(); r++)
				if (strcasecmp((*r).name.c_str(), "OPENGL") == 0) { chosen = &(*r); break; }
		if (!chosen && !g_video->get_renderer_list()->empty()) chosen = &g_video->get_renderer_list()->front();
		if (chosen)
		{
			SDL_Log("SELECTED=%s", chosen->name.c_str());
			g_video->select_renderer(chosen, chosen->zbuffer_depth);
			g_display_ok = true;
			g_windowed = windowed;
		}
	}

	InitLayers();				// 0x4b1710
	InitViewPort();				// 0x4b1560
	InitFont();
	InitCaptions();
	InitAlpha();				// 0x4a0ce0
	// 0x56aff4 = 0
}

// --------------------------------------------------------------------------- not yet decompiled

void CGfx::InitFont(void)		{ /* TODO(0x4b18a0 InitFont): 0x56afb8 / 0x56afbc font objects */ }
void CGfx::UnInitFont(void)		{ }
void CGfx::InitCaptions(void)	{ /* TODO: caption list 0x56af94 */ }
void CGfx::UnInitCaptions(void)	{ }
