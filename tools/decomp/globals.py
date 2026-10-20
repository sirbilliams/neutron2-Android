#!/usr/bin/env python3
"""The game's global variables (.data/.bss of Neutron2.exe) for the reconstructed source.

Globals overlap freely in the original (arrays whose elements are also addressed directly,
records reached by fixed offsets), so they are not declared as separate C++ variables. Instead
the data section is one block with the exe's initial bytes, and every address gets a typed view:

    neg::g_data[]   the section's bytes (non-pointer values), initialised from the exe
    neg::g_ptrs[]   one 64-bit pointer slot per 4-byte word of the section (pointer values,
                    which no longer fit in 4 bytes)

    #define g_545e50  (*(unsigned int *)&neg::g_data[0x13e50])     scalar
    #define g_53ef44  ((short *)&neg::g_data[0xcf44])             array
    #define g_545e78  (neg::g_ptrs[0x4f9e])                       pointer (void *)
    #define g_545da8  (&neg::g_ptrs[0x4f76a])                     array of pointers

Whether an address holds a pointer is decided from how the code dereferences it. Pointer slots
whose initial value points at a string in .rdata are initialised with that string; other
initial pointers (vtables, functions) are listed for hand-written setup.
Addresses already named in hand-written headers (`extern ...; // 0x5xxxxx`) are skipped.

    globals.py raw/ Neutron2.exe game/src  ->  game/src/gen/globals.h, game/src/gen/globals.cpp
"""
import os, re, sys, struct
from collections import defaultdict
import pefile

DATA_LO, DATA_HI = 0x532000, 0x6f8bd8
INIT_HI = 0x542000
CT = {'int': 'int', 'uint': 'unsigned int', 'int32_t': 'int', 'uint32_t': 'unsigned int', 'float': 'float',
      'char': 'char', 'uchar': 'unsigned char', 'byte': 'unsigned char', 'bool': 'bool', 'int16_t': 'short',
      'uint16_t': 'unsigned short', 'ushort': 'unsigned short', 'short': 'short', 'double': 'double',
      'undefined4': 'unsigned int', 'uint8_t': 'unsigned char', 'int8_t': 'signed char', 'code': 'unsigned int',
      'float10': 'float', 'longlong': 'long long', 'undefined': 'unsigned char', 'undefined2': 'unsigned short'}
RANK = ['double', 'float', 'int', 'unsigned int', 'short', 'unsigned short', 'char', 'unsigned char', 'signed char', 'bool', 'long long']
A6 = r'(0x[56][0-9a-f]{5})'


