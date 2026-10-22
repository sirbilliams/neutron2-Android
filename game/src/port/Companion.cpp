/*  port: the dual-screen companion - see Companion.h. Reads game state only. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"		// (double constants in the OMT headers)
#include "Companion.h"
#include "LauncherArt.h"
#include "../C3DJimmy.h"
#include "../C3DAI.h"
#include "../C3DTerrain.h"
#include "../C2DInGameMenu.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../InterfaceElement.h"
#include "../gen/globals.h"
#include "../friends/C3DFriends.h"
#include "../lab/C3DHerman.h"
#include "../lab/C3DSwingDoor.h"
#include "../levels/CLoadLevel.h"
#include "../pickups/C3DPickupItem.h"
#include "../pickups/C3DRedNeutron.h"
#include "../C3DGoddard.h"
#include "../CanvasWrapper.h"
#include "../objects/C3DArrow.h"
#include "../objects/C3DNeutron.h"
#include "../objects/C3DCheckPoint.h"
#include "VoxPicker.h"
#include "OMediaElement.h"
#include "OMedia3DShape.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DMaterial.h"
#include "OMediaCanvas.h"
#include "OMediaDataBase.h"
#include "OMediaFileStream.h"
#include "OMediaFilePath.h"
#pragma GCC diagnostic pop
#include <SDL.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace neg;

namespace {

SDL_atomic_t		s_enabled;				// 0 / 1 (zero-initialised)
bool				s_log = false;
bool				s_env_read = false;
std::string			s_dump_dir;				// NEG_COMPANION_DUMP (desktop debugging)
float				s_clock = 0;			// since the first tick
float				s_since_build = 1.0f;
float				s_since_log = 0;
float				s_since_dump = 0;
float				s_level_time = 0;		// game time played in this level
SDL_mutex			*s_mutex = SDL_CreateMutex();	// (created at load: SDL mutexes need no SDL_Init)
std::string			s_snapshot;				// guarded by s_mutex

// --------------------------------------------------------------------------- the map image

enum { kMapMax = 1536 };					// longest side in pixels
const float kMapScale = 6.0f;				// world units per pixel at most; a bigger level gets a
											// window of the map around Jimmy, moved as he goes
const float kFloorY = 0.7f;					// physics: |normal.y| below this is a wall (Physics.cpp kSteep)
const float kNoHeight = -1e30f;
const float kCutAbove = 350.0f;				// floors this far above Jimmy's feet are left out (roofs,
											// the storeys above him)
const float kCutMove = 200.0f;				// cut again when his height has changed by more than this

// The map shown (game thread; the snapshot's map coordinates refer to it).
struct MapGeom
{
	int		gen;
	int		w, h;
	float	x0, z1, s;			// image pixel (0, 0) = world (x0, z1); s world units per pixel
	bool	cut;				// floors above cut_y left out
	float	cut_y;
};
MapGeom				s_map;					// (zero-initialised: no map)
std::string			s_map_key;				// level file + terrain shapes of the geometry copy
std::vector<uint32_t>	s_map_px;			// ARGB (Android Bitmap order), guarded by s_mutex
int					s_map_px_gen = 0, s_map_px_w = 0, s_map_px_h = 0;	// guarded by s_mutex

inline void WorldToMap(const MapGeom &g, float x, float z, float &mx, float &my)
{
	mx = (x - g.x0) / g.s;
	my = (g.z1 - z) / g.s;					// north (+z) up: see Companion.h for the check
}
inline void WorldToMap(float x, float z, float &mx, float &my) { WorldToMap(s_map, x, z, mx, my); }

// A texture reduced to at most 32 x 32 (box filter), RGB.
struct SmallTex { int w, h; std::vector<uint32_t> rgb; };

void ReduceTexture(OMediaCanvas *c, SmallTex &t)
{
	t.w = t.h = 0;
	t.rgb.clear();
	if (!c) return;
	c->lock(omlf_Read);
	const omt_RGBAPixel *px = c->get_pixels();
	const int cw = c->get_width(), ch = c->get_height();
	if (px && cw > 0 && ch > 0)
	{
		t.w = std::min(cw, 32);
		t.h = std::min(ch, 32);
		t.rgb.resize((size_t)(t.w * t.h));
		for (int ty = 0; ty < t.h; ty++)
			for (int tx = 0; tx < t.w; tx++)
			{
				const int xa = tx * cw / t.w, xb = std::max(xa + 1, (tx + 1) * cw / t.w);
				const int ya = ty * ch / t.h, yb = std::max(ya + 1, (ty + 1) * ch / t.h);
				unsigned r = 0, g = 0, b = 0, n = 0;
				for (int y = ya; y < yb; y++)
					for (int x = xa; x < xb; x++)
					{
						const omt_RGBAPixel p = px[y * cw + x];		// bytes R, G, B, A
						r += p & 0xff; g += (p >> 8) & 0xff; b += (p >> 16) & 0xff; n++;
					}
				t.rgb[(size_t)(ty * t.w + tx)] = ((r / n) << 16) | ((g / n) << 8) | (b / n);
			}
	}
	c->unlock();
}

// A copy of the level's terrain for the map worker (world coordinates).
struct GeoVertex { float x, z, h, u, v; };
struct GeoTri { GeoVertex p[3]; int tex; float cr, cg, cb; };	// a floor triangle; tex -1 = none
struct GeoWall { GeoVertex a, b; float low; };				// a wall polygon's edge, its lowest point
struct Geo
{
	std::vector<GeoTri>		floors;
	std::vector<GeoWall>	walls;
	std::vector<SmallTex>	texs;
	float minx, maxx, minz, maxz;
};
Geo					*s_geo = NULL;			// the current level's (game thread)
std::vector<Geo *>	s_geo_retired;			// replaced while the worker used them

bool UsablePolygon(OMedia3DPolygon &p)
{
	return p.get_num_points() >= 3 && !(p.get_flags() & om3pf_Hide);
}

// The terrain shapes, and whether each is shown (CJimmyGame hides the "ik..." collision-only
// shapes; their walls still bound the level, their floors are not drawn on the map).
void Terrain(std::vector<OMedia3DShape *> &out, std::vector<bool> &shown)
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (C3DTerrain *t = dynamic_cast<C3DTerrain *>(*i))
			if (t->f_414)
			{
				out.push_back(t->f_414);
				shown.push_back(t->getvisible());
			}
}

std::string MapKey(const std::vector<OMedia3DShape *> &shapes)
{
	std::string k = g_game_file;
	char b[48];
	snprintf(b, sizeof b, "|%d|%p", (int)shapes.size(), shapes.empty() ? NULL : (void *)shapes[0]);
	return k + b;
}

// Copies the terrain polygons: the drawn floors (normal.y >= 0.7) as triangles with their
// texture (reduced) and material colour, and the walls' edges (also the invisible walls).
Geo *CopyTerrain(const std::vector<OMedia3DShape *> &shapes, const std::vector<bool> &shown)
{
	Geo *g = new Geo;
	g->minx = g->minz = 1e30f;
	g->maxx = g->maxz = -1e30f;
	std::map<OMediaCanvas *, int> tex_index;
	for (size_t k = 0; k < shapes.size(); k++)
	{
		std::vector<OMedia3DPoint> &v = *shapes[k]->get_vertices();
		std::vector<OMedia3DPolygon> &polys = *shapes[k]->get_polygons();
		for (size_t j = 0; j < polys.size(); j++)
		{
			OMedia3DPolygon &p = polys[j];
			if (!UsablePolygon(p)) continue;
			const int n = p.get_num_points();
			std::vector<GeoVertex> gv((size_t)n);
			float low = 1e30f;
			for (int i = 0; i < n; i++)
			{
				const OMedia3DPoint &q = v[p.get_point((short)i)];
				GeoVertex &o = gv[(size_t)i];
				o.x = q.x; o.z = q.z; o.h = q.y;
				o.u = p.get_vertices()[(size_t)i].u;
				o.v = p.get_vertices()[(size_t)i].v;
				low = std::min(low, q.y);
				g->minx = std::min(g->minx, q.x); g->maxx = std::max(g->maxx, q.x);
				g->minz = std::min(g->minz, q.z); g->maxz = std::max(g->maxz, q.z);
			}
			const float ny = p.get_normal().y;
			OMedia3DMaterial *m = p.get_material();
			if (ny >= kFloorY && m && shown[k])		// a floor that is drawn
			{
				int tex = -1;
				if (OMediaCanvas *c = m->get_texture())
				{
					std::map<OMediaCanvas *, int>::iterator t = tex_index.find(c);
					if (t == tex_index.end())
					{
						g->texs.push_back(SmallTex());
						ReduceTexture(c, g->texs.back());
						t = tex_index.insert(std::make_pair(c, (int)g->texs.size() - 1)).first;
					}
					tex = t->second;
				}
				float cr = m->get_diffuse_r(), cg = m->get_diffuse_g(), cb = m->get_diffuse_b();
				if (std::max(cr, std::max(cg, cb)) < 0.2f) cr = cg = cb = 1.0f;	// unlit / emissive materials
				for (int i = 1; i + 1 < n; i++)
				{
					GeoTri t;
					t.p[0] = gv[0]; t.p[1] = gv[(size_t)i]; t.p[2] = gv[(size_t)i + 1];
					t.tex = tex; t.cr = cr; t.cg = cg; t.cb = cb;
					g->floors.push_back(t);
				}
			}
			else if (fabsf(ny) < kFloorY)			// a wall
			{
				for (int i = 0; i < n; i++)
				{
					GeoWall w;
					w.a = gv[(size_t)i]; w.b = gv[(size_t)((i + 1) % n)]; w.low = low;
					g->walls.push_back(w);
				}
			}
		}
	}
	return g;
}

// --- the worker: draws a map from a Geo off the game thread

struct MapJob
{
	Geo		*geo;
	MapGeom	g;
	std::vector<uint32_t> px;
	Uint32	ms;
};
SDL_Thread		*s_worker = NULL;			// game thread
SDL_atomic_t	s_worker_done;
MapJob			*s_job = NULL;

struct Raster
{
	int w, h;
	float cut;
	std::vector<float>		height;		// top floor height, kNoHeight = none
	std::vector<uint32_t>	colour;		// its RGB
	std::vector<float>		wall;		// highest wall point, kNoHeight = none
};

struct MapVertex { float x, y, h, u, v; };

// One floor triangle into the raster (pixel centres; the highest point under the cut wins).
void FloorTriangle(Raster &r, const MapVertex &a, const MapVertex &b, const MapVertex &c,
				   const SmallTex *tex, float cr, float cg, float cb)
{
	const float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	if (fabsf(area) < 1e-6f) return;
	const int x0 = std::max(0, (int)floorf(std::min(a.x, std::min(b.x, c.x))));
	const int x1 = std::min(r.w - 1, (int)ceilf(std::max(a.x, std::max(b.x, c.x))));
	const int y0 = std::max(0, (int)floorf(std::min(a.y, std::min(b.y, c.y))));
	const int y1 = std::min(r.h - 1, (int)ceilf(std::max(a.y, std::max(b.y, c.y))));
	const float inv = 1.0f / area;
	for (int y = y0; y <= y1; y++)
		for (int x = x0; x <= x1; x++)
		{
			const float px = (float)x + 0.5f, py = (float)y + 0.5f;
			const float w0 = ((c.x - b.x) * (py - b.y) - (c.y - b.y) * (px - b.x)) * inv;
			const float w1 = ((a.x - c.x) * (py - c.y) - (a.y - c.y) * (px - c.x)) * inv;
			const float w2 = 1.0f - w0 - w1;
			if (w0 < -1e-4f || w1 < -1e-4f || w2 < -1e-4f) continue;
			const float hh = w0 * a.h + w1 * b.h + w2 * c.h;
			if (hh > r.cut) continue;
			const size_t i = (size_t)(y * r.w + x);
			if (hh <= r.height[i]) continue;
			r.height[i] = hh;
			float R = 170, G = 170, B = 170;
			if (tex && tex->w > 0)
			{
				float u = w0 * a.u + w1 * b.u + w2 * c.u, v = w0 * a.v + w1 * b.v + w2 * c.v;
				u -= floorf(u);
				v -= floorf(v);
				const int tx = std::min(tex->w - 1, (int)(u * (float)tex->w));
				const int ty = std::min(tex->h - 1, (int)(v * (float)tex->h));
				const uint32_t t = tex->rgb[(size_t)(ty * tex->w + tx)];
				R = (float)((t >> 16) & 0xff); G = (float)((t >> 8) & 0xff); B = (float)(t & 0xff);
			}
			R *= cr; G *= cg; B *= cb;
			r.colour[i] = ((uint32_t)std::min(255.0f, R) << 16) | ((uint32_t)std::min(255.0f, G) << 8)
						| (uint32_t)std::min(255.0f, B);
		}
}

// A wall's edge (a projected line, heights cut at the cut) into the wall buffer, keeping the
// highest point per pixel.
void WallLine(Raster &r, const MapVertex &a, const MapVertex &b)
{
	const float dx = b.x - a.x, dy = b.y - a.y;
	const int n = (int)ceilf(std::max(fabsf(dx), fabsf(dy))) + 1;
	for (int k = 0; k <= n; k++)
	{
		const float t = (float)k / (float)n;
		const int x = (int)(a.x + dx * t), y = (int)(a.y + dy * t);
		if (x < 0 || y < 0 || x >= r.w || y >= r.h) continue;
		const float hh = std::min(r.cut, a.h + (b.h - a.h) * t);
		float &w = r.wall[(size_t)(y * r.w + x)];
		if (hh > w) w = hh;
	}
}

void DrawMap(MapJob &job)
{
	const Uint32 t0 = SDL_GetTicks();
	const Geo &geo = *job.geo;
	const MapGeom &g = job.g;
	Raster r;
	r.w = g.w; r.h = g.h;
	r.cut = g.cut ? g.cut_y : 1e30f;
	const size_t n = (size_t)(r.w * r.h);
	r.height.assign(n, kNoHeight);
	r.colour.assign(n, 0);
	r.wall.assign(n, kNoHeight);
	MapVertex mv[3];
	for (size_t k = 0; k < geo.floors.size(); k++)
	{
		const GeoTri &t = geo.floors[k];
		for (int i = 0; i < 3; i++)
		{
			WorldToMap(g, t.p[i].x, t.p[i].z, mv[i].x, mv[i].y);
			mv[i].h = t.p[i].h; mv[i].u = t.p[i].u; mv[i].v = t.p[i].v;
		}
		FloorTriangle(r, mv[0], mv[1], mv[2], t.tex >= 0 ? &geo.texs[(size_t)t.tex] : NULL, t.cr, t.cg, t.cb);
	}
	for (size_t k = 0; k < geo.walls.size(); k++)
	{
		const GeoWall &w = geo.walls[k];
		if (w.low >= r.cut) continue;					// a wall of a storey above
		WorldToMap(g, w.a.x, w.a.z, mv[0].x, mv[0].y);
		WorldToMap(g, w.b.x, w.b.z, mv[1].x, mv[1].y);
		mv[0].h = w.a.h; mv[1].h = w.b.h;
		WallLine(r, mv[0], mv[1]);
	}

	// fill single-pixel cracks between triangles (a pixel with most neighbours drawn)
	for (int y = 1; y + 1 < r.h; y++)
		for (int x = 1; x + 1 < r.w; x++)
		{
			const size_t i = (size_t)(y * r.w + x);
			if (r.height[i] != kNoHeight) continue;
			int cnt = 0; float hh = kNoHeight; uint32_t col = 0;
			for (int dy = -1; dy <= 1; dy++)
				for (int dx = -1; dx <= 1; dx++)
				{
					const size_t j = (size_t)((y + dy) * r.w + x + dx);
					if (j != i && r.height[j] != kNoHeight)
					{
						cnt++;
						if (r.height[j] > hh) { hh = r.height[j]; col = r.colour[j]; }
					}
				}
			if (cnt >= 6) { r.height[i] = hh - 0.001f; r.colour[i] = col; }
		}

	// height range (2nd to 98th percentile of the drawn floor)
	std::vector<float> hs;
	hs.reserve(n / 16 + 1);
	for (size_t i = 0; i < n; i += 16) if (r.height[i] != kNoHeight) hs.push_back(r.height[i]);
	float lo = 0, hi = 1;
	if (!hs.empty())
	{
		std::sort(hs.begin(), hs.end());
		lo = hs[hs.size() * 2 / 100];
		hi = hs[std::min(hs.size() - 1, hs.size() * 98 / 100)];
	}
	if (hi - lo < 1) hi = lo + 1;
	const float step = 30.0f;				// a height step that is a ledge, world units

	job.px.assign(n, 0);
	for (int y = 0; y < r.h; y++)
		for (int x = 0; x < r.w; x++)
		{
			const size_t i = (size_t)(y * r.w + x);
			const float hh = r.height[i];
			if (hh == kNoHeight) continue;			// (walls with no floor: the outsides)
			// slope light from the upper left, height brightness, ledge and edge outline
			float hn[4];
			const int nx[4] = { x - 1, x + 1, x, x }, nyy[4] = { y, y, y - 1, y + 1 };
			float drop = 0;
			for (int k = 0; k < 4; k++)
			{
				float v = hh;
				if (nx[k] >= 0 && nyy[k] >= 0 && nx[k] < r.w && nyy[k] < r.h)
				{
					const float q = r.height[(size_t)(nyy[k] * r.w + nx[k])];
					if (q != kNoHeight) v = q;
					else drop = std::max(drop, step * 2);	// the floor's outer edge
				}
				hn[k] = v;
				drop = std::max(drop, hh - v);
			}
			const float gx = (hn[1] - hn[0]) / (2 * g.s), gy = (hn[3] - hn[2]) / (2 * g.s);
			float shade = 1.0f + std::max(-0.35f, std::min(0.35f, (gx + gy) * 0.35f));
			const float t = std::max(0.0f, std::min(1.0f, (hh - lo) / (hi - lo)));
			shade *= 0.55f + 0.6f * t;
			if (drop > step) shade *= 0.35f;
			const uint32_t c = r.colour[i];
			float R = (float)((c >> 16) & 0xff) * shade, G = (float)((c >> 8) & 0xff) * shade,
				  B = (float)(c & 0xff) * shade;
			const float wl = r.wall[i];
			if (wl != kNoHeight && wl > hh + 20.0f) { R = 235; G = 236; B = 228; }	// a wall standing on it
			job.px[i] = 0xff000000u | ((uint32_t)std::min(255.0f, R) << 16)
						| ((uint32_t)std::min(255.0f, G) << 8) | (uint32_t)std::min(255.0f, B);
		}
	job.ms = SDL_GetTicks() - t0;
}

int MapWorker(void *)
{
	DrawMap(*s_job);
	SDL_AtomicSet(&s_worker_done, 1);
	return 0;
}

// The map window for Jimmy at (cx, cz) (or the level's middle) and his height (game thread).
MapGeom MapWindow(const Geo &geo, bool have_jimmy, float cx, float cy, float cz)
{
	MapGeom g;
	g.gen = s_map.gen + 1;
	const float ext = std::max(geo.maxx - geo.minx, geo.maxz - geo.minz);
	g.s = std::max(2.0f, std::min(kMapScale, ext / (float)(kMapMax - 4)));
	const float half = 0.5f * (float)kMapMax * g.s, m = 2 * g.s;
	if (!have_jimmy) { cx = 0.5f * (geo.minx + geo.maxx); cz = 0.5f * (geo.minz + geo.maxz); }
	// the level's box, cut to a window of kMapMax pixels around the centre
	g.x0 = std::max(geo.minx - m, std::min(cx - half, geo.maxx + m - 2 * half));
	g.z1 = std::min(geo.maxz + m, std::max(cz + half, geo.minz - m + 2 * half));
	g.w = std::max(1, std::min((int)kMapMax, (int)ceilf((geo.maxx + m - g.x0) / g.s)));
	g.h = std::max(1, std::min((int)kMapMax, (int)ceilf((g.z1 - (geo.minz - m)) / g.s)));
	g.cut = have_jimmy;
	g.cut_y = cy + kCutAbove;
	return g;
}

// Once per snapshot (game thread): takes a finished map, copies a new level's terrain, and
// starts a new map when the level changed, Jimmy nears the edge of the window, or his height
// changed (the cut).
void MapTick()
{
	if (s_worker && SDL_AtomicGet(&s_worker_done))
	{
		SDL_WaitThread(s_worker, NULL);
		s_worker = NULL;
		if (s_job->geo == s_geo)						// (not a replaced level's)
		{
			s_map = s_job->g;
			SDL_LockMutex(s_mutex);
			s_map_px.swap(s_job->px);
			s_map_px_gen = s_map.gen;
			s_map_px_w = s_map.w;
			s_map_px_h = s_map.h;
			SDL_UnlockMutex(s_mutex);
			SDL_Log("COMPANION map %s #%d: %dx%d at x %.0f z %.0f, %.1f units/px, cut %s%.0f, %u ms",
					g_game_file, s_map.gen, s_map.w, s_map.h, s_map.x0, s_map.z1, s_map.s,
					s_map.cut ? "" : "none ", s_map.cut ? s_map.cut_y : 0.0f, (unsigned)s_job->ms);
		}
		delete s_job;
		s_job = NULL;
		for (size_t i = 0; i < s_geo_retired.size(); i++) delete s_geo_retired[i];
		s_geo_retired.clear();
	}
	if (g_game_mode == 1) return;					// loading

	std::vector<OMedia3DShape *> shapes;
	std::vector<bool> shown;
	Terrain(shapes, shown);
	const std::string key = MapKey(shapes);
	bool want = false;
	if (key != s_map_key)
	{
		s_map_key = key;
		s_level_time = 0;
		if (s_geo) { if (s_worker) s_geo_retired.push_back(s_geo); else delete s_geo; }
		s_geo = shapes.empty() ? NULL : CopyTerrain(shapes, shown);
		const int gen = s_map.gen;
		s_map = MapGeom();
		s_map.gen = gen;
		s_map.s = 1;
		SDL_LockMutex(s_mutex);
		s_map_px.clear();
		s_map_px_w = s_map_px_h = 0;
		SDL_UnlockMutex(s_mutex);
		want = s_geo != NULL;
	}
	if (!s_geo || s_worker) return;
	float jp[4] = { 0, 0, 0, 0 };
	const bool have_jimmy = g_jimmy != NULL;
	if (have_jimmy) g_jimmy->v195(jp);
	if (!want && s_map.w > 0)
	{
		if (have_jimmy != s_map.cut) want = true;
		else if (have_jimmy)
		{
			float mx, my;
			WorldToMap(jp[0], jp[2], mx, my);
			const float edge = 0.2f * (float)kMapMax;
			const bool cut_x = s_map.w >= kMapMax, cut_y = s_map.h >= kMapMax;
			want = (cut_x && (mx < edge || mx > (float)s_map.w - edge))
				|| (cut_y && (my < edge || my > (float)s_map.h - edge))
				|| fabsf(jp[1] + kCutAbove - s_map.cut_y) > kCutMove;
		}
	}
	else if (!want && s_map.w == 0) want = true;
	if (!want) return;
	s_job = new MapJob;
	s_job->geo = s_geo;
	s_job->g = MapWindow(*s_geo, have_jimmy, jp[0], jp[1], jp[2]);
	s_job->ms = 0;
	SDL_AtomicSet(&s_worker_done, 0);
	s_worker = SDL_CreateThread(MapWorker, "companion-map", NULL);
	if (!s_worker)									// no thread: draw it here
	{
		DrawMap(*s_job);
		SDL_AtomicSet(&s_worker_done, 1);
		s_worker = NULL;
		s_map = s_job->g;
		SDL_LockMutex(s_mutex);
		s_map_px.swap(s_job->px);
		s_map_px_gen = s_map.gen;
		s_map_px_w = s_map.w;
		s_map_px_h = s_map.h;
		SDL_UnlockMutex(s_mutex);
		delete s_job;
		s_job = NULL;
	}
}

// --------------------------------------------------------------------------- icons

struct Icon { int w, h; std::vector<uint32_t> px; };
std::map<int, Icon>	s_icons;				// guarded by s_mutex (inserted by the game thread)
std::set<int>		s_icons_tried;			// game thread
std::vector<int>	s_icons_new;			// game thread: loaded since the last dump

std::set<int>		s_icons_real;			// game thread: loaded and bigger than a placeholder
// Icons drawn by the game's own objects (their sprite canvas), keyed above the screens.omt ids.
enum { kIconNeutron = 0x10000, kIconObjective, kIconCheckpoint, kIconExit, kIconGadget = 0x10100 };

// Copies a canvas for the panel once under `key`; true when it is a real picture (the menu pages
// use 2 x 2 canvases for items with no picture).
bool IconFromCanvas(int key, OMediaCanvas *c)
{
	if (s_icons_tried.count(key)) return s_icons_real.count(key) != 0;
	if (!c) return false;
	s_icons_tried.insert(key);
	Icon ic;
	c->lock(omlf_Read);
	const omt_RGBAPixel *p = c->get_pixels();
	ic.w = c->get_width();
	ic.h = c->get_height();
	if (p && ic.w > 0 && ic.h > 0)
	{
		ic.px.resize((size_t)(ic.w * ic.h));
		for (size_t i = 0; i < ic.px.size(); i++)
		{
			const omt_RGBAPixel q = p[i];				// bytes R, G, B, A -> ARGB
			ic.px[i] = (q & 0xff000000u) | ((q & 0xff) << 16) | (q & 0xff00) | ((q >> 16) & 0xff);
		}
	}
	c->unlock();
	if (ic.px.empty() || ic.w < 8 || ic.h < 8) return false;
	s_icons_real.insert(key);
	s_icons_new.push_back(key);
	SDL_LockMutex(s_mutex);
	s_icons[key] = ic;
	SDL_UnlockMutex(s_mutex);
	return true;
}

// Canvas `id` of screens.omt (the menu pages' pictures).
bool NeedIcon(int id)
{
	if (id < 0 || !g_screens_db) return false;
	if (s_icons_tried.count(id)) return s_icons_real.count(id) != 0;
	OMediaCanvas *c = LoadCanvasID(g_screens_db, id);
	if (!c) { s_icons_tried.insert(id); return false; }
	return IconFromCanvas(id, c);
}

// The sprite canvas an object shows (C3DSprite is an OMediaCanvasElement), once under `key`.
int ObjectIcon(int key, CGameObject *o, OMediaCanvas *c = NULL)
{
	if (!c && !s_icons_tried.count(key))
		if (OMediaCanvasElement *e = dynamic_cast<OMediaCanvasElement *>(o)) c = e->get_canvas();
	return IconFromCanvas(key, c) ? key : -1;
}

// --------------------------------------------------------------------------- tiny JSON writer
void Str(std::string &o, const char *s)
{
	o += '"';
	for (; s && *s; s++)
	{
		unsigned char c = (unsigned char)*s;
		if (c == '"' || c == '\\') { o += '\\'; o += (char)c; }
		else if (c == '\n') o += "\\n";
		else if (c < 0x20) o += ' ';
		else if (c < 0x80) o += (char)c;
		else o += '?';						// the game's texts are Latin-1; keep the JSON plain ASCII
	}
	o += '"';
}
void Key(std::string &o, const char *k) { if (o.size() > 1) o += ','; Str(o, k); o += ':'; }
void Num(std::string &o, double v)
{
	char b[32];
	snprintf(b, sizeof b, "%.6g", v);
	o += b;
}
void Num1(std::string &o, double v)			// map pixels: one decimal is plenty
{
	char b[32];
	snprintf(b, sizeof b, "%.1f", v);
	o += b;
}
void Int(std::string &o, long v)
{
	char b[24];
	snprintf(b, sizeof b, "%ld", v);
	o += b;
}

void FourCC(char *out, int id)
{
	for (int i = 0; i < 4; i++)
	{
		char c = (char)(id >> (24 - 8 * i));
		out[i] = (c >= 32 && c < 127) ? c : '?';
	}
	out[4] = 0;
}

bool Visible(CGameObject *o)
{
	OMediaElement *e = dynamic_cast<OMediaElement *>(o);
	return e && e->getvisible();
}

void Position(CGameObject *o, float &x, float &z)
{
	float p[4];
	o->v195(p);
	x = p[0];
	z = p[2];
}

// A pickup is collected when its savegame object state is set (CPickupType::v260 0x496de0 hides
// it then); one without a state slot (PickupIndex 0) counts as collected while hidden.
bool Collected(CPickupType *p)
{
	if (p->m_pickup_index > 0 && p->m_pickup_index < 4200) return g_object_states[p->m_pickup_index] != 0;
	return !Visible(p);
}

// The canvas id of item `item` of menu page `page` (-1 = none).
int ItemIcon(int page, int item)
{
	C2DInGameMenu::MenuPage *p = g_menu ? C2DInGameMenu::Page(page) : NULL;
	if (!p || item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return -1;
	return p->items[item].active_id;
}

// CLoadLevel's own test (0x48e9f0) of whether its task lets Jimmy through (GoIfActive included).
bool ExitOpen(CLoadLevel *l)
{
	if (l->m_go_if_active != -1)
	{
		C2DInGameMenu::MenuPage *p = g_menu ? C2DInGameMenu::Page(3) : NULL;
		if (!p || l->m_go_if_active < 0 || l->m_go_if_active >= C2DInGameMenu::MENU_ITEMS
			|| !p->items[l->m_go_if_active].type) return false;
	}
	if (strcasecmp(l->m_required_task, "none") == 0) return true;
	const int state = GetTaskState(l->m_required_task);
	if (state == -1) return true;
	if (state < l->m_required_level) return false;
	if (l->m_exact_level != -1 && l->m_exact_level != state) return false;
	return true;
}

// The caption: the TRIGTEXTSTRING lines of the current group typed so far (C2DInGameMenu::v340).
void Caption(std::string &out)
{
	if (!g_menu || !g_trig_text_group[0]) return;
	for (std::list<C2DInGameMenu::MenuObject *>::iterator i = C2DInGameMenu::s_objects.begin();
		 i != C2DInGameMenu::s_objects.end(); ++i)
	{
		C2DInGameMenu::MenuObject *m = *i;
		if (!m || m->type != C2DInGameMenu::MOBJ_TRIGTEXTSTRING) continue;
		if (strcasecmp(m->trigger, g_trig_text_group) != 0) continue;
		if (m->id > g_trig_text_line) continue;
		if (!out.empty()) out += '\n';
		out += m->text;
	}
}

// A map point: [kind, x, y, extra] (extra: an icon id, a label, or nothing).
void Point(std::string &pts, char kind, CGameObject *g, int icon, const char *label)
{
	float x, z, mx, my;
	Position(g, x, z);
	WorldToMap(x, z, mx, my);
	if (pts.size() > 1) pts += ',';
	pts += "[\""; pts += kind; pts += "\","; Num1(pts, mx); pts += ','; Num1(pts, my);
	if (label) { pts += ','; Str(pts, label); }
	else if (icon >= 0) { pts += ','; Int(pts, icon); }
	pts += ']';
}

void Build(std::string &o)
{
	MapTick();

	o = "{";
	Key(o, "t"); Num(o, s_clock);
	Key(o, "level"); Str(o, g_game_file);
	if (g_gametype)
	{
		char id[5];
		FourCC(id, static_cast<CGameObject *>(g_gametype)->m_object_id);
		Key(o, "id"); Str(o, id);
	}
	Key(o, "mode"); Int(o, g_game_mode);
	Key(o, "cut"); Int(o, g_game_mode == 3 ? 1 : 0);
	Key(o, "energy"); Num(o, g_health);
	Key(o, "energy_max"); Int(o, 100);
	// the HUD's neutron energy picture: screens.omt canvas 0x17, placed at (32, 0) beside the
	// energy bar by the C2DInGameMenu constructor (0x401380)
	Key(o, "energy_icon"); Int(o, NeedIcon(0x17) ? 0x17 : -1);
	Key(o, "score"); Int(o, g_score);

	// the current gadget (C3DJimmy::ActivateMenuItem, 0 = none) with its HUD picture: the
	// gadget sprites g_menu_item_sprites[gadget] the C2DInGameMenu constructor sets up (screens.omt
	// 80 x 70 canvases), and its charge: the gadget bar's height while it shows (0x404d60 sizes it
	// to count * 1.04 / 480 of the screen; red when empty)
	Key(o, "gadget"); Int(o, g_cur_gadget);
	if (g_menu && g_cur_gadget > 0 && g_cur_gadget < 14 && g_menu_item_sprites[g_cur_gadget])
		{ Key(o, "gadget_icon"); Int(o, ObjectIcon(kIconGadget + g_cur_gadget, NULL, g_menu_item_sprites[g_cur_gadget]->get_canvas())); }
	if (g_menu && g_gadget_bar && g_gadget_bar_on && g_screen_h > 0)
	{
		const float unit = (float)(short)g_screen_h * 0.00208333344f * 1.04f;
		Key(o, "gadget_charge"); Num(o, unit > 0 ? g_gadget_bar->get_height() / unit : 0);
	}
	// time in this level (game time while playing)
	Key(o, "time"); Num(o, s_level_time);

	// the inventory sheet (the pickups page Jimmy opens: page 2, built by CreateMenuPages): every
	// item with a picture, how many Jimmy has (g_pickup_counts), whether he has it (the item's
	// menu state) and where the page puts it (640 x 480 page coordinates)
	if (g_menu)
	{
		C2DInGameMenu::MenuPage *pk = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
		Key(o, "sheet"); o += '[';
		bool first = true;
		for (int i = 0; pk && i < C2DInGameMenu::MENU_ITEMS; i++)
		{
			const int icon = pk->items[i].active_id;
			if (!NeedIcon(icon)) continue;
			if (!first) o += ',';
			first = false;
			o += '['; Int(o, i); o += ','; Int(o, g_pickup_counts[i * 2]); o += ','; Int(o, icon);
			o += ','; Int(o, pk->items[i].type ? 1 : 0); o += ','; Int(o, (long)pk->items[i].x);
			o += ','; Int(o, (long)pk->items[i].y); o += ']';
		}
		o += ']';
	}

	std::string cap;
	Caption(cap);
	if (!cap.empty()) { Key(o, "caption"); Str(o, cap.c_str()); }

	// objects: counts, the boss, map points
	int pk_got = 0, pk_all = 0, ne_got = 0, ne_all = 0, ck_got = 0, ck_all = 0;
	int s_icon_neutron = -1;
	std::map<int, std::pair<int, int> > kinds;		// the level's pickups by menu item: collected, all
	std::string pts = "[";
	C3DHerman *boss = NULL;
	CGameObject *jimmy = g_jimmy;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *g = *i;
		if (!g || g == jimmy) continue;
		if (C3DPickupItem *p = dynamic_cast<C3DPickupItem *>(g))
		{
			if (p->m_is_ambient) continue;
			pk_all++;
			std::pair<int, int> &k = kinds[p->m_pic_number];
			k.second++;
			if (Collected(p)) { pk_got++; k.first++; continue; }
			if (!Visible(g)) continue;					// not there yet (its task holds it back)
			int icon = p->m_pic_number >= 0 ? ItemIcon(C2DInGameMenu::MENU_PICKUPS, p->m_pic_number) : -1;
			if (!NeedIcon(icon)) icon = -1;
			Point(pts, 'p', g, icon, NULL);
		}
		else if (C3DRedNeutron *rn = dynamic_cast<C3DRedNeutron *>(g))
		{
			std::pair<int, int> &k = kinds[46];			// its menu pickup item (C3DRedNeutron.h)
			k.second++;
			if (Collected(rn)) { k.first++; continue; }
			if (!Visible(g)) continue;
			int icon = ItemIcon(C2DInGameMenu::MENU_PICKUPS, 46);
			Point(pts, 'r', g, NeedIcon(icon) ? icon : -1, NULL);
		}
		else if (C3DNeutron *ne = dynamic_cast<C3DNeutron *>(g))
		{
			ne_all++;
			// its first spin frame (sprites.omt canvas 0) is the neutron's picture
			if (s_icon_neutron < 0) s_icon_neutron = ObjectIcon(kIconNeutron, g, ne->m_frames[0]);
			if (Visible(g)) Point(pts, 'n', g, s_icon_neutron, NULL);
			else ne_got++;
		}
		else if (C3DCheckPoint *c = dynamic_cast<C3DCheckPoint *>(g))
		{
			ck_all++;
			if (c->m_checked) ck_got++;
			else Point(pts, 'c', g, ObjectIcon(kIconCheckpoint, g), NULL);
		}
		else if (C3DHerman *h = dynamic_cast<C3DHerman *>(g))
		{
			boss = h;
			if (Visible(g)) Point(pts, 'e', g, -1, NULL);
		}
		else if (dynamic_cast<C3DGoddard *>(g))
		{
			if (Visible(g)) Point(pts, 'g', g, -1, NULL);
		}
		else if (dynamic_cast<C3DFriends *>(g))
		{
			if (Visible(g)) Point(pts, 'f', g, -1, NULL);
		}
		else if (dynamic_cast<C3DAI *>(g))
		{
			if (Visible(g)) Point(pts, 'e', g, -1, NULL);
		}
		else if (dynamic_cast<C3DArrow *>(g))
		{
			if (Visible(g)) Point(pts, 'o', g, ObjectIcon(kIconObjective, g), NULL);
		}
		else if (dynamic_cast<C3DSwingDoor *>(g))
		{
			Point(pts, 'd', g, -1, NULL);
		}
		else if (CLoadLevel *l = dynamic_cast<CLoadLevel *>(g))
		{
			if (strcasecmp(l->m_level_name, "none") == 0) continue;
			Point(pts, ExitOpen(l) ? 'x' : 'X', g, -1, l->m_level_name);
		}
	}
	pts += ']';
	Key(o, "pickups"); o += '['; Int(o, pk_got); o += ','; Int(o, pk_all); o += ']';
	Key(o, "neutrons"); o += '['; Int(o, ne_got); o += ','; Int(o, ne_all); o += ','; Int(o, s_icon_neutron); o += ']';
	Key(o, "checkpoints"); o += '['; Int(o, ck_got); o += ','; Int(o, ck_all); o += ','; Int(o, ObjectIcon(kIconCheckpoint, NULL)); o += ']';
	Key(o, "collect"); o += '[';
	for (std::map<int, std::pair<int, int> >::iterator k = kinds.begin(); k != kinds.end(); ++k)
	{
		int icon = k->first >= 0 ? ItemIcon(C2DInGameMenu::MENU_PICKUPS, k->first) : -1;
		if (!NeedIcon(icon)) icon = -1;
		if (k != kinds.begin()) o += ',';
		o += '['; Int(o, k->first); o += ','; Int(o, k->second.first); o += ','; Int(o, k->second.second);
		o += ','; Int(o, icon); o += ']';
	}
	o += ']';
	if (boss)
	{
		Key(o, "boss"); o += '{';
		Str(o, "name"); o += ':'; Str(o, "Herman");
		o += ','; Str(o, "hp"); o += ':'; Num(o, boss->f_1d58);
		o += ','; Str(o, "bar"); o += ':'; Int(o, boss->f_1d60 > 0 ? 1 : 0);
		o += '}';
	}
	VoxCard cards[8];										// the Vox world picker (port/VoxPicker.h)
	int sel = -1;
	const int ncards = VoxPickerCards(cards, 8, &sel);
	if (ncards > 0)
	{
		Key(o, "picker"); o += '{';
		Str(o, "sel"); o += ':'; Int(o, sel);
		o += ','; Str(o, "cards"); o += ":[";
		for (int k = 0; k < ncards; k++)
		{
			if (k) o += ',';
			o += '['; Str(o, cards[k].name);
			o += ','; Int(o, NeedIcon(cards[k].wire) ? cards[k].wire : -1);
			o += ','; Int(o, NeedIcon(cards[k].colour) ? cards[k].colour : -1);
			o += ']';
		}
		o += "]}";
	}
	Key(o, "map"); o += '{';
	Str(o, "gen"); o += ':'; Int(o, s_map.w > 0 ? s_map.gen : 0);
	o += ','; Str(o, "w"); o += ':'; Int(o, s_map.w);
	o += ','; Str(o, "h"); o += ':'; Int(o, s_map.h);
	o += ','; Str(o, "s"); o += ':'; Num(o, s_map.s);
	o += '}';
	if (jimmy)
	{
		float p[4], mx, my, fx, fy;
		jimmy->v195(p);
		WorldToMap(p[0], p[2], mx, my);
		const OMedia3DVector f = jimmy->v226(0, 0, 1.0f);		// his local +z: the way he faces
		WorldToMap(p[0] + f.x, p[2] + f.z, fx, fy);
		fx -= mx; fy -= my;
		const float len = sqrtf(fx * fx + fy * fy);
		if (len > 1e-6f) { fx /= len; fy /= len; } else { fx = 0; fy = -1; }
		Key(o, "jimmy"); o += '['; Num1(o, mx); o += ','; Num1(o, my); o += ','; Num(o, fx); o += ',';
		Num(o, fy); o += ','; Num(o, p[1]); o += ']';
	}
	Key(o, "pts"); o += pts;
	o += '}';
}

// --------------------------------------------------------------------------- desktop dump

void WritePng(const std::string &file, int w, int h, const uint32_t *argb)
{
	if (w <= 0 || h <= 0) return;
	OMediaCanvas c;
	c.create(w, h);
	c.lock(omlf_Write);
	omt_RGBAPixel *d = c.get_pixels();
	if (d)
		for (size_t i = 0; i < (size_t)(w * h); i++)
		{
			const uint32_t q = argb[i];						// ARGB -> bytes R, G, B, A
			d[i] = (q & 0xff000000u) | ((q >> 16) & 0xff) | (q & 0xff00) | ((q & 0xff) << 16);
		}
	c.unlock();
	try
	{
		OMediaFilePath path(file.c_str());
		OMediaFileStream f;
		f.setpath(&path);
		f.open(omcfp_Write, true);
		c.png_export(f);
		f.close();
	}
	catch (...)
	{
		SDL_Log("COMPANION: cannot write %s", file.c_str());
	}
}

std::string LevelStem()
{
	std::string s = g_game_file;
	size_t dot = s.rfind('.');
	if (dot != std::string::npos) s.erase(dot);
	return s.empty() ? std::string("none") : s;
}

// The overlay: the map with the snapshot's points (squares) and Jimmy (a ring and a heading line).
void Dot(std::vector<uint32_t> &img, int w, int h, float x, float y, int r, uint32_t col)
{
	for (int dy = -r; dy <= r; dy++)
		for (int dx = -r; dx <= r; dx++)
		{
			const int px = (int)x + dx, py = (int)y + dy;
			if (px >= 0 && py >= 0 && px < w && py < h) img[(size_t)(py * w + px)] = col;
		}
}

void Dump(const std::string &snap)
{
	const std::string stem = s_dump_dir + "/" + LevelStem();
	static int s_dumped_gen = 0;
	std::vector<uint32_t> map;
	SDL_LockMutex(s_mutex);
	map = s_map_px;
	SDL_UnlockMutex(s_mutex);
	const int w = s_map.w, h = s_map.h;
	if (s_map.gen != s_dumped_gen && !map.empty())
	{
		s_dumped_gen = s_map.gen;
		WritePng(stem + "_map.png", w, h, map.data());
	}
	if (FILE *f = fopen((stem + "_snap.json").c_str(), "w")) { fputs(snap.c_str(), f); fclose(f); }
	for (size_t i = 0; i < s_icons_new.size(); i++)
	{
		const int id = s_icons_new[i];
		char b[32];
		snprintf(b, sizeof b, "/icon_%d.png", id);
		SDL_LockMutex(s_mutex);
		Icon ic = s_icons[id];
		SDL_UnlockMutex(s_mutex);
		WritePng(s_dump_dir + b, ic.w, ic.h, ic.px.data());
	}
	s_icons_new.clear();
	if (map.empty()) return;

	// points: parse the few fields back out of our own JSON
	const size_t at = snap.find("\"pts\":[");
	if (at != std::string::npos)
	{
		const char *p = snap.c_str() + at + 7;
		while (*p == '[' || *p == ',')
		{
			char kind = 0; float x = 0, y = 0;
			if (sscanf(p, "[\"%c\",%f,%f", &kind, &x, &y) == 3)
			{
				uint32_t col = 0xffffffffu;
				int r = 3;
				switch (kind)
				{
					case 'p': col = 0xffffd246u; r = 4; break;
					case 'n': col = 0xff50c8ffu; break;
					case 'c': col = 0xff78e678u; break;
					case 'f': col = 0xffc896ffu; break;
					case 'e': col = 0xfff05a50u; break;
					case 'o': col = 0xffff8000u; r = 5; break;
					case 'd': col = 0xff8b5a2bu; break;
					case 'x': col = 0xff00ffffu; r = 5; break;
					case 'X': col = 0xff008080u; r = 5; break;
				}
				Dot(map, w, h, x, y, r + 1, 0xff000000u);
				Dot(map, w, h, x, y, r, col);
			}
			const char *e = strchr(p, ']');
			if (!e) break;
			p = e + 1;
		}
	}
	const size_t j = snap.find("\"jimmy\":[");
	float jx, jy, hx, hy;
	if (j != std::string::npos && sscanf(snap.c_str() + j + 9, "%f,%f,%f,%f", &jx, &jy, &hx, &hy) == 4)
	{
		for (int k = 0; k <= 30; k++) Dot(map, w, h, jx + hx * k, jy + hy * k, 1, 0xffffffffu);
		Dot(map, w, h, jx, jy, 5, 0xff000000u);
		Dot(map, w, h, jx, jy, 3, 0xffffffffu);
	}
	WritePng(stem + "_overlay.png", w, h, map.data());
}

} // namespace

void neg::CompanionSetEnabled(bool on)
{
	SDL_AtomicSet(&s_enabled, on ? 1 : 0);
}

int neg::CompanionCopy(char *buf, int size)
{
	if (!s_mutex) return 0;
	SDL_LockMutex(s_mutex);
	int n = (int)s_snapshot.size();
	if (buf && size > 0)
	{
		if (n > size - 1) n = size - 1;
		memcpy(buf, s_snapshot.data(), (size_t)n);
		buf[n] = 0;
	}
	SDL_UnlockMutex(s_mutex);
	return n;
}

void neg::CompanionTick(float dt)
{
	LauncherArtTick();							// port: the launcher's pictures (port/LauncherArt.h)
	if (!s_env_read)
	{
		s_env_read = true;
		const char *e = getenv("NEG_COMPANION_LOG");
		s_log = e && *e && *e != '0';
		const char *d = getenv("NEG_COMPANION_DUMP");
		if (d && *d) s_dump_dir = d;
		if (s_log || !s_dump_dir.empty()) SDL_AtomicSet(&s_enabled, 1);
	}
	if (!SDL_AtomicGet(&s_enabled) || !s_mutex) return;
	if (dt > 0 && dt < 1.0f) s_clock += dt;
	if (dt > 0 && dt < 1.0f && g_game_mode == 5) s_level_time += dt;
	s_since_build += dt;
	if (s_since_build < 0.1f) return;			// at most 10 a second
	s_since_build = 0;
	std::string snap;
	Build(snap);
	if (s_log)
	{
		s_since_log += 0.1f;
		if (s_since_log >= 1.0f)
		{
			s_since_log = 0;
			SDL_Log("COMPANION %s", snap.c_str());
		}
	}
	if (!s_dump_dir.empty())
	{
		s_since_dump += 0.1f;
		if (s_since_dump >= 2.0f)
		{
			s_since_dump = 0;
			Dump(snap);
		}
	}
	SDL_LockMutex(s_mutex);
	s_snapshot.swap(snap);
	SDL_UnlockMutex(s_mutex);
}

// --------------------------------------------------------------------------- JNI (Android)

#ifdef __ANDROID__
#include <jni.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSnapshot(JNIEnv *env, jclass)
{
	std::string copy;
	if (s_mutex)
	{
		SDL_LockMutex(s_mutex);
		copy = s_snapshot;
		SDL_UnlockMutex(s_mutex);
	}
	return env->NewStringUTF(copy.c_str());		// ASCII only (Str), so modified UTF-8 is fine
}

extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetEnabled(JNIEnv *, jclass, jboolean on)
{
	neg::CompanionSetEnabled(on != 0);
}

// The map image: [gen, w, h, ARGB pixels...], or null when there is none.
extern "C" JNIEXPORT jintArray JNICALL
Java_com_sirbilliams_negatron_Companion_nativeMap(JNIEnv *env, jclass)
{
	if (!s_mutex) return NULL;
	jintArray out = NULL;
	SDL_LockMutex(s_mutex);
	const int w = s_map_px_w, h = s_map_px_h;
	if (!s_map_px.empty() && (size_t)(w * h) == s_map_px.size())
	{
		out = env->NewIntArray(3 + w * h);
		if (out)
		{
			const jint head[3] = { s_map_px_gen, w, h };
			env->SetIntArrayRegion(out, 0, 3, head);
			env->SetIntArrayRegion(out, 3, w * h, (const jint *)s_map_px.data());
		}
	}
	SDL_UnlockMutex(s_mutex);
	return out;
}

// An icon (a screens.omt canvas the snapshot named): [w, h, ARGB pixels...], or null.
extern "C" JNIEXPORT jintArray JNICALL
Java_com_sirbilliams_negatron_Companion_nativeIcon(JNIEnv *env, jclass, jint id)
{
	if (!s_mutex) return NULL;
	jintArray out = NULL;
	SDL_LockMutex(s_mutex);
	std::map<int, Icon>::iterator i = s_icons.find((int)id);
	if (i != s_icons.end() && !i->second.px.empty())
	{
		const int n = i->second.w * i->second.h;
		out = env->NewIntArray(2 + n);
		if (out)
		{
			const jint head[2] = { i->second.w, i->second.h };
			env->SetIntArrayRegion(out, 0, 2, head);
			env->SetIntArrayRegion(out, 2, n, (const jint *)i->second.px.data());
		}
	}
	SDL_UnlockMutex(s_mutex);
	return out;
}
#endif
