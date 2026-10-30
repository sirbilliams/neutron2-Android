# neutron2-Android
Native Android reconstruction of *Jimmy Neutron vs. Jimmy Negatron*.

## Layout
- `engine/`: OMT 2.5 port and SDL2 layer
- `game/`: reconstructed game C++ code
- `android/`: Gradle APK project, launcher, and companion panel
- `tools/`: De-compilation and generation scripts
- `docs/`: Screenshots, release gap checklist, controls

## Build
1. Ensure podman and git are installed.
2. Run: `bash build-android.sh`
3. APK output: `~/Negatron-Android/Neutron-vs-Negatron.apk`
4. First run builds the SDK image via `Containerfile.android`.
5. Dependencies fetched via `fetch-deps.sh`.

## Test
- GitHub Actions: `workflow android.yml` (Build, Linux desktop check).
- Desktop debug: `tools/test/hdpack.sh`, CMake build with SDL2/gl4es.
- Headless capture: `SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1 OMT_FRAMES=600`
- Diagnostics: In-game menu displays FPS, memory, and controller data.

## Rules
- Never commit game files, ROMs, or ISOs; user must provide `.iso`.
- Signing secrets are handled by repo secrets, never in git.
- License: Engine is LGPL 2.1; fan project not affiliated with Nickelodeon/THQ/AWE Games.
- Controls and UI settings apply on next launch unless modified in-port.

## Release
- Publish APK to `https://github.com/sirbilliams/neutron2-android/releases/latest`.
- Use consistent signing key across builds to avoid Android "App not installed" errors.
- Update README and CHANGELOG with version status upon release.
