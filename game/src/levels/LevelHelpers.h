/*  Small pieces the level game types (CLevelXXGame, game/src/levels) share: the original
    repeats them inline in every InitGame (0x486730..0x48e150).                              */
#pragma once
#include "../CJimmyGame.h"
#include "../C3DSky.h"
#include "../CGameObject.h"
#include "../Globals.h"
#include "../Todo.h"
#include "../gen/globals.h"
#include "OMediaLayer.h"
#include "OMediaViewPort.h"
#include <string>

namespace levels {

// CreateObject('3SKY') and its element (the original subtracts 0xc0; NULL stays NULL).
static inline C3DSky *CreateSky(void)
{
	CGameObject *o = neg::CreateObject('3SKY');
	return o ? dynamic_cast<C3DSky *>(o) : NULL;
}

// C3DSky element slot 58 (+0xe8, 0x49bcc0): open the sky database `file` and put its shape on
// the sky; `alt` picks the "sky1alt".. textures and the "fogcoloralt" fog colour (0x54e860).
// Every level passes false. (m_sky is kept as a C3DObject; it is always a '3SKY'.)
static inline void SkyLoad(C3DObject *sky, std::string file, bool alt)
{
	static_cast<C3DSky *>(sky)->LoadSky(file, alt);
}

// 0x4a3930 (as in CEditor.cpp): every InitGame ends with CaptureMouse(true).
static inline void CaptureMouse(bool capture)
{
	if (capture && !g_56aed0)
	{
		NEG_TODO(0x4a3930, "hide the cursor, remember and centre it");
		g_mouse_captured = 1;
		return;
	}
	// (restore the cursor position, OMediaMouseCursor::force_show)
	g_mouse_captured = 0;
}

// The viewport back at the origin, unrotated (2002 +0x44..+0x4c, +0x50..+0x54; inline).
static inline void ViewportToOrigin(void)
{
	neg::g_viewport->place(0.0f, 0.0f, 0.0f);
	neg::g_viewport->set_angle(0, 0, 0);
}

// 2002 layer flag 0x800 (fog on) is OMT 2.5's fog_enabled. Set inline (flags |= 0x800) or
// cleared by 0x4b1860(layer, 0x800). (2.5's enable_fog() always sets it.)
namespace detail {
struct LayerFog : OMediaLayer { static bool OMediaLayer::*Enabled(void) { return &LayerFog::fog_enabled; } };
}
static inline void SetLayerFog(OMediaLayer *l, bool on)	{ l->*detail::LayerFog::Enabled() = on; }

// A 2002 OMediaFARGBColor stored as r, g, b, a (layer +0xc8.. fog, +0x10c.. clear colour).
static inline OMediaFARGBColor RGBA(float r, float g, float b, float a)	{ return OMediaFARGBColor(a, r, g, b); }

// Inline in LEV6, LV6A, LEV7 and VR02, once through each world-layer global (0x56afc8, then
// 0x56afd4): the far plane, the fog range and colour, and the fog on (flags |= 0x800).
static inline void WorldFog(OMediaLayer *world, const OMediaFARGBColor &c, float far_clip, float start, float end)
{
	world->set_far_clip(far_clip);							// 2002 +0xc0
	world->set_fog_range(start, end);						// 2002 +0xd8 / +0xdc
	world->set_fog_color(c);								// 2002 +0xc8
	SetLayerFog(world, true);
}

}	// namespace levels
