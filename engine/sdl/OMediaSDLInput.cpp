/*  OMT SDL platform layer - event manager (SDL event pump -> OMT events) and input engine
    (keyboard / mouse / game controllers exposed as OMT HID devices).  LGPL. */

#include "OMediaSDLRetarget.h"
#include "OMediaEventManager.h"
#include "OMediaInputEngine.h"
#include "OMediaSupervisor.h"
#include "OMediaWindow.h"
#include "OMediaAnimPeriodical.h"
#include "OMediaDefMessages.h"

// ---------------------------------------------------------------------------
// Event manager

OMediaEventManager *OMediaEventManager::event_manager;

OMediaEventManager::OMediaEventManager(void) { event_manager = this; key_trap = NULL; }
OMediaEventManager::~OMediaEventManager(void) { if (event_manager == this) event_manager = NULL; }
void *OMediaEventManager::get_local_message_handler(void) { return NULL; }
void OMediaEventManager::update_menu_states(void) {}

bool OMediaEventManager::filter_event(void *local_event)
{
	OMediaFilterLocalEvent filter;
	filter.handled = false;
	filter.local_event = local_event;
	filtermsg_broadcaster.broadcast_message(omsg_LowlevelEventFilter, &filter);
	return filter.handled;
}

static omt_SpecialKey omf_special(SDL_Keycode k)
{
	switch (k)
	{
		case SDLK_RETURN:    return omtsk_Return;
		case SDLK_KP_ENTER:  return omtsk_Enter;
		case SDLK_LEFT:      return omtsk_ArrowLeft;
		case SDLK_RIGHT:     return omtsk_ArrowRight;
		case SDLK_UP:        return omtsk_ArrowTop;
		case SDLK_DOWN:      return omtsk_ArrowBottom;
		case SDLK_TAB:       return omtsk_Tab;
		case SDLK_BACKSPACE: return omtsk_Backspace;
		case SDLK_DELETE:    return omtsk_Delete;
		case SDLK_ESCAPE:    return omtsk_Escape;
		case SDLK_AC_BACK:   return omtsk_Escape;      // Android back button
		case SDLK_KP_0: return omtsk_Keypad0; case SDLK_KP_1: return omtsk_Keypad1;
		case SDLK_KP_2: return omtsk_Keypad2; case SDLK_KP_3: return omtsk_Keypad3;
		case SDLK_KP_4: return omtsk_Keypad4; case SDLK_KP_5: return omtsk_Keypad5;
		case SDLK_KP_6: return omtsk_Keypad6; case SDLK_KP_7: return omtsk_Keypad7;
		case SDLK_KP_8: return omtsk_Keypad8; case SDLK_KP_9: return omtsk_Keypad9;
		case SDLK_KP_PERIOD: return omtsk_KeypadDot;
		default: break;
	}
	if (k >= SDLK_F1 && k <= SDLK_F12) return (omt_SpecialKey)(omtsk_F1 + (k - SDLK_F1));
	if (k >= SDLK_F13 && k <= SDLK_F15) return (omt_SpecialKey)(omtsk_F13 + (k - SDLK_F13));
	return omtsk_Null;
}

static omt_CommandKey omf_cmdkeys(void)
{
	SDL_Keymod m = SDL_GetModState();
	omt_CommandKey c = 0;
	if (m & KMOD_SHIFT) c |= omtck_Shift;
	if (m & KMOD_ALT)   c |= omtck_Alt;
	if (m & KMOD_CTRL)  c |= omtck_Control;
	if (m & KMOD_GUI)   c |= omtck_Command;
	return c;
}

// Map a window-pixel mouse position to the window's logical coordinates (letterbox aware).
void omf_SDLWindowToLogical(OMediaWindow *ow, int px, int py, int &lx, int &ly);
static void omf_mouse_to_logical(OMediaWindow *w, int px, int py, int &lx, int &ly)
{
	omf_SDLWindowToLogical(w, px, py, lx, ly);
}

static void omf_fill_extra(OMediaEvent &e, int px, int py)
{
	e.command_key = omf_cmdkeys();
	if (e.window)
	{
		omf_mouse_to_logical(e.window, px, py, e.win_mouse_x, e.win_mouse_y);
		e.mouse_x = e.win_mouse_x + e.window->get_x();
		e.mouse_y = e.win_mouse_y + e.window->get_y();
	}
}

