# Phase 2 brief (all agents). Token budget is tight: be economical.

Project: faithful C++ reconstruction of Neutron2.exe (Jimmy Neutron vs. Jimmy Negatron, 2002) on our SDL2/gl4es
OMT port, for Android (AYN Thor). Rules and conventions: docs/HANDOFF.md ("Bill's standing rules", "Setting
up", "Next") and docs/decomp-guide.md. Read only the parts you need.

## Economy (Bill's instruction)
- Read slices, grep, `r2 ... pdg/pdf @ addr` for single functions; don't dump whole raw files you don't need.
- Testing: build + smoke test (CLEAN EXIT) before each commit, plus one quick check that your change works.
  No long captures, Wine comparisons or repeated runs unless something is actually wrong.
- Report in <= 12 lines.

## Hard rules
- Never commit game files or bytes from the exe/DLLs (values are read from the player's exe at run time).
- No leaked GBA source. Faithful; deviations marked `// port:`. Builds for x86-64 Linux and arm64 Android.
- `game/src/gen/*` is generated: regenerate with the tools, never hand-edit.
- Commit only what builds: `git -c user.name=sirbilliams -c user.email=bravehooves@gmail.com commit`, message
  ending with a blank line and:
  Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
  Claude-Session: https://claude.ai/code/session_015zEU1ZTRDWHNzdA6Fa1a1r
- After each commit: `git push -f origin HEAD:backup/agent-<letter>`. Never push main or other branches.
- Don't run fetch-deps.sh. Don't touch other agents' worktrees, build dirs, game installs or scratch.

## Your workspace (<x> = your letter)
- Worktree /home/claude/wt/<x> (branch agent-<x>, at backup/integ (= main 045a333)). Build:
  `cmake -S engine -B build-<x> -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -C build-<x> -j8 negatron`
  (re-run cmake after adding files).
- Game install /home/claude/gamefiles/game-<x>; scratch /home/claude/scratch-<x>.
- Inputs (read-only): /home/claude/decomp (raw/<Class>.c, functions.json, classes.json, abi.json,
  factory.json, *_slots.json, Neutron2.exe); `r2` on PATH (radare2 + r2ghidra); python capstone/pefile.
- Smoke test, from your game folder (Xvfb :99 is running):
  `DISPLAY=:99 OMT_WINDOW_SIZE=640x480 OMT_FRAMES=60 OMT_SCREENSHOT=x.bmp timeout 60 /home/claude/wt/<x>/build-<x>/negatron`
- Level test: add `NEG_TEST_PLAY=<Level>.gam` (e.g. OMT_FRAMES=600). Which classes a level still lacks:
  `python3 tools/decomp/gamcensus.py /home/claude/decomp/factory.json game/src /home/claude/gamefiles/game/gam/<Level>.gam`
  (byte scan: a 4-char ID inside a string can show up falsely). LoadGame stops at the first missing class.
- New classes: own new files; factory = define `New_<Class>` in your file. Match the style of existing classes
  (e.g. game/src/objects/C3DFadeObject.*, game/src/lab/C3DTesla.*). Small missing helpers: `static` in your file.
  Shared files: minimal edits, list them in your report.

## Lanes (wave 1)
A foundations (shared classes), B Level3c's own classes, C level select + lab exit + engine TODOs,
D finish 16:9 widescreen. B builds on A: A commits each class on its own; B runs `git merge agent-a`
(same repository) whenever it needs A's latest.

## Wave 2: levels must PLAY THROUGH, not just load (Bill found the lab's gameplay missing)
- "Done" = the level can be played from its start to its exit: its objectives work (pickups, gadgets, buttons,
  enemies attack, doors/lifts move, the exit fires and loads the next level).
- Classes created at run time (projectiles, effects, spawned enemies) never appear in .gam files, so the census
  can't see them: play the level (OMT_FAKE_PAD presses, temporary teleport hacks, not committed) and grep the
  log for `TODO` / `new C3D...` stubs; those are your missing pieces.
- Find the level's intended sequence from its own triggers/tasks (C3DAITrigger chains, task names, CLoadLevel
  exits), not from memory of the game.
- Agent C is on the lab's gun/firing (C3DPlayerObject slots 103/104, C3DWaterGun, C3DBubble, C3DBub). If you
  need firing too, don't write it: `git merge agent-c` once C commits it.
- Android's clang rejects some things desktop GCC accepts (C++98 is enforced: no brace-init `T{...}`, no
  `auto`, no range-for, no `nullptr`). Before reporting, check CI on your backup branch is green:
  `gh api "repos/sirbilliams/neutron2-android/actions/runs?branch=backup/agent-<x>&per_page=1" --jq '.workflow_runs[0]|.status+" "+(.conclusion//"")'`
