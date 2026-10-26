# Jimmy Neutron vs. Jimmy Negatron Recomp

**Neutron vs Negatron** for Android: the 2002 PC game *Jimmy Neutron vs. Jimmy Negatron* (AWE Games /
THQ), rebuilt as a faithful C++ reconstruction of its original code and running natively on Android.
Made for the **AYN Thor** (both screens, its gamepad, its lid), and it runs on other arm64 Android
devices with a gamepad or touch screen.

The whole story plays, from the logos and the museum to the escape from Negatron's lab and the end,
with the original's saves, menus, sounds and quirks (the VR bonus level is playable too, from the
Test page's level picker). On top of the
original there is an optional 16:9 widescreen mode, a second-screen companion panel and an optional
refreshed in-game UI ("Bill's UI").

**You need your own copy of the game.** Nothing of the game is included in this app or this
repository: you install it from the game's CD (an `.iso` image of the disc). The reconstructed code
reads what it needs from your copy when it runs.

## Installing on the AYN Thor

1. On the Thor, download the APK of the newest release:
   **https://github.com/sirbilliams/neutron2-android/releases/latest** (open its APK asset).
   For the newest verification candidate, rebuilt on main changes:
   https://github.com/sirbilliams/neutron2-android/releases/download/latest/Neutron-vs-Negatron.apk.
   This development APK is a prerelease; see [release gaps and the Thor smoke test](docs/release-gap.md).
   Open it and allow installing from your browser / file manager when Android asks.
2. Copy an image of the game's CD onto the Thor (e.g. `Jimmy Neutron vs. Jimmy Negatron.iso` into
   Downloads).
3. Open **Neutron vs Negatron** and tap **Choose the game's .iso…**, then pick that file. The app
   installs the game from it the way the CD's setup program does (about 270 MB, a minute or two). The
   `.iso` isn't needed afterwards.

   Also accepted: a zip of an installed PC copy's `Program_Executable_Files` folder, or that folder's
   contents copied over USB into `Android/data/com.sirbilliams.negatron/files/`.

**Updating:** install the new APK over the old one; the game and your saves stay. If Android refuses
("App not installed" / "conflicts with an existing package": older builds were signed with a
different key), export your saves first (**Saves → Export saves**), uninstall, install the new APK,
install the game again from the `.iso` and import the saves.

## The launcher

The app opens on a menu in a stainless-steel and green-LED lab style. Pad: d-pad / left stick to
choose, A to select, left / right to change a setting, B to go back. Touch works too.

| | |
|---|---|
| **Continue** | carry on from the autosave |
| **New Game** | start the story from the beginning (asks first: it replaces the autosave, as the original does) |
| **Load Game** | the game's ten save slots, with level, score and date |
| **Saves** | export all saves to a `.zip` (Downloads, Drive...) or import them back; Y on a Load Game slot exports just that one. They are the PC game's own save files, so they also work in the PC version's `save` folder and back |
| **Settings** | see below |
| **Diagnostics** | live render FPS, game resident memory and HD counts (last sample in launcher), displays/controllers, and Share report with logs |
| **Asset Viewer** | fly freely through any level (noclip) or browse the game's 3D models, from your copy |
| **Quit** | close the app |

