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

Classes with several bases (an OMT element + CLocalGameObject, e.g. C3DObject): pass --go-offset
(where the CLocalGameObject subobject sits, e.g. 0xc0) and --el-slots (how many element-vtable
slots belong to OMT, e.g. 49 for OMedia3DMorphAnim-based classes). Then two slot families are
generated: CGameObject-family slots (`// SLOT n`, names v<n> or logged names) and element-family
game slots (`// ESLOT n`, names ev<n>). Members: offsets inside the CLocalGameObject subobject
keep the CGameObject names (f_<offset from its start>); the class's own members are
f_<offset from the object start>; OMT element members are never generated.
"""
import argparse, json, os, re, sys
sys.path.insert(0, os.path.dirname(__file__))
from fields import load, this_scale, num, analyse

GT = {'uint': 'U', 'int': 'I', 'uchar': 'B', 'char': 'B', 'bool': 'B', 'byte': 'B', 'float': 'F', 'float10': 'F',
      'int32_t': 'I', 'uint32_t': 'U', 'int16_t': 'S', 'uint16_t': 'S', 'ushort': 'S', 'short': 'S',
      'uint8_t': 'B', 'int8_t': 'B', 'undefined4': 'U', 'undefined': 'B'}
CT = {'P': 'void *', 'F': 'float', 'B': 'unsigned char', 'S': 'short', 'I': 'int', 'U': 'unsigned int'}
RANK = 'PFBSIU'                   # strongest evidence first (pointers: 8 bytes on 64-bit)

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


GO_SIZE = 0x4c8                   # CLocalGameObject


class Fam:
    """one vtable family of the class: 'go' (CGameObject family) or 'el' (element family)"""
    def __init__(s, kind, vi, this_off, methods, first):
        s.kind, s.vi, s.this, s.methods, s.first = kind, vi, this_off, methods, first
    def key(s, slot): return (s.kind, slot)


class Gen:
    def __init__(s, cls, classes, funcs, abi, names, go_off, el_slots, base_size):
        s.cls, s.classes, s.funcs, s.abi, s.names = cls, classes, funcs, abi, names
        s.uf = UF(); s.fields_used = set(); s.ftype_override = {}; s.base_decl = {}
        s.go_off, s.base_size = go_off, base_size
        vts = classes[cls]['vtables']
        s.fams = []
        if go_off is None:                               # single chain: the only vtable is the go family
            s.fams.append(Fam('go', 0, 0, vts[0]['methods'], 0))
        else:
            for vi, vt in enumerate(vts):
                if vt['this_offset'] == go_off and len(vt['methods']) >= 265:
                    s.fams.append(Fam('go', vi, go_off, vt['methods'], 0))
                if vt['this_offset'] == 0:
                    s.fams.append(Fam('el', vi, 0, vt['methods'], el_slots))

    def slotname(s, fam, slot):
        n = s.names.get((fam.kind, slot))
        if n: return n
        return f'v{slot}' if fam.kind == 'go' else f'ev{slot}'

    def fam_of(s, kind):
        for f in s.fams:
            if f.kind == kind: return f

    # field naming -------------------------------------------------------------
    def fname(s, fam, k):
        """field at offset k from this (in fam) -> (full key, member name) or None"""
        full = k + fam.this
        if s.go_off is None:
            return full, f'f_{full:03x}'
        if full < s.go_off: return None                  # OMT element member
        if full < s.go_off + GO_SIZE:
            r = full - s.go_off
            return ('go', r), f'f_{r:03x}'
        return full, f'f_{full:03x}'

    def arity(s, fam, slot):
        if slot >= len(fam.methods): return None
        a = hex(int(fam.methods[slot]['addr'], 16))
        pops = s.abi.get(a, {}).get('ret_pop', [])
        if len(pops) == 1: return pops[0] // 4
        f = s.funcs.get(int(a, 16))
        if f:
            m = SIG.match(f[1].strip())
            if m:
                args = [x for x in m.group('args').split(',') if x.strip() and x.strip() != 'void']
                return max(0, len(args) - 1)
        return None

    # evidence -----------------------------------------------------------------
    def collect_evidence(s, fieldrows):
        for r in fieldrows:
            fam = next((f for f in s.fams if f.this == r['sub']), None)
            if not fam: continue
            fn = s.fname(fam, r['off'])
            if not fn: continue
            key = ('F', fn[0])
            for t in r['types']:
                if t in ('float', 'float10'): s.uf.e(key, 'F')
                elif t in ('char', 'uchar', 'bool'): s.uf.e(key, 'B')
                elif t in ('short', 'ushort', 'int16_t'): s.uf.e(key, 'S')
                elif t in ('int', 'int32_t'): s.uf.e(key, 'I')
                elif t == 'ptr': s.uf.e(key, 'P')
            d = r.get('default')
            if isinstance(d, str) and d.startswith('0x') and len(d) >= 9:
                v = int(d, 16)
                if 0x3a000000 <= v <= 0x4b000000 or 0xba000000 <= v <= 0xcb000000:
                    s.uf.e(key, 'F')
        fam_classes = [c for n, c in s.classes.items() if n == s.cls or s.cls in c.get('bases', [])]
        for fam in s.fams:
            for c in fam_classes:
                for vt in c['vtables']:
                    if len(vt['methods']) < len(fam.methods): continue
                    if fam.kind == 'el' and vt['this_offset'] != 0: continue
                    if fam.kind == 'go' and len(vt['methods']) < 265: continue
                    for si, m in enumerate(vt['methods'][:len(fam.methods)]):
                        f = s.funcs.get(int(m['addr'], 16))
                        if not f: continue
                        mm = SIG.match(f[1].strip())
                        if not mm: continue
                        args = [x.strip() for x in mm.group('args').split(',') if x.strip() and x.strip() != 'void']
                        for a in args:
                            pm = re.fullmatch(r'([\w ]+?)\s*(\**)\s*param_(\d+)', a)
                            if pm and int(pm.group(3)) >= 2:
                                t = GT.get(pm.group(1).strip())
                                if pm.group(2): s.uf.e(('PE', fam.kind, si, int(pm.group(3))), t if t in 'FBS' else None)
                                elif t in ('F', 'B', 'S', 'I'): s.uf.e(('P', fam.kind, si, int(pm.group(3))), t)
                        rt = mm.group('ret').strip()
                        if s.abi.get(hex(int(m['addr'], 16)), {}).get('fret'): s.uf.e(('R', fam.kind, si), 'F')
                        elif not rt.endswith('*'):
                            t = GT.get(rt)
                            if t in ('F', 'B', 'S', 'I'): s.uf.e(('R', fam.kind, si), t)

    # translation ----------------------------------------------------------------
    def translate(s, fam, slot, addr, body):
        m = SIG.match(body.strip())
        if not m: return None
        n = s.arity(fam, slot)
        bd = s.base_decl.get((fam.kind, slot))
        if bd is not None:
            if n is not None and n != len(bd['types']): return None
            n = len(bd['types'])
            if any('*' in t or '&' in t for t in bd['types']): return None
        if n is None: return None
        sc = this_scale(body) or 4
        stmts = statements(body)
        if stmts is None: return None
        rett = m.group('ret').strip()
        if rett.endswith('*'): return None
        void = rett == 'void' and not s.abi.get(hex(addr), {}).get('fret')
        ptrs = set(); lines = []
        K = fam.kind

        def field(mo):
            if mo.group('t0') is not None: return None
            if mo.group('i') is not None: k = num(mo.group('i')) * sc
            else: k = num(mo.group('k')) * (1 if mo.group('cast') else sc)
            if k == 0: return None
            return s.fname(fam, k)

        def expr(e, unify_with=None):
            e = e.strip(); fail = []; refs = []
            def fs(mo):
                r = field(mo)
                if r is None: fail.append(1); return 'X'
                s.fields_used.add(r[0]); refs.append(('F', r[0])); return r[1]
            e2 = FIELD.sub(fs, e)
            def ps(mo):
                i = int(mo.group(1))
                if i < 2 or i >= 2 + n: fail.append(1); return 'X'
                refs.append(('P', K, slot, i)); return f'a{i}'
            e2 = re.sub(r'\bparam_(\d+)\b', ps, e2)
            e2 = re.sub(r'\((?:uint|int|uchar|char|float|float10|ushort|short|int16_t|int32_t|uint32_t|uint8_t|bool)\)', '', e2)
            if fail or re.search(r'code|fcn\.|0x5[0-9a-f]{5}|\*\s*\(|&|\[|"|_sym', e2): return None
            if not re.fullmatch(r'[\w\s+\-*/.()]+', e2): return None
            if unify_with is not None:
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
                e = expr(st[7:], ('R', K, slot))
                if e is None: return None
                lines.append(f'return {e};'); continue
            vm = VCALL.fullmatch(st)
            if vm:
                c = s.vcall(fam, slot, vm, n, expr)
                if c is None: return None
                lines.append(c + ';'); continue
            if re.fullmatch(r'CGameObject__v13\(.*\)', st):
                lines.append('// (debug log - empty in the shipped build)'); continue
            mm = re.fullmatch(r'(?P<lhs>[^=]+?)\s*=\s*(?P<rhs>[^=].*)', st)
            if not mm: return None
            lhs, rhs = mm.group('lhs').strip(), mm.group('rhs')
            fm = FIELD.fullmatch(lhs)
            if fm:
                r = field(fm)
                if r is None: return None
                s.fields_used.add(r[0])
                e = expr(rhs, ('F', r[0]))
                if e is None: return None
                lines.append(f'{r[1]} = {e};'); continue
            om = OUTP.fullmatch(lhs)
            if om:
                p = int(om.group('p') or om.group('q')); i = int(om.group('i') or 0)
                if p < 2 or p >= 2 + n: return None
                fm2 = FIELD.fullmatch(rhs.strip())
                if not fm2: return None
                r = field(fm2)
                if r is None: return None
                s.fields_used.add(r[0]); ptrs.add(p)
                s.uf.u(('PE', K, slot, p), ('F', r[0]))
                lines.append(f'a{p}[{i}] = {r[1]};'); continue
            return None
        return dict(fam=fam, slot=slot, addr=addr, n=n, void=void, ptrs=ptrs, lines=lines)

    def vcall(s, fam, slot, vm, n, expr):
        tgt = num(vm.group('off')) // 4
        args = [a.strip() for a in re.split(r',(?![^(]*\))', vm.group('args')) if a.strip()]
        if args and args[0] == 'param_1':
            if fam.kind != 'go' or tgt != LOG: return None
            es = []
            for a in args[1:]:
                if a.startswith('"'): es.append(a); continue
                x = expr(a)
                if x is None: return None
                es.append(x)
            return f'Log({", ".join(es)})'
        if fam.kind == 'el' and tgt < fam.first: return None     # OMT method: write by hand
        bd = s.base_decl.get((fam.kind, tgt))
        k = len(bd['types']) if bd else s.arity(fam, tgt)
        if k is None: return None
        if bd and any('*' in t or '&' in t for t in bd['types']): return None
        if len(args) > k: args = args[:k]
        if len(args) < k: return None
        es = []
        for j, a in enumerate(args):
            if a.startswith('"'): return None
            x = expr(a, ('P', fam.kind, tgt, 2 + j))
            if x is None: return None
            es.append(x)
        return f'{s.slotname(fam, tgt)}({", ".join(es)})'

    def sig(s, fam, slot, n, void, ptrs):
        d = s.base_decl.get((fam.kind, slot))
        if d:
            return d['ret'], ', '.join(f'{t} a{2 + j}' for j, t in enumerate(d['types']))
        ps = []
        for i in range(2, 2 + n):
            if i in ptrs: ps.append(f'{s.uf.t(("PE", fam.kind, slot, i))} *a{i}')
            else: ps.append(f'{s.uf.t(("P", fam.kind, slot, i))} a{i}')
        ret = 'void' if void else s.uf.t(('R', fam.kind, slot))
        return ret, ', '.join(ps)


TK = {'float': 'F', 'unsigned char': 'B', 'bool': 'B', 'char': 'B', 'short': 'S', 'int': 'I', 'unsigned int': 'U'}
DECL = re.compile(r'virtual\s+(?P<ret>[\w ]+?)\s*(?P<rp>\*?)\s*(?P<name>\w+)\s*\((?P<ps>[^)]*)\)[^\n]*//\s*(?P<tag>SLOT|slot|ESLOT|eslot)\s+(?P<n>\d+)')


def parse_decls(text):
    out = {}
    for d in DECL.finditer(text):
        ps = [x.strip() for x in d.group('ps').split(',') if x.strip() and x.strip() != 'void']
        types = [re.sub(r'\s*\b\w+$', '', x).strip() if re.search(r'[\w*&]\s+\**\w+$', x) else x for x in ps]
        kind = 'el' if d.group('tag').lower() == 'eslot' else 'go'
        out[(kind, int(d.group('n')))] = dict(ret=(d.group('ret').strip() + (' *' if d.group('rp') else '')),
                                             types=types, name=d.group('name'))
    return out


def main():
    import glob
    ap = argparse.ArgumentParser()
    ap.add_argument('--class', dest='cls', required=True)
    ap.add_argument('--raw', required=True); ap.add_argument('--classes', required=True)
    ap.add_argument('--abi', required=True); ap.add_argument('--src', required=True)
    ap.add_argument('--out', required=True); ap.add_argument('--names')
    ap.add_argument('--base', default='', help='base classes, nearest first (CLocalGameObject,CGameObject)')
    ap.add_argument('--base-size', default='0', help='size of the base part (object start to the first own member)')
    ap.add_argument('--go-offset', default=None, help='offset of the CLocalGameObject subobject (multi-base classes)')
    ap.add_argument('--el-slots', default='43', help='element-vtable slots that belong to OMT')
    a = ap.parse_args()
    chain = [x for x in a.base.split(',') if x]
    base_size = num(a.base_size)
    go_off = num(a.go_offset) if a.go_offset else None
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
            if v.get('name'): names[('go', int(k))] = v['name']
    hand_files = sorted(set(glob.glob(f'{a.src}/**/{a.cls}.cpp', recursive=True) + glob.glob(f'{a.src}/**/{a.cls}.h', recursive=True)
                            + glob.glob(f'{a.src}/**/{a.cls}_*.*', recursive=True)) - set(glob.glob(f'{a.out}/*')))
    hand_slots, hand_fields = set(), []
    for fn in hand_files:
        t = open(fn).read()
        hand_slots |= {('go', int(x)) for x in re.findall(r'//\s*SLOT\s+(\d+)', t)}
        hand_slots |= {('el', int(x)) for x in re.findall(r'//\s*ESLOT\s+(\d+)', t)}
        hand_fields += [(num(o), num(z)) for o, z in re.findall(r'//\s*FIELD\s+(0x[0-9a-f]+|\d+)\s+(0x[0-9a-f]+|\d+)', t)]
    base_decl = {}
    for b in reversed(chain):
        for fn in sorted(set(glob.glob(f'{a.src}/**/{b}.h', recursive=True) + glob.glob(f'{a.src}/**/{b}_*.inc', recursive=True))) + [f'{a.out}/{b}_slots.inc']:
            if os.path.exists(fn) and not fn.startswith(a.out) or fn == f'{a.out}/{b}_slots.inc' and os.path.exists(fn):
                base_decl.update(parse_decls(open(fn).read()))
    for k, d in base_decl.items():
        names.setdefault(k, d['name'])
    g = Gen(a.cls, classes, funcs, abi, names, go_off, num(a.el_slots), base_size)
    g.base_decl = base_decl
    g.collect_evidence(fieldrows)
    hdrtext = ''.join(open(fn).read() for fn in hand_files if not fn.endswith('.cpp'))
    CK = {'float': 'F', 'int': 'I', 'unsigned': 'U', 'uint': 'U', 'uchar': 'B', 'short': 'S', 'ptr': 'P'}
    for o, t in re.findall(r'//\s*TYPE\s+(0x[0-9a-f]+)\s+([^\n]+)', hdrtext):
        t = t.strip(); key = ('F', num(o))
        if t in CK: g.uf.ev[g.uf.f(key)] = {CK[t]}
        else: g.uf.ev[g.uf.f(key)] = {'P'}; g.ftype_override[num(o)] = t
    own_decl = parse_decls(hdrtext)
    for k, d in own_decl.items():
        names.setdefault(k, d['name'])
    for k, d in list(base_decl.items()) + list(own_decl.items()):      # declared types are authoritative
        kind, si = k
        if d['ret'] in TK: g.uf.ev[g.uf.f(('R', kind, si))] = {TK[d['ret']]}
        for j, t in enumerate(d['types']):
            if t in TK: g.uf.ev[g.uf.f(('P', kind, si, 2 + j))] = {TK[t]}
    # base field types (from the bases' generated members) are authoritative too
    for b in chain:
        fn = f'{a.out}/{b}_fields.inc'
        if os.path.exists(fn):
            for t, o in re.findall(r'^\t([\w *]+?)\s+f_([0-9a-f]+);', open(fn).read(), re.M):
                key = ('go', int(o, 16)) if go_off is not None and b in ('CGameObject', 'CLocalGameObject') else int(o, 16)
                rk = {v: k2 for k2, v in CT.items()}.get(t.strip())
                if rk: g.uf.ev[g.uf.f(('F', key))] = {rk}
    # own slots: overrides (differ from the nearest base's vtable) and new slots
    bvts = classes[chain[0]]['vtables'] if chain else []
    own = {}
    for fam in g.fams:
        bm = next((vt['methods'] for vt in bvts if vt['this_offset'] == fam.this
                   and (fam.kind == 'el' or len(vt['methods']) >= 265)), None)
        if fam.kind == 'go' and go_off is not None and bm is None:
            bm = classes['CLocalGameObject']['vtables'][0]['methods']
        for si, m in enumerate(fam.methods):
            if fam.kind == 'el' and si < fam.first: continue     # OMT virtuals: hand-written
            ad = int(m['addr'], 16)
            f = funcs.get(ad)
            if m.get('imported'): continue
            if bm is not None:
                if si < len(bm) and bm[si]['addr'] == m['addr']: continue
            elif not (f and f[0].startswith(a.cls + '__')):
                continue
            if f: own[(fam.kind, si)] = (fam, ad, f[1])
    gen, todo = {}, {}
    for k in sorted(own, key=lambda x: (x[0] != 'go', x[1])):
        fam, ad, body = own[k]
        r = None if k in hand_slots else g.translate(fam, k[1], ad, body)
        if r: gen[k] = r
        else:
            m = SIG.match(body.strip())
            n = g.arity(fam, k[1]) or 0
            void = bool(m) and m.group('ret').strip() == 'void' and not abi.get(hex(ad), {}).get('fret')
            ptrs = set()
            if m:
                for x in m.group('args').split(','):
                    pm = re.fullmatch(r'\s*[\w ]+?\s*\*+\s*param_(\d+)\s*', x)
                    if pm and int(pm.group(1)) >= 2: ptrs.add(int(pm.group(1)))
            todo[k] = dict(fam=fam, slot=k[1], addr=ad, n=n, void=void, ptrs=ptrs)
    os.makedirs(a.out, exist_ok=True)
    claimed = lambda k: any(o <= k < o + z for o, z in hand_fields)
    hdr = '// generated by tools/decomp/gen_class.py from the decompilation of Neutron2.exe - do not edit\n'
    allf = set(x for x in g.fields_used if isinstance(x, int))
    for r in fieldrows:
        fam = next((f for f in g.fams if f.this == r['sub']), None)
        if fam and r['off'] >= 4:
            fn = g.fname(fam, r['off'])
            if fn and isinstance(fn[0], int): allf.add(fn[0])
    allf = {k for k in allf if k >= max(4, base_size) and not (go_off is not None and go_off <= k < go_off + GO_SIZE)}
    with open(f'{a.out}/{a.cls}_fields.inc', 'w') as fh:
        fh.write(hdr)
        SZ = {'float': 4, 'unsigned int': 4, 'int': 4, 'unsigned char': 1, 'short': 2, 'void *': 8}
        end = None
        for k in sorted(allf):
            if claimed(k):
                end = None; continue
            t = g.uf.t(('F', k))
            if k in g.ftype_override: t = g.ftype_override[k]
            if end is not None and k < end:
                fh.write(f'\t// f_{k:03x} overlaps the previous field\n'); continue
            if end is not None and k > end and not any(claimed(x) for x in range(end, k)):
                fh.write(f'\tunsigned char  _gap_{end:03x}[{k - end}];\n')
            fh.write(f'\t{t:<14} f_{k:03x};\n')
            end = k + (4 if '*' in t else SZ[t])
    tagname = lambda k: 'slot' if k[0] == 'go' else 'eslot'
    with open(f'{a.out}/{a.cls}_slots.inc', 'w') as fh:
        fh.write(hdr)
        for k in sorted(set(gen) | set(todo), key=lambda x: (x[0] != 'go', x[1])):
            if k in hand_slots: continue
            r = gen.get(k) or todo[k]
            ret, ps = g.sig(r['fam'], k[1], r['n'], r['void'], r['ptrs'])
            tag = 'gen' if k in gen else 'TODO'
            fh.write(f'\tvirtual {ret} {g.slotname(r["fam"], k[1])}({ps});\t// {tagname(k)} {k[1]} {hex(r["addr"])} {tag}\n')
    with open(f'{a.out}/{a.cls}_gen.cpp', 'w') as fh:
        fh.write(hdr + f'#include "../{a.cls}.h"\n\n')
        for k, r in sorted(gen.items(), key=lambda x: (x[0][0] != 'go', x[0][1])):
            ret, ps = g.sig(r['fam'], k[1], r['n'], r['void'], r['ptrs'])
            fh.write(f'{ret} {a.cls}::{g.slotname(r["fam"], k[1])}({ps})\t// {hex(r["addr"])}\n{{\n')
            for l in r['lines']:
                if l == 'return;' and l is r['lines'][-1]: continue
                fh.write(f'\t{l}\n')
            fh.write('}\n\n')
    pending = sorted((k for k in todo if k not in hand_slots), key=lambda x: (x[0] != 'go', x[1]))
    with open(f'{a.out}/{a.cls}_todo.cpp', 'w') as fh:
        fh.write(hdr + f'// Stubs for slots not reconstructed yet: each logs its original address once.\n#include "../{a.cls}.h"\n#include "../Todo.h"\n\n')
        for k in pending:
            r = todo[k]
            ret, ps = g.sig(r['fam'], k[1], r['n'], r['void'], r['ptrs'])
            fh.write(f'{ret} {a.cls}::{g.slotname(r["fam"], k[1])}({ps})\t{{ NEG_TODO({hex(r["addr"])}, "{a.cls} {tagname(k)} {k[1]}"); ')
            fh.write('}\n' if ret == 'void' else f'return ({ret})0; }}\n')
    json.dump(dict(todo=[f'{hex(todo[k]["addr"])}:{tagname(k)}{k[1]}' for k in pending],
                   generated=[f'{tagname(k)}{k[1]}' for k in gen]),
              open(f'{a.out}/{a.cls}_status.json', 'w'), indent=1)
    print(f'{a.cls}: {len(gen)} generated, {len(hand_slots)} hand-written, {len(pending)} still to write')
    print('to write:', ' '.join(f'{tagname(k)}{k[1]}:{hex(todo[k]["addr"])}' for k in pending))


if __name__ == '__main__':
    main()
