# Update: 1.0.2-rc1 verification candidate

- Added bounded HD PNG decoding with format/CRC checks and original-texture fallback. New native
  regression fixtures cover RGBA, palette alpha, packed grayscale, 16-bit channels, Adam7, row filters,
  corrupt input and over-budget images. This is a per-image working-memory estimate, not an app RAM cap.
- Android HD rendering can be disabled without uninstalling the pack; LOW / BALANCED / HIGH presets
  cap upload memory, decode memory and scale. These apply on next game launch.
- Diagnostics is visible in the launcher and the in-game Back menu: rendered FPS, resident game memory,
  HD counts/limits, current displays and controllers, timestamped samples and a shareable report with logs.
- Side-positioned score pop-ups now keep their right-edge offset throughout the animation; the central
  pop-up keeps its original starting point. Coordinate regressions cover 4:3 and widescreen. The help
  icons already register left anchors; no speculative placement change was made.
- Still requires physical Thor checks: Diagnostics readability/controller navigation and refreshing,
  native texture appearance/fallbacks with the real pack, LOW/BALANCED/HIGH transitions, saved-game
  retention, and visual HUD inspection in both UI modes. No game files were used in automated tests.

Smallest next check: install over the previous candidate, Continue with BALANCED, open Diagnostics
from hold Back, check that FPS/HD figures refresh, then exit, disable HD, Continue and confirm original
textures return. Share report after any problem. Confirm a score pickup in widescreen and the mouse-help
icon in Vanilla and Bill's UI. The original report below remains useful historical context.

---

# Release gap: Thor verification candidate

Baseline reviewed: `adf6ddd` (main, 2026-10-22). This report distinguishes code and recorded
results from checks still needing a physical AYN Thor. No Thor or retail game data was available
in this review environment.

| Area | Implemented / evidence | Remaining verification |
|---|---|---|
| Native game and dual screen | arm64 SDL/gl4es build; pad, companion map/UI, lid/app suspend, TV display filtering. HANDOFF's Oct 22 entry records Bill's full Thor story run: “perfect, no complaints”. | Regression-test this APK's install-over update, retained saves, bottom display, suspend/resume and TV casting. The README's native/dual-screen claim has implementation and prior user evidence, but does not validate every later feature. |
| Asset Viewer | Launcher level explorer with frozen noclip camera; model orbit/zoom browser; save-write/delete guards. HANDOFF records desktop testing. | Thor controls, both screens, repeated viewer exit/re-entry, and unchanged saves after exploring. |
| HD loader | OMT, loose PNG and Granny replacements; original logical sizes; aspect/integer-scale validation; GL size clamp, memory budget and fallback logs. Desktop synthetic 2x pack recorded. | Bill's real upscaled pack on Thor, alpha/HUD alignment, load time and memory across levels. Oversize clamp, wrong-size skip and budget fallback need runtime evidence. The budget applies to uploaded textures; temporary PNG decoding can allocate additional memory. |
| HD installation | Settings document picker, progress, install/remove and desktop tool. This candidate adds staged replacement with rollback/recovery, expanded-byte free-space checks, case-collision rejection and honest removal failure reporting. | Android document-provider behavior, low-storage error UX, install/replace/remove/relaunch with a real pack. JVM regression tests cover file handling; they do not exercise Android UI or native texture decoding. |
| Original limitations | README still lists the stuck lab bot, widescreen score/help placement and missing logo sun. It conservatively lists dinosaur/boss/crane/lander checks despite the later full-story report. | Keep these open until specific observed results resolve each one. Android libm Granny parity and the desktop NEG_TEST_PLAY shutdown hang also remain in HANDOFF. |
| Delivery | Main pushes build an APK and publish the `latest` prerelease. This candidate gates Android on Linux build + installer regression checks, requires stable signing for publication, verifies APK signatures and includes SHA-256. | CI success proves build/signing, not device playability. Verify install-over on Thor. The existing `Final` release has a differently named APK, so README now links its release page rather than assuming an asset filename. |

## Current update (2026-10-25 Pacific) — supersedes historical gaps above

- 1.0.2-rc1 already added bounded PNG decoding, HD presets/on-off, timestamped diagnostics and
  widescreen score anchoring. The older unbounded-decode and score-placement recommendations are historical.
- 1.0.3-rc1 (`de3cf72`) passed CI and was published. Bill reports expanded touch-map functionality
  works, with poor stretched metal appearance and cropped-map coverage reported afterward.
- 1.0.4-rc1 (`008e21a`) fixes chrome width, fits copied terrain to the existing 1536-pixel map budget,
  adds Fit Level/adaptive zoom, and moves PDA credits inside the panel. Physical visual verification
  remains pending. Build results and complete implementation handoff: [HANDOFF.md](HANDOFF.md).
- Next test is install-over/save retention, PDA credits at both aspect ratios, metal animation,
  distant terrain at full zoom-out, map coordinates, filters, multi-touch, controller focus, height
  changes, Vox, suspend/resume and casting. Build success is not evidence of these device behaviors.

## Smallest next verification step (about 5–10 minutes)

1. Export saves in the current app. Install the new `latest` APK **over** it, without uninstalling.
   Confirm Continue and the save slots still work. Record the APK SHA/commit, device firmware and result.
2. On Thor, open Asset Viewer → one level. Move/look/rise/fall, then leave. Open one model, orbit/zoom,
   and leave. Confirm both displays remain correct and Continue still resumes the same save.
3. Install your HD ZIP in Settings, Continue into the lab, check one model + HUD texture, then return,
   replace the pack once, remove it and relaunch. Confirm original textures return and saves remain.
   Share `omt-log.txt` (hidden Test → Share log) including the `HD:` lines.

This single session clears the newest feature integration gap. It does not clear the remaining
boss/vehicle, texture-limit or long-session performance checks.

## Follow-up priorities

1. Run deliberate wrong-size / above-GL-limit / low-budget synthetic pack checks; capture fallback logs.
2. Measure real-pack memory and level-transition times on Thor before changing the 1536 MB upload budget.
   Consider bounded native PNG decoding before accepting arbitrarily large source images.
3. Address the small widescreen offsets with before/after screenshots; investigate the stuck bot using
   a reproducible save. Keep retail-derived art and HD packs out of this repository and release assets.

## Reproduce automated checks

`bash tools/test/hdpack.sh` (JDK 17) runs the production Java installer against synthetic ZIPs with
minimal Android type stubs: malformed paths, case collisions, corrupt entries, unknown ZIP size,
low storage, failed final rename rollback, interrupted swap recovery, successful replacement and removal.
GitHub Actions also builds the Linux engine/game and the Android arm64 APK. See the exact commit's
Actions run for results; this document does not claim that a pending run passed.
