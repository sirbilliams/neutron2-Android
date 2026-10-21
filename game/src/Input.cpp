/*  The input module of Neutron2.exe (0x4b3260..0x4b3a03): CInput, FindDevice and the per-frame
    joystick poll. Reconstructed from the machine code.

    The keyboard and the mouse do not go through here: they arrive as window messages
    (CAweReal::listen_to_message fills g_keys[]). This module only reads one OMT HID device - the
    first joystick FindDevice finds - into the g_joy_* globals, which the player, vehicle and menu
    code test next to their key bindings:
      X  0x4659f0 turn left (< -0.5), 0x465a60 turn right (> 0.5), C3DVehicle slot 112 0x49f440
      Y  0x465910 go (> 0.5), 0x465980 back (< -0.5), C3DCar slot 126, C3DVehicle slot 112
      button 0  0x465750 action, 0x465ad0 cut-scene bypass, C3DPlayerObject 0x465650, C3DJeep 125/126,
                C3DVehicle 112/125/126/127        button 1  0x465830 speed, 0x4658a0 jump,
                C3DPlayerObject 0x4655d0, C3DCar 126, C3DGoddard 243, C3DVehicle 126/127
      button 2  C3DVehicle 112/127                button 3  0x465b40 inventory, 0x407510 pickup
      button 4  0x407240 pause menu               button 8  0x4657c0 turn around
      button 9  0x406ee0 help
    Nothing reads Z, rX, rY, rZ, the POV angle, g_joy_prev_buttons or g_joy_any_button.
    (That list is every read of 0x56b020..0x56b071 in the exe outside this module; buttons 5..7 and
    10..19 are polled but never read.) The JOY_01..JOY_09 names in dat\keys.txt play no part: the
    exe's TextToKeys (0x4660d0) has no JOY_ names, so they log "not found" and count as KEY_NONE,
    on Windows as here. Which physical button carries which number is up to the HID layer
    (engine/sdl/OMediaSDLInput.cpp; the Thor layout is in docs/controls.md).

    0x4b3260 CInput::CInput      0x4b3370 CInput::~CInput     0x4b33f0 CreateInputEngine
    0x4b3410 CInput::Release     0x4b3460 FindDevice          0x4b35d0 UpdateInput (the poll)
    Static initialisers: 0x4b31b0 constructs g_input_device_name (destroyed at exit by 0x4b31e0),
    0x4b3240 the OMedia3DVector constructors of 0x56b038 / 0x56b048 (they only zero w).
    0x4b30c0 / 0x4b30f0, just before, are std::list helpers of the CGfx file (SetDisplayMode,
    UnInitDatabase), not part of this module.

    OMT 2002 vs 2.5: the original reads the HID objects by offset; the 2.5 members are used here.
      device  +0x08 flags, +0x14 product_name, +0x38 lock_count (is_locked), +0x3c fftype_list
              (is_force_feedback_started); vtable +4 lock, +8 unlock, +0xc start_force_feedback,
              +0x10 stop_force_feedback
      element +0x04 type, +0x10 name, +0x20 axis_type, +0x24 the 16-bit member of the key union
              (command_key): OMT 2002's DirectInput engine stores the button number there
              ("Button %d" -> %d, -1 if the name has none); vtable +4 read_data
      engine  +0x44 the HID list (get_hid())                                                      */

#include "Game.h"
#include "gen/globals.h"
#include "OMediaEngineFactory.h"
#include "OMediaInputEngine.h"
#include "OMediaViewPort.h"
#include "Mouse.h"
#include "port/Widescreen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

using namespace neg;

std::string neg::g_input_device_name;		// 0x56b010 (static initialiser 0x4b31b0)

enum { JOY_BUTTONS = 20, JOY_CLEAR_FRAMES = 30 };

// g_joy_buttons[20] at 0x56b05c: the consumers read single bytes of it (0x56b05c..0x56b065),
// which keep their own names in gen/globals.h.
static unsigned char *JoyButtons(void) { return &g_joy_button0; }

