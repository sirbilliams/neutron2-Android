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
3. **Game files** live in Bill's private repo **sirbilliams/negatron-gamefiles** (never commit them here):
   - release `iso`: `Jimmy.Neutron.vs.Jimmy.Negatron.iso` (226,605,056 bytes, sha256 `65c8fc3f...8d88`) and
     `OMTFullV250.zip` (the full OMT 2.5 distribution: original sources, docs, extras).
   - `decomp/decomp-inputs.tar.gz` in the repo: the regenerated decompilation inputs (see below), so a new
     session can skip the r2ghidra run.
   Getting them in a cloud session: attach the repo (add_repo), then download the release assets with the
   REST API (`gh release download` fails: GraphQL is blocked, and creating releases is blocked too):
   ```
   gh api repos/sirbilliams/negatron-gamefiles/releases/tags/iso --jq '.assets[]|"\(.id) \(.name)"'
   gh api -H "Accept: application/octet-stream" repos/sirbilliams/negatron-gamefiles/releases/assets/<id> > <name>
   ```
   archive.org is blocked by the default network policy. Install with
   `build-desk/neg_import game.iso game/`, which produces the installed game folder: `omt/ gam/ grn/ ase/ png/ dat/ save/` plus `Neutron2.exe`, `granny.dll` and `OMT2.dll`.
   Unzip `OMTFullV250.zip` and point `omt_layout.py` at `OMT/sources/OMTClasses` (our ported `engine/OMTClasses` lays out 0 classes).
4. **Run headless** (start Xvfb first with `Xvfb :99 &`), from inside the game folder:
   ```
   DISPLAY=:99 OMT_WINDOW_SIZE=640x480 OMT_FRAMES=60 OMT_SCREENSHOT=x.bmp timeout 60 ../build-desk/negatron
   ```
   - `OMT_SCREENSHOT` is required, or the game never exits.
   - Starting a new game deletes the autosaves in that folder, so give every agent its own install.
   - The shipped saves are "DUMMY" placeholders. The game starts a new game from `level1d.gam`.
5. Android: `bash build-android.sh` (podman), or just push and let CI build.

## Regenerating the decompilation inputs (not in the repo; they are derived from the exe)

