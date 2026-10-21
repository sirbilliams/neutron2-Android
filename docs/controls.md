# Controls on the AYN Thor (and any SDL game controller)

## How the original reads a joystick

Checked against `Neutron2.exe`:

- The game polls the first OMT HID joystick once per frame (`neg::UpdateInput` at 0x4b35d0 in
  `game/src/Input.cpp`).
  - DirectInput button *n* goes to `g_joy_buttons[n]`, and the X and Y axes go to `g_joy_x` and
    `g_joy_y`. Stick up is +1.
  - The game tests fixed button numbers next to its key bindings. Every read is listed in
    `Input.cpp`.
  - Nothing reads Z, rX, rY, rZ, the POV hat, or buttons 5..7 and 10..19.
- `dat\keys.txt` names `JOY_01`..`JOY_09`, but the exe's `TextToKeys` (0x4660d0) has no `JOY_`
  names. The "`TextToKeys JOY_xx not found`" lines in the log appear on Windows too. Those entries
  count as `KEY_NONE`, so they do nothing.
- Keyboard and mouse input arrives as window messages: the keys go into `g_keys[]` (Windows key
  codes) and the mouse buttons into `omsg_Event`.
- The menus and the yes/no boxes follow the mouse pointer. In play, the mouse turns the camera
  (mouse look: the pointer's offset from the centre, 0x4a3990).
- In menus, the stick moves the pointer (0x40a170), and the action button clicks.

The SDL layer (`engine/sdl/OMediaSDLInput.cpp`) shows the gamepad to the game as a DirectInput
joystick did. Each physical button carries the number the game expects for that action. On top
of that, it sends key presses and moves the pointer for actions the game only takes from the
keyboard or the mouse. Those parts are marked `port:` and do not change game logic.

## Default layout

| Thor control | The game sees | What it does in level 1 |
|---|---|---|
| Left stick / d-pad | X / Y axes | Move forward and back, turn left and right; moves the pointer in menus (once the menu's pointer stubs are filled, see below) |
| A | button 0 + key Space (port) | Action / fire, skip cut scene, click a menu item, any key on "press any key" screens (the score screen) |
| B | button 1 | Jump, speed |
| X | button 2 | Brake (vehicles) |
| Y | button 3 | Inventory / pickups page |
| Start | button 4 + key Esc (port) | Pause menu; Esc also closes text boxes, skips cut scenes and puts the item away |
| Select | button 9 | Help text |
| R1 | button 8 | Turn around |
| L1 | key R (port) | Show / hide the HUD panel |
| R2 | rZ axis + key Y (port, past half way) | Yes in confirmations |
| L2 | Z axis + key N (port) | No in confirmations |
| Right stick | rX / rY + mouse pointer (port) | Camera (mouse look) in play; pointer in menus |
| L3 / R3 / Guide | buttons 5 / 6 / 7 | Unused by the game |
| Android back | key Esc | Same as Esc; never closes the app (`SDL_HINT_ANDROID_TRAP_BACK_BUTTON`) |
| Touch screen | mouse | Pointer and left click, as the mouse |

- **Button numbers:** DirectInput numbered a pad's buttons in its driver's order, so Windows had
  no single order. The face buttons follow keys.txt's `JOY_01`.. order (action, jump, brake).
- **Pad keys:** they go through the same path as keyboard keys (pushed as SDL key events), so
  the game cannot tell them apart. A key is down while its control is held on any controller.
- **Mapped controllers first:** the game takes the first joystick, so SDL game controllers come
  before other joystick devices.

## Pointer helpers

`game/src/Mouse.h` has the shared pointer helpers: `GetMousePos` 0x4b1d10, `SetMousePos`
0x4b1d30, `CenterMouse` 0x4b1d50 and `GetMouseDelta` 0x4a3990. The menu, C3DPlayer,
C3DFlyingObject and CEditor use them, so the right stick turns the camera in play and the left
stick moves the menu pointer as in the original. Mouse capture (0x4a3930) centres the pointer;
hiding and restoring the Windows cursor has no equivalent on the Thor.

## Test hooks (desktop)

- `OMT_FAKE_PAD="<tick>:<control>=<value>,..."` attaches an SDL virtual game controller and sets
  its controls at the given ticks (about one per frame).
  - Controls use SDL's names: `a`, `b`, `start`, `leftshoulder`, `dpup`, `leftx`,
    `righttrigger`...
  - `lmouse` / `rmouse` click at the pointer, and `acback` is the Android back key.
  - Example: `OMT_FAKE_PAD="60:start=1,64:start=0"`.
  - The game clears the buttons for its first 30 polls.
- `OMT_PAD_LOG=1` logs the devices and their elements, the pad keys and the pointer.
- `NEG_JOY_LOG=1` logs what the game sees whenever it changes: the buttons, X/Y, `g_keys` and the
  mouse buttons.
