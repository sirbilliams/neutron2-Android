# Reconstructing Neutron2.exe — working guide

The goal is source that behaves exactly like the shipped game, compiled for 64-bit Linux/Android
against Open Media Toolkit 2.5 (`engine/OMTClasses`). Read the binary, not your memory of games.

## Inputs (outside the repo, on this machine)

| What | Where |
|---|---|
| Raw decompilation (r2ghidra), one file per class + free functions | `/home/claude/decomp/raw/<Class>.c`, `_free.c`, `_extra.c` |
| Function index (address → name, size) | `/home/claude/decomp/raw/functions.json` |
| Calling convention facts from the machine code | `/home/claude/decomp/abi.json` (`ret_pop` = bytes of stack args popped: `ret 8` → 2 args; `fret` = returns a float) |
| Class map from RTTI (vtables, bases, subobject offsets) | `/home/claude/decomp/classes.json` |
| OMT 2.5 class layouts | `/home/claude/decomp/omt_layout.json`, query with `python3 tools/decomp/omt_layout.py --lookup /home/claude/decomp/omt_layout.json OMediaElement 0x58` |
| The exe and OMT2.dll | `/home/claude/jn/game/Program_Executable_Files/` (game data lives there too) |
| Disassembler | `LD_LIBRARY_PATH=/opt/r2/lib /opt/r2/bin/r2 -q -e scr.color=0 -e bin.relocs.apply=true -c 's 0x4ab290; af; pdf' Neutron2.exe` (`pdg` = decompile) |

The decompiler output is a guide, not the truth. **When it looks odd, read the disassembly**:
x87 float code, struct returns, `unaff_` registers, `in_stack_` values, `CONCAT` and wrong argument
lists are all decompiler artefacts. Calls through `fcn.004ba850` are `_ftol` (float→int truncation);
`fcn.004b3aa0` is `operator new`, `fcn.004b3ab0` `operator delete`; `fcn.004c4520` is `_stricmp`,
`fcn.004c45ac` `_strupr`, `fcn.004bb850` `strstr`, `fcn.004bb627` `vsprintf`, `fcn.004bb55b`
`toupper`; `fcn.004b7cf0` / `fcn.004a2930` are `std::list` internals. `CGameObject__v13(...)` is the
debug log, compiled empty in the shipped build: drop it (keep a comment if it names the function).
`fcn.004b3b10` is `neg::ErrorLog` (active in release).

## Layout of the source

```
game/src/<Class>.h, <Class>.cpp      hand-written class parts
game/src/<dir>/<Class>_<part>.inc    slot declarations of one part (included inside the class)
game/src/<dir>/<Class>_<part>.cpp    bodies of that part
game/src/gen/                        GENERATED - never edit; regenerate (commands below)
tools/decomp/global_types/*.txt      hand-given types/names for globals
```

## Classes, slots and fields

* Virtual functions are **slots**: a call `(**(code **)(*param_1 + 0x1f0))(x)` is slot `0x1f0/4 = 124`.
  Slots keep the name the generator gave them (`v124`, or a name recovered from logs such as
  `InitObject`); call them by that name. Check `game/src/gen/<Class>_slots.inc` and the base class
  headers for the declaration.
* A slot you implement by hand: declare it in your `.inc` with a trailing `// SLOT <n> <address>`
  comment, and define it in your `.cpp`. If it overrides a base slot, keep the base signature exactly.
* Members are named `f_<offset>` (byte offset in the original object) and declared by the
  generator in `gen/<Class>_fields.inc`. Adjacent members keep their original order. If you need
  a member the generator does not know or that is not a scalar (a `std::list`, an array, a string),
  declare it in your `.inc` preceded by `// FIELD <offset> <size>` so the generator leaves that
  range out. If a generated member has the wrong type, add `// TYPE <offset> float` (or int, uint,
  uchar, short) in your `.inc`.
* `this` in the decompilation is the subobject whose vtable the function came from. For single-
  inheritance game classes (CGameObject → CLocalGameObject → C2DInGameMenu) offsets are the same
  everywhere: `*(int *)(param_1 + 0x4c8)` is `f_4c8`.
