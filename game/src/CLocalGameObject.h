/*  CLocalGameObject - a CGameObject that lives in this game (as opposed to a networked one): adds
    a task name, debug flag, radar flag and an optional private object database.
    Reconstructed from Neutron2.exe: vtable 0x515ab0 (286 slots), constructor 0x48ed70.      */
#pragma once
#include "CGameObject.h"

class OMediaDataBase;
class OMediaStream;
class OMediaViewPort;
class OMediaMatrix_4x4;

class CLocalGameObject : public CGameObject
{
	public:
	CLocalGameObject();														// 0x48ed70
	virtual ~CLocalGameObject();											// SLOT 0  0x48ee50

	virtual void InitObject(void);											// SLOT 7  0x48eec0
	virtual void ResetObject(void);											// SLOT 10 0x48ee80
	virtual void InitPhysics(void);											// SLOT 11 0x48efc0
	virtual void UnInitDatabases(void);										// SLOT 13 0x48f540
	virtual unsigned char v133(void);										// SLOT 133 0x48f010
	virtual void AddRotation(float x, float y, float z);					// SLOT 204 0x48f8d0
	virtual void Process(float dt);											// SLOT 240 0x48efe0
	virtual void v265(float a, float b, float c, float d);					// SLOT 265 0x48ef80
	virtual void DebugLog(const char *fmt, ...);							// SLOT 272 0x48f520
	virtual void CloseObjectDatabase(void);									// SLOT 273 0x48f560
	virtual void OpenObjectDatabase(void);									// SLOT 274 0x48f5f0
	virtual int  CountObjectsWithID(int id);								// SLOT 276 0x48f700
	virtual void PauseAll(bool pause);										// SLOT 279 0x48f6a0
	virtual unsigned char v281(OMediaViewPort *vp, float *pos, float *sx, float *sy);	// SLOT 281 0x48f900 project pos (x, y, z) to the screen (centre-relative, y up); 0: off screen
	virtual void v282(OMediaMatrix_4x4 &m, float *w, float *h) { (void)m; (void)w; (void)h; }	// SLOT 282 0x4ae440 (empty) projection matrix for v281 (C3DSpriteType, C3DAnimated: layer's)
	virtual void WriteState(OMediaStreamOperators &s);						// SLOT 284 0x48fbb0
	virtual void ReadState(OMediaStreamOperators &s);						// SLOT 285 0x48fbf0

#include "gen/CLocalGameObject_slots.inc"

	// FIELD 0x43c 100
	char				m_task_name[100];	// 0x43c  "TaskName" property
	// FIELD 0x4b0 4
	int					m_debug;			// 0x4b0  "Debug" property
	// FIELD 0x4b4 8
	OMediaDataBase		*m_db;				// 0x4b4
	OMediaStream		*m_db_stream;		// 0x4b8
	// FIELD 0x4c0 4
	int					m_show_on_radar;	// 0x4c0  "ShowOnRadar" property
#include "gen/CLocalGameObject_fields.inc"
};
