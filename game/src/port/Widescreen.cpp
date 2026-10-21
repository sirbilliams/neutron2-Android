/*  port: widescreen - see Widescreen.h. */
#include "Widescreen.h"
#include "../Globals.h"
#include "OMediaSDLRetarget.h"
#include <SDL.h>
#include <stdlib.h>
#include <map>

namespace neg {

bool WidescreenOn(void)
{
	static int on = -1;
	if (on < 0)
	{
		const char *e = getenv("NEG_WIDESCREEN");
		if (e && *e) on = atoi(e) != 0;
		else on = 0;			// off by default until the HUD / letterbox / mouse parts are done
	}
	return on != 0;
}

void WidescreenSetup(void)
{
	if (WidescreenOn()) OMediaSDLVideoEngine::set_widescreen(480);
	SDL_Log("Widescreen %s: logical window %dx%d", WidescreenOn() ? "on" : "off",
			OMediaSDLVideoEngine::logical_width, OMediaSDLVideoEngine::logical_height);
}

int WindowWidth(void)	{ return OMediaSDLVideoEngine::logical_width; }
int WindowHeight(void)	{ return OMediaSDLVideoEngine::logical_height; }

float WideMargin(void)
{
	// (g_screen_w stays the game's 640; the window is wider by twice the margin)
	const int extra = OMediaSDLVideoEngine::logical_width - (int)(short)g_screen_w;
	return extra > 0 ? extra * 0.5f : 0.0f;
}

static std::map<const void *, int> s_anchors;

void WideAnchor(const void *piece, int side)	{ if (piece) s_anchors[piece] = side; }
void WideAnchorClear(void)						{ s_anchors.clear(); }

float WideShift(const void *piece)
{
	if (s_anchors.empty()) return 0.0f;
	std::map<const void *, int>::const_iterator it = s_anchors.find(piece);
	return it == s_anchors.end() ? 0.0f : it->second * WideMargin();
}

}