// The first 30 polls clear the buttons (with or without a device): dword, dword, dword, dword,
// word, byte = 19 bytes, so button 19 (0x56b06f) is not cleared (sic).
static void ClearJoyButtons(void)
{
	memset(JoyButtons(), 0, 19);
}

// Inlined in FindDevice, the destructor and Release.
static void UnlockDevice(void)
{
	if (g_joy_device->is_force_feedback_started()) g_joy_device->stop_force_feedback();	// vtable +0x10
	g_joy_device->unlock();												// vtable +8
	g_joy_device = NULL;
}

static OMediaInputEngine *NewInputEngine(void)
{
	// OMediaEngineFactory vtable +0x18, engine id 6
	return OMediaEngineFactory::get_factory()->create_input_engine(ommeic_Best, g_window);
}

// --------------------------------------------------------------------------- CInput

CInput::CInput()															// 0x4b3260
{
	// "CInput()" (debug log)
	g_input_engine = NewInputEngine();
	g_input_device_name = g_empty_string;		// 0x545f14 ""
}

CInput::~CInput()															// 0x4b3370
{
	// "~CInput()" (debug log)
	if (g_joy_device && g_joy_device->is_locked()) UnlockDevice();
	if (g_input_engine)
	{
		delete g_input_engine;					// vtable +0 (deleting destructor)
		g_input_engine = NULL;
	}
}

void CInput::CreateInputEngine(void)										// 0x4b33f0
{
	// sic: the engine already there is not deleted (never called)
	g_input_engine = NewInputEngine();
}

void CInput::Release(void)													// 0x4b3410
{
	if (g_joy_device && g_joy_device->is_locked()) UnlockDevice();
	if (g_input_engine)
	{
		delete g_input_engine;
		g_input_engine = NULL;
	}
}

// --------------------------------------------------------------------------- 0x4b3460

// Only caller: the C2DInGameMenu constructor (0x401380), with 2 (omhidtc_Joystick). It throws the
// input engine away and makes a new one, so the devices are enumerated again each time a menu
// object is made; a joystick plugged in later is not seen until then.
bool neg::FindDevice(int type)
{
	// "FindDevice(%d)" (debug log)
	if (g_joy_device) UnlockDevice();			// (no is_locked test here, unlike the destructor)
	if (g_input_engine)
	{
		delete g_input_engine;
		g_input_engine = NULL;
	}
	g_input_engine = NewInputEngine();

	omt_HIDList *hid = g_input_engine->get_hid();
	g_joystick_count = 0;
	for (omt_HIDList::iterator i = hid->begin(); i != hid->end(); i++)
	{
		OMediaHumanInterfaceDevice *d = *i;
		// "DEVICE=%s" product_name (debug log)
		if (d->type == omhidtc_Joystick || d->type == omhidtc_Wheel) g_joystick_count++;	// sic: only the devices up to the one taken
		// sic: the first device of the type whose name is NOT g_input_device_name. That name is
		// always "" (CInput's constructor), so this takes the first one with a name.
		if (d->type == type && d->product_name != g_input_device_name)
		{
			g_joy_device = d;
			g_joy_elements = d->lock();			// vtable +4
			// bit 1 of the 2002 flags; no OMT 2002 engine sets it (2.5's omhidf_ForceFeedback is bit 0)
			if (g_joy_device->flags & 2) g_joy_device->start_force_feedback();	// vtable +0xc
			return true;
		}
	}
	return false;
}

// Port: test hook. NEG_JOY_LOG=1 logs what the game sees of the joystick whenever it changes
// (the pressed button numbers and the X/Y axes rounded to tenths), with the keys down in g_keys
// (Windows key codes, hex) and the mouse buttons.
static void LogJoyChanges(void)
{
	static int on = -1;
	if (on < 0) on = getenv("NEG_JOY_LOG") != NULL;
	if (!on) return;
	static char last[256];
	char now[256];
	int n = snprintf(now, sizeof now, "x %.1f y %.1f buttons", (double)g_joy_x, (double)g_joy_y);
	for (int b = 0; b < JOY_BUTTONS && n < (int)sizeof now - 4; b++)
		if (JoyButtons()[b]) n += snprintf(now + n, sizeof now - n, " %d", b);
	n += snprintf(now + n, sizeof now - n, " keys");
	for (int k = 0; k < 256 && n < (int)sizeof now - 4; k++)
		if (g_keys[k]) n += snprintf(now + n, sizeof now - n, " %02x", k);
	if (n < (int)sizeof now - 16) snprintf(now + n, sizeof now - n, " mouse %d%d", g_mouse_down, g_mouse_rdown);
	if (strcmp(now, last) == 0) return;
	strcpy(last, now);
	SDL_Log("joystick: %s", now);
}

