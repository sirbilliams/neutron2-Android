/*  port: the Asset Viewer's level explorer and save\levels.txt - see AssetView.h. Not in the
    original.                                                                                  */
#include "AssetView.h"
#include "Companion.h"
#include "../C3DJimmy.h"
#include "../CJimmyGame.h"
#include "../CGameType.h"
#include "../objects/C3DStartPoint.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../TextureAnims.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaTrigo.h"
#include <SDL.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <algorithm>
#include <string>
#include <vector>

std::string omf_SDLResolvePath(const std::string &in);		// engine/sdl: DOS path -> real file
using namespace neg;

namespace {
bool		s_on = false;				// the level is loaded and frozen
char		s_level[64];
char		s_name[96];					// the panel's name of the level (or its file name)
float		s_home[3];					// where the explorer started
short		s_home_ax, s_home_ay;
int			s_speed = 1;				// 0 slow, 1 normal, 2 fast
bool		s_fog = true, s_fog_was = true, s_jimmy = true;
unsigned	s_prev = 0;					// the buttons held last frame (edges)
float		s_hint = 6.0f;				// seconds the controls help stays on screen

const float		kSpeed[3] = { 150.0f, 600.0f, 2400.0f };	// world units per second
const char *const kSpeedName[3] = { "SLOW", "NORMAL", "FAST" };
const float		kTurn = 7000.0f;		// OMT angle units (16384 = a full turn) per second
const short		kPitchMax = 3800;		// about 83 degrees

// 2002 layer flag 0x800 (fog on) is OMT 2.5's fog_enabled (as CJimmyGame.cpp).
struct LayerFogAccess : OMediaLayer { static bool OMediaLayer::*Enabled(void) { return &LayerFogAccess::fog_enabled; } };

// The game controllers, opened here as well (SDL counts the opens).
std::vector<SDL_GameController *> s_pads;
int s_pad_count = -1;

void Pads(void)
{
	int n = SDL_NumJoysticks();
	if (n == s_pad_count) return;
	for (size_t i = 0; i < s_pads.size(); i++) SDL_GameControllerClose(s_pads[i]);
	s_pads.clear();
	for (int i = 0; i < n; i++)
		if (SDL_IsGameController(i))
			if (SDL_GameController *gc = SDL_GameControllerOpen(i)) s_pads.push_back(gc);
	s_pad_count = n;
}

float Axis(SDL_GameControllerAxis a)
{
	float best = 0;
	for (size_t i = 0; i < s_pads.size(); i++)
	{
		float v = SDL_GameControllerGetAxis(s_pads[i], a) / 32767.0f;
		if (fabsf(v) > fabsf(best)) best = v;
	}
	if (fabsf(best) < 0.18f) return 0;					// dead zone
	return best > 0 ? (best - 0.18f) / 0.82f : (best + 0.18f) / 0.82f;
}

bool Button(SDL_GameControllerButton b)
{
	for (size_t i = 0; i < s_pads.size(); i++)
		if (SDL_GameControllerGetButton(s_pads[i], b)) return true;
	return false;
}

enum { B_SPEED = 1, B_FOG = 2, B_JIMMY = 4, B_HOME = 8, B_BACK = 16 };

// The camera-relative direction (as the editor's 0x4b1ee0, CEditor.cpp).
void CameraDir(float out[3], float x, float y, float z)
{
	const float sx = omd_Sin(g_viewport->get_anglex()), cx = omd_Cos(g_viewport->get_anglex());
	const float sy = omd_Sin(g_viewport->get_angley()), cy = omd_Cos(g_viewport->get_angley());
	const float sz = omd_Sin(g_viewport->get_anglez()), cz = omd_Cos(g_viewport->get_anglez());
	const float t = cz * y - sz * x;
	const float u = sz * y + cz * x;
	const float v = cx * z - t * sx;
	out[0] = u * cy - v * sy;
	out[1] = sx * z + t * cx;
	out[2] = u * sy + v * cy;
}

void Home(void)
{
	g_viewport->place(s_home[0], s_home[1], s_home[2]);
	g_viewport->set_angle(s_home_ax, s_home_ay, 0);
}

void ShowJimmy(bool on)
{
	if (C3DJimmy *jim = neg::Jimmy())
		if (OMediaElement *e = dynamic_cast<OMediaElement *>(jim)) e->hide(!on);
}

void Text(short x, short y, const char *s)
{
	neg::DrawText(x, y, 'SMAL', "%s", s);
}
}