def main():
    raw, exe, src = sys.argv[1:4]
    pe = pefile.PE(exe)
    img = pe.get_memory_mapped_image()
    base = pe.OPTIONAL_HEADER.ImageBase
    named = {}
    for fn in os.listdir(src):
        if fn.endswith('.h'):
            for m in re.finditer(r'extern\s+[^;]*?\b(\w+)\s*(?:\[[^\]]*\])*\s*;\s*//\s*(0x[0-9a-f]{6})', open(f'{src}/{fn}').read()):
                named[int(m.group(2), 16)] = m.group(1)
    # tools/decomp/global_types/*.txt: `0x545e50 ptr OMediaDataBase* g_screens_db` (kind: ptr, ptrarray, val, array)
    override = {}
    gt = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'global_types')
    if os.path.isdir(gt):
        for fn in sorted(os.listdir(gt)):
            for line in open(os.path.join(gt, fn)):
                line = line.split('#')[0].strip()
                if not line: continue
                f = line.split()
                if len(f) >= 3:
                    a = int(f[0], 16)
                    kind, ty = f[1], (' '.join(f[2:-1]) if len(f) > 3 else f[2])
                    nm = f[-1] if len(f) > 3 else None
                    if a in override:                      # several files: keep every name as an alias
                        k0, t0, n0 = override[a]
                        if (k0, t0.replace(' ', '')) != (kind, ty.replace(' ', '')):
                            print(f'globals.py: conflicting types for 0x{a:06x}: {k0} {t0} vs {kind} {ty} ({fn})')
                        names = [x for x in (n0 or '').split(',') if x] + ([nm] if nm and nm not in (n0 or '').split(',') else [])
                        override[a] = (k0, t0, ','.join(names) or None)
                    else:
                        override[a] = (kind, ty, nm)
    text = ''.join(open(f'{raw}/{fn}', errors='replace').read() for fn in sorted(os.listdir(raw)) if fn.endswith('.c'))
    inr = lambda a: DATA_LO <= a < DATA_HI
    types = defaultdict(set); arrays = {}; ptr = set(); seen = set()
    for m in re.finditer(r'\*\s*\(\s*(\w+)\s*(\*+)\s*\)\s*' + A6 + r'\b', text):
        a = int(m.group(3), 16)
        if not inr(a): continue
        seen.add(a)
        if len(m.group(2)) > 1: ptr.add(a)
        else: types[a].add(CT.get(m.group(1), 'unsigned int'))
    for m in re.finditer(r'\*\s*\(\s*(\w+)\s*(\*+)\s*\)\s*\(\s*(?:[^()]|\([^()]*\))*?\*\s*(0x[0-9a-f]+|\d+)\s*\+\s*' + A6 + r'\s*\)', text):
        a = int(m.group(4), 16)
        if not inr(a): continue
        seen.add(a)
        st = int(m.group(3), 0)
        arrays[a] = max(arrays.get(a, 0), 0) or st
        if len(m.group(2)) > 1: ptr.add(a)
        else: types[a].add(CT.get(m.group(1), 'unsigned int'))
    # value used as a pointer: *(T *)(*(U *)A + k), (*(U **)A)[k], *(int *)A passed then deref'd
    for m in re.finditer(r'\*\s*\(\s*\w+\s*\*+\s*\)\s*\(\s*\*\s*\(\s*\w+\s*\*+\s*\)\s*' + A6 + r'\s*\+', text):
        a = int(m.group(1), 16)
        if inr(a): ptr.add(a)
    for m in re.finditer(r'\(\s*\*\s*\(\s*\w+\s*\*+\s*\)\s*' + A6 + r'\s*\)\s*\[', text):
        a = int(m.group(1), 16)
        if inr(a): ptr.add(a)
    for m in re.finditer(r'\*\s*\(\s*\w+\s*\*+\s*\)\s*\(\s*\*\s*\(\s*\w+\s*\*+\s*\)\s*\(\s*(?:[^()]|\([^()]*\))*?\*\s*4\s*\+\s*' + A6, text):
        a = int(m.group(1), 16)
        if inr(a): ptr.add(a)
    for m in re.finditer(r'\b' + A6 + r'\b', text):
        a = int(m.group(1), 16)
        if inr(a): seen.add(a)

    def pick(ts):
        for r in RANK:
            if r in ts: return r
        return 'unsigned int'

    fwd = sorted({re.sub(r'[\s*]', '', t) for _, t, _ in override.values() if re.match(r'\s*(C|OMedia)\w+', t)})
    hdr = ['// generated by tools/decomp/globals.py - typed views of Neutron2.exe\'s data section',
           '#pragma once', '#include <stddef.h>'] + [f'class {c};' for c in fwd] + [
           'namespace neg {',
           f'enum {{ DATA_BASE = 0x{DATA_LO:x}, DATA_SIZE = 0x{DATA_HI - DATA_LO:x} }};',
           'extern unsigned char g_data[DATA_SIZE];',
           'extern void *g_ptrs[DATA_SIZE / 4];',
           'void InitDataPointers(void);\t// pointer slots the exe initialised (strings)',
           '}', '']
    strings_init = []; other_ptr_init = []
    for a in sorted(set(seen) | set(override)):
        if a in named: continue
        off = a - DATA_LO
        n = f'g_{a:06x}'
        if a in override:
            kind, ty, name = override[a]
            alias = [n] + (name.split(',') if name else [])
            for nn in alias:
                if kind == 'ptr':        hdr.append(f'#define {nn} (*({ty} *)&neg::g_ptrs[0x{off // 4:x}])')
                elif kind == 'ptrarray': hdr.append(f'#define {nn} (({ty} *)&neg::g_ptrs[0x{off // 4:x}])')
                elif kind == 'array':    hdr.append(f'#define {nn} (({ty} *)&neg::g_data[0x{off:x}])')
                else:                    hdr.append(f'#define {nn} (*({ty} *)&neg::g_data[0x{off:x}])')
            continue
        if a in ptr:
            if a % 4:
                hdr.append(f'// {n}: pointer at an unaligned address - write by hand'); continue
            if a in arrays:
                hdr.append(f'#define {n} (&neg::g_ptrs[0x{off // 4:x}])\t\t// pointer array, stride {arrays[a]}')
            else:
                hdr.append(f'#define {n} (neg::g_ptrs[0x{off // 4:x}])\t\t// pointer')
            if a < INIT_HI:
                v = struct.unpack('<I', img[a - base:a - base + 4])[0]
                if v:
                    if 0x4ca000 <= v < 0x532000 or 0x532000 <= v < INIT_HI:
                        s = img[v - base:v - base + 256].split(b'\0')[0]
                        if s and all(32 <= c < 127 for c in s):
                            strings_init.append((off // 4, s.decode()))
                            continue
                    other_ptr_init.append((a, v))
            continue
        ty = pick(types.get(a, set()))
        if a in arrays:
            hdr.append(f'#define {n} (({ty} *)&neg::g_data[0x{off:x}])\t\t// array, stride {arrays[a]}')
        elif not types.get(a):
            hdr.append(f'#define {n} (&neg::g_data[0x{off:x}])\t\t// address only')
        else:
            hdr.append(f'#define {n} (*({ty} *)&neg::g_data[0x{off:x}])')
    init = img[DATA_LO - base:INIT_HI - base]
    last = len(init.rstrip(b'\0'))
    cpp = ['// generated by tools/decomp/globals.py - the data section with the exe\'s initial values',
           '#include "globals.h"', 'namespace neg {',
           f'unsigned char g_data[DATA_SIZE] __attribute__((aligned(16))) = {{']
    for i in range(0, last, 32):
        cpp.append('\t' + ','.join(str(b) for b in init[i:i + 32]) + ',')
    cpp += ['};', 'void *g_ptrs[DATA_SIZE / 4];', '', 'void InitDataPointers(void)', '{']
    for slot, s in strings_init:
        esc = s.replace('\\', '\\\\').replace('"', '\\"')
        cpp.append(f'\tg_ptrs[0x{slot:x}] = (void *)"{esc}";')
    cpp.append('}')
    cpp.append('// initial pointers to code/vtables, to set up by hand where they matter:')
    for a, v in other_ptr_init:
        cpp.append(f'//   0x{a:06x} = 0x{v:08x}')
    cpp.append('}')
    os.makedirs(f'{src}/gen', exist_ok=True)
    open(f'{src}/gen/globals.h', 'w').write('\n'.join(hdr) + '\n')
    open(f'{src}/gen/globals.cpp', 'w').write('\n'.join(cpp) + '\n')
    print(f'{len(seen)} addresses: {len(ptr)} pointers, {len(arrays)} arrays, {len(named)} named by hand; '
          f'{len(strings_init)} string pointers, {len(other_ptr_init)} other initial pointers')


if __name__ == '__main__':
    main()
