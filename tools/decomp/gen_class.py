#!/usr/bin/env python3
"""Generate C++ for the mechanical methods of a game class (accessors, forwarders, vector get/set,
log-only stubs) from the raw decompilation, and declare every virtual slot the class defines.

Types are solved, not guessed per function: fields, slot parameters and slot returns are type
variables, unified through every store, forward and out-parameter copy, with evidence from all
casts in the class and from the decompiler's prototypes of every override in the class family.
(The game copies floats through `uint` parameters a lot; a per-function guess would turn those bit
copies into int->float conversions.)

Outputs, kept under game/src/gen/ and regenerated when the tools improve:
  <Class>_fields.inc   data members used by generated code (f_<offset>)
  <Class>_slots.inc    declarations of every virtual slot the class defines (generated or not)
  <Class>_gen.cpp      bodies of the generated slots
  <Class>_status.json  which slots still need writing by hand
Slots written by hand (`// SLOT <n>` in game/src/<Class>.cpp) are not generated; members declared
by hand (`// FIELD <off> <size>` in game/src/<Class>.h) are left out of the fields include.

    gen_class.py --class CGameObject --raw raw/ --classes classes.json --abi abi.json \
                 --src game/src --out game/src/gen [--names slots.json]
"""
import argparse, json, os, re, sys
sys.path.insert(0, os.path.dirname(__file__))
from fields import load, this_scale, num, analyse

GT = {'uint': 'U', 'int': 'I', 'uchar': 'B', 'char': 'B', 'bool': 'B', 'byte': 'B', 'float': 'F', 'float10': 'F',
      'int32_t': 'I', 'uint32_t': 'U', 'int16_t': 'S', 'uint16_t': 'S', 'ushort': 'S', 'short': 'S',
      'uint8_t': 'B', 'int8_t': 'B', 'undefined4': 'U', 'undefined': 'B'}
CT = {'F': 'float', 'B': 'unsigned char', 'S': 'short', 'I': 'int', 'U': 'unsigned int'}
RANK = 'FBSIU'                    # strongest evidence first

SIG = re.compile(r'^\s*(?P<ret>[\w ]+?\s*\**)\s*(?:__thiscall|__fastcall|__cdecl|__stdcall)?\s*(?P<name>[\w.]+)\s*\((?P<args>[^)]*)\)', re.S)
FIELD = re.compile(r'\*\s*\(\s*(?P<t>\w+)\s*\*+\s*\)\s*\(\s*(?P<cast>\(int\)\s*)?param_1\s*\+\s*(?P<k>0x[0-9a-f]+|\d+)\s*\)|\bparam_1\s*\[\s*(?P<i>0x[0-9a-f]+|\d+)\s*\]|\*\s*\(\s*(?P<t0>\w+)\s*\*+\s*\)\s*param_1\b(?!\s*\[)')
VCALL = re.compile(r'\(\s*\*\*\s*\(\s*code\s*\*\*\s*\)\s*\(\s*\*\s*param_1\s*\+\s*(?P<off>0x[0-9a-f]+|\d+)\s*\)\s*\)\s*\((?P<args>[^;]*)\)')
OUTP = re.compile(r'\*\s*param_(?P<p>\d+)\b|\bparam_(?P<q>\d+)\s*\[\s*(?P<i>\d+)\s*\]')
LOG = 254                         # CGameObject::Log(this, fmt, ...) - cdecl, takes this explicitly


class UF:
    def __init__(s): s.p = {}; s.ev = {}
    def f(s, x):
        s.p.setdefault(x, x)
        while s.p[x] != x:
            s.p[x] = s.p[s.p[x]]; x = s.p[x]
        return x
    def u(s, a, b):
        ra, rb = s.f(a), s.f(b)
        if ra != rb:
            s.p[ra] = rb
            s.ev.setdefault(rb, set()).update(s.ev.pop(ra, set()))
    def e(s, x, t):
        if t: s.ev.setdefault(s.f(x), set()).add(t)
    def t(s, x, default='U'):
        ev = s.ev.get(s.f(x), set())
        for r in RANK:
            if r in ev: return CT[r]
        return CT[default]


