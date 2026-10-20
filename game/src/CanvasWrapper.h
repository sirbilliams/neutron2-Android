/*  OMediaCanvasElementWrapper - an OMediaCanvasElement with a serial number (0xd8 bytes in the
    original). Every 2D sprite of the menus and HUD is one.
    Reconstructed from Neutron2.exe: constructor 0x496a40, destructor 0x496b50.             */
#pragma once
#include "OMediaCanvasElement.h"

class OMediaCanvasElementWrapper : public OMediaCanvasElement
{
	public:
	OMediaCanvasElementWrapper();				// 0x496a40
	virtual ~OMediaCanvasElementWrapper();		// 0x496b50

	int		m_c8;			// 0xc8  = -1
	int		m_serial;		// 0xcc  from the counter at 0x54a4d4
};
