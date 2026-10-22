/*  port: the Android launcher's pictures - see LauncherArt.h. Not in the original.

    The launcher (android LauncherView) shows a picture for each menu option. They are the game's
    own full-screen pictures (screens.omt canvases, 640 x 480), read from the player's installed
    files here and written as PNGs into the app's files folder (NEG_LAUNCHER_ART, set by
    NegatronActivity), so the launcher can show them from its next start. Nothing of the game's art
    is in the repository. Canvases (screens.omt, by id, as `neg_hdtool list` shows them):
      menu.png      the main menu page's background (dat\menu.dat BACKGROUND MAINMENU: the PDA)
      loadsave.png  239 "loadsavegame"      options.png  236 "options"
      museum.png    541 "museum"            loading.png  252 "loadscreen"
      inventory.png 53 "menu inventorydark4" credits.png 246 "menu credits4"
    and level-<gam, lower case>.png: the loading picture (CLoadLevel "LoadScreen") of every exit
    of the levels played, so Continue can show the autosave's level.                          */

#include "LauncherArt.h"
#include "../C2DInGameMenu.h"
#include "../Game.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "../levels/CLoadLevel.h"
#include "OMediaCanvas.h"
#include "OMediaDataBase.h"
#include "OMediaFileStream.h"
#include "OMediaFilePath.h"

#include <SDL.h>
#include <ctype.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace
{

std::string s_dir;
bool s_env_read = false, s_fixed_done = false;
int s_scan_wait = 0;
std::set<std::string> s_done;			// files written or found this run

bool Exists(const std::string &file)
{
	if (FILE *f = fopen(file.c_str(), "rb")) { fclose(f); return true; }
	return false;
}

// Canvas `id` of screens.omt -> <dir>/<name>, unless it is there already.
void Save(int id, const std::string &name)
{
	if (id < 0 || s_done.count(name)) return;
	s_done.insert(name);
	const std::string file = s_dir + "/" + name;
	if (Exists(file)) return;
	OMediaCanvas *c = neg::LoadCanvasID(g_screens_db, id);
	if (!c || c->get_width() < 64 || c->get_height() < 64) return;
	try
	{
		OMediaFilePath path(file.c_str());
		OMediaFileStream f;
		f.setpath(&path);
		f.open(omcfp_Write, true);
		c->png_export(f);
		f.close();
		SDL_Log("LAUNCHER ART: screens.omt canvas %d -> %s", id, file.c_str());
	}
	catch (...)
	{
		SDL_Log("LAUNCHER ART: cannot write %s", file.c_str());
	}
}

int MainMenuBackground()
{
	int id = -1;
	for (std::list<C2DInGameMenu::MenuObject *>::iterator i = C2DInGameMenu::s_objects.begin();
		 i != C2DInGameMenu::s_objects.end(); ++i)
		if (*i && (*i)->type == C2DInGameMenu::MOBJ_BACKGROUND && strcasecmp((*i)->name, "MAINMENU") == 0)
			id = (*i)->canvas_hi;
	return id;
}

// The loading picture of every exit in the loaded level, as level-<gam>.png.
void ScanExits()
{
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
	{
		CLoadLevel *l = dynamic_cast<CLoadLevel *>(*i);
		if (!l || l->m_load_screen <= 0) continue;
		std::string gam = l->m_level_name;
		for (size_t k = 0; k < gam.size(); k++) gam[k] = (char)tolower((unsigned char)gam[k]);
		if (gam.size() < 5 || gam.compare(gam.size() - 4, 4, ".gam") != 0) continue;
		Save(l->m_load_screen, "level-" + gam.substr(0, gam.size() - 4) + ".png");
	}
}

} // namespace

void neg::LauncherArtTick(void)
{
	if (!s_env_read)
	{
		s_env_read = true;
		const char *d = getenv("NEG_LAUNCHER_ART");
		if (d && *d)
		{
			s_dir = d;
			mkdir(s_dir.c_str(), 0755);
		}
	}
	if (s_dir.empty() || !g_screens_db || C2DInGameMenu::s_objects.empty()) return;
	if (!s_fixed_done)
	{
		s_fixed_done = true;
		Save(MainMenuBackground(), "menu.png");
		Save(239, "loadsave.png");
		Save(236, "options.png");
		Save(541, "museum.png");
		Save(252, "loading.png");
		Save(53, "inventory.png");
		Save(246, "credits.png");
	}
	if (--s_scan_wait > 0) return;
	s_scan_wait = 120;							// the exits: every couple of seconds
	ScanExits();
}
