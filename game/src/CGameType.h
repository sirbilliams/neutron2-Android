/*  CGameType - the base of every level's game type: CDefaultGame (main menu), CJimmyGame and the
    CLevelXXGame classes. It owns the level-wide databases (effects, objects, songs, sounds, terrain,
    vehicles: one .omt file each), the fog colour, pause and help state, the active camera and the
    per-player input devices.
    Reconstructed from Neutron2.exe: constructor 0x4af5a0, destructor 0x4af6e0 ("~CGameType()").

    The object is both an OMT element and a game object (classes.json):
        +0x000  OMediaElement           element vtable 0x51ba84 (87 slots)
        +0x030  OMediaListener          vtable 0x51ba70
        +0x08c  CLocalGameObject        vtable 0x51b5f4 (286 slots)
        +0x558  OMediaClassStreamer     shared virtual base, vtable 0x51b5e4
    It has no members of its own (0x554..0x558 is never used). The constructor links it into
    g_world and makes it the running game type (neg::g_gametype); the CGameObject constructor puts
    it in g_objects like every game object.

    Per frame: OMT calls update_logic on it (it is linked into the world), which only forwards to
    OMediaElement. The game's own update comes from CViewPort::render_viewport (0x4b96a0), which
    calls v4(dt) on every object in g_objects (0x4ae580) -> Process(dt) below (CGameObject 0x48f880).

    Element-family slots 43..86 are the game-type interface the rest of the game calls through
    neg::g_gametype (vtable +0xac..+0x158). Unnamed ones keep ev<N>.                           */
#pragma once
#include "OMediaElement.h"
#include "CLocalGameObject.h"
#include <string>

class OMediaDataBase;
class OMediaFileStream;
class OMediaElement;

class CGameType : public OMediaElement, public CLocalGameObject
{
	public:
	CGameType();															// 0x4af5a0
	virtual ~CGameType();													// 0x4af6e0 (deleting: 0x4b0d60 / 0x4b0d70 / 0x4af6b0)

	// ---- OMT element slots
	virtual void update_logic(float millisecs_elapsed);						// ESLOT 15  0x4af7b0

	// ---- CLocalGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x4af9b0
	virtual void UnInitObject(void);										// SLOT 8   0x4af9f0
	virtual void UnInitDatabases(void);										// SLOT 13  0x4afa90
	virtual void Process(float dt);											// SLOT 240 0x4b0a90

