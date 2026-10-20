#include "Globals.h"

namespace neg {

CAweReal			*g_app = 0;
unsigned int		 g_granny = 0;
unsigned short		 g_granny_scale = 0;
OMediaWindow		*g_window = 0;
OMediaVideoEngine	*g_video = 0;
OMediaSoundEngine	*g_sound = 0;
OMediaInputEngine	*g_input_engine = 0;
CJimmyGame			*g_game = 0;

OMediaWorld			*g_world = 0;
OMediaLayer			*g_layer_clear = 0;
OMediaLayer			*g_layer_back2d = 0;
OMediaLayer			*g_layer_world = 0;
OMediaLayer			*g_layer_hud2d = 0;
OMediaLayer			*g_layer_top2d = 0;
OMediaViewPort		*g_viewport = 0;

unsigned short		 g_screen_w = 640;
unsigned short		 g_screen_h = 480;
unsigned short		 g_screen_depth = 16;
float				 g_world_far_clip = 15000.0f;
bool				 g_windowed = false;
bool				 g_display_ok = false;
bool				 g_minimized = false;

unsigned char		 g_keys[256];
bool				 g_cheat_keys = false;

unsigned int		*g_random_seed = 0;

}	// namespace neg
