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
#include "../C3DASEObj.h"
#include "../C3DGrannyObj.h"
#include "../../granny/granny_compat.h"
#include "../gen/globals.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaTrigo.h"
#include "OMediaDataBase.h"
#include "OMediaFileStream.h"
#include "OMedia3DShape.h"
#include "OMedia3DShapeElement.h"
#include "OMediaError.h"
#include "OMediaLight.h"
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

// --------------------------------------------------------------------------- the model viewer
// NEG_ASSET_VIEW=models: the game's own loaders show one model at a time on an empty stage (the
// small Template.gam level, every one of its objects hidden, no sky, no fog): .grn models as a
// C3DGrannyObj makes them (InitModel: the bind pose), .ase as a C3DASEObj does (its first frame;
// the texture named by the file's *BITMAP, from png\), and the 3D shapes of the .omt databases.

struct Model { std::string file; char kind; };	// kind: 'g' .grn, 'a' .ase, 'o' .omt
bool				s_models_mode = false;
std::vector<Model>	s_models;
int					s_mi = -1;					// the model shown
CGameObject			*s_obj = NULL;				// its game object (.grn / .ase)
OMedia3DShapeElement *s_elem = NULL;			// its element (.omt shape)
OMediaFileStream	*s_db_fs = NULL;			// the .omt database shown
OMediaDataBase		*s_db = NULL;
std::vector<OMedia3DShape *> s_shapes;			// its 3D shapes (locked while listed)
std::vector<std::string> s_shape_names;
int					s_si = 0;
float				s_yaw = 0.6f, s_pitch = 0.35f, s_dist = 300, s_radius = 100;
float				s_center[3];
bool				s_spin = true;
char				s_info[96];
float				s_repeat = 0;				// d-pad repeat
const float			kStage[3] = { 0.0f, 20000.0f, 0.0f };

bool HasExt(const char *n, const char *ext)
{
	size_t l = strlen(n), e = strlen(ext);
	return l > e && strcasecmp(n + l - e, ext) == 0;
}

bool NoCaseLess(const Model &a, const Model &b)
{
	if (a.kind != b.kind) return a.kind == 'g' || (a.kind == 'a' && b.kind == 'o');
	return strcasecmp(a.file.c_str(), b.file.c_str()) < 0;
}

void ListDir(const char *dir, const char *ext, char kind)
{
	DIR *d = opendir(omf_SDLResolvePath(dir).c_str());
	if (!d) return;
	while (struct dirent *e = readdir(d))
	{
		if (!HasExt(e->d_name, ext)) continue;
		if (kind == 'o' && (!strncasecmp(e->d_name, "music", 5) || !strncasecmp(e->d_name, "sound", 5))) continue;
		Model m;
		m.file = e->d_name;
		m.kind = kind;
		s_models.push_back(m);
	}
	closedir(d);
}

// The texture an .ase names (*BITMAP "...\name.bmp") as png\name.png, if the game has it.
bool AseTexture(const char *ase, char *png, size_t size)
{
	FILE *f = fopen(omf_SDLResolvePath(std::string("ase\\") + ase).c_str(), "rb");
	if (!f) return false;
	char line[512];
	bool found = false;
	while (!found && fgets(line, sizeof line, f))
	{
		char *b = strstr(line, "*BITMAP \"");
		if (!b) continue;
		b += 9;
		char *q = strchr(b, '"');
		if (q) *q = 0;
		char *base = b;
		for (char *c = b; *c; c++) if (*c == '\\' || *c == '/') base = c + 1;
		char *dot = strrchr(base, '.');
		if (dot) *dot = 0;
		snprintf(png, size, "%s.png", base);
		FILE *t = fopen(omf_SDLResolvePath(std::string("png\\") + png).c_str(), "rb");
		if (t) { fclose(t); found = true; }
	}
	fclose(f);
	return found;
}