const char *neg::AssetViewLevel(void)
{
	const char *want = getenv("NEG_ASSET_VIEW");
	if (!want || !*want || strlen(want) >= 64) return NULL;	// (m_level_name holds 80)
	return want;
}

bool neg::AssetViewOn(void)
{
	return s_on;
}

void neg::AssetViewLevelLoaded(const char *name)
{
	const char *want = AssetViewLevel();
	if (!want || strcasecmp(want, name) != 0) return;
	snprintf(s_level, sizeof s_level, "%s", name);
	const char *nm = CompanionLevelName(name);
	snprintf(s_name, sizeof s_name, "%s", nm ? nm : name);
	// The level's start trigger stays unfired (nothing runs anyway).
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (C3DStartPoint *s = dynamic_cast<C3DStartPoint *>(*i)) s->m_trigger_armed = 0;
	// Start behind and above Jimmy, looking his way, a little down.
	if (C3DJimmy *jim = Jimmy())
	{
		float p[4], d[3];
		jim->v195(p);
		OMediaElement *e = dynamic_cast<OMediaElement *>(jim);
		g_viewport->set_angle(-700, e ? e->get_angley() : 0, 0);
		CameraDir(d, 0, 0, -1);								// (the view looks along +z)
		g_viewport->place(p[0] + d[0] * 260.0f, p[1] + 160.0f + d[1] * 260.0f, p[2] + d[2] * 260.0f);
	}
	s_home[0] = g_viewport->getx(); s_home[1] = g_viewport->gety(); s_home[2] = g_viewport->getz();
	s_home_ax = g_viewport->get_anglex(); s_home_ay = g_viewport->get_angley();
	if (g_layer_world) s_fog = s_fog_was = g_layer_world->*LayerFogAccess::Enabled();
	if (g_gametype) g_gametype->SetCamera(NULL);
	// The HUD is hidden (its 2D layer); the overlay's text is on the top 2D layer.
	if (g_layer_hud2d) g_layer_hud2d->set_flags(g_layer_hud2d->get_flags() & ~omlayerf_Visible);
	s_on = true;
	s_hint = 6.0f;
	SDL_Log("NEG_ASSET_VIEW: %s (%s) loaded, explorer on", name, s_name);
}