// --------------------------------------------------------------------------- 0x4b35d0

// Every frame, from CViewPort::render_viewport (before the objects update). Reads every element
// of the locked device:
//   button (type 0)   g_joy_buttons[n] (previous value to g_joy_prev_buttons[n]), n = the
//                     element's button number, or the count of buttons before it if it has none;
//                     n outside 0..19 is ignored
//   axis (type 1)     axis_type X, Y, Z, rX, rY, rZ -> g_joy_x .. g_joy_rz; others (sliders) ignored
//   d-pad (type 2)    the 8 directions -> g_joy_x / g_joy_y as -1/0/+1 (with up = -1, the opposite of
//                     the stick's Y); idle leaves them as they are. OMT 2002 on Windows never makes
//                     d-pad elements (MacOS InputSprocket does)
//   POV (type 8)      the angle -> g_joy_pov, which nothing reads
//   delta (3), movement (4): only shown by the debug display
// Returns whether there is a device.
bool neg::UpdateInput(void)
{
	short y = 30;								// debug display line
	if (!g_joy_device)
	{
		if (g_joy_frames < JOY_CLEAR_FRAMES)
		{
			ClearJoyButtons();
			g_joy_frames++;
		}
		return false;
	}

	if (g_joy_debug)
	{
		g_text_alpha = 1.0f;
		DrawText(0, 15, 'MED', "%s", g_joy_device->product_name.c_str());
	}
	g_joy_any_button = 0;
	unsigned char *buttons = JoyButtons();
	int button = 0;								// buttons met so far
	for (omt_HIDElementList::iterator i = g_joy_elements->begin(); i != g_joy_elements->end(); i++)
	{
		OMediaHIDElement *e = *i;
		OMediaHIDElementData data;
		e->read_data(data);						// vtable +4
		const char *name = e->name.c_str();
		switch (e->type)
		{
			case omhideltc_Button:
			{
				short n = (short)e->command_key;
				if (n == -1) n = (short)button;
				if (n >= 0 && n < JOY_BUTTONS)
				{
					g_joy_prev_buttons[n] = buttons[n];
					if (data.down)
					{
						buttons[n] = 1;
						g_joy_any_button = 1;
					}
					else buttons[n] = 0;
					if (g_joy_debug && g_joy_debug_mode != 2)
					{
						DrawText(0, y, 'MED', "%d%s: %d", (int)n, name, (int)buttons[n]);
						y += 15;
					}
				}
				button++;
				break;
			}

			case omhideltc_Axis:
				if (g_joy_debug && g_joy_debug_mode != 3)
				{
					DrawText(0, y, 'MED', "%s: %f", name, (double)data.axis);
					y += 15;
				}
				switch (e->axis_type)
				{
					case omhtdat_X:  g_joy_x  = data.axis; break;
					case omhtdat_Y:  g_joy_y  = data.axis; break;
					case omhtdat_Z:  g_joy_z  = data.axis; break;
					case omhtdat_rX: g_joy_rx = data.axis; break;
					case omhtdat_rY: g_joy_ry = data.axis; break;
					case omhtdat_rZ: g_joy_rz = data.axis; break;
					default: break;
				}
				break;

			case omhideltc_DPad:
				switch (data.pad_data)
				{
					case omhidpdc_PadLeft:      g_joy_x = -1.0f; g_joy_y =  0.0f; break;
					case omhidpdc_PadUpLeft:    g_joy_x = -1.0f; g_joy_y = -1.0f; break;
					case omhidpdc_PadUp:        g_joy_x =  0.0f; g_joy_y = -1.0f; break;
					case omhidpdc_PadUpRight:   g_joy_x =  1.0f; g_joy_y = -1.0f; break;
					case omhidpdc_PadRight:     g_joy_x =  1.0f; g_joy_y =  0.0f; break;
					case omhidpdc_PadDownRight: g_joy_x =  1.0f; g_joy_y =  1.0f; break;
					case omhidpdc_PadDown:      g_joy_x =  0.0f; g_joy_y =  1.0f; break;
					case omhidpdc_PadDownLeft:  g_joy_x = -1.0f; g_joy_y =  1.0f; break;
					default: break;				// idle: unchanged
				}
				if (g_joy_debug && g_joy_debug_mode != 3)
				{
					DrawText(0, y, 'MED', "%s: %d", name, (int)data.pad_data);
					y += 15;
				}
				break;

			case omhideltc_Delta:
				if (g_joy_debug && g_joy_debug_mode != 3)
				{
					DrawText(0, y, 'MED', "%s: %f", name, (double)data.delta);
					y += 15;
				}
				break;

			case omhideltc_Movement:
				if (g_joy_debug && g_joy_debug_mode != 3)
				{
					DrawText(0, y, 'MED', "%s: %f", name, (double)data.mov_data.x_axis);
					y += 15;
					DrawText(0, y, 'MED', "%s: %f", name, (double)data.mov_data.y_axis);
					y += 15;
				}
				break;

			case omhideltc_POV:
				g_joy_pov = data.angle;
				if (g_joy_debug && g_joy_debug_mode != 3)
				{
					DrawText(0, y, 'MED', "%s: %f", name, (double)data.angle);
					y += 15;
				}
				break;

			default:							// key elements (5..7): nothing
				break;
		}
	}

	if (g_joy_frames < JOY_CLEAR_FRAMES)
	{
		g_joy_frames++;
		ClearJoyButtons();
	}
	LogJoyChanges();							// port: test hook
	return true;
}

