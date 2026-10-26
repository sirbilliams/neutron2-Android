# Handoff: where the project stands and how to continue

Read this whole file before doing anything. Then read `docs/decomp-guide.md`, which holds the conventions.

## Current visual fix — 2026-10-26 Pacific, 1.0.5-rc1 / 105

**Supersedes the failed-inspection blocker and unreleased status below. Actual game now launched.**
After Bill approved GitHub Mobile sign-in, the browser downloaded the ISO. Its initial returned
per-download path disappeared, but the completed transfer was present at
`/workspace/scratch/Jimmy.Neutron.vs.Jimmy.Negatron.iso`. SHA-256 verified exactly:
`65c8fc3f6811b7f03d9bd16211b4b6089c398fad9d9af8a687bbde8098e38d88`.
The browser later denied further observation; it was not retried or bypassed. Local inspection used
only the already-downloaded authorized file. No credentials, game files or captured retail artwork
were added to the public source repository.

### Actual launch and observed correction

- Built the production desktop `negatron` and `neg_import`; installed ISO to
  `/workspace/scratch/06ee8e92910a/private-game` (outside the git checkout).
- Launched Level3 via `NEG_TEST_PLAY`, SDL offscreen video + dummy audio + software OpenGL.
  Used the existing `OMT_AUTOKEYS` hook to open the actual PDA with Escape at frame100/release102.
  Production screenshot hook saved frame600. No fake PDA artwork or mock font rendering.
- Viewed before capture: 1.0.4's four credit rows float centrally under a shrunken scope. Bill had
  asked for flush alignment to the PDA screen edge. The actual inner CRT bounds are approximately
  x80..560, y74..406 in the game's 640x480 space.
- `BillsUi.cpp` now restores original scope x205/y108/size230. Footer: title left x88, By left x88,
  version right-aligned at x552; lowest baseline y398. Eight-unit inset from inner side/bottom edges.
  Title uses MED only if it fits 464 units, otherwise SMAL. Title baseline uses actual SMAL font
  height + eight-unit separation; the game's own fonts are used. Centering is disabled for footer,
  then restored. Existing phosphor colors/fade retained. Vanilla unaffected.
- Viewed real after captures at **1280x720 widescreen and 640x480 4:3**: footer sits just inside the
  CRT lower edge, title/By left, version right; no clipping or scope overlap in these captures.
- Viewed companion normal/expanded previews using actual native Level3 JSON/map/icon dumps through
  restored Java2D tools. Full case background correction remains in this release: previously only
  the left clipped chrome was drawn, so margins became black when the dashboard slid away. The
  case is now independently size-cached and drawn first, behind moving panels. Map chrome remains
  cached at its real width; no horizontal stretch. Synthetic and real-state previews are consistent.
  This is still Java2D preview evidence, not a physical Android display/touch test.

### Reproduce without Xvfb

Local environment lacked cmake/SDL development headers/GL headers and apt setup stalled. Working
setup: install cmake from its PyPI wheel into `/tmp/visual-build-python`; use official SDL2.30.0
source headers with installed SDL2.30 runtime `/usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0`; extract
Ubuntu libgl-dev/libglx-dev 1.7.0-1build1 packages under `/tmp/visual-deps`; unshield1.5.1 source
matches deps.lock. A temporary SDL2Config.cmake supplies imported SDL2::SDL2 and include path.
These are temporary build dependencies, not application source changes.

```
/tmp/visual-build-python/cmake/data/bin/cmake -S engine -B /tmp/neutron-visual-build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSDL2_DIR=/tmp/visual-deps/sdl-config \
  -DOPENGL_gl_LIBRARY=/usr/lib/x86_64-linux-gnu/libGL.so.1 \
  -DOPENGL_INCLUDE_DIR=/tmp/visual-deps/usr/include -DOpenGL_GL_PREFERENCE=LEGACY \
  -DUNSHIELD_DIR=/tmp/visual-deps/unshield-1.5.1 -DCMAKE_CXX_FLAGS=-I/tmp/visual-deps/usr/include
/tmp/visual-build-python/cmake/data/bin/cmake --build /tmp/neutron-visual-build --target negatron neg_import -j8
/tmp/neutron-visual-build/neg_import /workspace/scratch/Jimmy.Neutron.vs.Jimmy.Negatron.iso /workspace/scratch/06ee8e92910a/private-game
```