* Fields of OMT objects (the original used OMT 2002): `OMediaElement` is identical up to 0x8c. For
  classes derived from OMediaElement, a 2002 offset ≥ 0x8c is the 2.5 offset **minus 0x14**. So for
  an `OMediaCanvasElement`: 0x8c canvas, 0x90..0x9c diffuse r,g,b,a, 0xa0..0xa8 canv x,y,z,
  0xac/0xb0 canv w,h, 0xb4/0xb8 src/dest blend, 0xbc/0xc0 align h/v, 0xc4 canvas flags (short).
  Use the OMT 2.5 accessors (`set_blend`, `set_size`, `set_diffuse`, `place`, `hide`, `link`,
  `link_layer`, `set_canvas`...) rather than poking memory. OMT imports appear as
  `_sym.imp.OMT2.dll__set_canvas_OMediaCanvasElement__...` — call that method.
  `OMediaCanvasElementWrapper` (game/src/CanvasWrapper.h) is the 0xd8-byte sprite class (`fcn.00496a40`).
* Common OMediaElement vtable offsets in the game code: +0x2c `link(world)`, +0x30 `unlink()`,
  +0x34 `link_layer(layer)`, +0x38 `unlink_layer()`, +0x58 `hide(bool)`, +0xac `set_canvas(canvas)`
  (canvas elements). Verify others against OMT2.dll's vtable if unsure.

## Globals

The data section is one block (`gen/globals.h`): `g_545e50` style macros give typed views. A
global used as a pointer must live in the pointer table — if the generated view is an integer but
the value is a pointer (assigned from `new`, a database, a canvas...), add a line to
`tools/decomp/global_types/<your part>.txt`:

```
0x545e50 ptr OMediaDataBase* g_screens_db       # ptr | ptrarray | val | array, C type, optional name
```

and regenerate. Use the name in code. Already named by hand: everything in `game/src/Globals.h`,
`Game.h`, `CGameObject.h` (`g_objects`), `Text.cpp` (fonts, DrawText), `Alpha.cpp`, `CEditor.h`
(`g_splash`, `g_editor_*`; level loading: `g_editor->RequestLevel(name)` / `LoadGame`).

Existing reconstructed helpers you should call instead of re-implementing:
`neg::DrawText / ClearText / PlaceGlyph` (0x4a0f60 / 0x4a0f00 / 0x4a2010), `neg::InitFileStream`
(0x4b1130), `InitDatabase` (0x4b0fc0), `LoadCanvasID` (0x4b2860), `UnInitDatabase` (0x4b1040),
`UnInitFileStream` (0x4b1250), `ErrorLog` (0x4b3b10), the layer globals `g_layer_*`, `g_world`,
`g_viewport`, `g_screen_w/h`, `g_keys[]` (Windows VK codes), `g_editor`, `g_gametype`.

## Things not reconstructed yet

Call into them through small functions with a TODO body so the call site reads like the
original. Free helpers you need (from `_free.c`): if small and self-contained, reconstruct them
as `static` functions in your `.cpp` named after what they do with the address in a comment;
otherwise write a `static` stub that does `NEG_TODO(0xADDR, "what")` (game/src/Todo.h) and
returns a neutral value. Do not create shared files — `static` keeps parts from colliding.

## Regenerate, build, run

```
cd <repo>
python3 tools/decomp/globals.py /home/claude/decomp/raw /home/claude/jn/game/Program_Executable_Files/Neutron2.exe game/src
python3 tools/decomp/gen_class.py --class C2DInGameMenu --base CLocalGameObject,CGameObject --base-size 0x4c8 \
    --raw /home/claude/decomp/raw --classes /home/claude/decomp/classes.json --abi /home/claude/decomp/abi.json \
    --src game/src --out game/src/gen --names /home/claude/decomp/lgo_slots.json
cmake -S engine -B <your build dir> -DCMAKE_BUILD_TYPE=Debug && make -C <your build dir> -j8 negatron
cd /home/claude/jn/game/Program_Executable_Files && DISPLAY=:99 OMT_WINDOW_SIZE=640x480 OMT_FRAMES=30 \
    OMT_SCREENSHOT=/tmp/shot.bmp timeout 20 <your build dir>/negatron
```

Warnings from `-Wfloat-conversion` in your files usually mean a type is wrong — fix them.

## Style

Each function starts with its address in a comment. Name things by what they do once you know;
keep the original's behaviour, including its quirks (comment them). No speculative "fixes".

## Classes with several bases (game objects that are also OMT elements)

Most game classes are both an OMT element and a game object, e.g. `CGameType : OMediaElement,
CLocalGameObject` (with `OMediaClassStreamer` as a shared virtual base, as in OMT). Write them as