The tools read these from a work folder, `/home/claude/decomp` (the guide's paths). Fastest: unpack the backup,
`mkdir -p /home/claude/decomp && tar xzf decomp-inputs.tar.gz -C /home/claude/decomp`, and copy `Neutron2.exe` in.
The guide also expects `/home/claude/jn/game/Program_Executable_Files` (symlink it to the game install) and
`/opt/r2/bin/r2`. Python needs `pefile capstone unicorn r2pipe` (pip `--break-system-packages`), and clang for omt_layout.py.
To regenerate from scratch instead:

1. `python3 tools/decomp/rtti.py Neutron2.exe classes.json` gives classes, bases and vtables from RTTI.
2. radare2 **6.2.4** plus **r2ghidra** (commit `c1287cc`), both built from source from GitHub
   (`radareorg/radare2`, `radareorg/r2ghidra`), and `pip install r2pipe`. Then
   `python3 tools/decomp/decompile_all.py Neutron2.exe classes.json raw/` writes `raw/<Class>.c`,
   `raw/_free.c` and `raw/functions.json`.
   - The vtable functions at 0x46e3c0, 0x48f010, 0x49ab40, 0x49d880, 0x4ab3e0 and 0x4ac8f0
     (plus 0x4ba08e and 0x4ba176) weren't found by analysis. They go in `raw/_extra.c`: force
     `af` at each, with headers in the same format as the other raw files
     (`/* ==== 0x4ab3e0 CGameObject__v8 size 22 ==== */`), or gen_class.py drops those slots.
     Check: regenerating CGameObject, CLocalGameObject, C2DInGameMenu, CTaskList and C3DObject must give no git diff.
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
- **Level 1 world (Oct 21):** C3DTerrain and C3DSky (the moon surface and starfield render); the
  level objects level1d uses: C3DStartPoint, C3DPatrolPoint, C3DAITrigger, C3DCutSceneCamera (`game/src/objects/`).
- **Granny without the DLL:** granny_compat reads the `.grn` files directly (textures by its own Bink decoder,
  meshes and bindings built as the DLL does), bit-identical to the DLL on all 389 files on x86-64 and arm64
  (`tools/granny/verify.sh`); `granny.dll` is only read as a data file for its Bink tables. Not yet checked
  against Android's libm (acos/sin/sqrt/fmod).
- **Characters (Oct 21):** C3DAnimated, C3DGrannyObj, C3DPlayerObject, C3DPlayer (layout notes in their
  headers); C3DJimmy (`C3DJimmy.h/.cpp` + `jimmy/C3DJimmy_a.*`, `_b.*`, split at 0x448b40); C3DAI,
  C3DAIGranny, C3DFish; C3DFlyingObject, C3DRocketShip, C3DPermanentSmoke, C3DFireStrato.
- **Port fixes:** the menu clears `CEditor::m_load_pending` at its level reset (0x40ac30), so the level
  loads once (it reloaded every frame = the flashing on the Thor); static .ase files in
  C3DAnimated::CreateAnim; game objects are allocated zeroed (CGameObject::operator new, `port:`).
- **Where a run gets to now:** level1d loads once, all 61 objects are created, Jimmy, the menu and the
  task list exist, and 300 frames run to CLEAN EXIT. The screen shows the terrain and sky from a camera
  that moves (the opening cut scene has the camera); Jimmy isn't on screen yet.

## What level1d contains

Parsed with LoadGame's own logic: 61 objects. 3PAT x28, 3AIT x14, 3FIS x10, 3CAM x4, and one each of
STRT, 3JIM, 3ROC, 3GRN, LOAD (plus the terrain and sky). No CTrigger, checkpoints, mines or lights.
(A naive byte count of the IDs doubles everything, because each object also stores its ID as a property.)
The 10 "fish" are the intro logo letters (T, H, Q, AWE games...): level1d opens with the logo cut scene.
LoadGame stops at the first NULL object, so every class in the file must exist or nothing after it loads.

## Next, for a playable level1d

1. **The story opening (checked against the original running under Wine, `tools/wine/run_original.sh`):**
   level1d is only the logo intro (THQ, AWE, Nick: the C3DFish letters, cut scene cameras cam4 / cam6 /
   nickcam2 timed by their voice clips). It ends with the LOAD trigger `startgame`, which loads
   **level5.gam** (the museum: Fowl, the friends, a flying camera); that level's exit `tolab` loads
   **level3.gam** (Negatron's lab), and only there, about 6 minutes in, does the player get Jimmy (HUD on).
   The port now plays the logo intro as the original and loads level5.gam; level5's LoadGame stops at
   its first missing class (3FOW), so its opening never starts and the game sits in mode 1 (Loading).
   Needed next, in file order: level5 - C3DFowl 3FOW, C3DSheen 3SHE, C3DLibby 3LIB, C3DCarl 3CAR,
   C3DCindy 3CIN, C3DNick 3NIC, C3DBenny 3BEN, C3DFlyingCamera 3FLY, C3DAIJimmy 3AIJ,
   C3DMultiCutSceneCamera 3MCA, C3DTree 3TRE, C3DFog 3FOG (3ASE exists now); level3 - 3GRO, 3FAD,
   3NEU, 3CHK, 3TSP, 3PIC, 3HEL, 3BOT, 3TEX, 3TES, 3SPH, 3SWN, 3SCR, 3NUM, 3MUS, 3HER, 3BUT, 3ARR.
   `NEG_TEST_PLAY=level5.gam` (desktop) skips a level's opening and gives Jimmy the camera in mode 5:
   he walks, turns, jumps and stops at the museum walls. Under Wine the voice clips play fast (ALSA
   null device), so its timing is not a reference; the port's shots last as long as their clips.
   C3DRocketShip::v260 -> InitPhysics taking the element position is as in the original (0x46a852).
2. **Jimmy's helper objects** (created in his constructor, NULL until written; every use is NULL-checked):
   C3DShadow 0x46e9f0, C3DRadar (the HUD) 0x468180, C3DJetPack 0x442f10, C3DTargetCursor 0x475b20,
   C3DPointCursor 0x466a70, C3DASEObj 0x4207e0 (bubble), C3DMagicParticle 0x451700, C3DWaterBalloon
   0x47c780, C3DGraplingHook 0x43cdb0, C3DGoddard 0x4383d0. Plus `player HUD hide` 0x468aa0.
3. **Thor controls:** the joystick names JOY_01..JOY_09 aren't found by C3DPlayerObject::TextToKeys
   (`dat\options.opt` key names); map them, and give the keyboard-only actions buttons (below).
4. Open TODOs in a run (`grep TODO` in the log): CPhysics::AddObject 0x4b4ae0, CGameObject slots 248
   0x4ae3a0 and 262 0x4af030, GetMouseDelta 0x4a3990, texture animations 0x4a28c0, shape material setup
   0x4a23e0, clear layer view matrix 0x4b974e, C3DParticle::Reset 0x49b260.
5. Weak hooks (`grep -n weak game/src`) whose real owners aren't written yet: C3DMiniSub, C3DGoddard,
   C3DAIOmtObj, C3DDino/C3DPtero, mines - not used by level1d.

Loose ends:

- `CJimmyGame::GetSceneStage` should use `neg::GetTaskState("SCENE")`; check that it does.
- Keyboard-only actions need Thor buttons: Y/N confirmations, Esc (close text / skip), R (HUD), and
  "any key" on the score screen. That is a port-level mapping in the SDL layer, not a game change.
- Several modules have private static copies of the same small helpers: find object by tag
  (0x4aefb0), menu item state (0x404dd0/0x404ef0/0x404f20), mouse capture (0x4a3930), health
  (0x485730) and SetGameState (0x485360). Consolidate them into one shared definition each.
- The MP3-in-WAV clip "bird1" is silent; the SDL mixer has no MP3 decoder.
- Granny: not yet checked against Android's libm (acos/sin/sqrt/fmod).

## Working method that worked

- **The original as a reference:** `tools/wine/run_original.sh <game dir> <out dir> <seconds>` runs the
  retail Neutron2.exe under Wine on a private Xvfb (640x480x16, fake CD-ROM drive, ALSA null sound) and
  screenshots it. Compare with the port's `OMT_SHOTS` frames.

- Run several sub-agents in parallel, each in a git worktree with its own build dir and its own game
  install, each on one class or system. Give each one this file, the guide, the inputs, and the test
  command. Then merge their branches one at a time:
  1. For conflicts in `game/src/gen/*`, take either side, then regenerate: `globals.py`, then
     `gen_class.py` for the classes that changed.
  2. Reconcile duplicate structs and names between parts, so there is one definition of each.
  3. Build, run the smoke test, commit, push.
- Give each agent its own scratch files too (the scratchpad is shared; one agent overwrote another's script).
- The harness may refuse `fetch-deps.sh` inside a worktree (it runs git in subfolders); the desktop
  `negatron` build doesn't need those deps.
- Running too many agents at once hits the session usage limit. Four or five was fine; six was not.
- Desktop success means a "CLEAN EXIT" at the end of the headless run. The Android build only
  happens on CI, so watch the run (`gh api repos/sirbilliams/neutron2-android/actions/runs`).
- Brief template for parallel agents: `docs/agent-brief.md`.
