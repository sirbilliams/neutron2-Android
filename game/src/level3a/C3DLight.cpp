/*  C3DLight - see C3DLight.h. Reconstructed from Neutron2.exe 0x49a280..0x49a910. */
#include "C3DLight.h"
#include "../Globals.h"
#include "../gen/globals.h"
#include "OMediaLayer.h"
#include "OMediaWorld.h"

using namespace neg;

C3DLight::C3DLight()										// 0x49a280
{
	SetName((char *)"C3DLIGHT");							// 0x4ab1b0
	Log("C3DLight()");
	m_specular[0] = m_specular[1] = m_specular[2] = 1.0f;
	m_diffuse[0] = m_diffuse[1] = m_diffuse[2] = 1.0f;
	m_ambient[0] = m_ambient[1] = m_ambient[2] = 0;
	m_type = 0;
	m_range = 10000.0f;
	m_attenuation = 1.0f;
	C3DLight::InitObject();									// 0x49a4a0 (called directly)
	// (then a call to the empty 0x463890)
}

C3DLight::~C3DLight() {}									// 0x49a3e0

// --------------------------------------------------------------------------- element-family slots

void C3DLight::SetObjectID(int id)							// 0x49a490
{
	set_id(id);												// element +0x78
	m_object_id = id;										// +0x404
}

void C3DLight::InitLight(float a, float r, float g, float b)	// 0x49a650
{
	Log("InitLight()");
	set_diffuse(OMediaFARGBColor(a, r, g, b));				// element +0xc0..+0xcc
}

// --------------------------------------------------------------------------- CGameObject-family slots

void C3DLight::InitObject(void)								// 0x49a4a0
{
	Log("InitObject()");
	CLocalGameObject::InitObject();
	SetObjectID('3LIG');									// element +0xac
	m_object_id = get_id();									// (again, from the element)
	link(g_world);											// element +0x2c
	link_layer(g_layer_world);								// element +0x34
	CLocalGameObject::InitObject();							// (twice, as in the original)
	RegisterProperty("LightType",    &m_type,         PROP_SPECIFIC, 0);
	RegisterProperty("LightRange",   &m_range,        PROP_FLOAT, 0);
	RegisterProperty("CAttenuation", &m_attenuation,  PROP_FLOAT, 0);
	RegisterProperty("DifRed",       &m_diffuse[0],   PROP_FLOAT, 0);
	RegisterProperty("DifGreen",     &m_diffuse[1],   PROP_FLOAT, 0);
	RegisterProperty("DifBlue",      &m_diffuse[2],   PROP_FLOAT, 0);
	RegisterProperty("SpecRed",      &m_specular[0],  PROP_FLOAT, 0);
	RegisterProperty("SpecGreen",    &m_specular[1],  PROP_FLOAT, 0);
	RegisterProperty("SpecBlue",     &m_specular[2],  PROP_FLOAT, 0);
	RegisterProperty("AmbRed",       &m_ambient[0],   PROP_FLOAT, 0);
	RegisterProperty("AmbGreen",     &m_ambient[1],   PROP_FLOAT, 0);
	RegisterProperty("AmbBlue",      &m_ambient[2],   PROP_FLOAT, 0);
	v258();													// +0x408
}

void C3DLight::Process(float dt)							// 0x49a6c0
{
	CLocalGameObject::Process(dt);
}

void C3DLight::v247(void)									// 0x49a6d0
{
	float p[4];
	v195(p);												// +0x30c position
	place(p[0], p[1], p[2]);								// element +0x44..+0x4c
}

void C3DLight::v248(void)									// 0x49a720
{
	CGameObject::v248();									// 0x4ae3a0
	float a[4];
	v201(a);												// +0x324 angles (degrees)
	const float k = 16384.0f / 360.0f;						// 0x4cb228
	set_angle((short)((int)(a[0] * k) & 0x3fff),			// element +0x50..+0x54
			  (short)((int)(a[1] * k) & 0x3fff),
			  (short)((int)(a[2] * k) & 0x3fff));
}

void C3DLight::v258(void)									// 0x49a7a0
{
	CLocalGameObject::v258();
	if (m_type == 0) set_light_type(omclt_Point);			// element +0x94
	else if (m_type == 1) set_light_type(omclt_Directional);
	else if (m_type == 2) set_light_type(omclt_Spot);
	set_diffuse(OMediaFARGBColor(0, m_diffuse[0], m_diffuse[1], m_diffuse[2]));	// element +0xc0
	set_diffuse(OMediaFARGBColor(0, m_specular[0], m_specular[1], m_specular[2]));	// (the same place again)
	set_ambient(OMediaFARGBColor(0, m_ambient[0], m_ambient[1], m_ambient[2]));	// element +0xb0
	set_range(m_range);										// element +0xa4
	set_constant_attenuation(m_attenuation);				// element +0x98
}

namespace neg {
CGameObject *New_C3DLight(void)	{ return new C3DLight; }		// factory '3LIG' (0x604 bytes)
}