```cpp
class CGameType : public OMediaElement, public CLocalGameObject { ... };
```

`classes.json` lists each vtable with the `this_offset` of its subobject. For CGameType: the
element vtable at +0 (87 slots), the CLocalGameObject vtable at +0x8c (286 slots), the listener
vtable at +0x30 (4 slots), the virtual-base streamer at the end. A method found in the vtable at
offset X receives `this` = object + X. So:

* in an **element-family** method (`this` = object start): offsets < 0x8c are OMediaElement
  fields (use the OMT accessors), 0x8c..0x554 are CLocalGameObject fields at (offset − 0x8c),
  and offsets ≥ 0x554 are the class's own fields, named `f_<offset from the object start>`;
* in a **CLocalGameObject-family** method (`this` = object + 0x8c): offsets < 0x4c8 are
  CGameObject/CLocalGameObject fields as usual; own fields are at (offset + 0x8c).

Pointer adjustments in the original (`p - 0x8c`, `p + 0xc0`, `*(int *)(p + 4)` vbtable lookups)
are casts between bases; write them as C++ casts (`static_cast` when the type is known,
`dynamic_cast` from a `CGameObject *` to its element otherwise — RTTI is on).

Element-family slots 0..42 are OMT's own virtuals (2002 OMediaElement):
0 dtor, 1 db_destruct, 2 get_approximate_size, 3 approximate_size_changed, 4 set_chunk_name,
5 db_update, 6 db_link, 7 reset, 8 search_by_id, 9 db_get_type, 10 container_link_element,
11 link, 12 unlink, 13 link_layer, 14 unlink_layer, 15 update_logic, 16 render, 17 render_reject,
18 render_geometry, 19 get_absolute_info, 20 local_to_world, 21 local_to_world_matrix, 22 hide,
23 getvisible_extend, 24 pause, 25 compute_hxform, 26 compute_hxform_hierarchy,
27/28 compute_model_matrix, 29 clicked, 30 validate_closer_hit, 31 get_dynamic_offset,
32 collide, 33 ray_intersect, 34 world_link, 35 element_link, 36 update_world_ptr,
37 render_sub_elements, 38 container_link, 39 container_unlink, 40 fill_coll_bounding_spheres,
41 fill_coll_triangles, 42 find_global_sphere.
Override those with the OMT 2.5 signature. Game slots from 43 on are named `ev<N>` unless a
log string names them (`InitGame()` → `InitGame`). For OMediaCanvasElement-derived classes the
element vtable continues with OMediaCanvasElement's virtuals (check OMT2.dll's vtable at the
address the 2002 constructor installs if you need them).

The generator does not handle these classes yet: declare and write everything by hand in
`<Class>.h/.cpp`.

## More OMT 2002 facts (verified against OMT2.dll)

* `tools/decomp/omt_vtables.py OMT2.dll <Class>` prints a 2002 OMT class's vtable with names
  (it runs the constructor in an emulator to find the vtable). Names marked `?folded` are
  shared identical code and may be wrong.
* OMedia3DShapeElement element vtable: 0..42 as OMediaElement, then 43 set_shape, 44 get_shape,
  45 process_shape_picking, 46 process_polygon_surface_picking, 47 render_draw_shape;
  OMedia3DMorphAnim adds 48 set_anim_def. Game 3D objects (C3DObject...) add their slots from 49.
  The OMediaAnim subobject (2002: at +0x90) has its own vtable: 0 dtor, 3 set_anim_def,
  4 setcurrentsequence, 5/6 setcurrentframe, 7 getcurrentframe_pos, 8 advance_frame,
  9 setplay_timebased, 10 update_logic, 11 pause, 12 find_current_frame, 13 find_next_frame.
* The −0x14 rule (2002 offset ≥ 0x8c → 2.5 offset + 0x14) holds for OMedia3DShapeElement and
  OMedia3DMorphAnim too: 2002 +0x8c is the shape pointer (2.5 `shape` at +0xa0).
* OMedia3DShape (not an element): 2002 and 2.5 layouts agree as far as the code uses them
  (0x3c visual_sphere with the radius at +0x4c, five STL vectors from +0x64: vertices at 0x64,
  polygons at 0x84 (first/last at +0x88/+0x8c, 0x30 bytes per polygon)). Its vtable slot 17
  (+0x44) is `find_cube(minx, maxx, miny, maxy, minz, maxz)` (check the order).
