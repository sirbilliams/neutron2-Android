/*  OMT SDL platform layer - entry point, data folder, logging, test hooks.  LGPL.
    main() -> omt_main(), like OMediaWinMain.cpp. (On Android SDL renames main to SDL_main.)

    Data folder (where omt/, gam/, grn/ ... live):
      OMT_DATA=<dir> if set, else on Android the app's external files dir
      (Android/data/<package>/files), or its Program_Executable_Files subfolder.
    Log: also written to <data>/omt-log.txt on Android.
    Test env: OMT_SCREENSHOT=<x.bmp> saves frame OMT_FRAMES (default 30) and quits.

    This file is compiled into each app (not the static lib) so SDL can always find main. */

#include "OMediaSDLRetarget.h"
#include "OMediaSupervisor.h"
#include "OMediaDefMessages.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif
// GL comes from OMediaSysDefs.h: system GL on desktop, gl4es's headers (and gl4es itself) on Android.

extern void omt_main(void);

static int omg_frame = 0;
static FILE *omg_logfile = NULL;

// Called by OMediaGLRenderTarget::flip_buffers before swapping.
void omf_SDLFrameHook(SDL_Window *win)
{
	omg_frame++;
	const char *shot = getenv("OMT_SCREENSHOT");
	if (!shot) return;
	int want = getenv("OMT_FRAMES") ? atoi(getenv("OMT_FRAMES")) : 30;
	if (omg_frame != want) return;

	int w, h;
	SDL_GL_GetDrawableSize(win, &w, &h);
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 24, SDL_PIXELFORMAT_RGB24);
	unsigned char *tmp = (unsigned char*)malloc(w * h * 4);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tmp);      // RGBA works on GL and GLES
	for (int y = 0; y < h; y++)                                     // GL is bottom-up
	{
		unsigned char *d = (unsigned char*)s->pixels + y * s->pitch, *src = tmp + (h - 1 - y) * w * 4;
		for (int x = 0; x < w; x++) { d[x * 3] = src[x * 4]; d[x * 3 + 1] = src[x * 4 + 1]; d[x * 3 + 2] = src[x * 4 + 2]; }
	}
	free(tmp);
	SDL_SaveBMP(s, shot);
	SDL_FreeSurface(s);
	SDL_Log("OMT: saved %s (%dx%d) at frame %d", shot, w, h, omg_frame);

	SDL_Event q; SDL_zero(q); q.type = SDL_QUIT; SDL_PushEvent(&q);
}

static void SDLCALL omf_log(void *, int category, SDL_LogPriority priority, const char *message)
{
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "OMT", "%s", message);
#else
	fprintf(stderr, "%s\n", message);
#endif
	if (omg_logfile) { fprintf(omg_logfile, "%s\n", message); fflush(omg_logfile); }
}

static bool omf_isdir(const string &p) { struct stat st; return stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode); }

static string omf_find_data_dir(void)
{
	const char *env = getenv("OMT_DATA");
	if (env && *env) return env;
#ifdef __ANDROID__
	const char *ext = SDL_AndroidGetExternalStoragePath();
	if (ext)
	{
		string base = ext;
		if (omf_isdir(base + "/omt")) return base;
		if (omf_isdir(base + "/Program_Executable_Files/omt")) return base + "/Program_Executable_Files";
		return base;      // nothing copied yet; the app reports it
	}
#endif
	return "";
}

int main(int argc, char **argv)
{
	SDL_LogSetOutputFunction(omf_log, NULL);
	string data = omf_find_data_dir();
#ifdef __ANDROID__
	const char *ext = SDL_AndroidGetExternalStoragePath();
	if (ext) omg_logfile = fopen((string(ext) + "/omt-log.txt").c_str(), "w");
#endif
	if (!data.empty() && chdir(data.c_str()) != 0) SDL_Log("OMT: cannot open data folder %s", data.c_str());
	SDL_Log("OMT: data folder %s (%s)", data.empty() ? "." : data.c_str(),
			omf_isdir("omt") ? "game data found" : "NO omt/ folder here - copy the game files first");
	omt_main();
	if (omg_logfile) fclose(omg_logfile);
	SDL_Quit();
	return 0;
}