From the installed game folder:

```
SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1 \
NEG_TEST_PLAY=Level3.gam NEG_UI=bills NEG_APP_VERSION=1.0.5-rc1 NEG_WIDESCREEN=1 \
OMT_WINDOW_SIZE=1280x720 OMT_FRAMES=600 OMT_AUTOKEYS='100:+Escape,102:-Escape' \
OMT_SCREENSHOT=/tmp/neutron-captures/pda-after-wide.bmp \
timeout 15 /tmp/neutron-visual-build/negatron
```

Repeat with NEG_WIDESCREEN=0, OMT_WINDOW_SIZE=640x480 and a different screenshot path.
`NEG_COMPANION_DUMP=/tmp/neutron-captures/dump` on the first run produces real-state panel input;
`tools/companion/render.sh <dump> Level3 <out.png> [far] [expanded]` renders it. Screenshot hooks
saved the requested files and all three launches logged CLEAN EXIT. The known test-hook shutdown
hang may still require timeout; do not claim this unrelated historical issue is fixed.

### Scope, checks, remaining work

Release combines the prior review-branch case-background fix + preview tooling with the measured
PDA footer. Changed files: CompanionView.java; BillsUi.cpp; Android build.gradle version; preview
render.sh, RenderPanel and Canvas/View/gesture/animator shims; CHANGELOG and current handoff/controls/
release-gap documentation. Java2D gesture classes are compile-only and animator is endpoint-only.
Local Android14 Java compilation and existing HD/native regressions passed; native desktop build
and real screenshot inspection passed. CI result will be recorded after publication. Source branch
`fix/companion-background-inspection` preserves the earlier unreleased background fix.

