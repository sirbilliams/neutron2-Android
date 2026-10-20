/*  C3DSky ('3SKY') - the sky dome of a level: a shape from a sky database ("omt\wintersky.omt",
    ...), drawn in the clear layer (g_layer_clear, which has its own view matrix, so the sky
    stays around the camera). The level's InitGame creates it, loads it (ev58), places it at the
    origin and sets its radius; CJimmyGame turns it a little each frame when m_sky_rotates.

    Reconstructed from Neutron2.exe: constructor 0x49b8a0, destructor 0x49ba00 (thunks 0x49c090 /
    0x49c080 / 0x49b9d0 / 0x49c0a0). Same layout as C3DObject (0x590 bytes + the streamer,
    factory size 0x594), no members of its own. The sky database is kept in globals (one sky at
    a time): 0x54e85c the file stream, 0x54e858 the database, 0x54e860 the "alt" flag.

    Element vtable 0x5190a4: C3DObject's 56 slots plus 56..58 (below). CGameObject vtable
    0x518bd8 overrides slots 0, 7, 8 and 11 (InitPhysics: the shared empty 0x463890).          */
#pragma once
#include "C3DObject.h"
#include <string>

class C3DSky : public C3DObject
{
	public:
	C3DSky();																// 0x49b8a0
	virtual ~C3DSky();														// SLOT 0  0x49ba00

	// ---- element-family game slots
	virtual void ev56(std::string file);									// ESLOT 56 0x49bb30 +0xe0 open the sky database
	void OpenSkyDatabase(std::string file) { ev56(file); }
	virtual void ev57(void);												// ESLOT 57 0x49bc90 +0xe4 close the sky database
	void CloseSkyDatabase(void) { ev57(); }
	virtual void ev58(std::string file, bool alt);							// ESLOT 58 0x49bcc0 +0xe8 load the sky
	void LoadSky(std::string file, bool alt) { ev58(file, alt); }

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7  0x49baa0
	virtual void UnInitObject(void);										// SLOT 8  0x49baf0
	virtual void InitPhysics(void) {}										// SLOT 11 0x463890 (empty)
};
