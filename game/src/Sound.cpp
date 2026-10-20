/*  Sound and music - see Sound.h. Reconstructed from Neutron2.exe 0x4b7db0..0x4b8e30 and the game
    object helpers 0x48f030..0x48f420.

    OMT 2002 calls as they appear in the binary:
      engine (g_sound)  +0x20 set_nchannels, +0x24 get_nchannels, +0x28 play(ch, sound, queue, loop),
                        +0x40 the channel vector (get_channel)
      channel           +0x0c stop, +0x14 get_playing_sound_info(elapsed, total), +0x18 set_volume,
                        +0x1c set_frequency, +0x20 set_pan, +0x30 current_looped, +0x38 volume
      database          +0x10 get_object(type, name), +0x14 get_object(type, id),
                        +0x1c object_exists(type, name), +0x20 object_exists(type, id),
                        +0x38 purge_object(obj, break_lock), +0x4c set_dbsize_dirty
      'Wave' object     +0x14 lock count (db_unlock)
    The "CSOUND: ..." messages and "Playing channel %d" go to the debug log, compiled empty in
    the shipped build; only ErrorLog lines are kept.                                          */

#include "Sound.h"
#include "Game.h"
#include "Globals.h"
#include "CGameObject.h"
#include "CGameType.h"
#include "gen/globals.h"
#include "OMediaEngineFactory.h"
#include "OMediaSoundEngine.h"
#include "OMediaSoundChannel.h"
#include "OMediaSound.h"
#include "OMediaDataBase.h"
#include "OMediaViewPort.h"
#include "OMedia3DVector.h"
#include "OMediaTrigo.h"
#include <SDL.h>

namespace neg {
CGameObject		*g_chan_object[kSoundChannels];
bool			 g_chan_doppler[kSoundChannels];
int				 g_chan_freq[kSoundChannels];
OMediaSound		*g_chan_sound[kSoundChannels];
float			 g_chan_dist[kSoundChannels];
bool			 g_sound_muted = false;
unsigned short	 g_next_channel = 0;
}
using namespace neg;

static const int kDefaultFrequency = 11025;			// 0x2b11

// The test every sound-effect call starts with.
static inline bool SfxActive(void) { return g_sound && g_sfx_on && !g_sound_muted; }

// Inline in every caller: get_object, the debug-build database call, then db_unlock (the sound
// is not kept locked while it plays).
static OMediaSound *FetchSound(OMediaDataBase *db, int id)
{
	OMediaSound *s = (OMediaSound *)db->get_object(OMediaSound::db_type, (omt_FSChunkID)id);
	if (g_56afaa) db->set_dbsize_dirty();
	if (s) s->db_unlock();
	return s;
}

// 0x4b88e0 with release: the database the sound came from forgets it (purge_object breaking the
// lock), which also stops it wherever else it plays. Port safety, not in the original: a sound
// that is not (any more) in that database is left alone, and other channels' entries for the
// purged sound are cleared - the original kept them dangling and could purge them again.
static void ReleaseSound(OMediaDataBase *db, OMediaSound *s)
{
	if (!s || s->get_database() != db) return;
	for (int i = 0; i < kSoundChannels; i++)
		if (g_chan_sound[i] == s) g_chan_sound[i] = NULL;
	db->purge_object(s, true);
}

// The camera position minus the object's (inline in 0x4b8490 / 0x4b85c0 / 0x4b8720).
static float DistanceToCamera(CGameObject *o)
{
	float p[4];
	o->v195(p);											// CGameObject +0x30c: position
	OMedia3DVector d(g_viewport->get_x() - p[0], g_viewport->get_y() - p[1], g_viewport->get_z() - p[2]);
	return d.quick_magnitude();
}

// Tail shared by the 3D plays: the channel follows the object from now on.
static void Start3D(int ch, CGameObject *o, bool doppler)
{
	g_chan_object[ch] = o;
	g_chan_dist[ch] = DistanceToCamera(o);
	if (g_sound->get_channel(ch))
	{
		g_chan_freq[ch] = kDefaultFrequency;
		g_chan_doppler[ch] = doppler;
	}
	UpdateSound3D(ch);
}

