#!/usr/bin/env python3
"""Name the virtual slots of OMT 2002 classes from OMT2.dll.

For each class: run its constructor's first instructions to see which vtable it installs at +0,
then read the vtable and name every entry from OMT2.dll's exports. Identical-code folding makes
some names wrong (an empty function shared by many slots gets one arbitrary name); those are
flagged '?folded'. Entries inherited from a base keep the base's slot name; an entry that differs
from the base's at the same slot is the class's override.

    omt_vtables.py OMT2.dll Class [Class...]   -> prints  slot  address  name
"""
import re, struct, sys, subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM


def main():
    dll = sys.argv[1]
    pe = pefile.PE(dll)
    base = pe.OPTIONAL_HEADER.ImageBase
    img = pe.get_memory_mapped_image()
    exp = {}
    for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
        if e.name: exp.setdefault(e.address + base, []).append(e.name.decode())
    md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True

    def ctor(cls):
        for a, ns in exp.items():
            for n in ns:
                if n.startswith(f'??0{cls}@@QAE@XZ'): return a
        return None

    def vt_of(a):
        """run the constructor (calls skipped) and read what ends up at this+0"""
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
        from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_ECX, UC_X86_REG_EIP, UC_X86_REG_EAX
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0, 0x1000); uc.mem_map(base, (len(img) + 0xfff) & ~0xfff); uc.mem_write(base, bytes(img))
        uc.mem_map(0x10000000 - 0x100000 if base == 0x10000000 else 0x10000000, 0x100000)
        H = 0x30000000; uc.mem_map(H, 0x10000); uc.mem_map(0x40000000 - 0x10000, 0x10000)
        sp = 0x40000000 - 0x1000
        uc.mem_write(sp, struct.pack('<II', 0xdeadbeef, 0)); uc.reg_write(UC_X86_REG_ESP, sp); uc.reg_write(UC_X86_REG_ECX, H)
        def hook(uc, addr, size, u):
            b = uc.mem_read(addr, size)
            if b[0] == 0xe8 or (b[0] == 0xff and b[1] == 0x15):
                uc.reg_write(UC_X86_REG_EAX, uc.reg_read(UC_X86_REG_ECX)); uc.reg_write(UC_X86_REG_EIP, addr + size)
        uc.hook_add(UC_HOOK_CODE, hook)
        try: uc.emu_start(a, 0xdeadbeef, count=5000)
        except UcError: pass
        return struct.unpack('<I', uc.mem_read(H, 4))[0]

    def short(n):
        m = re.match(r'\?(\w+)@(\w+)@@', n)
        if m: return f'{m.group(2)}::{m.group(1)}'
        m = re.match(r'\?\?_[GE](\w+)@@', n)
        if m: return f'{m.group(1)}::~dtor'
        return n

    for cls in sys.argv[2:]:
        a = ctor(cls)
        v = vt_of(a) if a else None
        print(f'== {cls} ctor {a and hex(a)} vtable {v and hex(v)}')
        if not v: continue
        i = 0
        while True:
            f = struct.unpack('<I', img[v - base + 4 * i:v - base + 4 * i + 4])[0]
            if not (base <= f < base + len(img)) or (i and f in [struct.unpack('<I', img[x - base:x - base + 4])[0] for x in []]):
                break
            ns = exp.get(f, ['?'])
            own = [short(n) for n in ns if f'@{cls}@@' in n]
            name = own[0] if own else short(ns[0]) + (' ?folded' if len(ns) > 1 else '')
            print(f'{i:3d} {hex(f)} {name}')
            i += 1
            if i > 200: break


if __name__ == '__main__':
    main()
