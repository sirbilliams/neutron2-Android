# Handoff: where the project stands and how to continue

Read this whole file before doing anything. Then read `docs/decomp-guide.md`, which holds the conventions.

## The project

Bill (GitHub `sirbilliams`) wants the 2002 PC game **Jimmy Neutron vs. Jimmy Negatron**
(`Neutron2.exe`, MSVC 6, built on the Open Media Toolkit engine) to run as a native Android game on
his **AYN Thor**, an arm64 Android handheld with a built-in gamepad. The approach is a faithful
reconstruction of the game's code as C++ (`game/src`), decompiled from the retail exe. It builds
against our port of the OMT 2.5 engine (LGPL), which we moved from Windows/DirectX onto SDL2, made
64-bit, and run on OpenGL ES through gl4es.

- Repo: **github.com/sirbilliams/neutron2-android** (public). Push to `main`.
  - The old repo `sirbilliams/negatron-Android` is retired; Bill will delete it.
- Every push to main builds on GitHub Actions and replaces the **"latest"** release. Bill installs from:
  https://github.com/sirbilliams/neutron2-android/releases/download/latest/Negatron-Viewer.apk
- Git identity: `git -c user.name=sirbilliams -c user.email=bravehooves@gmail.com commit`.
  End every commit message with the attribution lines the session asks for.

### Bill's standing rules

- **Never commit game files**, and never put bytes copied from the exe or its DLLs in the repo: no
  data tables, strings or images dumped in bulk. Values the code needs are read from the player's own
  `Neutron2.exe` at run time (`game/src/ExeData.cpp`).
  - This is why the history was rewritten once. Keep it clean.
  - Small constants the code itself uses (a literal in a comparison, a file name) are fine.
- **Do not use the leaked GBA source.** Work only from the clean retail `Neutron2.exe` (not the
  `CRACK` exes on the disc).
- **Faithful.** Same logic, same order, same quirks, with comments where the original is odd. Any
  deviation is marked `port:` and explained.
- He wants it **playable on the Thor**. The current goal is **the first level the game loads:
  `level1d.gam` (CLevel01DGame)**.
- He doesn't want running commentary. Give him short status notes, and send a new APK when there
  is something to see.

## Repo map

| Path | What |
|---|---|
| `engine/` | OMT 2.5 (LGPL) ported to SDL2/gl4es. `sdl/` holds the platform layer: video, sound, input (gamepads appear as OMT HID joysticks) and main (data-folder search). |
| `engine/viewer/` | The model viewer (Android `libmain.so`). |
| `engine/importer/` | Installs the game from its CD image: an ISO 9660 reader plus unshield (InstallShield cabinets, MIT, fetched by `fetch-deps.sh`). Android `libimporter.so` (JNI); desktop `neg_import game.iso dir/`. |
| `game/src/` | The reconstructed game (Android `libnegatron.so`, desktop `negatron`). |
| `game/src/gen/` | Generated code: class slot/field includes, `globals.h/.cpp`, `factory.cpp`. Never edit by hand; regenerate. |
| `game/granny/` | Bit-exact replacement for the 26 `granny.dll` (RAD Granny 1.2b) functions the game uses. |
| `tools/decomp/` | Decompilation tooling (see below). `global_types/*.txt` names and types the globals. |
| `tools/granny/` | The Granny converter and the verification harness against the real DLL (under Wine). |
| `android/` | Gradle app: `SetupActivity` (installs the game from the .iso or a zip, then offers "Play (work in progress)" or "Model viewer") and `NegatronActivity` (SDLActivity; loads `negatron` or `main`). |
| `docs/` | `decomp-guide.md` (conventions, read it), `granny-api.md`, `granny-replacement.md`, this file. |

## Setting up a workspace

1. `bash fetch-deps.sh` fetches the pinned SDL2, gl4es and unshield (versions in `deps.lock`).
2. Desktop build: needs `libsdl2-dev`, zlib and cmake. Use one build dir per agent.
   ```
   cmake -S engine -B build-desk -DCMAKE_BUILD_TYPE=RelWithDebInfo
   make -C build-desk negatron neg_import
   ```
   `game/src` is globbed, so re-run cmake after adding files.