bool neg::AssetViewFrame(float dt)
{
	if (!s_on) return false;
	Pads();
	const Uint8 *k = SDL_GetKeyboardState(NULL);

	// buttons (edges)
	unsigned held = 0;
	if (Button(SDL_CONTROLLER_BUTTON_A) || k[SDL_SCANCODE_TAB]) held |= B_SPEED;
	if (Button(SDL_CONTROLLER_BUTTON_Y) || k[SDL_SCANCODE_F]) held |= B_FOG;
	if (Button(SDL_CONTROLLER_BUTTON_X) || k[SDL_SCANCODE_J]) held |= B_JIMMY;
	if (Button(SDL_CONTROLLER_BUTTON_START) || k[SDL_SCANCODE_HOME]) held |= B_HOME;
	if (Button(SDL_CONTROLLER_BUTTON_B) || Button(SDL_CONTROLLER_BUTTON_BACK) || k[SDL_SCANCODE_ESCAPE]
		|| k[SDL_SCANCODE_BACKSPACE] || k[SDL_SCANCODE_AC_BACK]) held |= B_BACK;
	const unsigned press = held & ~s_prev;
	s_prev = held;
	if (press & B_SPEED) s_speed = (s_speed + 1) % 3;
	if ((press & B_FOG) && g_layer_world)
	{
		s_fog = !s_fog;
		g_layer_world->*LayerFogAccess::Enabled() = s_fog && s_fog_was;
	}
	if (press & B_JIMMY) { s_jimmy = !s_jimmy; ShowJimmy(s_jimmy); }
	if (press & B_HOME) Home();
	if (press & B_BACK)
	{
		SDL_Log("NEG_ASSET_VIEW: back");
		s_on = false;
		g_app->quit();
		return true;
	}

	// look: right stick (stick up looks up), arrows
	float lx = Axis(SDL_CONTROLLER_AXIS_RIGHTX), ly = Axis(SDL_CONTROLLER_AXIS_RIGHTY);
	if (k[SDL_SCANCODE_LEFT]) lx -= 1; if (k[SDL_SCANCODE_RIGHT]) lx += 1;
	if (k[SDL_SCANCODE_UP]) ly -= 1; if (k[SDL_SCANCODE_DOWN]) ly += 1;
	int ay = g_viewport->get_angley() - (int)(lx * kTurn * dt);		// (a larger y angle turns left)
	int ax = (short)g_viewport->get_anglex() - (int)(ly * kTurn * dt);	// (a larger x angle looks up)
	if (ax > kPitchMax) ax = kPitchMax;
	if (ax < -kPitchMax) ax = -kPitchMax;
	g_viewport->set_angle((short)ax, (short)(ay & omc_MaxAngleMask), 0);

	// move: left stick forward / back and strafe (in the view direction), triggers / shoulders
	// down / up (straight up), WASD, Q / E
	float mx = Axis(SDL_CONTROLLER_AXIS_LEFTX), mz = Axis(SDL_CONTROLLER_AXIS_LEFTY);
	float my = Axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT) - Axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
	if (Button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) || k[SDL_SCANCODE_E] || k[SDL_SCANCODE_PAGEUP]) my += 1;
	if (Button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER) || k[SDL_SCANCODE_Q] || k[SDL_SCANCODE_PAGEDOWN]) my -= 1;
	if (k[SDL_SCANCODE_W]) mz -= 1; if (k[SDL_SCANCODE_S]) mz += 1;
	if (k[SDL_SCANCODE_A]) mx -= 1; if (k[SDL_SCANCODE_D]) mx += 1;
	const float v = kSpeed[s_speed] * dt;
	float d[3];
	CameraDir(d, mx, 0, -mz);								// (stick up is -1; forward is +z)
	g_viewport->place(g_viewport->getx() + d[0] * v, g_viewport->gety() + d[1] * v + my * v,
					  g_viewport->getz() + d[2] * v);

	// what keeps going: the texture animations and scrolls, the sky's turn
	UpdateTextureAnims(dt);
	if (CJimmyGame *g = dynamic_cast<CJimmyGame *>(g_gametype))
	{
		if (g->m_no_texture_scroll == 0) UpdateTexScrolls(dt);
		if (g->m_sky && g->m_sky_rotates) g->m_sky->AddRotation(0.0f, dt, 0.0f);
	}

	// the overlay (the text sprites are redrawn every frame)
	float rgb[3] = { g_text_rgb[0], g_text_rgb[1], g_text_rgb[2] };
	g_text_rgb[0] = 0.45f; g_text_rgb[1] = 1.0f; g_text_rgb[2] = 0.55f;
	char line[160];
	snprintf(line, sizeof line, "ASSET VIEWER  %s", s_name);
	Text(12, 8, line);
	snprintf(line, sizeof line, "X %.0f  Y %.0f  Z %.0f   SPEED %s%s", (double)g_viewport->getx(),
			 (double)g_viewport->gety(), (double)g_viewport->getz(), kSpeedName[s_speed], !s_fog_was ? "" : s_fog ? "   FOG ON" : "   FOG OFF");
	Text(12, 20, line);
	if (s_hint > 0)
	{
		s_hint -= dt;
		Text(12, 444, "L STICK MOVE   R STICK LOOK   L2 R2 DOWN UP   A SPEED");
		Text(12, 456, "Y FOG   X JIMMY   START RESET   B BACK");
	}
	else Text(12, 456, "B BACK");
	g_text_rgb[0] = rgb[0]; g_text_rgb[1] = rgb[1]; g_text_rgb[2] = rgb[2];
	return true;
}

void neg::WriteLevelList(void)
{
	std::string dir = omf_SDLResolvePath("gam");
	DIR *d = opendir(dir.c_str());
	if (!d) return;
	std::vector<std::string> files;
	while (struct dirent *e = readdir(d))
	{
		size_t l = strlen(e->d_name);
		if (l > 4 && strcasecmp(e->d_name + l - 4, ".gam") == 0) files.push_back(e->d_name);
	}
	closedir(d);
	std::sort(files.begin(), files.end());
	FILE *f = fopen(omf_SDLResolvePath("save\\levels.txt").c_str(), "w");
	if (!f) return;
	fputs("# gam file\tdisplay name (written by the game, see game/src/port/AssetView.h)\n", f);
	for (size_t i = 0; i < files.size(); i++)
	{
		const char *nm = CompanionLevelName(files[i].c_str());
		fprintf(f, "%s\t%s\n", files[i].c_str(), nm ? nm : "");
	}
	fclose(f);
}
