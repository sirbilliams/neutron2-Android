#include "../../../game/src/port/SavePreview.h"
#include "../../../game/src/gen/globals.h"
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
namespace neg {
unsigned char g_data[DATA_SIZE];
bool g_game_paused=false;
bool viewer=false;
const char *AssetViewLevel() { return viewer ? "Level3.gam" : NULL; }
}
void (*omf_SDLFrameObserver)(SDL_Window *)=NULL;
std::string omf_SDLResolvePath(const std::string &s) {
    std::string p=s; for (size_t i=0;i<p.size();++i) if (p[i]=='\\') p[i]='/'; return p;
}
SDL_Surface *omf_SDLCaptureThumbnail(SDL_Window *) {
    SDL_Surface *s=SDL_CreateRGBSurfaceWithFormat(0,128,72,24,SDL_PIXELFORMAT_RGB24);
    assert(s); SDL_FillRect(s,NULL,SDL_MapRGB(s->format,20,80,130)); return s;
}
void file(const char *name,const char *s) { FILE *f=fopen(name,"w");assert(f);fputs(s,f);fclose(f); }
void check(const char *name,int time,int photo) {
    FILE *f=fopen(name,"r"); assert(f);int v,p;unsigned long crc;double t;
    assert(fscanf(f,"%d %lu %lf %d",&v,&crc,&t,&p)==4);fclose(f);
    assert(v==1 && t==time && p==photo);
}
void tick(int n) { for(int i=0;i<n;++i) neg::SavePreviewTick(0.5f); }
int main() {
    char temp[]="/tmp/neg-preview-XXXXXX";assert(mkdtemp(temp));assert(chdir(temp)==0);mkdir("save",0700);
    g_game_mode=5;strcpy(g_game_file,"Level3.gam");
    file("save/autosave.tsk","checkpoint 1");tick(20);assert(omf_SDLFrameObserver);omf_SDLFrameObserver(NULL);
    neg::SavePreviewWritten("autosave.tsk");
    if (!neg::g_game_paused && g_game_mode==5) omf_SDLFrameObserver(NULL);
    check("save/autosave.tsk.preview",10,neg::g_game_paused || g_game_mode!=5 ? 0 : 1);
    SDL_Surface *bmp=SDL_LoadBMP("save/autosave.tsk.bmp");assert(bmp && bmp->w==128);SDL_FreeSurface(bmp);
    neg::g_game_paused=true;tick(20);neg::SavePreviewWritten("autosave.tsk");
    if (!neg::g_game_paused && g_game_mode==5) omf_SDLFrameObserver(NULL);
    check("save/autosave.tsk.preview",10,neg::g_game_paused || g_game_mode!=5 ? 0 : 1);
    neg::g_game_paused=false;g_game_mode=3;tick(20);neg::SavePreviewWritten("autosave.tsk");
    if (!neg::g_game_paused && g_game_mode==5) omf_SDLFrameObserver(NULL);
    check("save/autosave.tsk.preview",10,neg::g_game_paused || g_game_mode!=5 ? 0 : 1);
    neg::SavePreviewLoad("autosave.tsk");g_game_mode=5;tick(10);omf_SDLFrameObserver(NULL);
    neg::SavePreviewWritten("autosave.tsk");omf_SDLFrameObserver(NULL);check("save/autosave.tsk.preview",15,1);
    file("save/JimmyGame1.tsk","checkpoint 1");neg::SavePreviewCopy("save\\autosave.tsk","save\\JimmyGame1.tsk");
    check("save/JimmyGame1.tsk.preview",15,1);
    file("save/JimmyGame1.tsk","different save");neg::SavePreviewCopy("save\\autosave.tsk","save\\JimmyGame1.tsk");
    assert(access("save/JimmyGame1.tsk.preview",F_OK)!=0);
    file("save/autosave.tsk","imported save");neg::SavePreviewLoad("autosave.tsk");tick(10);
    neg::SavePreviewWritten("autosave.tsk");check("save/autosave.tsk.preview",5,0);
    omf_SDLFrameObserver(NULL);check("save/autosave.tsk.preview",5,1);
    neg::SavePreviewNewGame();neg::SavePreviewWritten("autosave.tsk");check("save/autosave.tsk.preview",0,0);
    neg::viewer=true;tick(20);neg::SavePreviewWritten("autosave.tsk");check("save/autosave.tsk.preview",0,0);
    neg::viewer=false;neg::SavePreviewDelete("save\\autosave.tsk");assert(access("save/autosave.tsk.bmp",F_OK)!=0);
    assert(access("save/autosave.tsk.preview",F_OK)!=0);
    puts("Save preview CRC, playtime, copy/delete, pending-image and viewer regressions passed");
}
