/*  C3DLight ('3LIG') - a game object that is an OMT light, linked into the world and the 3D world
    layer. Properties: "LightType" (0 point, 1 directional, 2 spot), "LightRange" (10000),
    "CAttenuation" (constant attenuation, 1), "DifRed/Green/Blue" (1), "SpecRed/Green/Blue"
    (1), "AmbRed/Green/Blue" (0). Applying them (v258) puts the diffuse colour and then the
    specular one into the light's diffuse colour (alpha 0): the specular colour wins and the
    light's own specular colour is never set (a quirk of the original). The mini sub makes one
    for its headlight.
    Reconstructed from Neutron2.exe: constructor 0x49a280 (factory '3LIG', 0x604 bytes),
    destructor 0x49a3e0 (only resets the vtables, then ~CLocalGameObject and ~OMediaLight;
    deleting thunks 0x49a8e0 / 0x49a3b0 / 0x49a8f0). Bases OMediaLight (element vtable 0x517fc4,
    45 slots: OMediaLight's 43 + 43 SetObjectID, 44 InitLight; listener 0x517fb0 at +0x30) and
    CLocalGameObject at +0x104 (vtable 0x517b34: 7, 240, 247, 248, 258 overridden), streamer
    0x517b24 (OMediaLight's read_class / write_class). Own members 0x5cc..0x5fc, vtordisp 0x5fc,
    streamer 0x600. OMediaLight 2002 fields used: +0x94 type, +0x98 constant attenuation,
    +0xa4 range, +0xb0 ambient, +0xc0 diffuse (r, g, b, a).                                   */
#pragma once
#include "OMediaLight.h"
#include "../CLocalGameObject.h"

class C3DLight : public OMediaLight, public CLocalGameObject
{
	public:
	C3DLight();																// 0x49a280
	virtual ~C3DLight();													// 0x49a3e0

	// ---- element-family game slots
	virtual void SetObjectID(int id);										// ESLOT 43 0x49a490 element id and ObjectID
	virtual void InitLight(float a, float r, float g, float b);				// ESLOT 44 0x49a650 "InitLight()": the diffuse colour

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x49a4a0 link, properties, apply
	virtual void Process(float dt);											// SLOT 240 0x49a6c0 (CLocalGameObject's)
	virtual void v247(void);												// SLOT 247 0x49a6d0 element position from the object
	virtual void v248(void);												// SLOT 248 0x49a720 element angles from the object
	virtual void v258(void);												// SLOT 258 0x49a7a0 properties -> the light

	float		m_range;				// 0x5cc  "LightRange" (10000)
	float		m_attenuation;			// 0x5d0  "CAttenuation" (1)
	int			m_type;					// 0x5d4  "LightType" (0 point, 1 directional, 2 spot)
	float		m_diffuse[3];			// 0x5d8  "DifRed/Green/Blue" (1)
	float		m_specular[3];			// 0x5e4  "SpecRed/Green/Blue" (1)
	float		m_ambient[3];			// 0x5f0  "AmbRed/Green/Blue" (0)
};