3. **Game files:** ask Bill for the game's CD image (`Jimmy Neutron vs. Jimmy Negatron.iso`, about
   216 MB) if it isn't already in the workspace. Never commit it. Then install it with
   `build-desk/neg_import game.iso game/`, which produces the installed game folder: `omt/ gam/ grn/ ase/ png/ dat/ save/` plus `Neutron2.exe` and `granny.dll`.
4. **Run headless** (start Xvfb first with `Xvfb :99 &`), from inside the game folder:
   ```
   DISPLAY=:99 OMT_WINDOW_SIZE=640x480 OMT_FRAMES=60 OMT_SCREENSHOT=x.bmp timeout 60 ../build-desk/negatron
   ```
   - `OMT_SCREENSHOT` is required, or the game never exits.
   - Starting a new game deletes the autosaves in that folder, so give every agent its own install.
   - The shipped saves are "DUMMY" placeholders. The game starts a new game from `level1d.gam`.
5. Android: `bash build-android.sh` (podman), or just push and let CI build.

## Regenerating the decompilation inputs (not in the repo; they are derived from the exe)

The tools read these from a work folder (it was `/home/claude/decomp`; recreate it):

1. `python3 tools/decomp/rtti.py Neutron2.exe classes.json` gives classes, bases and vtables from RTTI.
2. radare2 **6.2.4** plus **r2ghidra** (commit `c1287cc`), both built from source from GitHub
   (`radareorg/radare2`, `radareorg/r2ghidra`), and `pip install r2pipe`. Then
   `python3 tools/decomp/decompile_all.py Neutron2.exe classes.json raw/` writes `raw/<Class>.c`,
   `raw/_free.c` and `raw/functions.json`.
   - The vtable functions at 0x46e3c0, 0x48f010, 0x49ab40, 0x49d880, 0x4ab3e0 and 0x4ac8f0
     (plus 0x4ba08e and 0x4ba176) weren't found by analysis. They were decompiled by hand into
     `raw/_extra.c`; force `af` at those addresses.
3. `python3 tools/decomp/abi.py Neutron2.exe raw/functions.json abi.json` gives calling conventions
   from the machine code.
4. `python3 tools/decomp/factory.py Neutron2.exe factory.json abi.json` gives the object factory
   (4-char ID to class, size, constructor) by emulation.
   `python3 tools/decomp/gen_factory.py factory.json game/src/gen/factory.cpp`.
5. `python3 tools/decomp/slot_names.py raw/ classes.json --family CGameObject --json go_slots.json`
   gives slot names from the log strings. Also make `lgo_slots.json` for the CLocalGameObject family.
6. Globals: `python3 tools/decomp/globals.py raw/ Neutron2.exe game/src` writes `gen/globals.*`.
   Run it after every merge.
7. Per class: `tools/decomp/gen_class.py --class X --raw raw --classes classes.json --abi abi.json
   --src game/src --out game/src/gen ...`. Known options:
   - `CGameObject`: `--names go_slots.json`
   - `CLocalGameObject`: `--base CGameObject --base-size 0x43c`
   - `C2DInGameMenu`, `CTaskList`: `--base CLocalGameObject,CGameObject --base-size 0x4c8 --names lgo_slots.json`
   - `C3DObject`: `--base CLocalGameObject,CGameObject --base-size 0x588 --go-offset 0xc0 --el-slots 49`
   - Sprite family (C3DSprite and below): `--go-offset 0xc8 --el-slots 48`; see each header for its base size.

When the decompile is unclear, use capstone or `r2 -c 'pdg @ addr'` on the exe; x87 float code is
often decompiled wrong. `OMT2.dll` (the 2002 engine) gives the 2002 layouts (`tools/decomp/omt_vtables.py`).

## What is done

