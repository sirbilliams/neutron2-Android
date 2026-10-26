# HD texture pack

The game can use upscaled copies of its own textures. The pack is made from the player's own game
files and is never part of this repository (it is the game's art).

## Layout

A folder `hd/` in the game folder (Android: the game data folder, next to `omt/`):

| file | stands in for |
|---|---|
| `hd/omt/<database>/<id>.png` | canvas `<id>` of `omt/<database>.omt` (lower case, no extension): menus, HUD, font, sprites, the 3D levels' textures |
| `hd/png/<name>.png` | the loose texture `png/<name>.png` |
| `hd/grn/<model>_<n>.png` | texture `n` (0, 1 ... in GrannyLockNextNewTexture order) of `grn/<model>.grn` |

Each picture is an integer multiple (2x, 3x, 4x ...) of the original size, same aspect; alpha is
used as is (keep transparent pixels transparent). Names are matched without regard to case. Any PNG
works (all colour types and bit depths, palette transparency, interlaced).

## Install

- Android: launcher Settings, "HD texture pack: install (.zip)" (file picker; progress in the item;
  the counts are reported), "HD texture pack: remove".
- Desktop: `neg_hdtool install <game dir> <pack.zip>`, `neg_hdtool remove <game dir>`.

The zip may have `omt/ png/ grn/` at its top or inside one folder; other files (README,
manifest.tsv) are left out. Android rejects duplicate target names (including case-only duplicates),
keeps the installed pack until the replacement is staged, and restores it if the final move fails.
An interrupted swap is recovered when the launcher next checks the installed pack. Free space is
checked against the expanded bytes with a 64 MB reserve; each picture is limited to 256 MB on disk.
The native HD decoder also bounds its working-memory estimate before inflating or allocating pixels;
files over 64 MB (or one quarter of the decode budget) are skipped. Corrupt CRCs, invalid PNG methods,
row filters and palette references fall back to the original texture, with a log message. `neg_hdtool extract <game dir> <out> [db.omt ...]` writes the omt canvases
with these names (plus manifest.tsv) for upscaling.

## How it works (engine/sdl/OMediaHDPack.cpp)

- `hd/` is scanned once at start (`NEG_HD_DIR` overrides the folder).
- A canvas read from an .omt database (OMediaCanvas::read_class, the database's file name in
  OMediaDataBase::hd_name), from a loose png/ file, or filled from a granny texture (C3DAnimated::BindTex,
  GrannyCompatTextureName) is tagged with its pack name when the pack has it.
- When its GL texture is made (OMediaGLCanvas::create_texture), the picture is uploaded instead of the
  canvas pixels: one texture (exact / scaled canvases: normalised coordinates) or the 2D subdivided tiles,
  each 2^k times larger (the factor is rounded down to a power of two there). The canvas keeps its size and
  pixels, so every size and coordinate the game uses stays the original.
- A canvas the game draws into (a write lock) loses its tag and goes back to its own pixels.
- Limits: GL_MAX_TEXTURE_SIZE (the largest factor that fits, else the original, with a log line); a memory
  budget for the HD textures, `NEG_HD_BUDGET_MB` (default 1536; past it the rest stays original, logged).
- Log: `HD: texture pack with N pictures` at start, `HD: N textures replaced` at exit; `NEG_HD_LOG=1` lists
  each replaced picture; a wrong-size or unreadable picture is logged once and skipped.
- Without `hd/` nothing changes.

## HD rendering and memory presets (Android)

Settings → HD rendering switches the installed pack on/off without removing it. HD memory preset
changes startup limits; exit the game and launch it again to apply either setting.

| Preset | Estimated HD upload budget | PNG working-memory budget per picture | Maximum uploaded scale |
|---|---:|---:|---:|
| LOW | 256 MB | 96 MB | 2x |
| BALANCED (default) | 512 MB | 192 MB | 4x |
| HIGH | 1024 MB | 256 MB | 8x |

These are HD limits, not total app RAM limits; originals, driver allocations and other game systems
use additional memory. A source PNG over the decode budget is skipped, not resized before decoding.
For those pictures, supply a smaller upscale. Supported pictures can still be reduced for the GPU
or preset scale limit. Desktop overrides: `NEG_HD_ENABLED=0`, `NEG_HD_MAX_SCALE`, `NEG_HD_DECODE_MB`
(clamped to 32–512 MB); the desktop upload default remains 1536 MB unless `NEG_HD_BUDGET_MB` is set.

Diagnostics (launcher or hold Back → Diagnostics in game) reports cumulative upload count, current
estimated texture bytes, unique warning count and the actual runtime limits. “Rendered FPS” samples
buffer swaps over two seconds; it includes menus and cut scenes and is not a gameplay benchmark.