void ShowShape(OMedia3DShape *sh)
{
	if (!s_elem)
	{
		s_elem = new OMedia3DShapeElement;
		s_elem->link(g_world);
		s_elem->link_layer(g_layer_world);
		s_elem->place(kStage[0], kStage[1], kStage[2]);
	}
	s_elem->set_shape(sh);
}

// The view: around the shape's bounds (its vertices, at the stage).
void Frame(OMedia3DShape *sh, float scale)
{
	s_center[0] = kStage[0]; s_center[1] = kStage[1]; s_center[2] = kStage[2];
	s_radius = 100;
	omt_VertexList *v = sh ? sh->get_vertices() : NULL;
	if (!v || v->empty()) return;
	float lo[3] = { 1e30f, 1e30f, 1e30f }, hi[3] = { -1e30f, -1e30f, -1e30f };
	for (size_t i = 0; i < v->size(); i++)
	{
		const OMedia3DPoint &p = (*v)[i];
		const float c[3] = { p.x, p.y, p.z };
		for (int k = 0; k < 3; k++) { if (c[k] < lo[k]) lo[k] = c[k]; if (c[k] > hi[k]) hi[k] = c[k]; }
	}
	float r = 0;
	for (int k = 0; k < 3; k++)
	{
		s_center[k] += (lo[k] + hi[k]) * 0.5f * scale;
		r += (hi[k] - lo[k]) * (hi[k] - lo[k]);
	}
	s_radius = sqrtf(r) * 0.5f * scale;
	if (s_radius < 1) s_radius = 1;
	s_dist = s_radius * 2.6f;
}

void Unload(void)
{
	if (s_obj) { delete s_obj; s_obj = NULL; }
	if (s_elem) { s_elem->set_shape(NULL); delete s_elem; s_elem = NULL; }
	for (size_t i = 0; i < s_shapes.size(); i++) s_shapes[i]->db_unlock();
	s_shapes.clear();
	s_shape_names.clear();
	if (s_db) { UnInitDatabase(s_db); s_db = NULL; }
	if (s_db_fs) { UnInitFileStream(s_db_fs); s_db_fs = NULL; }
}

void ShowOmtShape(int i)
{
	if (s_shapes.empty()) return;
	s_si = (i + (int)s_shapes.size()) % (int)s_shapes.size();
	ShowShape(s_shapes[s_si]);
	Frame(s_shapes[s_si], 1.0f);
	snprintf(s_info, sizeof s_info, "SHAPE %d OF %d  %s", s_si + 1, (int)s_shapes.size(), s_shape_names[s_si].c_str());
}