	// ---- element-family game slots (vtable offset = slot * 4)
	virtual void SetID(int id);												// ESLOT 43 0x4af7a0  +0xac
	virtual void SetFogColor(void);											// ESLOT 44 0x4af7c0  +0xb0
	virtual void InitGame(void);											// ESLOT 45 0x4afa20  +0xb4
	virtual void UnInitGame(void);											// ESLOT 46 0x4afa60  +0xb8
	virtual void StartSong(void);											// ESLOT 47 0x4afb00  +0xbc
	virtual void ev48(void) {}												// ESLOT 48 0x463890 (empty)
	virtual void InitEffectsDatabase(std::string path);						// ESLOT 49 0x4afb70  +0xc4
	virtual void UnInitEffectsDatabase(void);								// ESLOT 50 0x4afcd0  +0xc8
	virtual void InitObjectDatabase(std::string path);						// ESLOT 51 0x4afd10  +0xcc
	virtual void UnInitObjectDatabase(void);								// ESLOT 52 0x4afe70  +0xd0
	virtual void InitSongDatabase(std::string path);						// ESLOT 53 0x4afeb0  +0xd4
	virtual void UnInitSongDatabase(void);									// ESLOT 54 0x4b0010  +0xd8
	virtual void InitSoundDatabase(std::string path);						// ESLOT 55 0x4b0050  +0xdc
	virtual void UnInitSoundDatabase(void);									// ESLOT 56 0x4b01d0  +0xe0
	virtual void InitTerrainDatabase(std::string path);						// ESLOT 57 0x4b0220  +0xe4
	virtual void UnInitTerrainDatabase(void);								// ESLOT 58 0x4b0380  +0xe8
	virtual void InitVehicleDatabase(std::string path);						// ESLOT 59 0x4b03c0  +0xec
	virtual void UnInitVehicleDatabase(void);								// ESLOT 60 0x4b0520  +0xf0
	virtual void CreateTerrain(void);										// ESLOT 61 0x4b0560  +0xf4
	virtual void CreateTerrainUnLit(void);									// ESLOT 62 0x4b07b0  +0xf8
	virtual void SetTerrainFlatShading(void);								// ESLOT 63 0x4b0b10  +0xfc
	virtual void SetTerrainGouraudShading(void);							// ESLOT 64 0x4b0b60  +0x100
	virtual void SetTerrainLit(void);										// ESLOT 65 0x4b0bb0  +0x104
	virtual void SetTerrainUnLit(void);										// ESLOT 66 0x4b0c50  +0x108
	virtual void SetTerrainVertexLit(void);									// ESLOT 67 0x4b0c00  +0x10c
	virtual void PauseGame(bool pause);										// ESLOT 68 0x4b0a10  +0x110
	virtual bool IsPaused(void);											// ESLOT 69 0x4803b0  +0x114
	virtual void ev70(void);												// ESLOT 70 0x4b0a50  +0x118
	virtual void OnActivate(void) {}										// ESLOT 71 0x41f890  +0x11c (empty) omsg_Activate
	virtual void OnDeactivate(void) {}										// ESLOT 72 0x41f890  +0x120 (empty) omsg_DeActivate
	virtual void SetCamera(CGameObject *camera);							// ESLOT 73 0x4b0a60  +0x124
	virtual void SetInputDevice(short player, unsigned int device);			// ESLOT 74 0x4b0a70  +0x128
	virtual void PlaySong(void);											// ESLOT 75 0x4b0ae0  +0x12c
	virtual void ev76(void) {}												// ESLOT 76 0x463890  +0x130 (empty; called from Process)
	// a checkpoint polygon of the terrain (material "cstart" = 0, "ccheck1".."ccheck9" = 1..9; PhysicsAddTerrain)
	virtual void ev77(int checkpoint, OMedia3DPolygon *poly, OMedia3DShape *shape)	// ESLOT 77 0x4ae440 +0x134 (empty)
		{ (void)checkpoint; (void)poly; (void)shape; }
	virtual void ev78(CGameObject *o, void *a, unsigned int b, unsigned int c)	// ESLOT 78 0x464250 +0x138 (empty)
		{ (void)o; (void)a; (void)b; (void)c; }
	// PhysicsAddTerrain asks for each terrain polygon: true keeps it out of the collision grid
	virtual unsigned char ev79(OMedia3DShape *shape, OMedia3DPolygon *poly, OMedia3DVector center)	// ESLOT 79 0x4b0cc0 +0x13c (returns 0)
		{ (void)shape; (void)poly; (void)center; return 0; }
	virtual void ToggleHelp(void);											// ESLOT 80 0x4b0cd0  +0x140
	virtual void ShowHelp(void) {}											// ESLOT 81 0x463890  +0x144 (empty)
	virtual void HideHelp(void) {}											// ESLOT 82 0x463890  +0x148 (empty)
	virtual int  TouchStart(CGameObject *o, CGameObject *other)	{ (void)o; (void)other; return -1; }	// ESLOT 83 0x4b0d30 +0x14c
	virtual int  TouchEnd(CGameObject *o, CGameObject *other)	{ (void)o; (void)other; return -1; }	// ESLOT 84 0x4b0d30 +0x150
	virtual int  Touch154(CGameObject *o, CGameObject *other)	{ (void)o; (void)other; return -1; }	// ESLOT 85 0x4b0d30 +0x154
	virtual void ev86(void) {}												// ESLOT 86 0x463890  +0x158 (empty)
};

// --------------------------------------------------------------------------- globals of the game type
namespace neg {
extern CGameObject		*g_camera;				// 0x56af7c  active camera (SetCamera); gets v243(dt) each frame
extern unsigned int		 g_song_id;				// 0x56af3c  chunk id of the level song (StartSong)
extern bool				 g_game_paused;			// 0x56af45  PauseGame / IsPaused
extern bool				 g_help_shown;			// 0x56af86  ToggleHelp
extern OMediaElement	*g_terrain;				// 0x56af80  the last '3TER' object CreateTerrain made (its element)
extern unsigned int		 g_input_device[4];		// 0x540598  per player, 'KEYB' initially (SetInputDevice)
extern OMediaDataBase	*g_active_sound_db;		// 0x6f75a0  sound-effect database the sound code plays from

extern OMediaDataBase	*g_effects_db;			// 0x56af48
extern OMediaDataBase	*g_object_db;			// 0x56af4c
extern OMediaDataBase	*g_song_db;				// 0x56af50
extern OMediaDataBase	*g_sound_db;			// 0x56af54
extern OMediaDataBase	*g_terrain_db;			// 0x56af58
extern OMediaDataBase	*g_vehicle_db;			// 0x56af5c
extern OMediaFileStream	*g_effects_stream;		// 0x56af60
extern OMediaFileStream	*g_object_stream;		// 0x56af64
extern OMediaFileStream	*g_song_stream;			// 0x56af68
extern OMediaFileStream	*g_sound_stream;		// 0x56af6c
extern OMediaFileStream	*g_terrain_stream;		// 0x56af70
extern OMediaFileStream	*g_vehicle_stream;		// 0x56af74
}
