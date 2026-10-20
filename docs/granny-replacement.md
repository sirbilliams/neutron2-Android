# Granny 1.2b replacement (`game/granny/`)

A portable reimplementation of the 26 RAD Granny functions that `Neutron2.exe` imports from
`granny.dll` ("1.2b", 2000), running on converted data. The API (names, arguments, handles, error
codes, call order) is the DLL's; see [granny-api.md](granny-api.md) for how the game calls it.

| | |
|---|---|
| runtime | `game/granny/granny_compat.h`, `granny_compat.cpp` (C++98, no dependencies), CMake target `granny_compat` |
| converter | `tools/granny/grn2grc.py` (+ `grnfile.py` .grn parser, `grnextract.c` Win32 extractor run under Wine) |
| verification | `tools/granny/verify.sh`, `grntest.c` (one script runner for both the DLL and the replacement), `grntest.py`, `tests/*.txt` |

## Status

Every output compared with the real DLL is **bit-identical**: skinned positions and normals, texture
coordinates, colours, triangle lists, transforms, texture pixels, bone states, animation parameters, error
codes and which handle words are filled in. `tools/granny/verify.sh` runs 15 scripts (the sweep alone
compares 5 449 records with 567 000 skinned vertices) with max error 0, covering:

- every one of the 389 `.grn` files opened as a model, rendered in its bind pose, and played (looping) on
  its base model (`sweep`); several characters and props side by side (`multi`);
- one animation at the game's frame rate, the game's own call pattern (lock `0x3b` at load, `0x11` per
  frame, blend-in/out, release, prop attached by bone name) (`basic`, `game`);
- two and more animations at once with blend weights, end-time moves, release/remove, upper/lower body
  layering (`blend2`); speed changes, a frozen control, finite loop counts, all weight parameters (`speed`);
- animations on models they were not made for (`cross`); b-spline + keyframe tracks (`rawmotion`);
- attach, detach, re-attach (`attach3`); textures queued per model instance (`textures1`);
- 22 100 frames of a looping walk (12 minutes): granny's root-motion accumulation drifts the root's scale
  by +0.042 % over that time and the replacement drifts identically (`drift`);
- error paths, stale handles, the lock state machine, closing objects that are still in use (`errors`,
  `handles`, `close2`, `notex`).

The comparison was made on x86-64 Linux (glibc). On other targets the only possible source of a
difference is `acos`/`sin` in the slerp of blended rotations (libm results may differ in the last bit of a
double, which only rarely survives the rounding to float); everything else is plain IEEE arithmetic.

What is deliberately different:

- **Handle ids.** The DLL draws ids from a counter advanced by a pseudo-random step that depends on heap
  addresses (`0x10011220`: `*0x1006e244 += table[(ptr >> 2) & 7]`), so they cannot be reproduced; the
  replacement counts up from the version handle. Which words of a handle are set (also after a failed
  open) matches. The version handle is the DLL's, `0x82ca5e2a`.
- **Use after close.** The DLL keeps answering `HasControlExpired`/`GetAnimationParameter` for a control
  whose sequence was closed by reading freed memory; the replacement returns `GrannyInvalidHandle`.
- **Materials without an image** (only in two animation files that carry meshes, `seagullmove.grn`,
  `hermanAttack2.grn`): the DLL queues a format-0 "new texture" with uninitialised width/height and has
  been seen to report blend modes 1/0 or 6/7 for it depending on what was loaded before; the
  replacement reports width = height = 0 and the blend modes extracted at conversion time (6/7).
- Not implemented (the game never does it): attaching by a child bone other than 0 (an approximate
  inverse is used), degrees of freedom other than `0x1ff`, texture formats other than 4.

## Conversion

```sh
python3 tools/granny/grn2grc.py <game>/Program_Executable_Files/grn <out dir> [--gxtr keep.gxtr] [files...]
```

Writes `<out dir>/<name lower case>.grc` for each `.grn` (389 files, 17 MB, ~4 s including the Wine run).
Skeletons, bone names and animation curves are parsed from the `.grn` files (`grnfile.py`). What granny
builds at load time is taken from the DLL itself, losslessly (raw float bits): `grnextract.exe` (built with
`i686-w64-mingw32-gcc`, cached in `~/.cache/grn2grc-wine` with a copy of `granny.dll` from the game
directory) opens every file under Wine and dumps the decoded textures (the files store them Bink-compressed),
the bind-pose rendering states, and the internal mesh object of each state: render-order vertex arrays,
bone bindings with their inverse rest transforms, per-vertex weight lists, copy lists, normal-to-vertex
mapping and per-state triangle lists. The mesh object is reached from the version object
(`*(*(0x100721fc) + 0x10)`): `mesh = *((V[0x194] & ~3) + 0x0c)` while a state is locked; `--gxtr` keeps
/ reuses that dump. Converted data must not be committed (`.gitignore`: `*.grc`, `*.gxtr`, `*.gtst`).

