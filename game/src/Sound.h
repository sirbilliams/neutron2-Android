/*  Sound and music - the game's "CSOUND" layer over OMT's sound engine (g_sound), reconstructed
    from Neutron2.exe 0x4b7db0..0x4b8e30, plus the gated helpers the game objects call
    (0x48f030..0x48f420).

    Channels: g_sound has 32 channels. A play call either names a channel or passes -1 (or any
    number >= the channel count) to take the next free one, searched round-robin from
    g_next_channel; a channel picked that way is reset to 11025 Hz, the sound-effect volume and
    centre pan (a named channel keeps its settings). Calls return the channel or -1.

    Sounds come from g_active_sound_db (CGameType.h, 0x6f75a0) as 'Wave' objects, by name or by
    chunk id. The song plays from the database given to PlayMusic, which becomes g_music_db, on
    g_music_channel. A "3D" sound follows a game object: each frame UpdateSounds (called by the
    viewport) sets its volume from the distance to the camera (g_sound_falloff per unit, times
    the sound-effect volume), its pan from the camera's heading and, for the Doppler variants,
    shifts its frequency with the change in distance. Music volume is applied each frame too.

    Volumes in the options are 0..10 (g_music_volume, g_sfx_volume); the channel volume is that
    times 30, at most 255 (SetChannelVolume does not clamp, see there).

    Everything is off while g_sound is NULL, g_sfx_on is 0 or g_sound_muted is set (never set in
    the shipped game); music only needs g_sound and g_music_on. A play request refused because
    the sound is off sets g_sound_skipped, which cut-scenes and triggers read as "do not wait for
    this voice".

    The Play/Stop "Game" helpers (0x48f...) are what game objects use: they do nothing until the
    sound-effect database is loaded (g_sfx_db_loaded) or while the sound-effect volume is 0, and
    set the channel volume after starting the sound.                                          */
#pragma once
#include <string>

class CGameObject;
class OMediaSound;
class OMediaDataBase;

namespace neg {

enum { kSoundChannels = 32 };			// InitSoundEngine reserves 32 channels

// Channel bookkeeping (index = channel)
extern CGameObject		*g_chan_object[kSoundChannels];		// 0x6f7334  object a 3D sound follows (NULL: not 3D)
extern bool				 g_chan_doppler[kSoundChannels];	// 0x6f73b4  frequency follows the distance
extern int				 g_chan_freq[kSoundChannels];		// 0x6f73d4  base frequency of a 3D sound (11025)
extern OMediaSound		*g_chan_sound[kSoundChannels];		// 0x6f7458  sound started on the channel (NULL: free)
extern float			 g_chan_dist[kSoundChannels];		// 0x6f751c  distance to the camera when last updated (see UpdateSound3D)
extern bool				 g_sound_muted;						// 0x6f75a8  never set: would silence everything
extern unsigned short	 g_next_channel;					// 0x6f75ac  where the search for a free channel starts

// Start-up
void InitSoundEngine(void);																	// 0x4b7db0

// Playing (each returns the channel, or -1)
int  PlaySoundOnChannel(int channel, OMediaSound *sound, bool loop);						// 0x4b7ec0
int  PlaySound(int channel, const std::string &name, bool loop);							// 0x4b8380 (0x4b8080)
int  PlaySoundIndex(int channel, int id, bool loop);										// 0x4b8240
void LoadSound(const std::string &name);													// 0x4b82f0
int  PlaySound3D(CGameObject *o, int channel, const std::string &name, bool loop);			// 0x4b8490
int  PlaySound3DIndex(CGameObject *o, int channel, int id, bool loop);						// 0x4b85c0
int  PlaySound3DNoDoppler(CGameObject *o, int channel, const std::string &name, bool loop);	// 0x4b8720
int  PlayMusic(OMediaDataBase *db, int channel, int id, bool loop);						// 0x4b8850

// Channels
void StopChannel(int channel, bool release);												// 0x4b88e0
void StopAllChannels(void);																	// 0x4b8990
bool IsChannelPlaying(int channel);															// 0x4b89d0
void UpdateSounds(void);																	// 0x4b8a30 (once per frame)
void UpdateSound3D(int channel);															// 0x4b8b60
void StopObjectSounds(CGameObject *o);														// 0x4b8dc0
void SetChannelVolume(short channel, int volume);											// 0x4b8e30 (volume 0..10)

// Game-object helpers: only with g_sfx_db_loaded and g_sfx_volume != 0
int  PlayGameSoundIndex(int channel, int id, bool loop);									// 0x48f030
int  PlayGameSound3DIndex(CGameObject *o, int channel, int id, bool loop);				// 0x48f080
void StopGameSound(int channel, bool release);												// 0x48f0d0 (only needs g_sfx_db_loaded)
int  PlayGameSound3DNoDoppler(CGameObject *o, int channel, const std::string &name, bool loop);	// 0x48f0f0
int  PlayGameSound3D(CGameObject *o, int channel, const std::string &name, bool loop);	// 0x48f200
int  PlayGameSound(int channel, const std::string &name, bool loop);						// 0x48f420

}
