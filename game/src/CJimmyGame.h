/*  CJimmyGame - the game type every level's game type derives from (CLevel01Game ... CLevelVR02).
    It adds to CGameType the logic all levels share: the terrain with its per-texture effects
    (scrolling and wobbling textures, unlit "nc" / additive "nc1" "nc2" pieces), the level's fog
    and clear colour, the power-up timers (stopwatch, x-ray...), surface effects when Jimmy walks
    on special textures, the score rank and its promotions, the level music, pausing, focus
    changes and the per-level save state.
    Reconstructed from Neutron2.exe: constructor 0x4827f0, destructor 0x4828d0 (both empty: all
    the work is CGameType's), deleting thunks 0x486540 / 0x4828a0 / 0x486550.

    Layout (classes.json), the same as CGameType's plus own members:
        +0x000  OMediaElement           element vtable 0x506dac (109 slots: CGameType's 87 + 87..108)
        +0x030  OMediaListener          vtable 0x506d98
        +0x08c  CLocalGameObject        vtable 0x50691c (286 slots)
        +0x554  own members (below), up to the vtordisp at +0x684
        +0x688  OMediaClassStreamer     shared virtual base, vtable 0x50690c
    The factory has no ID for CJimmyGame itself: only the CLevelXXGame classes are created.

    Members are named after their offset from the object start (the element-family view; a
    CLocalGameObject-family method of the original sees them 0x8c lower).                     */
#pragma once
#include "CGameType.h"
#include <list>
#include <string>

class C3DObject;
class OMedia3DPolygon;
class OMedia3DMaterial;
class OMediaCanvas;
class OMediaStreamOperators;

class CJimmyGame : public CGameType
{
	public:
	CJimmyGame();															// 0x4827f0
	virtual ~CJimmyGame();													// 0x4828d0

	// ---- CLocalGameObject-family overrides
	virtual void ResetObject(void);											// SLOT 10  0x482a70 (jmp to CLocalGameObject's)
	virtual void Process(float dt);											// SLOT 240 0x484260
	virtual void v242(float t);												// SLOT 242 0x484640 per frame while the editor is editing
	virtual void v260(void);												// SLOT 260 0x483b50 after a level load (CEditor ResetLevel)
	virtual void v277(void);												// SLOT 277 0x485e80 PauseAll(true): stop the stopwatch tick
	virtual void v278(void);												// SLOT 278 0x485ea0 PauseAll(false): restart it
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x486170 savegame
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x485ef0 savegame

	// ---- element-family overrides of CGameType
	virtual void InitGame(void);											// ESLOT 45 0x482910
	virtual void UnInitGame(void);											// ESLOT 46 0x482a30
	virtual void InitTerrainDatabase(std::string path);						// ESLOT 57 0x4851a0
	virtual void CreateTerrain(void);										// ESLOT 61 0x482bc0
	virtual void CreateTerrainUnLit(void);									// ESLOT 62 0x483040
	virtual void PauseGame(bool pause);										// ESLOT 68 0x485410
	virtual void OnActivate(void);											// ESLOT 71 0x482a80 "GetFocus()"
	virtual void OnDeactivate(void);										// ESLOT 72 0x482ad0 "LoseFocus()"
	virtual void TouchSurface(CGameObject *o, OMedia3DMaterial *mat, OMediaCanvas *tex, float dt);	// ESLOT 78 0x484740

