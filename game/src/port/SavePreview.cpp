/* port: tracked active playtime and checkpoint thumbnails for Continue / Load Game.
   Sidecars are bound to the .tsk's complete CRC, so replaced/imported saves cannot show old art.
   A thumbnail is captured on the first gameplay frame after a checkpoint write, not a new save point. */
#include "SavePreview.h"
#include "AssetView.h"
#include "../gen/globals.h"
#include <SDL.h>
#include <zlib.h>
#include <map>
#include <string>
#include <stdio.h>
#include <string.h>
#include <math.h>

namespace neg { extern bool g_game_paused; }
extern std::string omf_SDLResolvePath(const std::string &);
extern SDL_Surface *omf_SDLCaptureThumbnail(SDL_Window *);
extern void (*omf_SDLFrameObserver)(SDL_Window *);
namespace {
double seconds = 0;
SDL_Surface *cached = NULL;
std::string cachedLevel;
struct Pending { unsigned long crc; double seconds; std::string level; };
std::map<std::string, Pending> pending;
std::string path(const char *name) { return omf_SDLResolvePath(std::string("save\\")+name); }
bool checksum(const std::string &file, unsigned long &crc) {
    FILE *f=fopen(file.c_str(),"rb"); if (!f) return false;
    crc=crc32(0,NULL,0); unsigned char b[8192]; size_t n;
    while ((n=fread(b,1,sizeof b,f))!=0) crc=crc32(crc,b,(uInt)n);
    bool ok=!ferror(f); fclose(f); return ok;
}
bool metadata(const std::string &base, unsigned long &crc, double &time, int &photo) {
    FILE *f=fopen((base+".preview").c_str(),"r"); if (!f) return false;
    int version=0; bool ok=fscanf(f,"%d %lu %lf %d",&version,&crc,&time,&photo)==4;
    fclose(f); return ok && version==1 && time>=0 && time<=315360000 && (photo==0 || photo==1);
}
void write(const std::string &base, const Pending &p, bool photo) {
    unsigned long current;
    if (!checksum(base,current) || current!=p.crc) return;
    // An image failure falls back to text rather than claiming stale art is current.
    if (photo) {
        std::string tmp=base+".bmp.tmp";
        photo=cached && SDL_SaveBMP(cached,tmp.c_str())==0 && rename(tmp.c_str(),(base+".bmp").c_str())==0;
        remove(tmp.c_str());
    }
    std::string tmp=base+".preview.tmp";
    FILE *f=fopen(tmp.c_str(),"w"); if (!f) return;
    fprintf(f,"1 %lu %.0f %d\n",p.crc,p.seconds,photo?1:0);
    if (fclose(f)==0) rename(tmp.c_str(),(base+".preview").c_str());
    remove(tmp.c_str());
}
bool playing() { return !neg::AssetViewLevel() && g_game_mode==5 && !neg::g_game_paused; }
void frame(SDL_Window *win) {
    if (!playing()) return;
    bool wanted=false;
    for (std::map<std::string,Pending>::iterator i=pending.begin();i!=pending.end();++i)
        if (i->second.level==g_game_file) wanted=true;
    if (!wanted) return;
    SDL_Surface *next=omf_SDLCaptureThumbnail(win); if (!next) return;
    SDL_FreeSurface(cached); cached=next; cachedLevel=g_game_file;
    for (std::map<std::string,Pending>::iterator i=pending.begin();i!=pending.end();) {
        if (i->second.level!=cachedLevel) { ++i; continue; }
        write(i->first,i->second,true); pending.erase(i++);
    }
}
}
void neg::SavePreviewTick(float dt) {
    if (AssetViewLevel()) return;
    omf_SDLFrameObserver=frame;
    if (playing() && dt>0 && dt<1) seconds+=dt;
}
void neg::SavePreviewWritten(const char *name) {
    if (AssetViewLevel()) return;
    const std::string base=path(name); Pending p; p.seconds=seconds; p.level=g_game_file;
    if (!checksum(base,p.crc)) return;
    write(base,p,false);
    pending[base]=p;
}
void neg::SavePreviewLoad(const char *name) {
    if (AssetViewLevel()) return;
    unsigned long crc, actual; double time; int photo;
    seconds=metadata(path(name),crc,time,photo) && checksum(path(name),actual) && crc==actual ? time : 0;
    pending.clear(); SDL_FreeSurface(cached); cached=NULL;
}
void neg::SavePreviewNewGame() {
    if (AssetViewLevel()) return;
    seconds=0; pending.clear(); SDL_FreeSurface(cached); cached=NULL;
}
void neg::SavePreviewDelete(const char *dosPath) {
    if (AssetViewLevel()) return;
    const std::string base=omf_SDLResolvePath(dosPath);
    remove((base+".preview").c_str()); remove((base+".bmp").c_str()); pending.erase(base);
}
void neg::SavePreviewCopy(const char *src, const char *dst) {
    if (AssetViewLevel()) return;
    const std::string from=omf_SDLResolvePath(src), to=omf_SDLResolvePath(dst);
    unsigned long crc, a, b; double time; int photo;
    SavePreviewDelete(dst);
    if (!metadata(from,crc,time,photo) || !checksum(from,a) || !checksum(to,b) || a!=b || a!=crc) return;
    bool copied=false;
    if (photo) {
        FILE *in=fopen((from+".bmp").c_str(),"rb");
        FILE *out=in ? fopen((to+".bmp.tmp").c_str(),"wb") : NULL;
        if (out) {
            copied=true; char buffer[8192]; size_t n;
            while ((n=fread(buffer,1,sizeof buffer,in))>0)
                if (fwrite(buffer,1,n,out)!=n) { copied=false; break; }
            copied=copied && !ferror(in);
            if (fclose(out)!=0) copied=false;
            if (copied) copied=rename((to+".bmp.tmp").c_str(),(to+".bmp").c_str())==0;
        }
        if (in) fclose(in);
        remove((to+".bmp.tmp").c_str());
    }
    Pending p; p.crc=crc;p.seconds=time;
    // write metadata only; the image was copied from the matching autosave, not the pause menu.
    std::string tmp=to+".preview.tmp"; FILE *f=fopen(tmp.c_str(),"w"); if (!f) return;
    fprintf(f,"1 %lu %.0f %d\n",p.crc,p.seconds,copied?1:0);
    if (fclose(f)==0) rename(tmp.c_str(),(to+".preview").c_str());
    remove(tmp.c_str());
}
