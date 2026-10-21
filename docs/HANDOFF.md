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
- He wants it **playable on the Thor**. Reached on desktop (Oct 21, session 2): the game runs from the
  logo intro through the museum (Level5) and Negatron's lab cut scenes (Level3) to player control of
  Jimmy. Next goals are below ("Next").
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
6. Fresh cloud machine (Ubuntu 24.04), what was needed in session 2: `apt-get install libsdl2-dev
   zlib1g-dev xvfb x11-utils imagemagick meson ninja-build pkg-config`; Wine needs
   `dpkg --add-architecture i386` then `apt-get install wine wine32:i386`; `pip install --break-system-packages
   pefile capstone unicorn r2pipe`. radare2 6.2.4 and r2ghidra c1287cc are not packaged: build both from
   GitHub with `--prefix=/opt/r2`, and run r2ghidra's `./configure` with
   `PKG_CONFIG_PATH=/opt/r2/lib/pkgconfig PATH=/opt/r2/bin:$PATH LD_LIBRARY_PATH=/opt/r2/lib` (without it
   it can't find r_core). `/opt/r2/bin/r2` is a symlink: put a wrapper script at `/usr/local/bin/r2` that
   sets `LD_LIBRARY_PATH=/opt/r2/lib`, never write through the symlink. `neg_import` needs `fetch-deps.sh`
   first (unshield).

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
- **Where a run gets to now:** see "Status" below: through Level5 and Level3's cut scenes to player control.

## What level1d contains

Parsed with LoadGame's own logic: 61 objects. 3PAT x28, 3AIT x14, 3FIS x10, 3CAM x4, and one each of
STRT, 3JIM, 3ROC, 3GRN, LOAD (plus the terrain and sky). No CTrigger, checkpoints, mines or lights.
(A naive byte count of the IDs doubles everything, because each object also stores its ID as a property.)
The 10 "fish" are the intro logo letters (T, H, Q, AWE games...): level1d opens with the logo cut scene.
LoadGame stops at the first NULL object, so every class in the file must exist or nothing after it loads.

## Status at the end of the Oct 21 session 2 (read this first)

- **Reached:** a normal Play (no test hooks) runs level1d.gam (THQ/AWE logos) -> LOAD `startgame` ->
  Level5.gam (museum cut scene) -> Level3.gam (the underwater fly-in `labflyin1`, then several minutes of
  lab cut scenes) -> **player control of Jimmy in the lab** at about 375-380 s from launch, as in the
  original under Wine (about 380 s). Checked frame by frame against the original's screenshots (every
  5-10 s) and with the virtual pad: holding the stick forward from launch (`OMT_FAKE_PAD="5:lefty=-32767"`)
  does nothing in the cut scenes and walks Jimmy once control starts. Both levels load all objects
  (`tools/decomp/gamcensus.py` reports nothing missing). Smoke test: CLEAN EXIT. main = Android CI green.
- **Not yet checked on the Thor itself** (Bill has the APK from main 378528e): voice timing and the
  gl4es rendering of the new objects. The Granny code is still unchecked against Android's libm.