	// ---- element-family slots CJimmyGame adds (vtable offset = slot * 4)
	virtual void ev87(void) {}												// ESLOT 87  0x463890 +0x15c (empty)
	virtual int  NextPickupIndex(void);										// ESLOT 88  0x484700 +0x160
	virtual bool FileIsThere(char *name, const char *dir, const char *cd_dir);	// ESLOT 89 0x484f40 +0x164
	virtual void CreateTerrainTinted(float r, float g, float b, float a);	// ESLOT 90  0x483780 +0x168
	virtual char GetSceneStage(void);										// ESLOT 91  0x483bf0 +0x16c
	virtual bool SetupTextureScroll(OMedia3DPolygon *p);					// ESLOT 92  0x484cc0 +0x170
	virtual void SetTerrainDBColor(float r, float g, float b, float a);		// ESLOT 93  0x4834b0 +0x174
	virtual void SetTerrainGrey(float v);									// ESLOT 94  0x4836a0 +0x178
	virtual void UpdateTimers(float dt);									// ESLOT 95  0x483c50 +0x17c
	virtual void SetTerrainColor(float r, float g, float b, float a);		// ESLOT 96  0x4835b0 +0x180
	virtual void ev97(float dt) { (void)dt; }								// ESLOT 97  0x464ee0 +0x184 (empty; per frame, for the levels)
	virtual bool CheckPromotion(void);										// ESLOT 98  0x485570 +0x188
	virtual int  GetRank(void);												// ESLOT 99  0x4854e0 +0x18c
	virtual void RefreshObjects(void);										// ESLOT 100 0x483c20 +0x190
	virtual void SetFog(float r, float g, float b, float a, float start, float end, float far_clip, bool on);	// ESLOT 101 0x484520 +0x194
	virtual void PlayLevelMusic(const char *name, int a);					// ESLOT 102 0x485d00 +0x198
	virtual void StopLevelMusic(void);										// ESLOT 103 0x485b80 +0x19c
	virtual void StopLevelMusicKeepName(void);								// ESLOT 104 0x485c60 +0x1a0
	virtual void StartGadgetEffect(const char *gadget);						// ESLOT 105 0x483570 +0x1a4
	virtual float SmoothFrameRate(float dt);								// ESLOT 106 0x485b40 +0x1a8
	virtual void StopwatchEnd(void);										// ESLOT 107 0x4841f0 +0x1ac
	virtual void RestoreTimerEffects(void);									// ESLOT 108 0x484110 +0x1b0

	// ---- members (not initialised by the constructor; InitGame sets them)
	// FIELD 0x554 4
	float		f_554;					// 0x554  1.0 (InitGame; not read here)
	// FIELD 0x558 4
	float		f_558;					// 0x558  1.0 (InitGame; not read here)
	// FIELD 0x55c 8
	int			m_surface_sound;		// 0x55c  surface (footstep) sound set given to Jimmy (slot 153)
	int			m_surface_sound_next;	// 0x560  set from the texture under Jimmy by TouchSurface
	// FIELD 0x564 4
	int			f_564;					// 0x564  0 (InitGame)
	// (0x568..0x5b8 is not used by CJimmyGame)
	// FIELD 0x5b8 8
	int			m_pickup_index_base;	// 0x5b8  100
	int			m_pickup_index;			// 0x5bc  NextPickupIndex counter, reset by v260
	// FIELD 0x5c0 8
	C3DObject	*m_sky;					// 0x5c0  the level's '3SKY' (the levels create it)
	unsigned char m_sky_rotates;		// 0x5c4  Process turns the sky about y by dt
	// FIELD 0x5c8 4
	int			f_5c8;					// 0x5c8  0 (InitGame)
	// FIELD 0x5cc 24
	float		m_fog_restore[3];		// 0x5cc  start, end, far given back to SetFog when the window gets the focus again
	float		m_fog_start;			// 0x5d8
	float		m_fog_end;				// 0x5dc
	float		m_far_clip;				// 0x5e0
	// FIELD 0x5e4 16
	float		m_fog_color[4];			// 0x5e4  r, g, b, a (also the clear colour)
	// FIELD 0x5f4 1
	unsigned char m_refog;				// 0x5f4  set by OnActivate: Process calls SetFog again
	// (0x5f5..0x660 is not used by CJimmyGame)
	// FIELD 0x660 12
	float		m_xray_time;			// 0x660  terrain flashing while the x-ray gadget runs (StartGadgetEffect "XRAY")
	float		m_xray_flash;			// 0x664  >0 counting down: green next; <=0 counting up: yellow next
	int			m_stopwatch_channel;	// 0x668  "stopwatch" tick sound, -1 when not playing
	// FIELD 0x66c 24
	float		f_66c;					// 0x66c  5.0 (InitGame; not read here)
	float		f_670;					// 0x670  20000.0
	float		f_674;					// 0x674  20000.0
	float		f_678;					// 0x678  15.0
	int			m_no_texture_scroll;	// 0x67c  0: Process runs the texture scrolls
	float		f_680;					// 0x680  0.7
};

// --------------------------------------------------------------------------- globals CJimmyGame owns
namespace neg {
extern std::string					g_queued_sfx;			// 0x54a4a8  played by Process once channel g_queued_sfx_after is free (set by 0x48f310)
}