void Load(int i)
{
	Unload();
	if (s_models.empty()) return;
	s_mi = (i + (int)s_models.size()) % (int)s_models.size();
	const Model &m = s_models[s_mi];
	s_info[0] = 0;
	OMedia3DShape *shape = NULL;
	float scale = 1.0f;
	try
	{
		if (m.kind == 'g')
		{
			C3DGrannyObj *o = dynamic_cast<C3DGrannyObj *>(CreateObject('3GRO'));
			if (o)
			{
				s_obj = o;
				o->m_object_id = '3GRO';
				snprintf(o->f_924, sizeof o->f_924, "%s", m.file.c_str());
				strcpy(o->f_988, "none"); strcpy(o->f_9ec, "none"); strcpy(o->f_a50, "none");
				strcpy(o->f_ab4, "none"); strcpy(o->f_b18, "none"); strcpy(o->f_b7c, "none");
				o->ResetObject();
				o->v260();
				o->v197(kStage[0], kStage[1], kStage[2]);
				o->v90(kStage[0], kStage[1], kStage[2]);
				o->place(kStage[0], kStage[1], kStage[2]);			// (the element; Process would)
				o->ShowAgain();
				shape = o->get_shape();
				scale = o->f_6f0 > 0 ? o->f_6f0 : 1.0f;
			}
		}
		else if (m.kind == 'a')
		{
			C3DASEObj *o = dynamic_cast<C3DASEObj *>(CreateObject('3ASE'));
			if (o)
			{
				s_obj = o;
				o->m_object_id = '3ASE';
				snprintf(o->f_924, sizeof o->f_924, "%s", m.file.c_str());
				snprintf(o->f_988, sizeof o->f_988, "%s", m.file.c_str());
				char png[128];
				if (AseTexture(m.file.c_str(), png, sizeof png)) snprintf(o->f_ab4, sizeof o->f_ab4, "%s", png);
				else { strcpy(o->f_ab4, "none"); strcpy(s_info, "NO TEXTURE"); }
				o->f_664 = 0;										// "SecondPass" off: its materials would wait for a pass the stage does not draw
				o->ResetObject();
				o->v260();
				o->v197(kStage[0], kStage[1], kStage[2]);
				o->v90(kStage[0], kStage[1], kStage[2]);
				o->place(kStage[0], kStage[1], kStage[2]);			// (the element; Process would)
				o->ShowAgain();
				shape = o->get_shape();
			}
		}
		else
		{
			s_db_fs = InitFileStream((std::string("omt\\") + m.file).c_str());
			s_db = s_db_fs ? InitDatabase(s_db_fs) : NULL;
			if (s_db)
			{
				omt_DBIterator it;
				OMediaDBObject *o = s_db->get_first_object(OMedia3DShape::db_type, it);
				while (o)
				{
					std::string name;
					o->get_chunk_name(name);
					s_shapes.push_back((OMedia3DShape *)o);
					s_shape_names.push_back(name);
					o = s_db->get_next_object(it);
				}
			}
			if (s_shapes.empty()) strcpy(s_info, "NO 3D SHAPES");
			else { ShowOmtShape(0); return; }
		}
	}
	catch (OMediaError &e)
	{
		SDL_Log("NEG_ASSET_VIEW: %s: %s", m.file.c_str(), e.error_string.c_str());
		snprintf(s_info, sizeof s_info, "COULD NOT LOAD");
	}
	Frame(shape, scale);
	if (!s_info[0])
	{
		omt_VertexList *v = shape ? shape->get_vertices() : NULL;
		if (!v || v->empty()) strcpy(s_info, m.kind == 'g' ? "NO MESH (AN ANIMATION)" : "NO MESH");
		else snprintf(s_info, sizeof s_info, "%d VERTICES", (int)v->size());
	}
}

void ModelsStart(void)
{
	// an empty stage: the level's objects hidden, no sky (the clear layer's colour), no fog
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (OMediaElement *e = dynamic_cast<OMediaElement *>(*i)) e->hide(true);
	if (g_layer_world) g_layer_world->*LayerFogAccess::Enabled() = false;
	if (g_layer_clear)
	{
		g_layer_clear->set_clear_color(OMediaFARGBColor(1.0f, 0.05f, 0.08f, 0.06f));
		g_layer_clear->set_flags(g_layer_clear->get_flags() | omlayerf_ClearColor);
	}
	// a light of our own (the level's are far away): a directional key light and a strong ambient
	OMediaLight *light = new OMediaLight;
	light->link(g_world);
	light->set_light_type(omclt_Directional);
	light->set_angle(omd_Deg2Angle(-50), omd_Deg2Angle(30), 0);
	light->set_ambient(OMediaFARGBColor(1.0f, 0.55f, 0.55f, 0.55f));
	light->set_diffuse(OMediaFARGBColor(1.0f, 0.6f, 0.6f, 0.6f));
	ListDir("grn", ".grn", 'g');
	ListDir("ase", ".ase", 'a');
	ListDir("omt", ".omt", 'o');
	std::sort(s_models.begin(), s_models.end(), NoCaseLess);
	int first = 0;
	if (const char *want = getenv("NEG_MODEL"))			// (desktop tests: the model to start with)
		for (size_t i = 0; i < s_models.size(); i++)
			if (!strcasecmp(s_models[i].file.c_str(), want)) first = (int)i;
	Load(first);
}

enum { M_UP = 1, M_DOWN = 2, M_LEFT = 4, M_RIGHT = 8, M_PGUP = 16, M_PGDN = 32, M_SPIN = 64, M_HOME = 128, M_BACK = 256 };

