/*  neg_hdtool - the desktop half of the HD image pack (docs/hd-pack.md).

      neg_hdtool list    <game dir> [db.omt ...]           list the canvases (id, name, size)
      neg_hdtool extract <game dir> <out dir> [db.omt ...]  write each canvas as <out>/<db>/<id>.png
      neg_hdtool install <game dir> <pack.zip>              unpack an HD pack into <game dir>/hd/

    It reads the player's own installed game files (omt/*.omt) through OMT, exactly as the game
    loads them, and writes RGBA PNGs (alpha kept) outside the repository. Without a database list
    it uses the HD set: the databases holding the full-screen art, the menus, the HUD and the font.
    Nothing it writes may be committed to the public repository: it is the game's art.          */

#include "OMediaDataBase.h"
#include "OMediaFileStream.h"
#include "OMediaFilePath.h"
#include "OMediaCanvas.h"
#include "OMediaError.h"
#include <SDL.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

// The databases the HD pack covers (docs/hd-pack.md lists who loads what).
static const char *kHDDatabases[] = { "screens.omt", "alpha.omt", "Editor.omt", "RetainedSprites.omt", NULL };

static string lower(string s) { for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]); return s; }
static void mkdirs(const string &p) { string cur; for (size_t i = 0; i <= p.size(); i++) { if (i == p.size() || p[i] == '/') { if (!cur.empty()) mkdir(cur.c_str(), 0755); } if (i < p.size()) cur += p[i]; } }

// The base name of a database file, lower case, without ".omt": the folder name in hd/.
static string db_key(const string &file)
{
	string b = file;
	size_t s = b.find_last_of("/\\");
	if (s != string::npos) b = b.substr(s + 1);
	b = lower(b);
	if (b.size() > 4 && b.compare(b.size() - 4, 4, ".omt") == 0) b.resize(b.size() - 4);
	return b;
}

static int run(const string &mode, const string &game, const string &out, vector<string> dbs)
{
	OMediaDataBase::set_cache_size(0);
	omd_REGISTERCLASS(OMediaCanvas);
	if (dbs.empty()) for (const char **d = kHDDatabases; *d; d++) dbs.push_back(*d);
	FILE *manifest = NULL;
	if (mode == "extract")
	{
		mkdirs(out);
		manifest = fopen((out + "/manifest.tsv").c_str(), "w");
		if (manifest) fprintf(manifest, "db\tid\tname\twidth\theight\topaque\n");
	}
	int total = 0;
	for (size_t i = 0; i < dbs.size(); i++)
	{
		const string fn = game + "/omt/" + dbs[i];
		try
		{
			OMediaFilePath path(fn);
			OMediaFileStream f; f.setpath(&path); f.open(omcfp_Read);
			OMediaDataBase *db = new OMediaDataBase(&f);
			const string key = db_key(dbs[i]);
			if (mode == "extract") mkdirs(out + "/" + key);
			omt_DBIterator it;
			int n = 0;
			for (OMediaDBObject *o = db->get_first_object(OMediaCanvas::db_type, it); o; o = db->get_next_object(it))
			{
				OMediaCanvas *c = (OMediaCanvas *)o;
				string name; c->get_chunk_name(name);
				const int id = (int)c->get_chunk_ID();
				c->lock(omlf_Read);
				const int w = c->get_width(), h = c->get_height();
				bool opaque = true;
				const unsigned char *px = (const unsigned char *)c->get_pixels();
				for (int k = 0; px && k < w * h; k++) if (px[k * 4 + 3] != 255) { opaque = false; break; }
				c->unlock();
				if (mode == "list")
					printf("%-20s %5d %-28s %4dx%-4d %s\n", key.c_str(), id, name.c_str(), w, h, opaque ? "opaque" : "alpha");
				else if (w > 0 && h > 0)
				{
					char file[512];
					snprintf(file, sizeof file, "%s/%s/%d.png", out.c_str(), key.c_str(), id);
					OMediaFilePath op(file);
					OMediaFileStream os; os.setpath(&op);
					os.open(omcfp_Write, true);
					c->png_export(os);
					os.close();
					if (manifest) fprintf(manifest, "%s\t%d\t%s\t%d\t%d\t%d\n", key.c_str(), id, name.c_str(), w, h, opaque ? 1 : 0);
				}
				n++;
			}
			fprintf(stderr, "%s: %d canvases\n", dbs[i].c_str(), n);
			total += n;
			delete db;
		}
		catch (OMediaError &e)
		{
			fprintf(stderr, "%s: cannot read (OMT error %d)\n", fn.c_str(), (int)e.errorcode);
		}
	}
	if (manifest) fclose(manifest);
	fprintf(stderr, "%d canvases\n", total);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc >= 3 && !strcmp(argv[1], "list"))
		return run("list", argv[2], "", vector<string>(argv + 3, argv + argc));
	if (argc >= 4 && !strcmp(argv[1], "extract"))
		return run("extract", argv[2], argv[3], vector<string>(argv + 4, argv + argc));
	if (argc == 4 && !strcmp(argv[1], "install"))
	{
		// The pack is a plain zip of the hd/ folder; unzip does the work on the desktop.
		const string dest = string(argv[2]);
		const string cmd = "unzip -o -q '" + string(argv[3]) + "' -d '" + dest + "'";
		const int r = system(cmd.c_str());
		if (r != 0) { fprintf(stderr, "unzip failed (%d)\n", r); return 1; }
		fprintf(stderr, "installed into %s/hd\n", dest.c_str());
		return 0;
	}
	fprintf(stderr, "usage: neg_hdtool list <game dir> [db.omt ...]\n"
					"       neg_hdtool extract <game dir> <out dir> [db.omt ...]\n"
					"       neg_hdtool install <game dir> <pack.zip>\n");
	return 2;
}