// --------------------------------------------------------------------------- 0x4b7db0

void neg::InitSoundEngine(void)
{
	SDL_Log("InitSoundEngine()");
	// The original creates (and releases) a DirectSound object for the default device first and,
	// without one, turns all sound off and returns. Opening and closing SDL's default device is
	// the same question here.
	SDL_AudioDeviceID probe = 0;
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0)
	{
		SDL_AudioSpec want, have;
		SDL_zero(want);
		want.freq = 44100; want.format = AUDIO_S16SYS; want.channels = 2; want.samples = 1024;
		probe = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
		if (probe) SDL_CloseAudioDevice(probe);
	}
	if (!probe)
	{
		SDL_Log("InitSoundEngine: no audio (%s)", SDL_GetError());
		g_sfx_on = false;
		g_music_on = false;
		g_sound_on = false;
		return;
	}

	OMediaEngineFactory *f = OMediaEngineFactory::get_factory();
	if (f)
	{
		// Every engine with omseaf_NoChannelLimitation replaces the choice (the last one wins);
		// without one the id stays 2. Only a chosen id of 0 falls back to the first
		// engine ("Default Sound Engine"), and with that 0 too no engine is made.
		omt_EngineID id = (omt_EngineID)2;				// the binary's default id (the SDL factory ignores ids)
		bool create = true;
		if (!f->sound_engines.empty())
		{
			for (omt_EngineList::iterator i = f->sound_engines.begin(); i != f->sound_engines.end(); i++)
				if (f->get_sound_engine_attr(*i) & omseaf_NoChannelLimitation) id = *i;		// "NoChannelLimitation"
			if (id == 0)
			{
				id = f->sound_engines.front();			// "Default Sound Engine"
				create = (id != 0);
			}
		}
		if (create)
		{
			g_sound = f->create_sound_engine(id, g_window);
			if (g_sound) g_sound->set_nchannels(kSoundChannels);
		}
		for (int i = 0; i < kSoundChannels; i++)
		{
			g_chan_dist[i] = 0;
			g_chan_object[i] = NULL;
			g_chan_freq[i] = 0;
		}
	}
	g_next_channel = 0;
}

// --------------------------------------------------------------------------- 0x4b7ec0

int neg::PlaySoundOnChannel(int channel, OMediaSound *sound, bool loop)
{
	if (!SfxActive()) return -1;

	bool retry = false;
	int tries = 0;
	do
	{
		// (retry: "CSOUND: bRetry")
		if (channel == -1 || channel >= g_sound->get_nchannels())
		{
			// Round-robin search for a channel with nothing started on it that is not the music
			// channel. Without one, 'channel' stays as given (-1, or an out-of-range number
			// the engine then refuses).
			int n = g_sound->get_nchannels();
			for (short tried = 0; tried < n; )
			{
				int c = (short)g_next_channel;
				OMediaSoundChannel *ch;
				if (!g_chan_sound[c] && c != g_music_channel && (ch = g_sound->get_channel(c)) != NULL)
				{
					ch->set_frequency(kDefaultFrequency);
					int v = g_sfx_volume * 30;
					if (v > 254) v = 255;
					ch->set_volume((short)v);
					ch->set_pan(0);
					channel = (short)g_next_channel;
					g_next_channel = (unsigned short)(((short)g_next_channel + 1) % g_sound->get_nchannels());
					break;
				}
				g_next_channel = (unsigned short)(((short)g_next_channel + 1) % g_sound->get_nchannels());
				tried++;
			}
		}
		if (channel == -1)
		{
			retry = false;								// "CSOUND: channel == -1"
		}
		else if (!sound)
		{
			retry = false;								// "CSOUND: sound == omc_NULL"
		}
		else if (g_sound->play(channel, sound, false, loop))
		{
			g_chan_sound[channel] = sound;
			g_chan_object[channel] = NULL;
			return channel;
		}
		else
		{
			channel = -1;								// "CSOUND: SoundEngine->play(%ld) failed"
			retry = true;
		}
		tries++;
	} while (retry && tries < 3);
	return -1;
}