// --------------------------------------------------------------------------- the mouse pointer
// (CGfx 0x4b1d10..0x4b1dc0 and CAweReal 0x4a3990; Mouse.h. Not part of the input module in the exe,
// kept here with the other input code.)

void omf_SDLSetMousePosition(short x, short y);		// engine/sdl/OMediaSDLInput.cpp

// 0x4b1d10: the world's input engine (vtable +0x38), its get_mouse_position (+0x20).
void neg::GetMousePos(short *x, short *y)
{
	*x = *y = 0;
	if (g_input_engine) g_input_engine->get_mouse_position(*x, *y);
	*x = (short)(*x - (int)WideMargin());		// port: widescreen, window -> the 4:3 area
}

// 0x4b1d30: the same engine's set_mouse_position (+0x24; OMT 2.5 has no such call, the SDL layer
// provides it).
void neg::SetMousePos(short x, short y)
{
	omf_SDLSetMousePosition((short)(x + (int)WideMargin()), y);	// port: widescreen, 4:3 area -> window
}

// 0x4b1da0 / 0x4b1dc0: half the viewport's bounds (2002 +0xd4..+0xe0: left, top, right, bottom).
// port: widescreen - minus the margin, so it is in the same (4:3 area) space as GetMousePos.
short neg::MouseCentreX(void) { return (short)(g_viewport->getbwidth() / 2 - (int)WideMargin()); }
short neg::MouseCentreY(void) { return (short)(g_viewport->getbheight() / 2); }

// 0x4b1d50
void neg::CenterMouse(void)
{
	SetMousePos(MouseCentreX(), MouseCentreY());
}

// 0x4a3990: how far the pointer is from the viewport's centre (centre - pointer, so right / down
// are negative), then it goes back to the centre. Nothing happens while g_56aed0 is set.
void neg::GetMouseDelta(short *dx, short *dy)
{
	if (g_56aed0) return;
	short x, y;
	GetMousePos(&x, &y);
	*dx = (short)(MouseCentreX() - x);
	*dy = (short)(MouseCentreY() - y);
	CenterMouse();
}