When the game ends (the pause menu's Exit), you are back on the launcher.

## Settings

| Setting | |
|---|---|
| **Widescreen 16:9** | fills the screen instead of the original 4:3; the HUD moves to the screen edges. On by default |
| **Dual screen** | the companion panel on the Thor's bottom screen (below). On by default when a second screen is there |
| **In-game UI: Bill's UI / Vanilla** | Bill's UI is a refreshed look for the HUD (energy meter, gadget hologram, framed help pictures, a livelier score, a radar scope on the pause screen). Vanilla is the original's look |
| **Energy meter: Tube / Orbit / Dial** | the energy meter's design with Bill's UI |
| **Gadget hologram: Bay / Console / Wheel / Off** | how the current gadget is shown with Bill's UI |
| **Box art** | a picture of your own game box for the bottom screen's main menu (choose / clear); none is included |
| **Menu background** | a picture of your own behind the launcher (choose / clear); brushed steel without one |
| **HD rendering / memory preset** | keep a pack installed while turning it off; LOW / BALANCED / HIGH limits, applied on the next game launch |
| **HD texture pack** | install a pack of higher-resolution textures (a `.zip` you supply); none is included |

The UI, energy meter, gadget hologram and dual-screen settings can also be changed during play from
the port menu (hold Back, below).

## Controls

The Thor's pad maps onto the original's joystick and keys:

| | |
|---|---|
| Left stick / d-pad | move and turn |
| Right stick | camera (mouse look); pointer in menus |
| A | action / fire, skip a cut scene |
| B | jump |
| X | brake (vehicles) |
| Y | inventory |
| Start | pause menu (save, load, options, exit) |
| Select | help |
| R1 / L1 | turn around / show or hide the HUD |
| R2 / L2 | yes / no in questions |
| L3 | the mini sub's headlight |
| Back: tap / hold or double-tap | Esc / the port menu |
| Touch | the mouse pointer |

Every control, the launcher and the port menu in detail: [docs/controls.md](docs/controls.md).

## The bottom screen

With dual screen on, the Thor's bottom screen shows the launcher's options and save-slot previews
(your box art on the main menu) and, in play, a companion panel styled as a stainless-steel lab
gadget: a map of the level drawn from its own geometry that follows Jimmy (pickups, friends, doors,
exits, the current objective), his energy and score, the current gadget, the level's collectables
and the inventory. **Hold Back** (or tap it twice) during play for the port menu: panel on / off,
map zoom near / far, the UI settings above, and Diagnostics. Diagnostics refreshes every two seconds
while rendering; the launcher labels the saved sample with its timestamp.

Casting the Thor to a TV shows the game; the bottom-screen panel stays on the Thor.
Closing the lid or switching apps pauses the game where it is.

Tap **Expand Map** on the companion to slide the info panel away. Drag/pinch to explore, toggle
marker filters, use **Fit Level** for all available level terrain or **Recenter** for Jimmy, then
**Dashboard** to restore the normal panel. Large levels use a coarser overview within the map
image-size limit; geometry not loaded by the game cannot appear. New 1.0.4 visuals await Thor testing.

## Known issues

- The Asset Viewer was desktop-tested; the real HD pack and texture-limit fallbacks still need
  Thor verification. See [the release-gap checklist](docs/release-gap.md).
- Some objectives were checked with test shortcuts rather than a full manual play-through (Level5f's
  dinosaurs, Level4b's boss fight, the dock crane's manual controls, a soft lunar-lander landing);
  please report anything that blocks you.
- In the lab, one of the goof bots can get stuck.
- Widescreen: the side score pop-up now follows the right edge throughout its animation. The help
  icons already have left-edge anchors in code; their appearance still needs confirmation on Thor.
- The sun behind the Nickelodeon logo at the start is missing.
- The log is at `Android/data/com.sirbilliams.negatron/files/omt-log.txt` (also Test → Share log:
  press Select / Y on the main menu to show the Test item). Please attach it to a bug report.

## Credits and legal

**Jimmy Neutron vs. Jimmy Negatron Recomp** by **sirbilliams**.

This is a fan project. It is not affiliated with or endorsed by Nickelodeon, Viacom or THQ (or AWE
Games). *Jimmy Neutron* and all of the game's content (art, sound, music, text, levels) belong to
their owners; none of it is included here, and you need your own copy of the game.

The engine is our port of the Open Media Toolkit 2.5 by Yves Schmid (LGPL 2.1; its full source is in
`engine/`), with SDL2, gl4es and unshield: see [THIRD_PARTY.md](THIRD_PARTY.md) for their licences.
Release notes: [CHANGELOG.md](CHANGELOG.md).

## For developers

- [docs/HANDOFF.md](docs/HANDOFF.md): where the project stands, the rules, how to set up and continue.
- [docs/decomp-guide.md](docs/decomp-guide.md): how the game's code is reconstructed, conventions.
- [docs/controls.md](docs/controls.md): input, the launcher's interface (`NEG_START`), test hooks.
- `engine/README.md`: every change made to OMT.

**Building.** Every push builds the APK on GitHub Actions (Actions → the run → *Neutron-vs-Negatron-apk*);
pushes to `main` update the `latest` development release, and a pushed `v*` tag makes a permanent
release with its [CHANGELOG](CHANGELOG.md) section as notes. Locally:

```bash
bash build-android.sh          # podman; -> ~/Negatron-Android/Neutron-vs-Negatron.apk

# desktop Linux (engine + game, for development)
sudo apt install build-essential cmake libsdl2-dev libgl-dev zlib1g-dev
cmake -S engine -B build && cmake --build build -j
cd /path/to/Program_Executable_Files && /path/to/build/negatron
```

On a PC, `build/neg_import game.iso folder/` installs the game from the `.iso` the same way.

**Signing.** CI signs with the key in the `NEG_KEYSTORE_B64` / `NEG_KEYSTORE_PASSWORD` repository
secrets (base64 of a keystore whose key alias is `negatron`), so each APK installs over the last;
published builds require them. Non-publishing test builds may use a one-off debug key.
CI builds the Linux target and runs `bash tools/test/hdpack.sh` before building Android; release assets
include an APK SHA-256 checksum.