- **All 29 classes are written** (one commit each, faithful, `port:` where they deviate):
  - friends (`game/src/friends/`): C3DFriends (base), C3DFowl, C3DSheen, C3DLibby, C3DCindy, C3DBenny,
    C3DNumey, C3DCarl, C3DNick (+ C3DSkateBoard, no factory ID), C3DAIJimmy;
  - cameras etc. (`game/src/objects/`): C3DMultiCutSceneCamera (the story cut scenes; its 18 camera types'
    constants are read from the player's exe at run time), C3DFlyingCamera, C3DTree, C3DFogTrigger;
  - pickups and triggers (`objects/`, `pickups/`): CPickupType + C3DPickupItem, C3DCheckPoint, C3DNeutron,
    C3DTransportEffect (+ C3DCorona '3COR'), C3DHelpSound, C3DTextTrigger, C3DMusicTrigger, C3DFadeObject,
    C3DArrow;
  - lab (`game/src/lab/`): C3DSwingDoor, C3DHerman, C3DBot (+ C3DShock '3SHO'), C3DTesla, C3DButton,
    C3DLabScreen;
  - shared slots: CLocalGameObject slot 281 (0x48f900, project a point to the screen: Carl's hunger bar,
    Herman's health bar), CGameObject slot 226 (0x4ad990, declared by hand with its real signature).
  - Stubs replaced: LibbySlot 0x41a039, C3DJimmy bot contact 0x44512a, CEditor 0x4a6520, C3DTriggerType
    0x457be0 / 0x433f50 and the text/music/fade hooks, C2DInGameMenu_d 0x406cd4 / 0x406cb0. C3DPlayerObject
    now owns the 40-byte buffer at 0x938 (C3DJimmy's `f_938` is gone).
- **Testing tools added:** `tools/decomp/gamcensus.py` (which classes a .gam holds, in load order, and which
  are missing) and `tools/test/run_port.sh` (screenshots the port every N seconds of real time, the
  counterpart of `tools/wine/run_original.sh`: headless runs draw thousands of frames a second while voice
  clips play in real time, so frame-numbered `OMT_SHOTS` don't line up with cut scenes).
- Reference timeline of the original (from Wine, launch = 0 s): logos 5-15 s, museum 20-65 s, Level3
  fly-in 70-100 s, lab cut scenes 105-370 s, player control from about 380 s.

## Next

1. **Bill's checks on the Thor** of the build on main; fix what he reports (**Share log** sends `omt-log.txt`).
2. **Open TODOs in the new classes:** the pickup menu announcement 0x408f80 (a menu pickup ticks but shows
   nothing), the OXYGEN pickup's C3DWaterPlane 0x45f2eb (class not written), Jimmy's point cursor over
   pickups 0x4489ac (reads C3DPickupItem `m_show_arrow` +0x914, which now exists). The C3DAI subclasses'
   slot 126 overrides (called when a cut scene releases a character; `C3DAI::ev126` is empty inline in
   `C3DAI.h`) were not checked for the friends/lab classes.
3. **Port features Bill asked about** (after the above, each optional and marked `port:`):
   - **Dual screen:** a companion panel on the Thor's bottom screen (1240x1080), shown/hidden from a
     back-button / edge-swipe menu: health, neutrons, current and owned gadgets, task list (CTaskList), cut
     scene captions, boss health, per-level pickups/checkpoints, a simple top-down map (C3DRadar has the
     data). Read-only game state over JNI; icons read from the player's own game files at run time.
   - **16:9:** render the 3D at the Thor's 1920x1080 width with the same vertical view (more world at the
     sides, nothing stretched); move the HUD pieces hard-wired to 640 (gadget bar, health/neutrons corner)
     to the screen edges; keep 4:3 full-screen art pillarboxed; a setting (4:3 original / 16:9). Risk: cut
     scenes framed for 4:3 may show unfinished edges, so cut scenes may need to stay 4:3.
4. Then the rest of the game: the next .gam files after Level3, the same way (census -> classes -> compare).

## Reference: the research for the Level5 and Level3 classes (all done now)

Bases and sizes from factory.json/classes.json.

1. **The friends** (base **C3DFriends**, C3DAI; no factory ID, ctor 0x436a50): C3DFowl '3FOW' 0x436540,
   C3DSheen '3SHE' 0x46ecd0, C3DLibby '3LIB' 0x44f720, C3DCarl '3CAR' 0x425840 (largest), C3DCindy '3CIN'
   0x427600, C3DNick '3NIC' 0x45a350, C3DBenny '3BEN' 0x421be0, C3DAIJimmy '3AIJ' 0x416af0, C3DNumey '3NUM'
   0x45ab70. Research so far: C3DFriends members 0x1b50..0x1d74 = TalkState0..4 (ints, 0 then -1 x4),
   TalkTrigger0..4 (100-byte strings at 0x1b64/0x1bc8/0x1c2c/0x1c90/0x1cf4, "none"), short 0x1d58, float
   0x1d5c (100.0, counted up by dt), shadow offset 0x1d60..0x1d68 (0,-60,0), C3DShadow* 0x1d70 (SetSize
   90x30; not created in windowed mode). Overrides: InitObject 0x436da0, ResetObject 0x436d70, Process
   0x436eb0 (places the shadow), ev113 0x436d90, new element slots 144..146. Shared compiled copies:
   InitObject 0x44ebc0 / 0x43e920, v260 0x417110, ev60 0x4369f0, v266 0x4368e0, v267 0x436900 / 0x471950.
   The v270 methods use C3DPlayerObject's 40-byte buffer at 0x938 (C3DJimmy declares it as `f_938`; move
   it to C3DPlayerObject). Stub to replace: `LibbySlot` in objects/C3DAITrigger.cpp (0x41a039).
2. **Cut scene cameras etc.:** C3DMultiCutSceneCamera '3MCA' (C3DTriggerType; Level3 has 15 - the story
   cut scenes), C3DFlyingCamera '3FLY' (C3DAI), C3DTree '3TRE' (ctor/dtor only), C3DFogTrigger '3FOG'.
   3MCA research: members 0x7a0..0x134c = eight shots, each CameraTarget, TargetAnim, LookatVOffset,
   SoundIndex, CameraType, SoundNames; InitObject 0x4570b0, Process 0x457400, slots 56-63. Quirks: Process
   passes the shot time from before adding dt to UpdateView (slot 62); UpdateView's 18 camera types each set
   a camera offset (constants read from the exe at run time, not copied); TriggerCamera looks up the C3DAI to
   release from CameraTarget[1]; the shot end hands the camera to PlayerControlled, calls CloseDatabase, then
   CallNextTrigger(NextTrigger, 0). Stubs: CEditor.cpp 0x4a6520, C3DTriggerType.cpp 0x457be0 / 0x433f50,
   C2DInGameMenu_d.cpp 0x406cd4.
3. **Pickups and triggers:** CPickupType (ctor 0x496c20, size 0x7b0: 0x7a0 PickupIndex=0, 0x7a4
   PIC_NUMBER=-1, 0x7a8 RequiredLevel=0, 0x7ac ExactLevel=-1; overrides 7, 242, 260, 266 (shared 0x4368e0),
   267), C3DPickupItem '3PIC' (ctor 0x45dc00; properties 0x7b0..0x938, corona 0x918; AddTouching 0x45e200;
   slot 56 = powerups by tag, calls C3DRadar::Open(g_camera, '3DIN', 23, 16, 40, FindObjectByTag("JOHNNY"));
   57 needs the required pickup; also 58, 59, 10, 240, 260, 268, 284, 285), C3DCheckPoint '3CHK', C3DNeutron
   '3NEU', C3DTransportEffect '3TSP', C3DHelpSound '3HEL', C3DTextTrigger '3TEX', C3DMusicTrigger '3MUS',
   C3DFadeObject '3FAD', C3DArrow '3ARR'.
4. **Lab objects:** C3DBot '3BOT' 0x422250 and C3DHerman '3HER' 0x43e2a0 (C3DAI), C3DTesla '3TES' 0x475ff0,
   C3DSwingDoor '3SWN' 0x474da0, C3DLabScreen '3SCR' 0x44ec70 (mostly worked out: InitObject, a texture
   cycle in Process, empty dtor, timer at 0x924), C3DButton '3BUT' 0x4238e0. Jimmy's C3DBot TODO 0x44512a.
5. **Then integration:** play Level5's opening to its exit `tolab`, then Level3's cut scenes to player
   control, comparing with the original under Wine. Give Bill an APK. (Done, see the status above.)

Other open TODOs seen in runs: texture animations 0x4a28c0 / 0x4a2270 / 0x490300 and shape material setup
0x4a23e0 (likely why the sun billboard behind the Nick logo isn't drawn), C3DRocketShip +0x9f4 show
0x445ce5, C3DJimmy surface flags 0x4848ba, clear layer view matrix 0x4b974e, C3DParticle::Reset 0x49b260,
CGameObject slot 221 0x4ad5e0, C3DBone (Goddard 0x43a94b).

Loose ends:

- `CJimmyGame::GetSceneStage` should use `neg::GetTaskState("SCENE")`; check that it does.
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
