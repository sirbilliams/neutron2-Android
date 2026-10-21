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
| Android back (tap) | key Esc | Same as Esc; never closes the app (`SDL_HINT_ANDROID_TRAP_BACK_BUTTON`). Reaches the game 0.35 s after release (port: see the port menu below) |
| Android back (hold 0.6 s, or two taps) | - | Opens the port menu (port) |
| Touch screen | mouse | Pointer and left click, as the mouse |

- **Button numbers:** DirectInput numbered a pad's buttons in its driver's order, so Windows had
  no single order. The face buttons follow keys.txt's `JOY_01`.. order (action, jump, brake).
- **Pad keys:** they go through the same path as keyboard keys (pushed as SDL key events), so
  the game cannot tell them apart. A key is down while its control is held on any controller.
- **Mapped controllers first:** the game takes the first joystick, so SDL game controllers come
  before other joystick devices.

## The launcher menu (port)

The app opens on a menu in the bottom-screen panel's style (`LauncherView.java`; drawn in code, no
artwork in the repo): **Continue** (the autosave), **New Game** (asks first: it wipes the autosave),
**Load Game** (the game's save slots `save/JimmyGame1..10.tsk`: level, score, date), **Settings**
(widescreen 16:9, dual screen, menu style, box art, "HD image pack" for later) and **Quit**. Select or Y on
the pad (or a long press on the title) shows a hidden **Test** item: jump to a level, the model
viewer, share the log. D-pad / left stick choose, A selects, left / right flip a setting, B goes
back; touch works too. On a dual-screen device (dual screen on) the bottom screen shows, on the
main menu, only the game's box art (as large as fits, aspect kept, in a steel plate with a bevel and corner screws); on the other pages
the page's options and the selected item's details or save-slot preview. Once the game starts the
bottom screen becomes the companion panel. The box art is the player's own picture: Settings ->
"Box art: choose image..." (the Android file picker) copies it into the app's private files
(`boxart.img`), "Box art: clear" removes it; never put the cover in the repo. Without one the
game's own menu picture (below) is shown, or a drawn atom before the game has saved it.

The game starts with `NEG_START` set (see "How the game starts" below): `new`, `continue`,
`load:<slot>`, or `menu` when Settings -> Menu style is ORIGINAL (the app then skips the launcher:
the original start, with the game's own Main Menu, the PDA; the port menu below switches back).
The slots come from the game's `save/slots.txt` (before the game first wrote it: the .tsk headers).
The info window shows the game's own main-menu background (screens.omt, read at run time): the
game saves it once to `launcher-art.png` in the app's files folder (`NEG_MENU_ART`), so it appears
from the second start. Desktop preview: `tools/companion/render-menu.sh <game folder> <out dir>
[picture] [box art stand-in]` draws every page for both screens.

## Port menu and the bottom-screen panel (port)

Hold the Android back button (or the back swipe from the screen edge) for 0.6 s, or give it two
quick taps (within 0.35 s), to open a small menu over the game (the game pauses while it is open):

- **Bottom screen panel: ON / OFF** - shows a companion panel on the Thor's second screen
  (any second Android display; the first presentation display if there are several), styled as a
  stainless-steel lab gadget with green LEDs: the level and game mode, neutron energy on a dial
  (green / yellow / red as the game's own bar), score and lives (or the boss's health while it
  shows), the pickups / neutrons / checkpoints collected, the gadgets Jimmy has (the current one
  lit) and the items collected (the game's own pictures), the caption being typed, the story
  tasks (lit = started, with their stage number), and a map of the level in a green CRT window:
  a top-down picture of the level drawn from its own geometry, following Jimmy (white arrow,
  north up), with the pickups, neutrons and checkpoints (their game pictures), friends (blue
  orbits), other characters (red), doors, exits (green = open, red = still shut, with the level
  they lead to) and the current objective (yellow pulse; objectives and exits off the screen
  get an arrow on the edge). On by default when a second screen is present (off otherwise); a
  choice made here or in the launcher's Settings is remembered and wins. With no second screen
  the menu says so and nothing is shown.
- **Panel map zoom: NEAR / FAR** - the map's scale (remembered).
- **Menu style: NEW / ORIGINAL** - the launcher or the game's own title menu, from the next start.

A single back tap still reaches the game as Esc, a third of a second late (the menu needs to tell
a tap from a hold or a double tap). The panel never takes the focus or touches, and only reads
the game state (game/src/port/Companion.h); on the desktop `NEG_COMPANION_LOG=1` logs the same
snapshot once a second, and `NEG_COMPANION_DUMP=<dir>` writes the map, the snapshot and the icons
there; `tools/companion/render.sh <dir> <Level> out.png` then draws the panel from them.

## How the game starts: NEG_START and save\slots.txt (port; launcher interface)

The original has no menu before the game (manual p. 6-7; CEditor::UpdateGame 0x4a5c70): the
installer's `save\StartLevel.txt` says `autosave`, so every start continues `save\autosave.tsk`;
without one (first run, or the shipped DUMMY placeholder) a new game starts in level1d.gam
(title / logo screens, then the story) and the autosaves are deleted. Its Main Menu is the PDA
that ESC opens; New / Load / Save / Delete are on its Load/Save page (slots
`save\JimmyGame1..10.tsk`). Details: `game/src/port/StartMode.h`.

The launcher sets `NEG_START` in the environment before the engine starts (like
`NEG_WIDESCREEN`; unset it when not wanted, the process can outlive a run):

| NEG_START | What happens |
|---|---|
| unset / `menu` | The original start; when a saved game was continued, the original Main Menu (PDA) opens as soon as Jimmy plays. The default ("original menu"). |
| `original` | Exactly the original start, no menu. |
| `new` | New game (level1d.gam; the autosaves are deleted, as the original's New). |
| `continue` | The autosave; a new game when there is none. |
| `load:<n>` | Save slot n = 1..10 (`save\JimmyGame<n>.tsk`); the original start when the slot is empty. |

`NEG_TEST_PLAY` (level picker) wins over `NEG_START`.

Save list for the launcher: `save/slots.txt` in the game folder, written by the game at
start-up and whenever the autosave or a slot is written or deleted (missing = the game has not
run yet, no saves). Lines starting with `#` are comments; one line per saved game, TAB-separated:
`<slot> <level id> <level name> <gam file> <score> <mtime>`, e.g.
`3	LV5F	dinosaurs	Level5f.gam	1200	1792617355`. slot = `autosave` or 1..10 (the
`load:<n>` number); level name = the game's own Load/Save page name (may be empty); mtime = Unix
seconds (the save date). Empty and DUMMY slots are not listed. (A launcher may also parse the
.tsk header itself: big-endian; strings are a length byte + chars: version ("DUMMY   " =
empty), int level 4cc, string gam file, string task file, float health, short lives, int score.)

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
