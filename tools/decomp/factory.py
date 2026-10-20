#!/usr/bin/env python3
"""Recover the object factory table (0x4912d0: 4-char ID -> class) by running the function.

The factory is a 18 KB tree of comparisons. Rather than reading it, run it in a CPU emulator once
per ID constant it compares against: `operator new` returns a fake block, every other call is
recorded and returns `this` (constructors do), and the debug-log call at the end carries the
class name. The result: ID, class name, allocation size, constructor, and the offset of the
returned pointer (the CGameObject subobject) inside the allocation.

    factory.py Neutron2.exe out.json
"""
import json, struct, sys, re
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
from unicorn.x86_const import *
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM

FACTORY, SIZE = 0x4912d0, 17979
NEW, LOG = 0x4b3aa0, 0x463890
HEAP = 0x10000000
STACK = 0x20000000


def main():
    exe, out = sys.argv[1:3]
    abi = json.load(open(sys.argv[3])) if len(sys.argv) > 3 else {}
    pe = pefile.PE(exe)
    base = pe.OPTIONAL_HEADER.ImageBase
    img = pe.get_memory_mapped_image()
    md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True
    ids = set()
    for i in md.disasm(img[FACTORY - base:FACTORY - base + SIZE], FACTORY):
        for o in i.operands:
            if o.type == X86_OP_IMM:
                v = o.imm & 0xffffffff
                if 0x20202020 <= v <= 0x7a7a7a7a:
                    ids.update(range(v - 8, v + 9))
                    if i.mnemonic in ('sub', 'add'): ids.update(range(v, v + 0x200))   # sub eax,A ; cmp eax,k chains
    # IDs reached through `sub eax, n / dec eax` chains: also try every 4cc whose
    # last character is printable near the compared values (cheap: a few thousand runs)
    rows = []
    todo = sorted(ids); tried = set()
    while todo:
        cid = todo.pop()
        if cid in tried: continue
        tried.add(cid)
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0, 0x1000)                                   # fs:[0] (SEH chain)
        uc.mem_map(base, (len(img) + 0xfff) & ~0xfff)
        uc.mem_write(base, bytes(img))
        uc.mem_map(HEAP, 0x100000); uc.mem_map(STACK - 0x100000, 0x100000)
        sp = STACK - 0x1000
        uc.mem_write(sp, struct.pack('<II', 0xdeadbeef, cid))  # return address, id
        uc.reg_write(UC_X86_REG_ESP, sp)
        rec = dict(size=None, ctor=None, name=None, calls=[])

        def hook(uc, addr, size, user):
            b = uc.mem_read(addr, size)
            if b[0] != 0xe8:
                # concolic: which ID would make this comparison on eax come out equal?
                ins = next(md.disasm(bytes(b), addr))
                if ins.mnemonic in ('cmp', 'sub', 'dec', 'test') and ins.operands and ins.op_str.startswith('eax'):
                    eax = uc.reg_read(UC_X86_REG_EAX)
                    if ins.mnemonic == 'dec': want = 1
                    elif ins.mnemonic == 'test': want = 0
                    elif len(ins.operands) == 2 and ins.operands[1].type == X86_OP_IMM: want = ins.operands[1].imm & 0xffffffff
                    else: return
                    wants = range(0, want + 1) if ins.mnemonic == 'cmp' and want < 0x400 else [want]   # jump tables
                    for w in wants:
                        c = (cid + w - eax) & 0xffffffff
                        if 0x20202020 <= c <= 0x7a7a7a7a and c not in tried: todo.append(c)
                return
            tgt = next(md.disasm(bytes(b), addr)).operands[0].imm
            esp = uc.reg_read(UC_X86_REG_ESP)
            arg = lambda n: struct.unpack('<I', uc.mem_read(esp + 4 * n, 4))[0]
            if tgt == NEW:
                rec['size'] = arg(0); uc.reg_write(UC_X86_REG_EAX, HEAP)
            elif tgt == LOG:
                try:
                    s = bytes(uc.mem_read(arg(2), 64)).split(b'\0')[0].decode()
                    if re.fullmatch(r'C\w+', s): rec['name'] = s
                except Exception: pass
                rec['ret'] = arg(0)
                uc.emu_stop(); return
            else:
                ecx = uc.reg_read(UC_X86_REG_ECX)
                if rec['size'] and rec['ctor'] is None and ecx == HEAP:
                    rec['ctor'] = tgt; rec['ctor_arg'] = arg(0)
                uc.reg_write(UC_X86_REG_EAX, ecx)
                pops = abi.get(hex(tgt), {}).get('ret_pop', [0])
                uc.reg_write(UC_X86_REG_ESP, esp + (pops[0] if len(pops) == 1 else 0))
            uc.reg_write(UC_X86_REG_EIP, addr + size)

        uc.hook_add(UC_HOOK_CODE, hook, begin=FACTORY, end=FACTORY + SIZE)
        try:
            uc.emu_start(FACTORY, 0xdeadbeef, count=20000)
        except UcError:
            pass
        rr = rec.get('ret')
        if rec['size'] and rec['name']:
            rows.append(dict(id=struct.pack('>I', cid).decode('latin-1'), idval=hex(cid), cls=rec['name'],
                             size=hex(rec['size']), ctor=hex(rec['ctor']) if rec['ctor'] else None,
                             ctor_arg=rec.get('ctor_arg'), subobject=hex(rr - HEAP) if rr and rr >= HEAP else None))
        continue
        eax = uc.reg_read(UC_X86_REG_EAX)
        if rec['size'] and rec['name']:
            rows.append(dict(id=struct.pack('>I', cid).decode('latin-1'), idval=hex(cid), cls=rec['name'],
                             size=hex(rec['size']), ctor=hex(rec['ctor']) if rec['ctor'] else None,
                             ctor_arg=rec.get('ctor_arg'), subobject=hex(eax - HEAP) if eax >= HEAP else None))
    seen = set(); uniq = []
    for r in rows:
        if r['idval'] not in seen: seen.add(r['idval']); uniq.append(r)
    json.dump(uniq, open(out, 'w'), indent=1)
    print(f'{len(uniq)} object types')


if __name__ == '__main__':
    main()
