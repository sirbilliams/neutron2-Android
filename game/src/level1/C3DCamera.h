/*  C3DCamera - the vehicles' camera probe: a hidden C3DCube (coloured, size 75) kept at the
    viewport. C3DVehicle / C3DCar ev110 place it there and, with the camera collision on
    (g_56af42), slide it against the terrain (PhysicsMoveAgainstTerrain) to keep the camera out
    of walls. The first vehicle constructed makes the one instance (g_54e844); its destructor
    clears the pointer. Its wall and ceiling hits are ignored.
    Reconstructed from Neutron2.exe: constructor 0x499420 (no factory ID, 0x598 bytes),
    destructor 0x499510 (deleting thunks 0x499610 / 0x499600 / 0x499550 / 0x499620).
    Original layout: as C3DCube (vtables 0x517450 / 0x51743c / 0x517400, CLocalGameObject
    0x516f84: 219, 220 overridden, streamer 0x516f74).                                        */
#pragma once
#include "C3DCube.h"

class C3DCamera : public C3DCube
{
	public:
	C3DCamera();															// 0x499420
	virtual ~C3DCamera();													// 0x499510

	virtual void v219(float nx, float ny, float nz, float nw, OMedia3DPolygon *poly);	// SLOT 219 0x4995d0 (nothing)
	virtual void v220(float a, float b, float c, float d, OMedia3DPolygon *poly);		// SLOT 220 0x4995d0 (nothing)
};

namespace neg {
C3DCamera *CameraProbe(void);			// g_54e844 (NULL until a vehicle makes it)
}