def statements(body):
    inner = body.split('{', 1)[1].rsplit('}', 1)[0]
    if '{' in inner or '}' in inner:
        return None
    out = []
    for st in inner.split(';'):
        st = ' '.join(st.split())
        if not st:
            continue
        if st != 'return' and not st.startswith('return ') and \
           re.fullmatch(r'(?:unsigned |const )?\w+(?:\s+|\s*\*+\s*)\w+(\s*\[\s*\d+\s*\])?', st):
            return None                   # has locals
        out.append(st)
    return out


class Gen:
    def __init__(s, cls, classes, funcs, abi, names):
        s.cls, s.classes, s.funcs, s.abi, s.names = cls, classes, funcs, abi, names
        s.uf = UF()
        s.vt = classes[cls]['vtables'][0]['methods']
        s.fields_used = set()

    def slotname(s, slot):
        return s.names.get(slot) or f'v{slot}'

    def arity(s, slot):
        """Stack arguments of a slot, from the machine code of this class's implementation."""
        if slot >= len(s.vt): return None
        a = hex(int(s.vt[slot]['addr'], 16))
        pops = s.abi.get(a, {}).get('ret_pop', [])
        if len(pops) == 1:
            return pops[0] // 4
        f = s.funcs.get(int(a, 16))
        if f:
            m = SIG.match(f[1].strip())
            if m:
                args = [x for x in m.group('args').split(',') if x.strip() and x.strip() != 'void']
                return max(0, len(args) - 1)
        return None

    # ---------------------------------------------------------------- evidence
    def collect_evidence(s, fieldrows):
        for r in fieldrows:
            if r['sub'] == 0:
                for t in r['types']:
                    if t in ('float', 'float10'): s.uf.e(('F', r['off']), 'F')
                    elif t in ('char', 'uchar', 'bool'): s.uf.e(('F', r['off']), 'B')
                    elif t in ('short', 'ushort', 'int16_t'): s.uf.e(('F', r['off']), 'S')
                    elif t in ('int', 'int32_t'): s.uf.e(('F', r['off']), 'I')
                d = r.get('default')
                if isinstance(d, str) and d.startswith('0x') and len(d) >= 9:
                    v = int(d, 16)
                    if 0x3a000000 <= v <= 0x4b000000 or 0xba000000 <= v <= 0xcb000000:
                        s.uf.e(('F', r['off']), 'F')        # a float constant (1.0 = 0x3f800000)
        # prototypes of every implementation of each slot, across the class family
        fam = [c for n, c in s.classes.items() if n == s.cls or s.cls in c.get('bases', [])]
        for c in fam:
            for vt in c['vtables']:
                if len(vt['methods']) < len(s.vt): continue
                for si, m in enumerate(vt['methods'][:len(s.vt)]):
                    f = s.funcs.get(int(m['addr'], 16))
                    if not f: continue
                    mm = SIG.match(f[1].strip())
                    if not mm: continue
                    args = [x.strip() for x in mm.group('args').split(',') if x.strip() and x.strip() != 'void']
                    for a in args:
                        pm = re.fullmatch(r'([\w ]+?)\s*(\**)\s*param_(\d+)', a)
                        if pm and int(pm.group(3)) >= 2:
                            t = GT.get(pm.group(1).strip())
                            if pm.group(2): s.uf.e(('PE', si, int(pm.group(3))), t if t in 'FBS' else None)
                            elif t in ('F', 'B', 'S', 'I'): s.uf.e(('P', si, int(pm.group(3))), t)
                    rt = mm.group('ret').strip()
                    if s.abi.get(hex(int(m['addr'], 16)), {}).get('fret'): s.uf.e(('R', si), 'F')
                    elif not rt.endswith('*'):
                        t = GT.get(rt)
                        if t in ('F', 'B', 'S', 'I'): s.uf.e(('R', si), t)

    # ---------------------------------------------------------------- translation
    def translate(s, slot, addr, body):
        m = SIG.match(body.strip())
        if not m: return None
        n = s.arity(slot)
        if n is None: return None
        sc = this_scale(body) or 4
        stmts = statements(body)
        if stmts is None: return None
        rett = m.group('ret').strip()
        ptr_ret = rett.endswith('*')
        if ptr_ret: return None
        void = rett == 'void' and not s.abi.get(hex(addr), {}).get('fret')
        ptrs = set()
        lines = []

        def field(mo):
            if mo.group('t0') is not None: return None
            if mo.group('i') is not None: k = num(mo.group('i')) * sc
            else: k = num(mo.group('k')) * (1 if mo.group('cast') else sc)
            return None if k == 0 else k

        def expr(e, unify_with=None):
            e = e.strip(); fail = []
            refs = []
            def fs(mo):
                k = field(mo)
                if k is None: fail.append(1); return 'X'
                s.fields_used.add(k); refs.append(('F', k)); return f'f_{k:03x}'
            e2 = FIELD.sub(fs, e)
            def ps(mo):
                i = int(mo.group(1))
                if i < 2 or i >= 2 + n: fail.append(1); return 'X'
                refs.append(('P', slot, i)); return f'a{i}'
            e2 = re.sub(r'\bparam_(\d+)\b', ps, e2)
            e2 = re.sub(r'\((?:uint|int|uchar|char|float|float10|ushort|short|int16_t|int32_t|uint32_t|uint8_t|bool)\)', '', e2)
            if fail or re.search(r'code|fcn\.|0x5[0-9a-f]{5}|\*\s*\(|&|\[|"|_sym', e2): return None
            if not re.fullmatch(r'[\w\s+\-*/.()]+', e2): return None
            if unify_with is not None:
                # a plain copy ties the types together; arithmetic only if every operand is float-like
                if len(refs) == 1 and re.fullmatch(r'\(?\s*\w+\s*\)?', e2): s.uf.u(unify_with, refs[0])
                elif refs and re.search(r'[+\-*/]', e2):
                    for r in refs: s.uf.u(unify_with, r)
            if re.search(r'\d\.\d|\d+\.0', e2) and unify_with is not None: s.uf.e(unify_with, 'F')
            return e2.strip()

        for st in stmts:
            if st == 'return':
                lines.append('return;'); continue
            if st.startswith('return '):
                if void: return None
                e = expr(st[7:], ('R', slot))
                if e is None: return None
                lines.append(f'return {e};'); continue
            vm = VCALL.fullmatch(st)
            if vm:
                c = s.vcall(slot, vm, n, expr)
                if c is None: return None
                lines.append(c + ';'); continue
            if re.fullmatch(r'CGameObject__v13\(.*\)', st):
                lines.append('// (debug log - empty in the shipped build)'); continue
            mm = re.fullmatch(r'(?P<lhs>[^=]+?)\s*=\s*(?P<rhs>[^=].*)', st)
            if not mm: return None
            lhs, rhs = mm.group('lhs').strip(), mm.group('rhs')
            fm = FIELD.fullmatch(lhs)
            if fm:
                k = field(fm)
                if k is None: return None
                s.fields_used.add(k)
                e = expr(rhs, ('F', k))
                if e is None: return None
                lines.append(f'f_{k:03x} = {e};'); continue
            om = OUTP.fullmatch(lhs)
            if om:
                p = int(om.group('p') or om.group('q')); i = int(om.group('i') or 0)
                if p < 2 or p >= 2 + n: return None
                fm2 = FIELD.fullmatch(rhs.strip())
                if not fm2: return None
                k = field(fm2)
                if k is None: return None
                s.fields_used.add(k); ptrs.add(p)
                s.uf.u(('PE', slot, p), ('F', k))
                lines.append(f'a{p}[{i}] = f_{k:03x};'); continue
            return None
        return dict(slot=slot, addr=addr, n=n, void=void, ptrs=ptrs, lines=lines)

    def vcall(s, slot, vm, n, expr):
        tgt = num(vm.group('off')) // 4
        args = [a.strip() for a in re.split(r',(?![^(]*\))', vm.group('args')) if a.strip()]
        if args and args[0] == 'param_1':
            if tgt != LOG: return None
            es = []
            for a in args[1:]:
                if a.startswith('"'): es.append(a); continue
                x = expr(a)
                if x is None: return None
                es.append(x)
            return f'Log({", ".join(es)})'
        k = s.arity(tgt)
        if k is None: return None
        if len(args) > k: args = args[:k]
        if len(args) < k: return None
        es = []
        for j, a in enumerate(args):
            if a.startswith('"'): return None
            x = expr(a, ('P', tgt, 2 + j))
            if x is None: return None
            es.append(x)
        return f'{s.slotname(tgt)}({", ".join(es)})'

    # ---------------------------------------------------------------- output
    def sig(s, slot, n, void, ptrs):
        ps = []
        for i in range(2, 2 + n):
            if i in ptrs: ps.append(f'{s.uf.t(("PE", slot, i))} *a{i}')
            else: ps.append(f'{s.uf.t(("P", slot, i))} a{i}')
        ret = 'void' if void else s.uf.t(('R', slot))
        return ret, ', '.join(ps)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--class', dest='cls', required=True)
    ap.add_argument('--raw', required=True); ap.add_argument('--classes', required=True)
    ap.add_argument('--abi', required=True); ap.add_argument('--src', required=True)
    ap.add_argument('--out', required=True); ap.add_argument('--names')
    a = ap.parse_args()
    classes, funcs = load(a.raw, a.classes)
    abi = json.load(open(a.abi))
    rows, _ = analyse(a.cls, classes, funcs)
    from fields import ctor_defaults
    defs = ctor_defaults(a.cls, classes, funcs)
    fieldrows = [dict(sub=k[0], off=k[1], types=sorted(v['types']), default=defs.get(k[1]) if k[0] == 0 else None)
                 for k, v in rows.items()]
    names = {}
    if a.names:
        for k, v in json.load(open(a.names)).items():
            if v.get('name'): names[int(k)] = v['name']
    hand_slots, hand_fields = set(), []
    for fn in (f'{a.src}/{a.cls}.cpp', f'{a.src}/{a.cls}.h'):
        if os.path.exists(fn):
            t = open(fn).read()
            hand_slots |= {int(x) for x in re.findall(r'//\s*SLOT\s+(\d+)', t)}
            hand_fields += [(num(o), num(z)) for o, z in re.findall(r'//\s*FIELD\s+(0x[0-9a-f]+|\d+)\s+(0x[0-9a-f]+|\d+)', t)]
    g = Gen(a.cls, classes, funcs, abi, names)
    g.collect_evidence(fieldrows)
    hdrfile = f'{a.src}/{a.cls}.h'
    if os.path.exists(hdrfile):
        # `// TYPE <off> <ctype>`: a field type known from hand-written code (authoritative)
        CK = {'float': 'F', 'int': 'I', 'unsigned': 'U', 'uint': 'U', 'uchar': 'B', 'short': 'S'}
        for o, t in re.findall(r'//\s*TYPE\s+(0x[0-9a-f]+)\s+(\w+)', open(hdrfile).read()):
            g.uf.ev[g.uf.f(('F', num(o)))] = {CK[t]}
        TK = {'float': 'F', 'unsigned char': 'B', 'bool': 'B', 'char': 'B', 'short': 'S', 'int': 'I', 'unsigned int': 'U'}
        for d in re.finditer(r'virtual\s+(?P<ret>[\w ]+?)\s*(?P<rp>\*?)\s*(?P<name>\w+)\s*\((?P<ps>[^)]*)\)[^\n]*//\s*SLOT\s+(?P<n>\d+)', open(hdrfile).read()):
            si = int(d.group('n'))
            names.setdefault(si, d.group('name'))
            if not d.group('rp') and d.group('ret').strip() in TK: g.uf.e(('R', si), TK[d.group('ret').strip()])
            ps = [x.strip() for x in d.group('ps').split(',') if x.strip() and x.strip() != 'void']
            for j, x in enumerate(ps):
                t = re.sub(r'\s*\w+$', '', x).strip()
                if '*' in t or '&' in t: continue
                if t in TK: g.uf.e(('P', si, 2 + j), TK[t])
                # a hand-declared type is authoritative
                g.uf.ev[g.uf.f(('P', si, 2 + j))] = {TK.get(t, 'U')} if t in TK else g.uf.ev.get(g.uf.f(('P', si, 2 + j)), set())
    own = {}
    for si, m in enumerate(g.vt):
        ad = int(m['addr'], 16)
        f = funcs.get(ad)
        if f and f[0].startswith(a.cls + '__'):
            own[si] = (ad, f[1])
    gen, todo = {}, {}
    for si in sorted(own):
        ad, body = own[si]
        r = None if si in hand_slots else g.translate(si, ad, body)
        if r: gen[si] = r
        else:
            m = SIG.match(body.strip())
            n = g.arity(si) or 0
            void = bool(m) and m.group('ret').strip() == 'void' and not abi.get(hex(ad), {}).get('fret')
            ptrs = set()
            if m:
                for x in m.group('args').split(','):
                    pm = re.fullmatch(r'\s*[\w ]+?\s*\*+\s*param_(\d+)\s*', x)
                    if pm and int(pm.group(1)) >= 2: ptrs.add(int(pm.group(1)))
            todo[si] = dict(slot=si, addr=ad, n=n, void=void, ptrs=ptrs)
    os.makedirs(a.out, exist_ok=True)
    claimed = lambda k: any(o <= k < o + z for o, z in hand_fields)
    hdr = '// generated by tools/decomp/gen_class.py from the decompilation of Neutron2.exe - do not edit\n'
    allf = set(g.fields_used) | {r['off'] for r in fieldrows if r['sub'] == 0 and r['off'] >= 4}
    with open(f'{a.out}/{a.cls}_fields.inc', 'w') as fh:
        fh.write(hdr)
        SZ = {'float': 4, 'unsigned int': 4, 'int': 4, 'unsigned char': 1, 'short': 2}
        end = None
        for k in sorted(allf):
            if claimed(k):
                end = None; continue
            t = g.uf.t(('F', k))
            if end is not None and k < end:
                fh.write(f'\t// f_{k:03x} overlaps the previous field\n'); continue
            if end is not None and k > end and not any(claimed(x) for x in range(end, k)):
                fh.write(f'\tunsigned char  _gap_{end:03x}[{k - end}];\n')
            fh.write(f'\t{t:<14} f_{k:03x};\n')
            end = k + SZ[t]
    with open(f'{a.out}/{a.cls}_slots.inc', 'w') as fh:
        fh.write(hdr)
        for si in sorted(set(gen) | set(todo)):
            if si in hand_slots: continue
            r = gen.get(si) or todo[si]
            ret, ps = g.sig(si, r['n'], r['void'], r['ptrs'])
            tag = 'gen' if si in gen else ('hand' if si in hand_slots else 'TODO')
            fh.write(f'\tvirtual {ret} {g.slotname(si)}({ps});\t// slot {si} {hex(r["addr"])} {tag}\n')
    with open(f'{a.out}/{a.cls}_gen.cpp', 'w') as fh:
        fh.write(hdr + f'#include "../{a.cls}.h"\n\n')
        for si, r in sorted(gen.items()):
            ret, ps = g.sig(si, r['n'], r['void'], r['ptrs'])
            fh.write(f'{ret} {a.cls}::{g.slotname(si)}({ps})\t// {hex(r["addr"])}\n{{\n')
            for l in r['lines']:
                if l == 'return;' and l is r['lines'][-1]: continue
                fh.write(f'\t{l}\n')
            fh.write('}\n\n')
    pending = sorted(si for si in todo if si not in hand_slots)
    with open(f'{a.out}/{a.cls}_todo.cpp', 'w') as fh:
        fh.write(hdr + f'// Stubs for slots not reconstructed yet: each logs its original address once.\n#include "../{a.cls}.h"\n#include "../Todo.h"\n\n')
        for si in pending:
            r = todo[si]
            ret, ps = g.sig(si, r['n'], r['void'], r['ptrs'])
            fh.write(f'{ret} {a.cls}::{g.slotname(si)}({ps})\t{{ NEG_TODO({hex(r["addr"])}, "{a.cls} slot {si}"); ')
            fh.write('}\n' if ret == 'void' else f'return ({ret})0; }}\n')
    json.dump(dict(todo=[hex(todo[s]['addr']) + f':{s}' for s in pending], generated=sorted(gen)),
              open(f'{a.out}/{a.cls}_status.json', 'w'), indent=1)
    print(f'{a.cls}: {len(gen)} generated, {len(hand_slots)} hand-written, {len(pending)} still to write')
    print('to write:', ' '.join(f'{s}:{hex(todo[s]["addr"])}' for s in pending))


if __name__ == '__main__':
    main()
