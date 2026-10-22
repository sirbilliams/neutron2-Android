/*  neg_hdtool - the desktop half of the HD image pack (docs/hd-pack.md).

      neg_hdtool list    <game dir> [db.omt ...]           list the canvases (id, name, size)
      neg_hdtool extract <game dir> <out dir> [db.omt ...]  write each canvas as <out>/<db>/<id>.png
      neg_hdtool install <game dir> <pack.zip>              unpack an HD pack into <game dir>/hd/
      neg_hdtool remove  <game dir>                         delete <game dir>/hd/

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
#include <dirent.h>
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

// ---------------------------------------------------------------- install (the desktop half of the
// launcher's "HD texture pack: install"; same rules as android/.../HdPack.java)

// A file of the unpacked zip -> its place in the pack ("omt/<db>/<id>.png", "png/<name>.png",
// "grn/<model>_<n>.png"), or "" for anything else.  Leading folders before omt/png/grn are dropped.
static string pack_path(const string &rel)
{
	vector<string> parts;
	string cur;
	for (size_t i = 0; i <= rel.size(); i++)
		if (i == rel.size() || rel[i] == '/' || rel[i] == '\\') { parts.push_back(cur); cur.clear(); }
		else cur += rel[i];
	const size_t n = parts.size();
	const string file = parts[n - 1], low = lower(file);
	if (low.size() <= 4 || low.compare(low.size() - 4, 4, ".png") != 0) return "";
	const string stem = file.substr(0, file.size() - 4);
	bool digits = !stem.empty();
	for (size_t i = 0; i < stem.size(); i++) if (!isdigit((unsigned char)stem[i])) digits = false;
	const size_t us = stem.find_last_of('_');
	bool model = us != string::npos && us > 0 && us + 1 < stem.size();
	for (size_t i = us + 1; model && i < stem.size(); i++) if (!isdigit((unsigned char)stem[i])) model = false;
	if (n >= 3 && lower(parts[n - 3]) == "omt" && digits && !parts[n - 2].empty()) return "omt/" + lower(parts[n - 2]) + "/" + file;
	if (n >= 2 && lower(parts[n - 2]) == "png") return "png/" + file;
	if (n >= 2 && lower(parts[n - 2]) == "grn" && model) return "grn/" + file;
	return "";
}

static void walk(const string &root, const string &rel, vector<string> &out)
{
	DIR *d = opendir((root + "/" + rel).c_str());
	if (!d) return;
	struct dirent *e;
	while ((e = readdir(d)) != NULL)
	{
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		const string r = rel.empty() ? string(e->d_name) : rel + "/" + e->d_name;
		struct stat st;
		if (stat((root + "/" + r).c_str(), &st) != 0) continue;
		if (S_ISDIR(st.st_mode)) walk(root, r, out);
		else out.push_back(r);
	}
	closedir(d);
}

static int install(const string &game, const string &zip)
{
	const string tmp = game + "/hd.import", stage = game + "/hd.stage", dest = game + "/hd";
	system(("rm -rf '" + tmp + "' '" + stage + "'").c_str());
	const string cmd = "unzip -o -q '" + zip + "' -d '" + tmp + "'";
	if (system(cmd.c_str()) != 0) { fprintf(stderr, "unzip failed\n"); system(("rm -rf '" + tmp + "'").c_str()); return 1; }
	vector<string> files;
	walk(tmp, "", files);
	int omt = 0, png = 0, grn = 0, skipped = 0;
	for (size_t i = 0; i < files.size(); i++)
	{
		const string to = pack_path(files[i]);
		if (to.empty()) { skipped++; continue; }
		const string target = stage + "/" + to;
		mkdirs(target.substr(0, target.find_last_of('/')));
		if (rename((tmp + "/" + files[i]).c_str(), target.c_str()) != 0) { skipped++; continue; }
		if (to[0] == 'o') omt++; else if (to[0] == 'p') png++; else grn++;
	}
	system(("rm -rf '" + tmp + "'").c_str());
	if (omt + png + grn == 0)
	{
		fprintf(stderr, "not an HD texture pack: no omt/<database>/<id>.png, png/<name>.png or grn/<model>_<n>.png\n");
		system(("rm -rf '" + stage + "'").c_str());
		return 1;
	}
	system(("rm -rf '" + dest + "'").c_str());
	if (rename(stage.c_str(), dest.c_str()) != 0) { fprintf(stderr, "cannot move the pack into %s\n", dest.c_str()); return 1; }
	fprintf(stderr, "installed into %s: %d omt, %d png and %d model pictures (%d other files left out)\n",
			dest.c_str(), omt, png, grn, skipped);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc >= 3 && !strcmp(argv[1], "list"))
		return run("list", argv[2], "", vector<string>(argv + 3, argv + argc));
	if (argc >= 4 && !strcmp(argv[1], "extract"))
		return run("extract", argv[2], argv[3], vector<string>(argv + 4, argv + argc));
	if (argc == 4 && !strcmp(argv[1], "install"))
		return install(argv[2], argv[3]);
	if (argc == 3 && !strcmp(argv[1], "remove"))
	{
		const string cmd = "rm -rf '" + string(argv[2]) + "/hd'";
		return system(cmd.c_str()) == 0 ? 0 : 1;
	}
	fprintf(stderr, "usage: neg_hdtool list <game dir> [db.omt ...]\n"
					"       neg_hdtool extract <game dir> <out dir> [db.omt ...]\n"
					"       neg_hdtool install <game dir> <pack.zip>\n"
					"       neg_hdtool remove  <game dir>\n");
	return 2;
}