The runtime maps a file name passed to `GrannyOpenModel`/`GrannyOpenAnimation` (`\` or `/`, any case) to
`<directory of that path>/<base name lower case>.grc`, or to `<dir>/<base name lower case>.grc` after
`GrannyCompatSetDataDirectory(dir)`. Files are shared between opens.

## `.grc` format (version 1, little-endian)

`str` = `u32 length` + bytes (no terminator). Floats are the DLL's bits.

```
"GRC1"  u32 version = 1  u32 flags (1: model part present, 2: animation part present)
model part:
  u32 nbones   { str name; i32 parent (root: 0); f32 pos[3]; f32 quat[4] (x y z w); f32 scale_shear[9] }
  u32 ntextures{ u32 width, height, format (4 = RGB24), bytes_per_pixel, pitch, size; u8 pixels[size] }
  u32 nmeshes  {
    u32 nbindings { u32 bone; f32 bt[3] (= -rest translation); f32 B[9] (inverse rest rotation*scale) }
    u32 nrender;  f32 positions[nrender][3], normals[nrender][3], texcoords[nrender][3], colours[nrender][3]
    u32 nsource                         (0 = rigid mesh: every vertex follows binding 0)
    u32 nw; u32 weights[nw]             per source vertex: n, n x { u32 binding, f32 weight }
    u32 nc; u32 copies[nc]              per source vertex: n, n x (render index - source index)
    u32 nn; u32 normal_vertex[nn]       per render normal: source vertex whose weights it uses (~0: none)
    u32 nstates { u32 texture (index into the textures, ~0: none); u32 src_blend, dst_blend;
                  u32 ntriangles; u16 indices[ntriangles][3]; pad to 4 bytes } }
animation part:
  f64 duration                          (largest last knot / key time of all tracks)
  u32 ntracks { str bone name; u32 kind;
    kind 0 (keyframes): u32 nkeys; f32 key[nkeys][17] = { time, pos[3], quat[4], scale_shear[9] }
    kind 1 (b-splines): 3 x { u32 nknots; f32 knots[nknots] }   (knot arrays)
                        3 x { u32 degree; u32 knot array; f32 control[nknots of that array][dim] }
                        for position (dim 3), rotation (4), scale/shear (9) }
```

Track 0 is the root track (`__Root`). All files have exactly one model part and one animation part (base
models carry an animation with 0 or a few tracks; some animation files carry meshes).

## Semantics (from `granny.dll`'s machine code)

**Arithmetic.** granny.dll's CRT sets the x87 control word to `0x027f` (53-bit precision), so its
expressions are C `double` arithmetic rounded to `float` where the code stores a float. Each routine in
`granny_compat.cpp` names the DLL function it follows and keeps its operation order, temporaries and
rounding points. Compile with `-ffp-contract=off` and SSE2/ARM64 doubles (the CMake target does).

**Transforms.** A bone's local transform is decomposed: position, quaternion, 3×3 scale/shear `S`
(meaning `x -> pos + R(q) S x`). World (composed) form: quaternion, translation, `M = R(q) S` (`0x100220b0`,
after one Newton normalisation step of q), composed with the parent's (`0x100217b0`).

**Time.** `GrannyAdvanceTime` adds `(double)(float)seconds` to the global clock. Then, for each control
in creation order: if it is alive at the old time (`active && end > old`) it is applied; a released
control that is not alive at the new time is deleted. Then every sequence (parent first) recomputes its
world transforms.

**Controls.** Defaults at `Add`: start = now, speed 1, loops 1, weights: before 0, hold 1, after 0, blend
times 0. `total = duration / speed * loops`, `end = start + total`; `HasControlExpired` = `now > end`.
Parameters (0–8, the ids in `granny_compat.h`): 0 start, 1 end (moves the start), 2 speed (rescales the
start so the current phase is kept; `0x10011e2e`), 3 loop count, 4/5/7 weights (stored as float), 6/8
blend-in/out seconds. Weight at the new time: before start → p5; ramp p5→p4 over the blend-in; p4; ramp
p4→p7 over the last blend-out seconds; p7 after the end.
Applying a control samples at the local time of the **old** clock: `local = (old - start) * speed`;
if `loops != 1` and `local/duration < loops` it wraps with `fmod`; clamped to the duration; frozen while
speed ≤ 1e-4.

**Tracks.** Tracks 1.. drive the bone of the same name (a later track wins). Sampling: b-splines of
degree 0–3 over knot arrays padded like granny's (3 knots before, 2 after; first control point ×3, last
×2), knot search from index 3, quaternion normalised once; keyframe tracks take the first key with
time ≥ t. The sample is composed with a per-bone offset — `rest ∘ rest⁻¹` computed the DLL's way
(`0x10031080` looks both transforms up in the same table), an identity up to rounding noise
(~1e-6 in position, ~1e-7 in scale) that is part of every pose — then written (weight 1) or blended
(`0x10021910`: lerp position and scale/shear, slerp rotation above 1e-4).

**Root motion.** Track 0 does not set the root: granny's default mode (sequence flag `+0x6d` = 1, never
changed by the game) moves the root by the track's change between the previous and the current local
time, `delta = S(t0)⁻¹ ∘ S(t1)` (`0x10026c60`; with a wrap-around part when `t1` passes the duration), and
`root = blend(root, w, root ∘ delta)`. With the game's identity root tracks this is rounding noise that
accumulates frame after frame, exactly as in the DLL.

**Skinning** (`0x100298e0`, `0x100291d0`, `0x10029710`, `0x10029880`): per binding `L = M_bone · B`,
`T = L · bt + t_bone`; positions per source vertex as a weighted sum accumulated in double and rounded once,
then copied to their duplicate render vertices; normals per render vertex with their source vertex's
weights, then normalised. Rigid meshes (no weight list, < 2 bindings) use binding 0. Deformed vertices are
in sequence space, so the state's transform is the identity.

**Rendering locks.** One lock at a time; the lock handle is (version, sequence id). Bits: 1 positions,
2 colour0 (always returned), 4 colour1, 8 texture coordinates, 0x10 normals, 0x20 triangles. A state's
texture cookie is 0 and `texture_pending` 1 until its texture was delivered through
`GrannyUnlockNewTexture`. Each `GrannyOpenModel` queues its own textures, even for the same file twice.

**Attach.** `GrannyAttachSequence(child, 0, parent, bone, 0x1ff, 0x1ff)`: an attached child is first
detached; the child's root becomes the placement `(-0, quat(-0,-0,-0,1), I)` under the parent bone
(effective at the next `AdvanceTime`). Detach flattens the chain into the root.

**Lifetimes.** Closing a model only invalidates its handle while sequences still use it (they render on,
textures included); closing an animation lets its live controls play on. Closing a sequence detaches its
children and deletes its controls.

## For `C3DGrannyObj`

- Include `granny_compat.h`, link `granny_compat`. Call the functions exactly as the original does;
  handles stay 32-bit words (`granny_uint32[2]` / `[3]`) and can live in the same object fields.
- `g_granny` is opened in `CAweReal::CAweReal` (`game/src/AweReal.cpp`, done).
- Read rendering states and new textures through the structs' **field names**
  (`granny_rendering_state`, `granny_new_texture`), not the 32-bit offsets — pointers are 64-bit here. The
  texture cookie is a `granny_cookie` (`uintptr_t`): pass the `OMedia3DMaterial*`.
- Per frame (`0x41e2cb`): lock with `0x11`, and for each state copy `positions` and `normals`
  (`count` × 3 floats) into the OMT shape's 16-byte vertices as `(-y, z, x)`; `0x3b` at load
  (`0x41d7d3`, `0x41dcb2`) also yields texture coordinates and triangles.
- Starting an animation (`0x41d5df`): `Add`, then param 2 = the object's speed (float at `+0x4f0`),
  param 3 = 9999 (looping) or 1, params 6 / 8 = blend time or 0.
- Place the `.grc` files next to where the game expects the `.grn` files (or call
  `GrannyCompatSetDataDirectory`).

## Verifying

```sh
tools/granny/verify.sh <game>/Program_Executable_Files/grn <grc dir> [work dir] [test...]
```

Builds `grntest.exe` (DLL, Wine) and the native runner, runs each `tools/granny/tests/<test>.txt` on both
and prints the comparison (exit status 1 on a mismatch, 2 on a nonzero float difference). Script commands
are listed in `grntest.c` (`model`, `seq`, `anim`, `add`, `param`, `advance`, `advancen`, `render`,
`textures`, `bones`, `attach`, `close*`, raw `lock`/`left`/`unlock`, …; `~` stands for a space in names).
