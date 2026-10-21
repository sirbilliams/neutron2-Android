/*  port: NEG_START and save\slots.txt - see StartMode.h. Not in the original.             */
#include "StartMode.h"
#include "../C2DInGameMenu.h"
#include "../CGameType.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include <SDL.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <string>

std::string omf_SDLResolvePath(const std::string &in);		// engine/sdl: DOS path -> real file

namespace {
enum Mode { MODE_MENU, MODE_ORIGINAL, MODE_NEW, MODE_CONTINUE, MODE_LOAD };
Mode		s_mode = MODE_MENU;
bool		s_menu_pending = false;		// open the Main Menu once Jimmy plays
int			s_loads = 0;				// levels loaded since start-up
char		s_task[32] = "autosave.tsk";

// The header of save\<name> (the fields ReadTaskFile 0x497b10 reads), without touching the
// game's globals. False for a missing file or a DUMMY placeholder.
bool ReadHeader(const char *name, int &level, char *gam, int &score)
{
	char path[64];
	snprintf(path, sizeof path, "save\\%s", name);
	OMediaFilePath fp(path, 'null', 'null');
	OMediaFileStream fs;
	fs.setpath(&fp);
	if (!fs.fileexists()) { fs.close(); return false; }
	fs.open(omcfp_Read, false, false);
	if (!fs.isopen()) return false;
	char version[256], task[256];
	float health;
	short lives;
	fs >> version;
	if (strcasecmp(version, "DUMMY   ") == 0) { fs.close(); return false; }
	fs >> level;
	fs >> gam;
	fs >> task;
	fs >> health;
	fs >> lives;
	fs >> score;
	fs.close();
	return true;
}
}

const char *neg::StartModeChoose(char *level_name)
{
	const char *s = getenv("NEG_START");
	s_mode = MODE_MENU;
	strcpy(s_task, "autosave.tsk");
	if (s && *s)
	{
		if (!strcasecmp(s, "original")) s_mode = MODE_ORIGINAL;
		else if (!strcasecmp(s, "new")) s_mode = MODE_NEW;
		else if (!strcasecmp(s, "continue")) s_mode = MODE_CONTINUE;
		else if (!strncasecmp(s, "load:", 5))
		{
			int n = atoi(s + 5);
			if (n >= 1 && n <= 10)
			{
				s_mode = MODE_LOAD;
				snprintf(s_task, sizeof s_task, "JimmyGame%d.tsk", n);
			}
		}
	}
	WriteSaveSlotList();
	switch (s_mode)
	{
		case MODE_NEW:		strcpy(level_name, "level1d.gam"); break;
		case MODE_CONTINUE:	strcpy(level_name, "autosave"); break;
		case MODE_LOAD:
		{
			int level, score;
			char gam[256];
			if (ReadHeader(s_task, level, gam, score)) strcpy(level_name, "autosave");
			else strcpy(s_task, "autosave.tsk");		// an empty slot: the original start
			break;
		}
		default:			break;						// menu / original: StartLevel.txt's choice
	}
	// The Main Menu only over a continued game (a new game opens with the title / logos).
	int level, score;
	char gam[256];
	// NEG_TEST_PLAY (the level picker) replaces the level later in CEditor: no menu then.
	const char *test = getenv("NEG_TEST_PLAY");
	s_menu_pending = s_mode == MODE_MENU && !(test && *test) && strcasecmp(level_name, "autosave") == 0
					 && ReadHeader(s_task, level, gam, score);
	SDL_Log("NEG_START: %s -> %s%s%s", s && *s ? s : "(menu)", level_name,
			strcasecmp(level_name, "autosave") == 0 ? " " : "",
			strcasecmp(level_name, "autosave") == 0 ? s_task : "");
	return s_task;
}

void neg::StartModeLevelLoaded(void)
{
	// The first load is the start-up level; a later one (the autosave's level, when the editor's
	// default level ran first) still belongs to the start. Give up after a few.
	if (++s_loads > 3) s_menu_pending = false;
}

void neg::StartModeFrame(void)
{
	if (!s_menu_pending || !s_loads) return;
	if (!g_menu || !g_gametype || g_game_mode != 5 || g_5324f8 != 3) return;
	// What ESC does in HandleEscape (0x407240): pause, the PDA zooms in, and after a second
	// the Main Menu (page 3) is activated.
	s_menu_pending = false;
	g_gametype->PauseGame(true);
	g_menu->v345(1);
	g_menu->f_4e0 = 1.0f;
	g_5324f8 = 0;
	SDL_Log("NEG_START: Main Menu opened");
}

void neg::WriteSaveSlotList(void)
{
	FILE *f = fopen(omf_SDLResolvePath("save\\slots.txt").c_str(), "w");
	if (!f) return;
	fputs("# slot\tlevel id\tlevel name\tgam file\tscore\tmtime (written by the game, see game/src/port/StartMode.h)\n", f);
	for (int k = 0; k <= 10; k++)
	{
		char name[32];
		if (k == 0) strcpy(name, "autosave.tsk");
		else snprintf(name, sizeof name, "JimmyGame%d.tsk", k);
		int level = 0, score = 0;
		char gam[256];
		if (!ReadHeader(name, level, gam, score)) continue;
		char path[64];
		snprintf(path, sizeof path, "save\\%s", name);
		struct stat st;
		long mtime = stat(omf_SDLResolvePath(path).c_str(), &st) == 0 ? (long)st.st_mtime : 0;
		char id[5] = { (char)(level >> 24), (char)(level >> 16), (char)(level >> 8), (char)level, 0 };
		const char *lname = SaveLevelName(level);
		if (k == 0) fprintf(f, "autosave");
		else fprintf(f, "%d", k);
		fprintf(f, "\t%s\t%s\t%s\t%d\t%ld\n", id, lname ? lname : "", gam, score, mtime);
	}
	fclose(f);
}
