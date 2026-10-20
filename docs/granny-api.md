# RAD Granny 1.x (granny.dll, "1.2b", 2000) — API as used by Jimmy Neutron vs. Jimmy Negatron

Recovered from `Neutron2.exe` (class `C3DGrannyObj`, around `0x41a000–0x41e400`, init in `0x4a3c00`).
All functions are `__stdcall`, return 0 on success. Handles are passed **by value** as 1–3 dwords.

| Handle | Size | From |
|---|---|---|
| version | 1 dword | `GrannyOpenVersion` |
| model, sequence, animation | 2 dwords | `GrannyOpenModel`, `GrannyOpenSequence`, `GrannyOpenAnimation` |
| rendering lock | 2 dwords (+ count) | `GrannyLockSequenceForRendering` |
| control | 3 dwords | `GrannyAddControlledAnimation` |

## Start-up (0x4a3d17)

```c
GrannyOpenVersion("1.2b", "win32", "10-4-2000",
                  "(C) Copyright 1999-2000 RAD Game Tools, Inc.  All Rights Reserved.", &ver);
GrannyAllowTextureFormat(ver, 4);                       // 24-bit RGB
```

## Model / sequence (0x41d137)

```c
GrannyOpenModel(ver, "jimmybase.grn", model);           // u32 model[2]
GrannyOpenSequence(model[0], model[1], seq);            // u32 seq[2]   (an instance)
```

## Textures (0x41ae10) — delivered after the first render lock

```c
GrannyGetNewTexturesLeft(ver, &n);
GrannyLockNextNewTexture(ver, tex);       // tex+0x14 width, +0x18 height, +0x28 RGB24 pixels
GrannyUnlockNewTexture(tex, cookie);      // cookie (the game passes its OMedia3DMaterial*)
```

## Animation (0x41d5df, 0x41dfb2, 0x484365)

```c
GrannyOpenAnimation(ver, "jimmydance.grn", anim);                     // u32 anim[2]
GrannyAddControlledAnimation(seq[0], seq[1], anim[0], anim[1], ctrl);  // u32 ctrl[3]
GrannySetAnimationParameter(ctrl[0], ctrl[1], ctrl[2], id, (double)v);
    // 2: time (object's start time)   3: loop count (1, or 9999 = loop)
    // 6: blend-in time   8: blend-out time
GrannyHasControlExpired(ctrl[0], ctrl[1], ctrl[2], &expired);
GrannyAdvanceTime(ver, (double)seconds);                              // global clock
GrannyRemoveControlledAnimation(ctrl...), GrannyReleaseControlledAnimation(ctrl...)
```

Also used: `GrannyGetBoneIndexFromName`, `GrannyAttachSequence` (props on bones, e.g. guns in hands),
`GrannyDetachSequence`, `GrannyClose*`.

## Rendering (0x41d7a0) — each frame

```c
u32 r[3];
GrannyLockSequenceForRendering(seq[0], seq[1], 0x3b, r);
GrannyGetRenderingStatesLeft(r[0], r[1], &r[2]);
for (i = 0; i < r[2]; i++) {
    u32 state[...];
    GrannyLockNextRenderingState(r[0], r[1], state);
    ... read streams ...
    GrannyUnlockRenderingState(state);
}
GrannyUnlockRendering(r[0], r[1]);
```

Rendering state: `+0x18` texture cookie; then streams of `0x18` bytes
`{present, ?, ?, count, ?, pointer}` starting at `+0x2c`:

| Offset | Stream | Element |
|---|---|---|
| `0x2c` | positions (skinned, current frame) | f32 × 3 |
| `0x44`, `0x5c` | (unused by the game) | |
| `0x74` | texture coordinates | f32 × 3 (u, v, w) |
| `0x8c` | normals | f32 × 3 |
| `0xa4` | triangles | u16 × 3 |

The game negates X when copying positions/normals into OMT shapes. Models are authored Z-up (3ds Max).

## Files

56 `*base.grn` models; animations are separate files named with the model's prefix
(`jimmydance.grn` → `jimmybase.grn`), plus a few aliases (`nummy*`/`NummyFlail` → `nummeybase`,
`numflyair` → `nummeyflyerbase`, `upperrun` → `jimmybase`). 22 further files are standalone props
(guns, PDA, shades, hands…). `gunbones.grn` is an animation whose target is not yet known.

## Tools (`tools/granny/`)

- `grndump.c` — Win32 tool (build: `i686-w64-mingw32-gcc -O2 -o grndump.exe grndump.c`), runs under Wine
  with the game's `granny.dll`; writes textures + bind pose + every animation sampled per frame (`.gdmp`).
- `dump_all.py` — groups all 389 files by model and dumps them (78 models, 311 animations, ~4 min).
- `gdmp.py` — reader, preview renderer, OBJ export.