void OMediaEventManager::osevent_to_omtevent(void *localevent, OMediaEvent *ev)
{
	SDL_Event *se = (SDL_Event*)localevent;
	int mx, my;
	switch (se->type)
	{
		case SDL_TEXTINPUT:
			ev->window = OMediaSDLRtgWindow::find(se->text.windowID);
			if (!ev->window || (unsigned char)se->text.text[0] >= 0x80) return;
			ev->type = omtet_KeyDown;
			ev->ascii_key = se->text.text[0];
			ev->special_key = omtsk_Null;
			SDL_GetMouseState(&mx, &my); omf_fill_extra(*ev, mx, my);
			if (key_trap) key_trap->listen_to_message(omsg_KeyDown, ev);
			else key_broadcaster.broadcast_message(omsg_KeyDown, ev);
			return;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			ev->window = OMediaSDLRtgWindow::find(se->key.windowID);
			if (!ev->window) return;
			bool down = se->type == SDL_KEYDOWN;
			ev->type = down ? omtet_KeyDown : omtet_KeyUp;
			ev->ascii_key = 0;
			ev->special_key = omf_special(se->key.keysym.sym);
			// Printable keys arrive through SDL_TEXTINPUT on key-down; send key-ups for everything
			// and key-downs only for non-printables.
			if (down && ev->special_key == omtsk_Null) return;
			SDL_GetMouseState(&mx, &my); omf_fill_extra(*ev, mx, my);
			omt_Message m = down ? omsg_KeyDown : omsg_KeyUp;
			if (key_trap) key_trap->listen_to_message(m, ev);
			else key_broadcaster.broadcast_message(m, ev);
			return;
		}

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			ev->window = OMediaSDLRtgWindow::find(se->button.windowID);
			if (!ev->window) return;
			bool down = se->type == SDL_MOUSEBUTTONDOWN;
			bool left = se->button.button == SDL_BUTTON_LEFT;
			ev->type = left ? (down ? omtet_LMouseDown : omtet_LMouseUp) : (down ? omtet_RMouseDown : omtet_RMouseUp);
			omf_fill_extra(*ev, se->button.x, se->button.y);
			send_click_message(ev, left && down, down);
			return;
		}

		case SDL_WINDOWEVENT:
			ev->window = OMediaSDLRtgWindow::find(se->window.windowID);
			if (!ev->window) return;
			switch (se->window.event)
			{
				case SDL_WINDOWEVENT_EXPOSED:      ev->type = omtet_WindowRefresh; break;
				case SDL_WINDOWEVENT_FOCUS_GAINED: ev->type = omtet_WindowActivated; ev->window->activated(); break;
				case SDL_WINDOWEVENT_FOCUS_LOST:   ev->type = omtet_WindowDeactivated; ev->window->deactivated(); break;
				case SDL_WINDOWEVENT_CLOSE:
					ev->type = omtet_WindowCloseButton;
					ev->window->listen_to_message(omsg_Close, ev->window);
					break;
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				case SDL_WINDOWEVENT_MOVED:
					ev->window->bounds_changed();
					break;
			}
			return;
	}
}

void OMediaEventManager::spend_time(void)
{
	SDL_Event se;
	while (SDL_PollEvent(&se))
	{
		if (filter_event(&se)) continue;
		if (se.type == SDL_QUIT)
		{
			if (OMediaSupervisor::get_main_supervisor())
				OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_Quit);
			continue;
		}
		OMediaEvent ev;
		osevent_to_omtevent(&se, &ev);
	}
}

// ---------------------------------------------------------------------------
// Input engine: HID devices backed by SDL state polling

class OMediaSDLKeyElement : public OMediaHIDElement
{
	public:
	OMediaSDLKeyElement(OMediaHumanInterfaceDevice *d, SDL_Scancode sc) : OMediaHIDElement(d), scancode(sc)
	{
		type = omhideltc_Button;
		name = SDL_GetScancodeName(sc);
	}
	virtual void read_data(OMediaHIDElementData &data)
	{
		const Uint8 *ks = SDL_GetKeyboardState(NULL);
		data.down = ks[scancode] != 0;
	}
	SDL_Scancode scancode;
};

class OMediaSDLMouseElement : public OMediaHIDElement
{
	public:
	OMediaSDLMouseElement(OMediaHumanInterfaceDevice *d, int which, const char *n) : OMediaHIDElement(d), which(which)
	{
		type = which < 2 ? omhideltc_Delta : omhideltc_Button;
		axis_type = which == 0 ? omhtdat_X : (which == 1 ? omhtdat_Y : omhtdat_Null);
		name = n;
	}
	virtual void read_data(OMediaHIDElementData &data)
	{
		int dx, dy;
		if (which < 2) { SDL_GetRelativeMouseState(&dx, &dy); data.delta = float(which == 0 ? dx : dy); }
		else data.down = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(which - 1)) != 0;
	}
	int which;    // 0=x 1=y 2..=button (2 -> SDL_BUTTON_LEFT)
};

