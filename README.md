# Negatron Android

An Android port, in progress, of the PC game **Jimmy Neutron vs. Jimmy Negatron** (AWE Games / THQ, 2002),
aimed at the **AYN Thor** and other Android handhelds.

The game runs on the **Open Media Toolkit (OMT)**, an LGPL C++ engine. This repo has OMT ported
off Windows/DirectX onto **SDL2**, made 64-bit and ARM clean, with **gl4es** translating its
OpenGL 1.x renderer onto OpenGL ES 2. Nothing from the game is in this repo; you use your own
copy of the game files.

## Status

| | |
|---|---|
| Engine (OMT 2.5) on SDL2, 64-bit | done — desktop Linux + Android |
| Game data (all 84 `.omt` archives) | loads; decodes bit-identically on x86-64 and ARM64 |
| Rendering via gl4es / GLES2 | done — matches desktop OpenGL output |
| Sound (OMT sound engine → SDL mixer) | done |
| **Viewer app** (fly through the game's levels) | **this APK** |
| Character models (RAD Granny `.grn`) | next |
| Game logic (`Neutron2.exe`, 238 classes) | to be decompiled |

## The viewer app

Flies a camera through the game's levels, rendered by the game's own engine.

| | Gamepad (Thor) | Touch | Keyboard |
|---|---|---|---|
| Move | left stick / d-pad | two-finger drag | WASD |
| Look | right stick | drag | arrows, R/F |
| Down / up | L2 / R2 | | Ctrl / Space |
| Fast | hold A | | Shift |
| Previous / next level | L1 / R1 | tap left / right edge | PageUp / PageDown |
| Reset camera | Start | | Home |
| Quit | B / Back | | Esc |

## Installing on the Thor

1. Install `Negatron-Viewer.apk` and open it once. It shows "Game files not found"; that's expected.
2. On your PC, copy everything **inside** the game's `Program_Executable_Files` folder
   (`omt/`, `gam/`, `grn/`, `ase/`, `png/`, `dat/` ...) into `Android/data/com.sirbilliams.negatron/files/`
   on the Thor (USB file transfer). Copying the `Program_Executable_Files` folder itself also works.
   The files come from the game's install CD (`data1.cab`, extractable with `unshield`) or an installed copy.
3. Open the app again.

Log: `Android/data/com.sirbilliams.negatron/files/omt-log.txt` (rewritten each launch).

## Building

**GitHub:** every push builds the APK (Actions → latest run → *Negatron-Viewer-apk* artifact).

**Your PC (podman):**
```bash
bash build-android.sh          # -> ~/Negatron-Android/Negatron-Viewer.apk
```

**Desktop Linux (engine + viewer):**
```bash
sudo apt install build-essential cmake libsdl2-dev libgl-dev zlib1g-dev
cmake -S engine -B build && cmake --build build -j
cd /path/to/Program_Executable_Files && /path/to/build/omt_viewer
```

## Layout

| Path | |
|---|---|
| `engine/OMTClasses/` | OMT 2.5 engine source, ported (see `engine/README.md` for every change) |
| `engine/sdl/` | SDL2 platform layer: windows, GL context, input, file I/O, sound mixer, entry point |
| `engine/viewer/` | the level viewer app |
| `engine/tests/omt_datatest.cpp` | loads every game archive and fingerprints the decoded data (cross-architecture check) |
| `engine/tools/portify.py` | makes MSVC-era OMT client code 64-bit/Linux clean |
| `android/` | Gradle project (SDL2 2.32 template, AGP 8.1.1, NDK r26), `jni/CMakeLists.txt` |
| `fetch-deps.sh`, `deps.lock` | pinned SDL2 and gl4es sources |
| `build-android.sh`, `Containerfile.android` | local podman build |

## Engine fixes found along the way

Bugs in stock OMT 2.5 that broke the game's data on 64-bit or ARM:
- Empty Pascal strings were read with `read(buf, -1)`, smashing the stack (the game's archives contain them).
- The 8-bit RLE decoder used plain `char` for run counts, so it fails on ARM (unsigned `char`).
- 8-bit palette lookups indexed through signed `char`, giving wrong colours for indices ≥128 even on x86.
- `long` assumed to be 32-bit throughout (file formats, pixels).

## Licence

OMT and everything under `engine/` are LGPL 2.1 (`engine/LICENSE-LGPL.txt`). SDL2 (zlib licence) and
gl4es (MIT) are fetched at build time. Jimmy Neutron is © Nickelodeon/Viacom; the game is © THQ. No game files are
included or distributed.
