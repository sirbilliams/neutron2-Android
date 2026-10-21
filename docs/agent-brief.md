# Sub-agent brief (template; give one to each parallel agent with its letter)

Project: a faithful C++ reconstruction of the 2002 PC game Jimmy Neutron vs. Jimmy Negatron
(Neutron2.exe, MSVC 6, OMT engine) so it runs natively on Android (AYN Thor, arm64, gamepad).
Read `docs/HANDOFF.md` and `docs/decomp-guide.md` in your worktree first; they are the rules.

## Hard rules
- Never commit game files or bytes copied from the exe/DLLs (no bulk tables, strings, images).
  Values come from the player's own exe at run time (`game/src/ExeData.cpp`).
- Do not use the leaked GBA source. Only the retail `Neutron2.exe`.
- Faithful: same logic, order and quirks. Deviations are marked `// port:` with the reason.
- Code must build for 64-bit Linux AND arm64 Android (clang, `-fsigned-char`). No x86 asm, no
  Windows APIs, no pointer-size assumptions (the original is 32-bit: never store pointers in int).
- `game/src/gen/*` is generated: never hand-edit; regenerate with the tools.
- Commit early, on your worktree branch only, and only commits that build (git -c user.name=sirbilliams
  -c user.email=bravehooves@gmail.com commit). Back up after each commit with
  `git push -f origin HEAD:backup/agent-<letter>`; never push main or any other branch.
  End every commit message with the attribution lines the lead gives you (the current session's
  Co-Authored-By and Claude-Session lines).

## Inputs (already generated, read-only; shared - do not modify)
`<gamefiles>` = where the lead put the ISO, the unzipped OMT zip and the game installs (e.g. /home/claude/gamefiles).
- `/home/claude/decomp/` (= the decomp folder): `raw/<Class>.c`, `raw/_free.c`, `raw/_extra.c`,
  `raw/functions.json`, `classes.json`, `abi.json`, `factory.json`, `go_slots.json`,
  `lgo_slots.json`, `omt_layout.json`, `Neutron2.exe`.
- radare2 6.2.4 + r2ghidra on PATH (`r2`, also `/opt/r2/bin/r2`), python r2pipe, capstone, pefile, unicorn.
- OMT 2.5 original sources (reference): `<gamefiles>/omtfull/OMT/sources/OMTClasses` (unzip OMTFullV250.zip).
- OMT2.dll (2002 engine): `<gamefiles>/game/OMT2.dll` (the master install).

## Your workspace
- Your git worktree (you are already in it). Don't run `fetch-deps.sh` there (the desktop `negatron`
  build doesn't need it, and the harness may refuse it in a worktree).
- Your own scratch folder (the lead names it): never write scratch files anywhere else.
- Build dir: `build-<yourletter>` inside the worktree:
  `cmake -S engine -B build-X -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -C build-X -j8 negatron`
  (`game/src` is globbed: re-run cmake after adding files).
- Your own game install: `<gamefiles>/game-<yourletter>` (never use another one; a run
  deletes autosaves). Xvfb is on :99 (start `Xvfb :99 &` if `xdpyinfo -display :99` fails).
- Smoke test, from inside your game folder:
  `DISPLAY=:99 OMT_WINDOW_SIZE=640x480 OMT_FRAMES=60 OMT_SCREENSHOT=x.bmp timeout 60 <worktree>/build-X/negatron`
  Success = "CLEAN EXIT" at the end, and the TODO for your class gone / the run getting further.
  Baseline: logo -> museum -> lab cut scenes -> player control (see HANDOFF.md "Status").

## Collisions with parallel agents
Four agents work at once on different classes. Keep new code in your own new files. If you need
a small helper that does not exist, make it `static` in your file (per the guide). If you must
touch a shared file (CMake, factory weak stubs are overridden by defining `New_<Class>` in your
file - no factory edit needed, global_types/*.txt: use your own new txt file), keep the edit minimal.
Regenerate `gen/globals.*` with globals.py if you add a global_types file.

## Report back
When done: the commit hash(es), what is implemented, what is still TODO (with addresses),
the smoke test result (tail of the log), and any shared files you touched.