// --------------------------------------------------------------------------- by name / by id

// 0x4b8080: the body of PlaySound, without its "sound off" handling.
static int PlayNamedSound(int channel, const std::string &name, bool loop)
{
	if (!SfxActive() || !g_active_sound_db) return -1;
	if (!g_active_sound_db->object_exists(OMediaSound::db_type, name))
		return -1;										// "CSOUND: SoundDatabase->object_exists(%s) failed"
	OMediaSound *s = (OMediaSound *)g_active_sound_db->get_object(OMediaSound::db_type, name);
	if (g_56afaa) g_active_sound_db->set_dbsize_dirty();
	if (s) s->db_unlock();
	return PlaySoundOnChannel(channel, s, loop);
}

int neg::PlaySound(int channel, const std::string &name, bool loop)		// 0x4b8380
{
	if (!SfxActive())
	{
		g_cutscene_skipped = true;
		return -1;
	}
	return PlayNamedSound(channel, name, loop);			// "Playing channel %d"
}

int neg::PlaySoundIndex(int channel, int id, bool loop)					// 0x4b8240
{
	if (!SfxActive())
	{
		g_cutscene_skipped = true;
		return -1;
	}
	if (!g_active_sound_db || !g_active_sound_db->object_exists(OMediaSound::db_type, (omt_FSChunkID)id)) return -1;
	int ch = PlaySoundOnChannel(channel, FetchSound(g_active_sound_db, id), loop);
	ErrorLog("ERROR: Playing a channel with an index, using index # %d", id);	// on every call, as shipped
	return ch;
}

// 0x4b82f0: starts the sound on a free channel and stops it at once - it is loaded into the
// database cache as a side effect (C3DJimmy uses it).
void neg::LoadSound(const std::string &name)
{
	StopChannel(PlaySound(-1, name, false), false);
}

// --------------------------------------------------------------------------- 3D sounds

int neg::PlaySound3D(CGameObject *o, int channel, const std::string &name, bool loop)	// 0x4b8490
{
	int ch = PlaySound(channel, name, loop);
	if (ch != -1) Start3D(ch, o, true);
	return ch;
}

int neg::PlaySound3DIndex(CGameObject *o, int channel, int id, bool loop)		// 0x4b85c0
{
	if (!SfxActive())
	{
		g_cutscene_skipped = true;
		return -1;
	}
	if (!g_active_sound_db || !g_active_sound_db->object_exists(OMediaSound::db_type, (omt_FSChunkID)id)) return -1;
	int ch = PlaySoundOnChannel(channel, FetchSound(g_active_sound_db, id), loop);
	ErrorLog("ERROR: Playing a channel with an index, using index # %d", id);
	if (ch != -1) Start3D(ch, o, true);
	return ch;
}

int neg::PlaySound3DNoDoppler(CGameObject *o, int channel, const std::string &name, bool loop)	// 0x4b8720
{
	int ch = PlaySound(channel, name, loop);
	if (ch != -1) Start3D(ch, o, false);
	return ch;
}

