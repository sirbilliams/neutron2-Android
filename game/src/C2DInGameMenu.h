/*  C2DInGameMenu - the PDA menus: main menu, pause menu, options, inventory, help, level select.
    Reconstructed from Neutron2.exe: vtable 0x4ca4c4 (348 slots), constructor 0x401380,
    destructor 0x403fa0. Created by C3DJimmy's constructor (0x443830) and kept in 0x545e78.
    Pages and items come from dat\menu.dat; artwork from omt\screens.omt.

    The class is written in parts (menu/C2DInGameMenu_<part>.{inc,cpp}); each part declares the
    slots it implements in its .inc with `// SLOT <n>`. Mechanical slots are generated.      */
#pragma once
#include "CLocalGameObject.h"
#include "gen/globals.h"
#include "InterfaceElement.h"

class OMediaFileStream;

class C2DInGameMenu : public CLocalGameObject
{
	public:
	C2DInGameMenu();														// 0x401380
	virtual ~C2DInGameMenu();												// SLOT 0  0x403f80 / 0x403fa0

#include "menu/C2DInGameMenu_types.inc"
#include "menu/C2DInGameMenu_a.inc"
#include "menu/C2DInGameMenu_b.inc"
#include "menu/C2DInGameMenu_c.inc"
#include "menu/C2DInGameMenu_d.inc"
#include "gen/C2DInGameMenu_slots.inc"

	// TYPE 0x4c8 OMediaFileStream *
	// FIELD 0x524 100
	char			f_524[100];			// 0x524  a name, compared with _stricmp
#include "gen/C2DInGameMenu_fields.inc"
};

namespace neg {
void SetGameMode(int mode);			// 0x485360 (C2DInGameMenu_b.cpp): 0 Starting 1 Loading 2 Saving 3 CutScene 4 Menu 5 Playing 6 Paused 9 None
void SetLevelItems(int which);		// 0x40a860 (C2DInGameMenu_b.cpp): the level items page for a level-progress value
}