class OMediaSDLPadElement : public OMediaHIDElement
{
	public:
	OMediaSDLPadElement(OMediaHumanInterfaceDevice *d, SDL_GameController *gc, bool axis, int idx)
		: OMediaHIDElement(d), gc(gc), is_axis(axis), idx(idx)
	{
		type = axis ? omhideltc_Axis : omhideltc_Button;
		if (axis)
		{
			static const omt_HIDAxisType at[] = { omhtdat_X, omhtdat_Y, omhtdat_rX, omhtdat_rY, omhtdat_Z, omhtdat_rZ };
			axis_type = at[idx];
			name = SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)idx);
		}
		else name = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)idx);
	}
	virtual void read_data(OMediaHIDElementData &data)
	{
		if (is_axis) data.axis = SDL_GameControllerGetAxis(gc, (SDL_GameControllerAxis)idx) / 32767.0f;
		else data.down = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)idx) != 0;
	}
	SDL_GameController *gc; bool is_axis; int idx;
};

class OMediaSDLHID : public OMediaHumanInterfaceDevice
{
	public:
	OMediaSDLHID(OMediaInputEngine *e) : OMediaHumanInterfaceDevice(e) { element_list = &elements; }
	virtual ~OMediaSDLHID()
	{
		for (omt_HIDElementList::iterator i = elements.begin(); i != elements.end(); i++) delete *i;
	}
	virtual omt_HIDElementList *lock(void) { lock_count++; SDL_PumpEvents(); return &elements; }
	virtual void unlock(void) { lock_count--; }
	omt_HIDElementList elements;
};

class OMediaSDLInputEngine : public OMediaInputEngine
{
	public:
	OMediaSDLInputEngine(OMediaWindow *master) : OMediaInputEngine(ommeic_OS, master)
	{
		OMediaSDLHID *kb = new OMediaSDLHID(this);
		kb->type = omhidtc_Keyboard; kb->product_name = kb->instance_name = "Keyboard";
		for (int sc = SDL_SCANCODE_A; sc < SDL_NUM_SCANCODES; sc++)
			if (SDL_GetScancodeName((SDL_Scancode)sc)[0])
				kb->elements.push_back(new OMediaSDLKeyElement(kb, (SDL_Scancode)sc));
		hdi_list.push_back(kb);

		OMediaSDLHID *ms = new OMediaSDLHID(this);
		ms->type = omhidtc_Mouse; ms->product_name = ms->instance_name = "Mouse";
		ms->elements.push_back(new OMediaSDLMouseElement(ms, 0, "X Axis"));
		ms->elements.push_back(new OMediaSDLMouseElement(ms, 1, "Y Axis"));
		ms->elements.push_back(new OMediaSDLMouseElement(ms, 2, "Button 0"));
		ms->elements.push_back(new OMediaSDLMouseElement(ms, 4, "Button 1"));
		ms->elements.push_back(new OMediaSDLMouseElement(ms, 3, "Button 2"));
		hdi_list.push_back(ms);

		if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER)) SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
		for (int j = 0; j < SDL_NumJoysticks(); j++)
		{
			if (!SDL_IsGameController(j)) continue;
			SDL_GameController *gc = SDL_GameControllerOpen(j);
			if (!gc) continue;
			OMediaSDLHID *pad = new OMediaSDLHID(this);
			pad->type = omhidtc_Joystick;
			pad->product_name = pad->instance_name = SDL_GameControllerName(gc) ? SDL_GameControllerName(gc) : "Controller";
			for (int a = 0; a < SDL_CONTROLLER_AXIS_MAX; a++) pad->elements.push_back(new OMediaSDLPadElement(pad, gc, true, a));
			for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; b++) pad->elements.push_back(new OMediaSDLPadElement(pad, gc, false, b));
			hdi_list.push_back(pad);
		}
	}

	virtual bool mouse_down(void) { return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0; }

	virtual void get_sysmouse_position(short &x, short &y)
	{
		int mx, my; SDL_GetMouseState(&mx, &my);
		OMediaWindow *w = master_window;
		int lx = mx, ly = my;
		if (w) omf_mouse_to_logical(w, mx, my, lx, ly);
		x = lx; y = ly;
	}

	virtual omt_CommandKey get_command_keys_status(void) { return omf_cmdkeys(); }
};

OMediaInputEngine *omf_SDLCreateInputEngine(OMediaWindow *master) { return new OMediaSDLInputEngine(master); }