Publication: 1.0.5-rc1/105 source commit `890b85ecfb84880645243a8a0466d15f1ee67c50`.
[CI 40284723379](https://github.com/sirbilliams/neutron2-Android/actions/runs/40284723379) passed
Linux, installer/native regressions, Android arm64 build, stable signing, APK signature verification,
artifact upload and latest-prerelease publication. Backup `backup/pre-pda-footer-5dfafd4` preserves
pre-fix main. Screenshot proof `neutron-pda-1.0.5.png` is saved privately for Bill; no retail screenshot
was committed to this public repo. Local captures: `/tmp/neutron-captures/pda-before.png`,
`pda-after-wide.png`, `pda-after-43.png`, `companion-normal.png`, `companion-expanded.png`.
This handoff-only completion commit skips CI because shipped application source is unchanged.

Remaining: Bill should check the footer and metal appearance on Thor, at both aspect settings,
and test panel transitions/gestures/controller focus. Keep earlier map-detail, height cutoff,
real-HD-pack, save/update/casting/lid and gameplay checks open. No claim of physical device testing.

## Visual-inspection follow-up — 2026-10-25 Pacific (unreleased branch)

Bill reports that **1.0.4 metal still looks wrong and the PDA credits are worse**. He explicitly
wants credits flush with the PDA screen edge, not guessed positions in the center, and requests
that the agent open the game directly. The prior PDA placement is NOT user-accepted or visually verified.
Do not describe the metal/PDA issues as resolved based on CI success.

Inspection attempted: no installed game data, ISO or runnable native binary exists in the workspace.
Private ISO release metadata is accessible via the GitHub connector, but its general fetch tool cannot
transfer binary release assets; unauthenticated direct download returns 404. A temporary private-repo
branch `inspection/pda-companion-20261025` ran a private-only screenshot workflow (no game data was
published). Run `40275514782` failed before any job steps/runner assignment; no logs or screenshots
were generated. Reason not available through supported endpoints; do not assume billing caused it.
The temporary workflow was then removed on that branch with a skip-CI cleanup commit. Private main
was untouched. Native local setup also failed (package-manager privilege errors, then stalled downloads).
**Actual gameplay/PDA menu was not opened.** A local ISO/installed-game ZIP is still needed for that.

What WAS inspected: production CompanionView through the repository's Java2D preview shim with
synthetic state/no map image, at 1240x1080 in normal and expanded endpoints. This is a panel rendering
approximation, NOT Android/gameplay/retail-art or animation/input verification. Visible defect: the
full metal case background was clipped with the sliding info-panel bitmap, leaving black margins
behind/around the map. Unreleased fix caches/draws the full brushed case before the moving panels;
normal/expanded previews were viewed before and after, and the black margins are corrected.
This does not prove every aspect of Bill's reported metal appearance is fixed.

Changes on the follow-up branch: CompanionView adds a separate size-cached case bitmap; existing
width-correct map chrome remains. Restored desktop preview compilation with minimal View lifecycle,
clipRect, MotionEvent, ScaleGestureDetector and ValueAnimator shims. Gesture shims are compile-only;
animator jumps to its endpoint. RenderPanel/render.sh support `far` and `expanded` options together.
No PDA placement change, APK version bump, public release, or real-game screenshot in this follow-up.
Java compilation against Android14 and preview compilation/rendering passed; git diff whitespace check
passed. CHANGELOG records this as unreleased. Next: obtain game data through an authorized binary
transfer, run the real menu, identify the actual PDA screen edge, and align from measured artwork.

## Current handoff — 2026-10-25 Pacific: companion exploration and polish

**Read this section before the historical session status below.** Older claims about a cropped map,
non-touch companion, first-presentation-display selection, and missing diagnostics are historical.
Bill explicitly requires complete change notes and handoff updates for future agents: every change,
test result, user observation, unresolved issue, and published build must be recorded; do not leave
code changes unlogged. Keep this file, CHANGELOG, controls, and release-gap documentation consistent.

### Delivered history and user evidence

| Version / code | Source and evidence | Changes |
|---|---|---|
| 1.0.1-rc1 / 101 | Main before d9ba975; earlier session's published APK and automated checks recorded in chat | HD ZIP replacement staging, rollback/interrupted-swap recovery, expanded-byte space checks, case-collision rejection, concurrent import handling; installer regressions and checksum/signature release gate. |
| 1.0.2-rc1 / 102 | `d9ba975d6006dc25a54135964c13d2cc20ad4396`; published before this session | Bounded native PNG decoding/validation; HD enable toggle and LOW/BALANCED/HIGH startup presets; timestamped Diagnostics and sharing from launcher/port menu; widescreen score animation anchoring. Help icons already left-anchored, so unchanged. |
| 1.0.3-rc1 / 103 | `de3cf72ddaa36d89e7ecb69b964d01091d5ce1fb`; [CI 40272258114](https://github.com/sirbilliams/neutron2-Android/actions/runs/40272258114) passed Linux, Android, regression checks, signature verification and publication | Sliding dashboard, expanded touch map, pan/pinch, recenter and filters; unchanged snapshots retry missing map images. Bill reports it **functions well**, but metal artwork looked poor. Later reports zooming out cannot show terrain beyond the cropped window. This is limited feature feedback, not a complete suspend/cast/save regression pass. |
| 1.0.4-rc1 / 104 | `008e21ace4b55411184f59e87351e1143721aa4e`; [CI 40273912157](https://github.com/sirbilliams/neutron2-Android/actions/runs/40273912157) — see build outcome below | Width-correct metal artwork; full copied-terrain map and Fit Level; PDA title/By/version moved inside central panel. These fixes have **not** yet been visually tested on Thor. |

The release named `latest` is mutable. Match its target commit and APK version before testing;
its workflow title uses UTC (2026-10-26 for this Pacific-evening session).

### Implementation inventory — no omitted changes

**1.0.3: `CompanionView.java`, `CompanionPanel.java`, `docs/controls.md`, `CHANGELOG.md`,
`android/app/build.gradle`.**

- 250 ms ValueAnimator slides the left information panel left; map bounds expand from
  x=440..1220 to x=20..1220 in the existing 1240x1080 logical layout. Collapse restores tracking.
- Touch converts physical coordinates to the letterboxed logical layout. Drag pans below controls;
  ScaleGestureDetector handles pinch. Multi-touch marks the gesture as moved and tracks the
  remaining finger on pointer-up to avoid a drag jump or accidental toolbar tap.
- Expanded-only ITEMS (p/n/r), ENEMIES (e), CHECKPOINTS (c), EXITS (d/x/X) filters; objectives,
  Goddard and friends stay visible. Normal mode ignores filters. Filters last for this panel instance.
- Recenter resets pan and zoom. Exploration compensates for Jimmy's movement to hold the viewed
  terrain steady. Level/map-generation changes reset pan; the panel is recreated on suspend/resume.
- Window remains FLAG_NOT_FOCUSABLE, but FLAG_NOT_TOUCHABLE was removed. Touch modifies only the
  companion view; no game-state write JNI was added. Controller focus on real hardware needs regression checks.
- Desired map generation is tracked separately; identical JSON snapshots now retry missing map
  images. JSON is marked consumed only after successful parse. Animator is canceled on detach.
- Near/Far settings, metrics, inventory, captions, map markers, Vox cards and casting-aware display
  selection remain. Vox temporarily replaces map controls; controller still selects cards.
- Initially the map chrome was cropped from the cached dashboard and stretched; **superseded in 1.0.4**.

**1.0.4: `CompanionView.java`, `game/src/port/Companion.cpp`, `Companion.h`, `BillsUi.cpp`,
`docs/controls.md`, `CHANGELOG.md`, `android/app/build.gradle`.**

- Map chrome has its own bitmap cached at the actual current logical width. During resizing it
  rerasterises at that width; once settled the cache is reused. `drawMapChrome` shares bezel/CRT
  drawing with the dashboard. This avoids horizontally stretching grain, screws and corners.
  No AI image or retail artwork was added. Check transition smoothness/bitmap churn on Thor.
- Native MapWindow now fits **all terrain copied from the currently loaded level** into max
  1536x1536 pixels: scale=max(2, longest extent/1532), with two-pixel margins at each edge.
  This replaces the six-world-units-per-pixel moving crop. It does not load absent geometry.
  Worker thread, terrain texture sampling, read-only snapshots and existing height cut remain.
- Edge-triggered map rebuilds were removed because the map is no longer cropped; level/geometry
  and height-cut changes still rebuild it. Coarser pixels on large levels are the memory/detail tradeoff.
- Expanded mode initially fits the level. New FIT LEVEL button centers the available map with room
  for controls; pinch minimum adapts to the fit scale, maximum remains 4x the Near/Far base.
  RECENTER remains distinct: Jimmy at normal zoom. Pan remains available to inspect distant terrain.
- PDA Start/Esc main page (Bill's UI only): scope changes from (205,108), size230 to (230,96),
  size180 in 640x480 units. Credits become four centered SMAL-font rows below it: title split over
  two rows, `BY: SIRBILLIAMS`, separate VERSION line (desktop falls back to DEV BUILD + commit).
  Baselines use FontHeight, 3-unit row gaps, 8-unit scope gap; brighter phosphor text. Vanilla unchanged.
  Check actual retail font widths and menu overlap at 4:3 and widescreen; no retail assets were available here.
- Version name/code bumped to 1.0.4-rc1/104; install-over should retain saves with the existing stable key.

### Validation and limits

- Local Java compilation: CompanionView/Panel/bridge/BottomScreen compiled using JDK17 and Android14
  API classes (`android-all` compile classpath). This is compile verification, not an Android runtime test.
- `bash tools/test/hdpack.sh` passed with JDK17 on PATH.
- `bash tools/test/native.sh` passed PNG decoder and HUD 4:3/widescreen regressions.
- Ad hoc deterministic math check: 10,000 random positive terrain extents spanning 0.01..10^7 checked
  full-map bounds, 1536 cap, padding coverage and overview fit. This script was a temporary verification,
  not a checked-in regression or a rendered-map test.
- `git diff --check` passed. CI compiles the native changes with the actual Linux/Android toolchains.
- No screenshots or physical device tests were performed for 1.0.4. Existing desktop companion
  preview shim may need Android animation/touch API support; it was not run in this session.
- Full-map raster/height buffer memory remains bounded by the old maximum dimensions; CPU cost and
  coarse-detail quality on large levels still require measurement. Height cutoff can exclude upper
  storeys, intentionally; this is not a multi-floor atlas. Fit Level cannot reveal unloaded geometry.
- No pan bounds, waypoint system, marker detail popup, navigation pathfinding, persistent exploration
  filters, larger caption mode, or low-power mode was implemented; these remain optional future work.
- HD real-pack, save-preserving install-over, casting/lid resume, Asset Viewer controls, Android libm
  parity and existing gameplay TODOs are still open unless separately evidenced. Do not close them
  based on a successful compile or Bill's limited map feedback.

### Next device check / handoff instructions

1. Install 1.0.4-rc1 over the current app without uninstalling; confirm saves and Continue.
2. In Start/PDA menu, check title, By and version inside the panel at 4:3 and 16:9. Capture a screenshot
   if clipping/overlap remains; use actual font metrics rather than guessing from source alone.
3. Expand/collapse the map repeatedly: check metal grain, round corners, screw sizes and frame pacing.
4. On a large level, Fit Level, zoom out fully, pan to distant exits/terrain, then Recenter and collapse.
   Check map/player heading and positions. Move vertically to verify height-cut rebuild behavior.
5. Toggle each filter; test one-finger drag, pinch and lifting either finger. Check controller focus.
6. Test Vox, level transitions, lid/app suspend and casting while expanded. Export display/diagnostic logs.
7. Record exact APK version, commit, firmware, level, observation and screenshots/logs in this file.

Backups: `backup/pre-map-exploration-d9ba975` and `backup/pre-map-polish-de3cf72` preserve the prior
source states. `feature/companion-map-exploration` points to the 1.0.3 source; main has newer polish.
HTTPS git push has no credentials in this workspace; authenticated GitHub connector Git tree/commit/ref
calls were used. Compare local/remote tree SHA when using that path (1.0.4 tree
`949436e9f4560b679c58cc00675a9d4ed5bb24a9` matched exactly). Never force-push over unrelated changes.

### Build outcome

[CI 40273912157](https://github.com/sirbilliams/neutron2-Android/actions/runs/40273912157) completed
successfully: Linux engine/game build, Java HD installer checks, native PNG/HUD checks, Android arm64
APK build, stable-key signing, APK signature verification, artifact upload and latest-prerelease
publication. Verified release target is `008e21ace4b55411184f59e87351e1143721aa4e`; APK size is
20,278,705 bytes. [APK](https://github.com/sirbilliams/neutron2-Android/releases/download/latest/Neutron-vs-Negatron.apk)
and adjacent `.apk.sha256` are published. No physical 1.0.4 Thor tests have been reported yet.

This documentation-only follow-up updates HANDOFF, README exploration instructions and release-gap
status to match the shipped source. Its `[skip ci]` commit leaves the above APK/source target intact;
no application code changed after that build. Future application changes must carry matching handoff
and changelog updates before publication.


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
  https://github.com/sirbilliams/neutron2-android/releases/download/latest/Neutron-vs-Negatron.apk (the old name Negatron-Viewer.apk still works)
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

## Status at v1.0 (Oct 22, session 4, cap-off) — READ THIS FIRST

- **Done and on main:** the whole game plays from the logos to "YOU WIN" (20 story levels + VR01; level4a has
  no art on the disc). **Bill played the full chain on the Thor: "perfect, no complaints".**
- **v1.0 polish:** app "Neutron vs Negatron", vector atom icon, versionName 1.0 / versionCode 100, APK
  `Neutron-vs-Negatron.apk` (the "latest" pre-release also carries `Negatron-Viewer.apk`), player README.md,
  THIRD_PARTY.md + assets/licenses/, CHANGELOG.md. Pushing a `v*` tag makes a permanent release whose notes
  are that version's CHANGELOG section.
- **Signing:** CI signs with Bill's stable key from the secrets `NEG_KEYSTORE_B64` / `NEG_KEYSTORE_PASSWORD`
  (alias `negatron`; build.gradle `stable` config when `NEG_KEYSTORE` is set). The workflow strips stray
  whitespace and checks the key opens, else fails with a clear error. Bill holds the only copy of the
  keystore + password: never in the repo; losing it means one more uninstall for players.
- **Phase 3 port features (all `port:`):** launcher main menu (Continue / New Game / Load Game / Settings /
  Asset Viewer / Quit; steel/green-LED style, the menu-style toggle was removed, box art on the bottom
  screen); Exit Game returns to the launcher; save export/import; suspend on lid close; dual-screen panel on
  by default when a second display exists (gadget sheet + collectables; Tasks/Lives removed; level display
  names via `neg::SaveLevelName`); only the top-screen game view casts (BottomScreen.java skips
  cast/HDMI/virtual displays); Bill's UI vs Vanilla (`NEG_UI`), energy bar styles a/b/c (`NEG_UI_ENERGY`),
  hologram frames a/b/c/off (`NEG_UI_HOLO`), pause screen version line; Vox picker fix.
- **Asset Viewer (agent A, `game/src/port/AssetView.*`):** `NEG_ASSET_VIEW=<level>.gam` = frozen level +
  noclip camera (sticks move/look, L/R up/down, A speed, Y fog, X hide Jimmy, Start reset, B leave; saves
  are never written: guards in WriteTaskFile/DeleteFile); `NEG_ASSET_VIEW=models` = all 744 .grn/.ase/.omt
  shapes with orbit/zoom. Launcher "Asset viewer" page. Desktop-tested only.
- **HD texture pack (agent B):** see "Next" 3 and docs/hd-pack.md. Not yet run on the Thor with Bill's real
  upscaled pack (he has the texture zip `negatron-textures.zip` for upscaling); watch the log's "HD:" lines.
- **Known open items:** the Smaller TODOs and untested-real-play list in the session 3 status below; the
  HD pack's oversize clamp and wrong-size skip are untested; NEG_TEST_PLAY runs hang after CLEAN EXIT.
- **To resume:** set up per "Setting up a workspace"; briefs for agents: `docs/agent-brief-p2.md` (update
  paths/commit first). Private (not GitHub-public) game-art repo: `negatron-gamefiles` (menu art, box art).

## Status at the end of session 3 (Oct 21, phase 2)

- **The whole story chain plays through** (main 8b7cee9, Android CI green, Bill has this APK):
  level1d (logos) -> Level5 (museum) -> Level3 (lab: 3 parts -> water gun -> button pad -> bots hunt ->
  PDA exit) -> Level3c (ravine, hover cycle vs the tidal wave) -> Level3d (dock, crane drops 4 sub parts)
  -> Level3a (sub) -> Level3e ... and the level1 run (level1b -> level1_b -> level1_a (scooter/jeep) ->
  level1_c -> Level1) -> level1c -> level5b -> level5b1 -> Level4b -> level5f / level5d / level5c / Level5a
  -> Level3e (lab escape; the game's ending reaches the "YOU WIN" score box). 20 levels.
  "Plays through" was checked per objective with temporary teleport/force hacks, not by a full manual run.
- **Not reached / not done:**
  - **VR01** (bonus level, no story level leads to it; level picker): done (C3DVRTank, C3DFlyingBot /
    C3DFlyingBug, C3DGiantWasp, C3DEnergyProjectile). Its sequence, from its own triggers: intro
    (bugsleep1 .. jim2tank puts Jimmy in the tank) -> five gates (block05..01), each opened by hitting 8
    moving targets (cond<n>trig; aim off the centre post, at rest: the shots climb) -> the giant wasp
    (its death switches BLOCKWASP off; its WASPDIE trigger does not exist in VR01) -> drive onto the
    touch trigger curevox in the arena -> outro -> vrtank_done -> loadlab -> level1b.gam. Checked per
    segment with teleports (gate 5, the wasp fight, curevox to level1b), not one full manual run.
  - **The sub's headlight** (key L): also on L3 (left stick click), `port:` in OMediaSDLInput.cpp,
    docs/controls.md.
  - **C3DBub** ('3BUU', the mini sub's 10 trail bubbles): done; C3DMiniSub creates them directly (the
    weak hook is gone). Level3a / Level3e run with them (bubbles rise and fade; no TODO).
  - **level4a**: C3DUltraLord is written, but the game files have no Ultra Lord art (ultra*.ase,
    ultralord.png), so the level can't load its textures; no level leads to it. Nothing to do.
  - Untested real play (hacks were used): shooting the dinos/pteros in Level5f, Level4b's 25-hit win,
    the dock crane's manual controls, the lab's bot2 sometimes gets stuck, a soft lunar-lander landing.
  - Smaller TODOs still logged: score pop-up not placed for widescreen, frame-rate overlay 0x4b2d90,
    menu phone-booth teleport 0x4648a0, C3DVehicle::ev110 (unused), surface hurt on '3COR' 0x4847c1,
    the missing sun billboard behind the Nick logo (not texture animations; still unknown).
  - Runs with NEG_TEST_PLAY print CLEAN EXIT and then hang until `timeout` (rc 124); pre-existing.
- **Port features on main:** 16:9 widescreen (on by default on Android, setup checkbox; HUD at the
  edges, full-width letterbox, touch mapping; the bottom-left mouse help icon didn't move outward);
  the level picker ("Jump to a level (test)"); the bottom-screen panel (hold Back 0.6 s or double-tap ->
  menu: panel on/off, map zoom near/far): stainless steel / green LED theme, real top-down level map
  from the level geometry, the game's own icons read at run time, tasks lit when active (no "done" state
  found). Desktop preview: `NEG_COMPANION_DUMP=<dir>` + `tools/companion/render.sh <dir> <Level> out.png`.
  Bill tried it on the Thor's bottom screen: "looks great", some polish still to do (ask him which).
  HD texture pack: loader + install done (docs/hd-pack.md).
- **Bill's requests from this session (all done in session 4, see above):**
  - Dual-screen panel ON by default when a second display is present (off otherwise); an explicit
    choice by the player is remembered and wins.
  - A main menu with **Continue** (autosave), **New Game** (warns: it wipes the autosave, as the
    original does) and **Load Game** (the game's own save slots, so saves stay compatible), plus a
    Settings page (widescreen, dual screen, later the HD pack) and the level picker under a hidden
    "test" item. **Two menus, switchable by a toggle:** (1) the original's own title menu, restored
    faithfully; (2) a new launcher menu in the panel's stainless-steel / green-LED style, Jimmy
    Neutron-inspired (no show logo or copied art; the game's own art read at run time is fine). On the
    Thor the bottom screen can show the menu options / a save-slot preview while choosing.
- **Bill's open checks:** play the full chain on the Thor; his list of panel polish items.
- **Method notes from this session:** the census can't see classes created at run time (projectiles,
  effects): play the level and grep the log for TODO. Android's clang enforces C++98 (no `T{...}`,
  `auto`, `nullptr`); check CI on each backup branch. Agents often wrote the same shared helpers in
  C3DJimmy*/CJimmyGame.cpp: tell agents to `git merge backup/integ` before touching those. Five agents hit
  the account's session usage limit after ~3 h; four is safer. Resumed agents with very long histories cost
  more than fresh agents with a short brief (the phase-2 brief used this session: `docs/agent-brief-p2.md`; update its paths/commit first).

## Status at the end of the Oct 21 session 2

- **Reached:** a normal Play (no test hooks) runs level1d.gam (THQ/AWE logos) -> LOAD `startgame` ->
  Level5.gam (museum cut scene) -> Level3.gam (the underwater fly-in `labflyin1`, then several minutes of
  lab cut scenes) -> **player control of Jimmy in the lab** at about 375-380 s from launch, as in the
  original under Wine (about 380 s). Checked frame by frame against the original's screenshots (every
  5-10 s) and with the virtual pad: holding the stick forward from launch (`OMT_FAKE_PAD="5:lefty=-32767"`)
  does nothing in the cut scenes and walks Jimmy once control starts. Both levels load all objects
  (`tools/decomp/gamcensus.py` reports nothing missing). Smoke test: CLEAN EXIT. main = Android CI green.
- **Bill tested it on the Thor:** "runs fantastic", all cut scenes true to the original. He reported two
  bugs (the lab's bolt barrier off its posts, Jimmy jittery when standing still); both are fixed on main
  (see "Next" 3). The Granny code is still unchecked against Android's libm.
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
   Next real test: Bill's upscaled HD pack and the Asset Viewer on the device.
2. **Open TODOs in the new classes:** the pickup menu announcement 0x408f80 (a menu pickup ticks but shows
   nothing), the OXYGEN pickup's C3DWaterPlane 0x45f2eb (class not written), Jimmy's point cursor over
   pickups 0x4489ac (reads C3DPickupItem `m_show_arrow` +0x914, which now exists). The C3DAI subclasses'
   slot 126 overrides (called when a cut scene releases a character; `C3DAI::ev126` is empty inline in
   `C3DAI.h`) were not checked for the friends/lab classes.
3. **Port features Bill asked for** (all `port:`, read-only or off by default; state at the end of session 2):
   - **Bug fixes after Bill's Thor test (done, on main):** the lab's Tesla bolt sat at (0,0,0) because
     C3DTesla slot 10 (0x482a70) is a jump to CLocalGameObject::ResetObject, not empty; Jimmy jittered when
     idle because C3DPlayer::ev109 (0x461b50) reset its turn-animation timers f_b14/f_b18 to 0.1 instead of 0.
   - **Dual screen companion panel (on agent-e; untested on a real second screen):** `game/src/port/
     Companion.*` snapshots read-only state as JSON (<= 10/s, only while the panel is on) and draws a
     top-down map of the level: the terrain shapes (C3DTerrain, "ik" collision-only shapes' floors left
     out) are copied once per level and a worker thread rasterises the floors (normal.y >= 0.7, textures
     box-filtered to 32x32 and sampled at the UVs, highest floor under Jimmy's feet + 350 winning, so
     roofs and storeys above him drop out), ledges and wall tops outlined; 6 world units/px, a 1536 px
     window that follows Jimmy (re-drawn near its edge or when his height changes by > 200). Axes checked
     on Level3 with the camera projection (map x = +x, map up = +z) and his heading (local +z, v226)
     against his walking direction. Icons are the game's own canvases read at run time: gadgets (the
     inventory page items' pictures), collected items and map pickups (the pickups page, by PIC_NUMBER),
     neutrons / checkpoints / objective (the objects' own sprite canvases). Exits show open/shut by
     CLoadLevel's own task test. Android: `Companion.java` (JNI: snapshot, map, icon), `CompanionPanel.java`,
     `CompanionView.java` (stainless-steel / green-LED theme, all drawn in code; map in a green CRT window
     with a sweep; Back menu: panel on/off, map zoom near/far). Desktop check: `NEG_COMPANION_DUMP=<dir>`
     writes the map, snapshot and icons; `tools/companion/render.sh <dir> <Level> out.png` draws the real
     CompanionView through a small Java2D shim. Open: task "done" states (the stage numbers' meaning is
     per task), the boss name is fixed to Herman, the display choice is the first presentation display.
   - **16:9 widescreen (done; on by default on Android, off on the desktop):** `NEG_WIDESCREEN=1` makes the
     window 480 lines at the panel aspect (854x480 at 16:9); the 3D shows more at the sides with the
     original vertical view; 2D stays a centred 640x480 area, with the edge HUD pieces anchored to the
     screen edges, the letterbox bars and fade widened, the cut-scene PiP viewport and mouse/touch shifted
     by the margin (`game/src/port/Widescreen.h`). Android: SetupActivity "Widescreen (16:9)" checkbox
     (prefs, default on). Unchecked visually: the PiP viewport (no PiP seen in the Level5/3 openings).
     Test: `NEG_WIDESCREEN=1 OMT_WINDOW_SIZE=1920x1080` vs `NEG_WIDESCREEN=0`.
   - **HD texture pack (done, docs/hd-pack.md):** `hd/omt/<db>/<id>.png`, `hd/png/<name>.png`,
     `hd/grn/<model>_<n>.png` (integer multiples of the originals) are uploaded as the textures of the
     canvases they stand for (engine/sdl/OMediaHDPack.cpp; hooks in OMediaCanvas, OMediaDataBase,
     OMediaGLCanvas, C3DAnimated::BindTex); the game's sizes stay the original. Launcher Settings
     "HD texture pack: install (.zip)" / "remove"; desktop `neg_hdtool install|remove`. Tested on the
     desktop with a fake 2x pack (3D, HUD, full-screen pause art, granny, loose png); not yet on the Thor
     with Bill's real pack (watch the log's "HD:" lines: replaced count, memory). Upscaler notes:
     Real-ESRGAN ncnn-vulkan (realesr-animevideov3 fast, realesrgan-x4plus-anime sharper); upscale the
     alpha separately and keep hard alpha hard. THE PACK IS GAME ART: never in this repo.
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