// 0x4b8b60: volume, pan and (Doppler) frequency of a channel that follows an object. Run when the
// sound starts and then every frame from UpdateSounds.
void neg::UpdateSound3D(int ch)
{
	if (!SfxActive()) return;
	CGameObject *o = g_chan_object[ch];
	OMediaSoundChannel *chan = g_sound->get_channel(ch);
	if (!o) return;

	float p[4];
	o->v195(p);
	// Quirk: the compiled code reuses the position buffer for the camera's y and z and then
	// subtracts the object's y from itself, so the distance here is measured in x/z only (the one
	// stored by the play call includes the height).
	OMedia3DVector d(g_viewport->get_x() - p[0], p[1] - p[1], g_viewport->get_z() - p[2]);
	float dist = d.quick_magnitude();
	if (dist == 0.0f || dist != dist) return;			// (fcomp: NaN compares equal)
	d.normalize();

	// Pan: the direction to the camera against the camera's right, (1,0,0) turned by its y angle
	// (the rotation is written out in full in the binary, with sin 0 and cos 0 from the table).
	const OMediaCosSinTable &t = OMediaCosSinTable::global;
	omt_Angle a = (omt_Angle)(g_viewport->get_angley() & 0x3fff);
	float s0 = t.sin(0), c0 = t.cos(0), sa = t.sin(a), ca = t.cos(a);
	float ss = -(s0 * -s0);
	float r0 = c0 * ca - ss * sa;						// stored as a float, the rest stays on the x87 stack
	double r1 = -(double)s0 * c0;
	double r2 = (double)ss * ca + (double)c0 * sa;
	double side = (double)d.z * r2 + (double)d.y * r1 + (double)r0 * d.x;
	chan->set_pan((long)(side * 1000.0));

	// Volume: 255 less g_sound_falloff per unit, never below 0, scaled by the effect volume (0..10).
	double v = 255.0 - (double)g_sound_falloff * dist;
	if (!(v > 0.0)) v = 0.0;
	int vi = (int)v;
	chan->set_volume((short)(int)((double)g_sfx_volume * 25.5 * (double)(1.0f / 255.0f) * vi));

	if (g_chan_doppler[ch])
	{
		// Quirk: the shift is (distance change * 0.75) taken off the base frequency when the object
		// moved away and added to it otherwise - the change is negative then, so the pitch drops
		// either way. And the delta, not the distance, is what is stored for the next frame.
		int delta = (int)(((double)dist - g_chan_dist[ch]) * 0.75);
		if (dist > g_chan_dist[ch]) chan->set_frequency((unsigned int)(g_chan_freq[ch] - delta));
		else chan->set_frequency((unsigned int)(g_chan_freq[ch] + delta));
		g_chan_dist[ch] = (float)delta;
	}
}

// --------------------------------------------------------------------------- 0x4b8850 music

int neg::PlayMusic(OMediaDataBase *db, int channel, int id, bool loop)
{
	if (!g_sound || !g_music_on)
	{
		g_cutscene_skipped = true;
		return g_music_channel;
	}
	StopChannel(g_music_channel, false);
	if (db)
	{
		g_music_db = db;
		// (no object_exists test here; a missing song ends in "sound == omc_NULL")
		g_music_channel = PlaySoundOnChannel(channel, FetchSound(db, id), loop);
	}
	return g_music_channel;
}

// --------------------------------------------------------------------------- channels

// 0x4b88e0. 'release' also drops the sound from its database (the music database for the music
// channel, which is then forgotten, the sound-effect database otherwise).
void neg::StopChannel(int channel, bool release)
{
	if (!g_sound || channel < 0 || channel >= g_sound->get_nchannels()) return;
	OMediaSoundChannel *c = g_sound->get_channel(channel);
	if (c) c->stop();
	if (release)
	{
		if (channel == g_music_channel)
		{
			if (g_music_db)
			{
				ReleaseSound(g_music_db, g_chan_sound[channel]);
				g_music_db = NULL;
			}
		}
		else if (g_active_sound_db)
			ReleaseSound(g_active_sound_db, g_chan_sound[channel]);
	}
	g_chan_sound[channel] = NULL;
	if (channel == g_music_channel) g_music_channel = -1;
}

void neg::StopAllChannels(void)											// 0x4b8990
{
	if (!g_sound) return;
	for (short i = 0; i < g_sound->get_nchannels(); i++) StopChannel(i, false);
}