- **Engine, viewer and import:** the engine port, the model viewer, and installing the game from the ISO on the phone (Bill confirmed it works on the Thor).
- **Startup:** startup, display, fonts and text, the data-section globals (filled from the player's exe), and the object factory.
- **Core game classes:** CGameObject, CLocalGameObject, C3DObject (partly), CEditor (level loader), CGameType, CDefaultGame, CJimmyGame, CViewPort (per-frame driver), the interface elements, and C2DInGameMenu (all 64 slots).
- **Levels and sprite bases:** all 37 level game types (CLevel01DGame ...) and CLoadLevel; C3DSprite, C3DPermanentSprite, C3DSpriteType and C3DTriggerType.
- **Systems:**
  - the database caches
  - the savegames and task list (CTaskList)
  - physics: objects, the terrain grid, sweeps and ray tests
  - sound and music
  - input: CInput and the joystick poll (the d-pad drives the X/Y axes)
  - the Granny replacement
- **Where a run gets to now:** level1d.gam loads, creates CLevel01DGame, and stops at the level's objects whose classes don't exist yet. The first is `'3JIM'` (C3DJimmy).

## In progress when this was written (check `git log`; merge if they landed, otherwise redo)

- **Level objects:** C3DStartPoint, C3DCheckPoint, C3DPatrolPoint, C3DMine, C3DAITrigger,
  C3DCutSceneCamera, C3DMultiCutSceneCamera, CTrigger/C3DLight.
- **Animated characters:** C3DAnimated, C3DPlayerObject, C3DPlayer, C3DGrannyObj.
- **Granny without the DLL:** done. granny_compat reads the `.grn` files directly (textures decoded by its
  own Bink decoder, meshes and bindings built as the DLL does), bit-identical to the DLL on all 389 files on
  x86-64 and arm64 (`tools/granny/verify.sh`); `granny.dll` is only read as a data file for its Bink tables.

## Next, for level1d

Roughly in this order. Each one goes in its own `game/src` files.

1. **C3DTerrain** (`'3TER'`) and **C3DSky** (`'3SKY'`). Without them there is no world: the port
   currently skips the NULL terrain and sky.
2. **C3DJimmy** (`'3JIM'`, 0x4efc bytes, 166 element slots) on C3DPlayerObject. It creates the
   menu, the task list and the HUD. Huge; split it the way the menu was split (parts a–d).
3. **C3DAI** (`'3DAI'`), then **C3DAIGranny** (`'3GRN'`) and **C3DFish** (`'3FIS'`).
4. **C3DRocketShip** (`'3ROC'`) on C3DFlyingObject.
5. Then the remaining object classes the levels use. `factory.json` plus a scan of the `.gam` files
   shows which: 118 classes are used across levels.

Loose ends:

- `CEditor::m_load_pending` is never cleared, so the level reloads every frame. Fix it once the
  level objects exist.
- `CJimmyGame::GetSceneStage` should use `neg::GetTaskState("SCENE")`; check that it does.
- Keyboard-only actions need Thor buttons once something is playable: Y/N confirmations,
  Esc (close text / skip), R (HUD), and "any key" on the score screen. That is a port-level mapping
  in the SDL layer, not a game change.
- Several modules have private static copies of the same small helpers: find object by tag
  (0x4aefb0), menu item state (0x404dd0/0x404ef0/0x404f20), mouse capture (0x4a3930) and
  SetGameState (0x485360). Consolidate them into one shared definition each.
- The MP3-in-WAV clip "bird1" is silent; the SDL mixer has no MP3 decoder.

## Working method that worked

- Run several sub-agents in parallel, each in a git worktree with its own build dir and its own game
  install, each on one class or system. Give each one this file, the guide, the inputs, and the test
  command. Then merge their branches one at a time:
  1. For conflicts in `game/src/gen/*`, take either side, then regenerate: `globals.py`, then
     `gen_class.py` for the classes that changed.
  2. Reconcile duplicate structs and names between parts, so there is one definition of each.
  3. Build, run the smoke test, commit, push.
- Running too many agents at once hits the session usage limit. Four or five was fine; six was not.
- Desktop success means a "CLEAN EXIT" at the end of the headless run. The Android build only
  happens on CI, so watch the run (`gh api repos/sirbilliams/neutron2-android/actions/runs`).
