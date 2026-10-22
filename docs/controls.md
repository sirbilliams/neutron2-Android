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
| L3 | button 5 + key L (port) | The mini sub's headlight on / off (Level3a, Level3e; the original only has the L key for it) |
| R3 / Guide | buttons 6 / 7 | Unused by the game |
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
(widescreen 16:9, in-game UI, dual screen, box art, menu background, "HD image pack" for later) and **Quit**. Select or Y on
the pad (or a long press on the title) shows a hidden **Test** item: jump to a level, the model
viewer, share the log. D-pad / left stick choose, A selects, left / right flip a setting, B goes
back; touch works too. On a dual-screen device (dual screen on) the bottom screen shows, on the
main menu, only the game's box art (as large as fits, aspect kept, in a steel plate with a bevel and corner screws); on the other pages
the page's options and the selected item's details or save-slot preview. Once the game starts the
bottom screen becomes the companion panel. The box art is the player's own picture: Settings ->
"Box art: choose image..." (the Android file picker) copies it into the app's private files
(`boxart.img`), "Box art: clear" removes it; never put the cover in the repo. "Menu background:
choose image..." / "clear" work the same way (`background.img`): the picture fills the main
screen behind the plates (cover-fit, dimmed); without one, brushed steel. Without one the
game's own PDA menu picture (below) is shown, or a drawn atom before the game has saved it.