bool ModelsFrame(float dt)
{
	Pads();
	const Uint8 *k = SDL_GetKeyboardState(NULL);
	unsigned held = 0;
	if (Button(SDL_CONTROLLER_BUTTON_DPAD_UP) || k[SDL_SCANCODE_UP]) held |= M_UP;
	if (Button(SDL_CONTROLLER_BUTTON_DPAD_DOWN) || k[SDL_SCANCODE_DOWN]) held |= M_DOWN;
	if (Button(SDL_CONTROLLER_BUTTON_DPAD_LEFT) || k[SDL_SCANCODE_LEFT]) held |= M_LEFT;
	if (Button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || k[SDL_SCANCODE_RIGHT]) held |= M_RIGHT;
	if (Button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER) || k[SDL_SCANCODE_PAGEUP]) held |= M_PGUP;
	if (Button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) || k[SDL_SCANCODE_PAGEDOWN]) held |= M_PGDN;
	if (Button(SDL_CONTROLLER_BUTTON_A) || k[SDL_SCANCODE_SPACE]) held |= M_SPIN;
	if (Button(SDL_CONTROLLER_BUTTON_START) || k[SDL_SCANCODE_HOME]) held |= M_HOME;
	if (Button(SDL_CONTROLLER_BUTTON_B) || Button(SDL_CONTROLLER_BUTTON_BACK) || k[SDL_SCANCODE_ESCAPE]
		|| k[SDL_SCANCODE_BACKSPACE] || k[SDL_SCANCODE_AC_BACK]) held |= M_BACK;
	unsigned press = held & ~s_prev;
	if (held & (M_UP | M_DOWN | M_LEFT | M_RIGHT))		// held: repeats after 0.4 s, 12 a second
	{
		s_repeat += dt;
		if (s_repeat > 0.4f) { press |= held & (M_UP | M_DOWN | M_LEFT | M_RIGHT); s_repeat -= 1.0f / 12; }
	}
	else s_repeat = 0;
	s_prev = held;
	if (press & M_BACK)
	{
		SDL_Log("NEG_ASSET_VIEW: back");
		s_on = false;
		Unload();
		g_app->quit();
		return true;
	}
	if (press & M_UP) Load(s_mi - 1);
	if (press & M_DOWN) Load(s_mi + 1);
	if (press & M_PGUP) Load(s_mi - 10);
	if (press & M_PGDN) Load(s_mi + 10);
	if ((press & M_LEFT) && !s_shapes.empty()) ShowOmtShape(s_si - 1);
	if ((press & M_RIGHT) && !s_shapes.empty()) ShowOmtShape(s_si + 1);
	if (press & M_SPIN) s_spin = !s_spin;
	if (press & M_HOME) { s_yaw = 0.6f; s_pitch = 0.35f; s_dist = s_radius * 2.6f; }

	// orbit: either stick (and W A S D); zoom: L2 / R2 (Q / E)
	float ox = Axis(SDL_CONTROLLER_AXIS_RIGHTX) + Axis(SDL_CONTROLLER_AXIS_LEFTX);
	float oy = Axis(SDL_CONTROLLER_AXIS_RIGHTY) + Axis(SDL_CONTROLLER_AXIS_LEFTY);
	if (k[SDL_SCANCODE_A]) ox -= 1; if (k[SDL_SCANCODE_D]) ox += 1;
	if (k[SDL_SCANCODE_W]) oy -= 1; if (k[SDL_SCANCODE_S]) oy += 1;
	float zoom = Axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT) - Axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
	if (k[SDL_SCANCODE_E]) zoom += 1; if (k[SDL_SCANCODE_Q]) zoom -= 1;
	if (ox != 0 || oy != 0) s_spin = false;
	s_yaw += (s_spin ? 0.5f : ox * 2.2f) * dt;
	s_pitch += oy * 1.6f * dt;
	if (s_pitch > 1.45f) s_pitch = 1.45f;
	if (s_pitch < -1.45f) s_pitch = -1.45f;
	s_dist *= expf(-zoom * 1.5f * dt);
	if (s_dist < s_radius * 0.3f) s_dist = s_radius * 0.3f;
	if (s_dist > s_radius * 20.0f) s_dist = s_radius * 20.0f;
	// the camera: looking at the centre from (yaw, pitch) at s_dist (the view looks along +z)
	const float kA = 16384.0f / 6.2831853f;
	g_viewport->set_angle((short)(int)(-s_pitch * kA), (short)((int)(s_yaw * kA) & omc_MaxAngleMask), 0);
	float d[3];
	CameraDir(d, 0, 0, 1);
	g_viewport->place(s_center[0] - d[0] * s_dist, s_center[1] - d[1] * s_dist, s_center[2] - d[2] * s_dist);
	if (s_obj && s_models[s_mi].kind == 'g')			// a granny model: its pose (the editor's frame)
	{
		if (g_granny) GrannyAdvanceTime(g_granny, (double)dt);
		s_obj->v242(dt);
	}
	UpdateTextureAnims(dt);

	// the overlay: the list around the selection, the model, the controls
	float rgb[3] = { g_text_rgb[0], g_text_rgb[1], g_text_rgb[2] };
	char line[160];
	g_text_rgb[0] = 0.45f; g_text_rgb[1] = 1.0f; g_text_rgb[2] = 0.55f;
	snprintf(line, sizeof line, "MODEL VIEWER   %d OF %d", s_mi + 1, (int)s_models.size());
	Text(12, 8, line);
	const int rows = 15;
	int top = s_mi - rows / 2;
	if (top > (int)s_models.size() - rows) top = (int)s_models.size() - rows;
	if (top < 0) top = 0;
	for (int r = 0; r < rows && top + r < (int)s_models.size(); r++)
	{
		const int i = top + r;
		const bool sel = i == s_mi;
		g_text_rgb[0] = sel ? 0.8f : 0.25f; g_text_rgb[1] = sel ? 1.0f : 0.7f; g_text_rgb[2] = sel ? 0.8f : 0.35f;
		snprintf(line, sizeof line, "%s %s", sel ? ">" : " ", s_models[i].file.c_str());
		Text(12, (short)(30 + r * 12), line);
	}
	g_text_rgb[0] = 0.45f; g_text_rgb[1] = 1.0f; g_text_rgb[2] = 0.55f;
	if (s_mi >= 0) { snprintf(line, sizeof line, "%s   %s", s_models[s_mi].file.c_str(), s_info); Text(12, 420, line); }
	Text(12, 444, s_shapes.size() > 1 ? "UP DOWN MODEL   LEFT RIGHT SHAPE   L1 R1 JUMP 10   STICKS TURN"
									   : "UP DOWN MODEL   L1 R1 JUMP 10   STICKS TURN");
	Text(12, 456, s_spin ? "L2 R2 ZOOM   A STOP TURNING   START RESET   B BACK" : "L2 R2 ZOOM   A TURN   START RESET   B BACK");
	g_text_rgb[0] = rgb[0]; g_text_rgb[1] = rgb[1]; g_text_rgb[2] = rgb[2];
	return true;
}
}

const char *neg::AssetViewLevel(void)
{
	const char *want = getenv("NEG_ASSET_VIEW");
	if (!want || !*want || strlen(want) >= 64) return NULL;	// (m_level_name holds 80)
	if (!strcasecmp(want, "models")) return "Template.gam";	// the model viewer's stage
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
	s_models_mode = !strcasecmp(getenv("NEG_ASSET_VIEW"), "models");
	if (s_models_mode) { ModelsStart(); SDL_Log("NEG_ASSET_VIEW: model viewer, %d models", (int)s_models.size()); return; }
	s_hint = 6.0f;
	SDL_Log("NEG_ASSET_VIEW: %s (%s) loaded, explorer on", name, s_name);
}

bool neg::AssetViewFrame(float dt)
{
	if (!s_on) return false;
	if (s_models_mode) return ModelsFrame(dt);
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
