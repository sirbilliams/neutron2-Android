/*  Negatron - reconstructed source of Jimmy Neutron vs. Jimmy Negatron (PC, AWE Games 2002).
    Globals of Neutron2.exe, named from how the code uses them. The original address is kept next
    to each so the reconstruction can be checked against the binary (tools/decomp).            */
#pragma once
#include "OMediaTypes.h"
#include "OMediaDefMessages.h"

class OMediaApplication;
class OMediaWindow;
class OMediaVideoEngine;
class OMediaSoundEngine;
class OMediaInputEngine;
class OMediaWorld;
class OMediaLayer;
class OMediaViewPort;
class CAweReal;
class CGfx;
class CInput;
class CEditor;
class CGameType;

namespace neg {

// --- application / engine objects
extern CAweReal				*g_app;				// 0x56aec4
extern unsigned int			 g_granny;			// 0x56aea0  Granny "version" handle
extern unsigned short		 g_granny_scale;	// 0x56aea4  = 100
extern OMediaWindow			*g_window;			// 0x56afe0  OMediaFullscreenWindow
extern OMediaVideoEngine	*g_video;			// 0x56afdc
extern OMediaSoundEngine	*g_sound;			// 0x6f75a4
extern OMediaInputEngine	*g_input_engine;	// 0x56b078
extern CEditor				*g_editor;		// 0x56af0c  the level editor/loader (CEditor, 0x82c bytes)
extern CGameType			*g_gametype;	// 0x56aeb0  the running game type (CDefaultGame, CLevel01Game, ...)

// --- world, layers (InitWorld 0x4b1270 / InitLayers 0x4b1710), viewport
extern OMediaWorld			*g_world;			// 0x56afe4
extern OMediaLayer			*g_layer_clear;		// 0x56afd0  L0: clear colour + z, custom view matrix
extern OMediaLayer			*g_layer_back2d;	// 0x56afc0  L1: 2D, behind the world
extern OMediaLayer			*g_layer_world;		// 0x56afc8 (= 0x56afd4) L2: 3D world
extern OMediaLayer			*g_layer_hud2d;		// 0x56afcc  L3: 2D
extern OMediaLayer			*g_layer_top2d;		// 0x56afc4  L4: 2D, on top
extern OMediaViewPort		*g_viewport;		// 0x56afe8  CViewPort (0x120 bytes)

// --- display
extern unsigned short		 g_screen_w;		// 0x5406d8  default 640
extern unsigned short		 g_screen_h;		// 0x5406d6  default 480
extern unsigned short		 g_screen_depth;	// 0x5406d4
extern float				 g_world_far_clip;	// 0x5406dc
extern bool					 g_windowed;		// 0x56afab
extern bool					 g_display_ok;		// 0x56afa8
extern bool					 g_minimized;		// 0x56aeac

// --- input: Windows virtual-key state, filled by CAweReal from key events
extern unsigned char		 g_keys[256];		// 0x56ada0
extern bool					 g_cheat_keys;		// 0x56adb2  (F4 quits when set)

// --- misc
extern unsigned int			*g_random_seed;		// 0x56afd8

}	// namespace neg

// Windows messages / OMT 2002 messages the original code handles
const omt_Message omsg_neg_Activate		= -118;		// OMT 2002
const omt_Message omsg_neg_DeActivate	= -119;		// OMT 2002
// (-600 omsg_ViewPortClicked -> CEditor::ViewportClicked, vtable +0xcc; -500 omsg_ScreenModeChanged -> 0x4ae450)
