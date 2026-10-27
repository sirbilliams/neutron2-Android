# Changelog

## Unreleased tooling / private HD pack repair (2026-10-26 Pacific)

- Confirm and reproduce neon corruption already present in the supplied upscale PNGs; restore clean
  HD-enabled desktop Level3 rendering with a privately rebuilt 2x resampled pack (no AI enhancement).
- Add an original-export-to-2x-pack tool, alpha/path/roundtrip checks, optional full native decoder
  comparison and five regression tests. Validate all 2,481 rebuilt PNGs through the game decoder.
- Record exact input/output hashes, desktop evidence, installation and remaining Thor checks in HANDOFF.
  Application code and published APK remain 1.0.5-rc1 / 105; game artwork stays outside this repo.

## v1.0.5-rc1

- Restore the brushed case background behind the sliding companion panels; preview-verified with
  synthetic state, not yet confirmed on Thor.
- Restore Java2D companion previews with compile-only gesture and endpoint-animation shims; support expanded snapshots.
- Launch the actual desktop game from the verified retail ISO; replace centered PDA credits with
  a footer inset eight units from the CRT side/bottom edges. Restore the original scope size.
- Visually inspect real PDA captures at 640x480 and 1280x720. Record setup, evidence and limits in HANDOFF.
- Version code 105; physical Thor verification remains pending.

## v1.0.4-rc1

- Render companion metal/bezel artwork at its real width rather than stretching the dashboard cache.
- Fit all copied level terrain into the existing 1536-pixel map budget, with a Fit Level button
  and zoom range that reaches the full overview. Unloaded geometry remains unavailable.
- Move title, By credit and version inside the PDA menu in separate readable lines.
- Version code 104; Thor visual verification pending.

## v1.0.3-rc1

- Expand the companion map with a sliding left dashboard, touch pan/pinch zoom, Recenter,
  and marker filters; collapse restores normal player tracking.
- Keep controller focus with the game and keep all companion interactions read-only.
- Retry unavailable map images even when the snapshot has not changed.
- Version code 103; new touch/animation behavior awaits physical Thor testing.

## v1.0.2-rc1

- Bound HD PNG working memory before large allocations; validate PNG CRCs, colour/depth combinations,
  methods, filters and palette indices. Unusable replacements fall back to original textures.
- Add HD rendering on/off and LOW / BALANCED / HIGH memory and scale presets (next launch).
- Add Diagnostics to the launcher and in-game Back menu: rendered FPS, resident game memory, HD
  accounting, display/controller detection, timestamped samples and Share report with logs.
- Keep side-positioned score pop-ups anchored in widescreen throughout the animation.
- Add native synthetic PNG and 4:3/widescreen coordinate regression checks to the CI release gate.
- Version code 102; physical Thor verification remains open. No game data or HD artwork bundled.

## v1.0.1-rc1

Thor verification candidate; device checks remain in [docs/release-gap.md](docs/release-gap.md).

- Android HD pack replacement preserves the old pack until staging succeeds, rolls back failed
  swaps and recovers interrupted swaps. Check expanded free space even with unknown ZIP size;
  reject ambiguous duplicate names and malformed paths. Report removal failures accurately.
- Add synthetic Java installer regression checks, including corrupt archives, low storage and
  failed swap recovery. Gate Android delivery on these checks and the Linux build.
- Require stable signing for published APKs, verify signatures and publish SHA-256 checksums.
- Correct the README's stable download link and document the remaining Thor verification gaps.
- Version 1.0.1-rc1 / code 101. No game data or HD artwork is bundled.

## v1.0

The first release of **Neutron vs Negatron**, the *Jimmy Neutron vs. Jimmy Negatron* recomp for
Android (it used to be the "Negatron Viewer" level fly-through). You need your own copy of the game
(its CD image); see the README.

### Game
- The whole story plays through, as on the PC: the logos, the museum, Negatron's lab, the hover
  cycle against the tidal wave, the dock and its crane, the mini sub, the scooter run, the rocket
  ship, the lunar lander, the dinosaurs, the escape from the lab and the ending's score screen.
  20 levels, plus the VR bonus level (Test page → Jump to a level).
- Every level's own objects rebuilt from the original code: friends and family, enemies and
  bosses, vehicles (hover cycle, scooter, jeep, mini sub, rocket ship, lunar lander, VR tank),
  gadgets and inventions, pickups, checkpoints, doors, lifts, buttons, cut scenes, music and voices.
- The original's pause menu, inventory, save slots and autosave; out of energy takes you back to
  the autosave, as the original does.
- Vox's world picker works with the gamepad, the worlds shown as cards on the bottom screen.

### Launcher and saves
- A new main menu in a stainless-steel / green-LED lab style: Continue, New Game, Load Game, Saves,
  Settings, Asset Viewer and Quit; gamepad and touch, a drawn picture for each option.
- Load Game shows the game's ten slots with level, score and date.
- Saves: export all saves (or one slot) to a `.zip`, import them back with a preview before
  anything is replaced. The files are the PC game's own, so saves move between PC and Thor.
- Your own box art and menu background pictures (Settings).
- Asset Viewer: fly freely through any level (noclip, frozen, no saves touched) or browse all of the game's 3D models.
- HD texture packs: install a pack of higher-resolution textures from Settings.
- The game installs from the CD's `.iso` in the app (or from a zip of an installed copy).
- New name and icon; the version shows on the pause screen.

### Dual screen
- A companion panel on the Thor's bottom screen: a live map of the level drawn from its geometry
  (pickups, friends, doors, exits and where they lead, the current objective), energy dial, score,
  current gadget, the level's collectables and the inventory.
- On by default when a second screen is present; your choice in Settings or the port menu wins.
- The launcher uses the bottom screen too: your box art on the main menu, options and save-slot
  previews on the other pages.
- Hold Back (or tap it twice) in play for the port menu: panel on / off, map zoom, UI settings.

### Bill's UI
- An optional refreshed in-game look (Settings → In-game UI; Vanilla keeps the original's).
- Energy meter in three designs (Tube, Orbit, Dial): smooth drain and fill, a flash on gain, a
  flicker when hurt, a warning pulse when low.
- Gadget hologram in three designs (Bay, Console, Wheel) or off.
- Help pictures and the scooter's rear-view mirror framed on green hologram plates; a livelier
  score; a radar scope and credit line on the pause screen; the sonar readout under the radar.

### Widescreen
- 16:9 widescreen, on by default: the HUD goes to the screen edges, letterbox bars and fades cover
  the whole screen, touch and the pointer map correctly, cut-scene picture-in-picture fits.

### Android
- Native arm64 build for the AYN Thor and other Android devices (OpenGL ES through gl4es).
- The Thor's pad mapped onto the original's joystick and keys (L3: the mini sub's headlight).
- Suspends cleanly when you switch apps or close the Thor's lid (sound and game time pause).
- Casting to a TV shows the game; the bottom-screen panel stays on the Thor.
- The pause menu's Exit returns to the launcher.
- Licences of the bundled libraries inside the APK (see THIRD_PARTY.md).

### Fixes
- Engine bugs that broke the game's data on 64-bit and ARM (empty strings, RLE decoding, palette
  lookups, 32-bit `long`), and an `atan2` case that crashed the dinosaur level.
- Cut scenes' picture-in-picture showed only its background.
- Invisible pickups; missing targeting arrows; two collision filters read byte-swapped (Goddard's among them).
- The sonar readout was cut off at the top of the screen (both UI modes).
- The rear-view mirror is centred in widescreen.