When the game ends (the PDA menu's Exit, or any normal end of the game), the app goes back to
the launcher's main menu, whose slot list and Continue then show what was just saved; the
launcher's Quit, or Back on its main menu, leaves the app. The game runs in its own process
(`android:process=":game"`), which ends after SDL has shut down: SDL and the game keep static
state that a second start in one process would inherit.

**Saves** (main menu): **Export saves** writes the save folder's files (`autosave*.tsk`,
`JimmyGame1..10.tsk`, `StartLevel.txt`; not `slots.txt`, which the game rewrites) to a .zip where
the player chooses (Android's file picker: Downloads, Drive...; suggested `negatron-saves-<date>.zip`).
On the Load Game page, Y (or holding a slot) exports that one slot as `JimmyGame<n>.tsk`. **Import
saves** takes such a .zip (or one .tsk), lists what it holds (level, score, NEW / REPLACES) and
asks before replacing anything (Cancel is preselected); a lone .tsk with another name asks which
slot it goes into. Paths inside the zip are ignored (only known save names are taken). The files
are the original game's own format: they work in the PC game's `save` folder and back.

The game starts with `NEG_START` set (see "How the game starts" below): `new`, `continue`,
`load:<slot>`. (The "original menu" style was removed at Bill's request: the app always shows the launcher;
the game's own Main Menu is the PDA, Esc / Start in play.)
The slots come from the game's `save/slots.txt` (before the game first wrote it: the .tsk headers).
The info window shows a drawing for the highlighted option, cross-faded on change
(`LauncherArt.java`: play dial, atom, floppy disk, floppy + PDA, gears + gauge, flask, rocket;
Bill's choice). The game's own pictures can be switched back on (`LauncherView.GAME_PICTURES`):
full-screen pictures (screens.omt, read from the player's files at run time; game/src/port/
LauncherArt.cpp saves them once into the app's files folder `launcher/`, `NEG_LAUNCHER_ART`, so
they appear from the second start): Continue = the autosave level's loading picture (saved as
levels are played; else the general loading screen), New Game = the museum, Load Game = the
load / save screen, Saves = the PDA menu, Settings = the options screen, Test = the inventory
screen, Quit = the credits. The other pages use their option's picture. Until a picture is there,
an illustration drawn in code takes its place (`LauncherArt.java`: atom, play dial, floppy disk,
floppy + PDA, gears + gauge, flask, rocket). Desktop preview: `tools/companion/render-menu.sh
<game folder> <out dir> [folder of the game's pictures, or -] [box art stand-in] [background
stand-in]` draws every page, and the main menu with each option highlighted (`opt_<id>`), for both
screens.

## Port menu and the bottom-screen panel (port)

Hold the Android back button (or the back swipe from the screen edge) for 0.6 s, or give it two
quick taps (within 0.35 s), to open a small menu over the game (the game pauses while it is open):

- **Bottom screen panel: ON / OFF** - shows a companion panel on the Thor's second screen
  (any second Android display; the first presentation display if there are several), styled as a
  stainless-steel lab gadget with green LEDs: the level, game mode and time in the level, neutron
  energy on a dial (green / yellow / red as the game's own bar), the score, the boss's health
  while it shows, the current gadget (its HUD picture, and the HUD's gadget bar while the game
  shows it), whether Goddard is about, this level's collectables (neutrons, checkpoints and each
  kind of pickup with the game's own picture, collected / all), the inventory page Jimmy opens
  (every item, lit when he has it, with how many, a dark silhouette when not), and a map of the
  level in a green CRT window with the nearest open exit's direction and the caption being typed:
  a top-down picture of the level drawn from its own geometry, following Jimmy (white arrow,
  north up), with the pickups, neutrons and checkpoints (their game pictures), friends (blue
  orbits), other characters (red), doors, exits (green = open, red = still shut, with the level
  they lead to) and the current objective (yellow pulse; objectives and exits off the screen
  get an arrow on the edge). On by default when a second screen is present (off otherwise); a
  choice made here or in the launcher's Settings is remembered and wins. With no second screen
  the menu says so and nothing is shown.
- **Panel map zoom: NEAR / FAR** - the map's scale (remembered).
- **In-game UI: BILL'S UI / VANILLA** - switches the in-game UI polish at once (remembered; also
  the launcher's Settings item "In-game UI", passed to the game as `NEG_UI=bills|vanilla`, default
  Bill's UI; desktop: the environment variable). See "Bill's UI" below.

A single back tap still reaches the game as Esc, a third of a second late (the menu needs to tell
a tap from a hold or a double tap). The panel never takes the focus or touches, and only reads
the game state (game/src/port/Companion.h); on the desktop `NEG_COMPANION_LOG=1` logs the same
snapshot once a second, and `NEG_COMPANION_DUMP=<dir>` writes the map, the snapshot and the icons
there; `tools/companion/render.sh <dir> <Level> out.png` then draws the panel from them.

## Bill's UI (port)

`NEG_UI=bills` (the default) or `vanilla` (game/src/port/BillsUi.h). Vanilla is the original's look.
Bill's UI adds, all drawn in code (only the game's own fonts and pictures are read at run time):

- **Sonar readout** (the radar's "2X".."5X" range while the sonar power-up runs): centred under
  the radar disc on a small green-hologram plate. (Both modes: the original drew it at y 10 as its
  bottom edge, so it was cut off at the top of the screen; Vanilla now has it at the top right,
  fully on screen, moving to the right edge with the disc in widescreen.)
- **Pause screen** (the PDA Main Menu, Esc / Start): its empty green grid shows a scope in the
  grid's phosphor green - a radar sweep with blips over range rings and Jimmy's atom (three orbits
  slowly turning, an electron on each) - and the credit line "Jimmy Neutron vs Jimmy Negatron
  Recomp - by sirbilliams - <version>" in the game's font. The version: Android's versionName
  (versionCode) (`NEG_APP_VERSION`, set by NegatronActivity) and the build's git commit, baked in
  at every build by `engine/cmake/neg_version.cmake` ("-dirty" with uncommitted changes).
- **Score**: counts up to its new value (about half a second) and, as points come in, pops (the
  digits grow for a moment) with a green glow behind it.
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

## Vox, the museum's world picker (port)

In the museum (Level5d) the question-mark terminal opens the PDA main menu with the world
pictures (VR tank, pyramid, King Carl), which the original drove with the Windows pointer. On
that page (only when the terminal opened it): **left / right** (d-pad, left stick, arrow keys)
move the highlight, **A** goes to the world, **B** (or Backspace) closes the menu and steps
Jimmy back out of the terminal (walk in again to reopen it). Each picture has its world's name
under it. Touching / moving the pointer gives the page back to the pointer (the original way)
until left / right is pressed again; `NEG_VOX_PICKER=0` turns the picker off. On a dual-screen
device the bottom panel shows the worlds as steel cards, the chosen one lit
(`game/src/port/VoxPicker.*`, `CompanionView.drawPicker`).

## Suspend (port)

When the app leaves the foreground the game is suspended, and nothing is saved (as the
original): on Android when the activity pauses or stops, or when the main display turns off while
it stays resumed (the Thor's lid: a DisplayManager listener in NegatronActivity); on the desktop
when the window is minimised.

- SDL blocks the game thread in its event pump until the app comes back
  (`SDL_HINT_ANDROID_BLOCK_ON_PAUSE`, SDL's default), pauses the audio devices and keeps / restores
  the GL context. NegatronActivity also asks SDL to pause in `onPause` (SDL itself only pauses in
  `onStop` on Android 7+). On the desktop the event loop waits for the window to come back.
- The engine (engine/sdl/OMediaSDLInput.cpp) pauses the sound device and OMT's clock on
  `SDL_APP_WILLENTERBACKGROUND` / minimise and resumes them on `SDL_APP_DIDENTERFOREGROUND` /
  restore. If the GL context was lost after all (`SDL_RENDER_DEVICE_RESET`), every texture and
  shape implementation is dropped and uploaded again when next drawn.
- The game deactivates as the original did on WM_ACTIVATEAPP (CAweReal: `PauseGame` while
  playing, the music and effects stop) and reactivates on return with the frame timer reset, so no
  game time passes. The focus events take the same path (sent once per change); before this port
  change they reached nothing, so losing the focus (e.g. the port menu's dialog) now pauses the
  game too, as alt-tab did on Windows.
- The bottom-screen panel stops polling while suspended.
- Desktop test hooks: `OMT_FAKE_PAD="<tick>:suspend=<ms>"` goes to the background for that long
  and comes back; `<tick>:glreset=1` acts as a lost GL context.

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