// 0x4b89d0: a sound is running (OMT reports a total time while it lasts) or loops.
bool neg::IsChannelPlaying(int channel)
{
	if (!g_sound || channel < 0 || channel >= g_sound->get_nchannels()) return false;
	OMediaSoundChannel *c = g_sound->get_channel(channel);
	if (!c) return false;
	unsigned int elapsed = 0, total = 0;
	c->get_playing_sound_info(elapsed, total);
	return total != 0 || c->get_current_looped();
}

// 0x4b8a30, once per frame from CViewPort::render_viewport: frees the channels whose sound ended,
// moves 3D sounds and keeps the music at the music volume.
void neg::UpdateSounds(void)
{
	if (!SfxActive()) return;
	for (short i = 0; i < g_sound->get_nchannels(); i++)
	{
		if (!IsChannelPlaying(i))
		{
			if (g_chan_sound[i])
			{
				g_chan_sound[i] = NULL;
				if (i == g_music_channel) g_music_channel = -1;
			}
		}
		else if (g_chan_object[i])
			UpdateSound3D(i);
		else if (i == g_music_channel && g_sound->get_channel(i))
		{
			OMediaSoundChannel *c = g_sound->get_channel(i);
			int v = g_music_volume * 30;
			if (v > 254) v = 255;
			if (c->get_volume() != v) c->set_volume((short)v);
		}
	}
}

// 0x4b8dc0: called by CGameObject's destructor.
void neg::StopObjectSounds(CGameObject *o)
{
	if (!SfxActive()) return;
	for (short i = 0; i < g_sound->get_nchannels(); i++)
		if (g_chan_object[i] == o)
		{
			g_chan_object[i] = NULL;
			StopChannel(i, true);
		}
}

// 0x4b8e30. Quirk: not clamped, so volumes 9 and 10 give 270 / 300, which DirectSound refuses
// (the channel keeps its volume; the SDL backend does the same).
void neg::SetChannelVolume(short channel, int volume)
{
	if (!SfxActive() || channel == -1) return;
	g_sound->get_channel(channel)->set_volume((short)(volume * 30));
}

// --------------------------------------------------------------------------- 0x48f030..0x48f420

int neg::PlayGameSoundIndex(int channel, int id, bool loop)						// 0x48f030
{
	if (!g_sfx_db_loaded || !g_sfx_volume) return -1;
	int ch = PlaySoundIndex(channel, id, loop);
	SetChannelVolume((short)ch, (short)g_sfx_volume);
	return ch;
}

int neg::PlayGameSound3DIndex(CGameObject *o, int channel, int id, bool loop)	// 0x48f080
{
	if (!g_sfx_db_loaded || !g_sfx_volume) return -1;
	int ch = PlaySound3DIndex(o, channel, id, loop);
	SetChannelVolume((short)ch, (short)g_sfx_volume);
	return ch;
}

void neg::StopGameSound(int channel, bool release)								// 0x48f0d0
{
	if (g_sfx_db_loaded) StopChannel(channel, release);
}

int neg::PlayGameSound3DNoDoppler(CGameObject *o, int channel, const std::string &name, bool loop)	// 0x48f0f0
{
	if (!g_sfx_db_loaded || !g_sfx_volume) return -1;
	int ch = PlaySound3DNoDoppler(o, channel, name, loop);
	SetChannelVolume((short)ch, (short)g_sfx_volume);
	return ch;
}

int neg::PlayGameSound3D(CGameObject *o, int channel, const std::string &name, bool loop)	// 0x48f200
{
	if (!g_sfx_db_loaded || !g_sfx_volume) return -1;
	int ch = PlaySound3D(o, channel, name, loop);
	SetChannelVolume((short)ch, (short)g_sfx_volume);
	return ch;
}

int neg::PlayGameSound(int channel, const std::string &name, bool loop)			// 0x48f420
{
	if (!g_sfx_db_loaded || !g_sfx_volume) return -1;
	int ch = PlaySound(channel, name, loop);
	SetChannelVolume((short)ch, (short)g_sfx_volume);
	return ch;
}
