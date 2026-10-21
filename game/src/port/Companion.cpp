/*  port: the dual-screen companion - see Companion.h. Reads game state only. */
#include "Companion.h"
#include "../C3DJimmy.h"
#include "../C3DAI.h"
#include "../C2DInGameMenu.h"
#include "../CGameType.h"
#include "../Game.h"
#include "../Globals.h"
#include "../SaveGame.h"
#include "../InterfaceElement.h"
#include "../gen/globals.h"
#include "../friends/C3DFriends.h"
#include "../lab/C3DHerman.h"
#include "../pickups/C3DPickupItem.h"
#include "../objects/C3DNeutron.h"
#include "../objects/C3DCheckPoint.h"
#include "OMediaElement.h"
#include <SDL.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace neg;

namespace {

SDL_atomic_t		s_enabled;				// 0 / 1 (zero-initialised)
bool				s_log = false;
bool				s_env_read = false;
float				s_clock = 0;			// since the first tick
float				s_since_build = 1.0f;
float				s_since_log = 0;
SDL_mutex			*s_mutex = SDL_CreateMutex();	// (created at load: SDL mutexes need no SDL_Init)
std::string			s_snapshot;				// guarded by s_mutex

// --- tiny JSON writer
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

const char *MenuItemName(int page, int id)
{
	for (std::list<C2DInGameMenu::MenuObject *>::iterator i = C2DInGameMenu::s_objects.begin();
		 i != C2DInGameMenu::s_objects.end(); ++i)
	{
		C2DInGameMenu::MenuObject *m = *i;
		if (m && m->type == C2DInGameMenu::MOBJ_MENUITEM && m->page == page && m->id == id) return m->name;
	}
	return NULL;
}

void Build(std::string &o)
{
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
	Key(o, "lives"); Int(o, g_lives);
	Key(o, "score"); Int(o, g_score);

	// gadgets: the inventory page (menu.dat MENUITEMs of INV_MENU)
	Key(o, "gadget"); Int(o, g_cur_gadget);
	if (g_menu)
	{
		const char *n = MenuItemName(C2DInGameMenu::MENU_INV, g_cur_gadget);
		if (n && g_cur_gadget) { Key(o, "gadget_name"); Str(o, n); }
		C2DInGameMenu::MenuPage *inv = C2DInGameMenu::Page(C2DInGameMenu::MENU_INV);
		Key(o, "gadgets"); o += '[';
		bool first = true;
		for (std::list<C2DInGameMenu::MenuObject *>::iterator i = C2DInGameMenu::s_objects.begin();
			 i != C2DInGameMenu::s_objects.end(); ++i)
		{
			C2DInGameMenu::MenuObject *m = *i;
			if (!m || m->type != C2DInGameMenu::MOBJ_MENUITEM || m->page != C2DInGameMenu::MENU_INV) continue;
			if (m->id < 0 || m->id >= C2DInGameMenu::MENU_ITEMS) continue;
			if (!first) o += ',';
			first = false;
			o += '['; Int(o, m->id); o += ','; Str(o, m->name); o += ',';
			Int(o, inv ? inv->items[m->id].type : -1); o += ']';
		}
		o += ']';
	}

	Key(o, "tasks"); o += '[';
	{
		bool first = true;
		for (std::list<Task *>::iterator i = g_tasks.begin(); i != g_tasks.end(); ++i)
		{
			if (!*i) continue;
			if (!first) o += ',';
			first = false;
			o += '['; Str(o, (*i)->name); o += ','; Int(o, (*i)->state); o += ']';
		}
	}
	o += ']';

	std::string cap;
	Caption(cap);
	if (!cap.empty()) { Key(o, "caption"); Str(o, cap.c_str()); }

	// objects: counts, the boss, map points
	int pk_got = 0, pk_all = 0, ne_got = 0, ne_all = 0, ck_got = 0, ck_all = 0;
	std::string pts = "[";
	C3DHerman *boss = NULL;
	CGameObject *jimmy = g_jimmy;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CGameObject *g = *i;
		if (!g || g == jimmy) continue;
		char kind = 0;
		if (C3DPickupItem *p = dynamic_cast<C3DPickupItem *>(g))
		{
			if (p->m_is_ambient) continue;
			pk_all++;
			if (Visible(g)) kind = 'p';
			else pk_got++;
		}
		else if (dynamic_cast<C3DNeutron *>(g))
		{
			ne_all++;
			if (Visible(g)) kind = 'n';
			else ne_got++;
		}
		else if (C3DCheckPoint *c = dynamic_cast<C3DCheckPoint *>(g))
		{
			ck_all++;
			if (c->m_checked) ck_got++;
			else kind = 'c';
		}
		else if (C3DHerman *h = dynamic_cast<C3DHerman *>(g))
		{
			boss = h;
			if (Visible(g)) kind = 'e';
		}
		else if (dynamic_cast<C3DFriends *>(g))
		{
			if (Visible(g)) kind = 'f';
		}
		else if (dynamic_cast<C3DAI *>(g))
		{
			if (Visible(g)) kind = 'e';
		}
		if (!kind) continue;
		float x, z;
		Position(g, x, z);
		if (pts.size() > 1) pts += ',';
		pts += "[\""; pts += kind; pts += "\","; Num(pts, x); pts += ','; Num(pts, z); pts += ']';
	}
	pts += ']';
	Key(o, "pickups"); o += '['; Int(o, pk_got); o += ','; Int(o, pk_all); o += ']';
	Key(o, "neutrons"); o += '['; Int(o, ne_got); o += ','; Int(o, ne_all); o += ']';
	Key(o, "checkpoints"); o += '['; Int(o, ck_got); o += ','; Int(o, ck_all); o += ']';
	if (boss)
	{
		Key(o, "boss"); o += '{';
		Str(o, "name"); o += ':'; Str(o, "Herman");
		o += ','; Str(o, "hp"); o += ':'; Num(o, boss->f_1d58);
		o += ','; Str(o, "bar"); o += ':'; Int(o, boss->f_1d60 > 0 ? 1 : 0);
		o += '}';
	}
	if (jimmy)
	{
		float p[4], r[4];
		jimmy->v195(p);
		jimmy->v201(r);
		Key(o, "jimmy"); o += '['; Num(o, p[0]); o += ','; Num(o, p[2]); o += ','; Num(o, r[1]); o += ']';
	}
	Key(o, "pts"); o += pts;
	o += '}';
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
	if (!s_env_read)
	{
		s_env_read = true;
		const char *e = getenv("NEG_COMPANION_LOG");
		s_log = e && *e && *e != '0';
		if (s_log) SDL_AtomicSet(&s_enabled, 1);
	}
	if (!SDL_AtomicGet(&s_enabled)) return;
	if (dt > 0 && dt < 1.0f) s_clock += dt;
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
	if (!s_mutex) return;
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
#endif
