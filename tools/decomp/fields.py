#!/usr/bin/env python3
"""Recover the data members of the game's classes from the raw decompilation.

For every method in raw/<Class>.c (and constructors/destructors found in _free.c by the
vtable they install), collect the `this`-relative accesses:

  * accessors   - `return *(T *)(param_1 + K)` / `*(T *)(param_1 + K) = param_2` slots
  * properties  - RegisterProperty(name, this + K, type) calls (vtable +0x3fc), which give
                  the real field names used by the .gam level files
  * defaults    - constant stores in the constructor

Offsets are byte offsets from the start of the subobject whose vtable the method was found in
(the `this` the method receives). With --class, prints a table for one class; with --json,
writes everything for the reconstruction to use.

    fields.py raw/ classes.json --class CGameObject
"""
import json, re, sys, argparse
from collections import defaultdict

FUNC_HDR = re.compile(r'^/\* ==== (0x[0-9a-f]+)\s+(\S+)\s+size (\d+) ==== \*/$', re.M)
TYPE_SIZE = {'char': 1, 'uchar': 1, 'byte': 1, 'bool': 1, 'undefined': 1, 'int8_t': 1, 'uint8_t': 1,
             'short': 2, 'ushort': 2, 'int16_t': 2, 'uint16_t': 2, 'word': 2,
             'int': 4, 'uint': 4, 'int32_t': 4, 'uint32_t': 4, 'float': 4, 'dword': 4, 'code': 4,
             'longlong': 8, 'double': 8, 'float10': 4}
PROP_TYPES = {1: 'tag', 2: 'bool', 3: 'float', 4: 'int?', 5: 'string?', 6: 'id'}


def split_functions(text):
    hs = list(FUNC_HDR.finditer(text))
    for i, h in enumerate(hs):
        end = hs[i + 1].start() if i + 1 < len(hs) else len(text)
        yield int(h.group(1), 16), h.group(2), text[h.end():end]


def this_scale(body):
    """Scale of `param_1 + n` from the declared type of param_1 (thiscall/fastcall this)."""
    m = re.search(r'\(\s*([A-Za-z_0-9]+)\s*(\**)\s*param_1\s*[,)]', body)
    if not m:
        return None
    base, stars = m.group(1), m.group(2)
    if not stars:
        return 1
    if len(stars) > 1:
        return 4
    return TYPE_SIZE.get(base, 4)


def num(s):
    return int(s, 16) if s.lower().startswith('0x') else int(s)


# *(T *)(param_1 + K)  |  *(T *)((int)param_1 + K)  |  param_1[K]
ACC_CAST = re.compile(r'\*\s*\(\s*(\w+)\s*\*+\s*\)\s*\(\s*(\(int\)\s*)?param_1\s*\+\s*(0x[0-9a-f]+|\d+)\s*\)')
ACC_IDX = re.compile(r'\bparam_1\s*\[\s*(0x[0-9a-f]+|\d+)\s*\]')
PROP = re.compile(r'0x3fc\)\)\s*\(\s*"([^"]+)"\s*,\s*(\(int\)\s*)?param_1\s*(?:\+\s*(0x[0-9a-f]+|\d+))?\s*,\s*(\d+)')


def scan(body, scale):
    acc = []
    for m in ACC_CAST.finditer(body):
        k = num(m.group(3)) * (1 if m.group(2) else scale)
        acc.append((k, m.group(1), m.start()))
    for m in ACC_IDX.finditer(body):
        acc.append((num(m.group(1)) * scale, None, m.start()))
    props = []
    for m in PROP.finditer(body):
        k = num(m.group(3)) if m.group(3) else 0
        k *= 1 if m.group(2) else scale
        props.append((m.group(1), k, int(m.group(4))))
    return acc, props


def classify_accessor(body, scale):
    """Recognise one-line getters/setters."""
    stmts = [s.strip() for s in body.split('{', 1)[-1].rsplit('}', 1)[0].split(';') if s.strip()]
    stmts = [s for s in stmts if not s.startswith('//')]
    if len(stmts) == 1 and stmts[0] == 'return':
        return None
    if len(stmts) == 1 and stmts[0].startswith('return'):
        e = stmts[0][6:].strip()
        a = ACC_CAST.search(e) or None
        if a:
            k = num(a.group(3)) * (1 if a.group(2) else scale)
            return ('get', k, a.group(1))
        m = re.fullmatch(r'\(?(\w+)\)?\s*(-?0x[0-9a-f]+|-?\d+)', e)
        if m:
            return ('const', m.group(2), m.group(1))
        b = re.fullmatch(r'(?:\(\w+\))?\s*param_1\s*\+\s*(0x[0-9a-f]+|\d+)', e)
        if b:
            return ('addr', num(b.group(1)) * scale, None)
    if len(stmts) == 2 and stmts[1] == 'return':
        a = ACC_CAST.search(stmts[0]) or ACC_IDX.search(stmts[0])
        if a and '= param_2' in stmts[0]:
            if a.re is ACC_CAST:
                k = num(a.group(3)) * (1 if a.group(2) else scale); t = a.group(1)
            else:
                k = num(a.group(1)) * scale; t = None
            return ('set', k, t)
    return None


def load(rawdir, classes_json):
    classes = json.load(open(classes_json))['classes']
    classes = {c['name']: c for c in (classes if isinstance(classes, list) else classes.values())}
    funcs = {}
    for fn in ['_free.c', '_extra.c'] + [c + '.c' for c in classes]:
        try:
            text = open(f'{rawdir}/{fn}', encoding='utf-8', errors='replace').read()
        except FileNotFoundError:
            continue
        for a, name, body in split_functions(text):
            funcs[a] = (name, body)
    return classes, funcs


def analyse(cls, classes, funcs):
    c = classes[cls]
    out = defaultdict(lambda: dict(types=set(), names=set(), get=[], set=[], used_by=set(), default=None))
    slots = []
    for vi, vt in enumerate(c['vtables']):
        for si, m in enumerate(vt['methods']):
            a = int(m['addr'], 16)
            f = funcs.get(a)
            if not f:
                slots.append((vi, si, a, None, None)); continue
            name, body = f
            sc = this_scale(body) or 4
            kind = classify_accessor(body, sc)
            slots.append((vi, si, a, name, kind))
            sub = vt['this_offset']
            if kind and kind[0] in ('get', 'set'):
                key = (sub, kind[1])
                out[key][kind[0]].append(f'vt{vi}[{si}]' if vi else f'v{si}')
                if kind[2]: out[key]['types'].add(kind[2])
            acc, props = scan(body, sc)
            for k, t, _ in acc:
                out[(sub, k)]['used_by'].add(name)
                if t: out[(sub, k)]['types'].add(t)
            for pname, k, pt in props:
                out[(sub, k)]['names'].add(f'{pname}:{PROP_TYPES.get(pt, pt)}')
    # constructors / destructors: functions that install this class's own vtable
    want = f'vtable.{cls}.0;'
    for a, (name, body) in funcs.items():
        if name.startswith('fcn.') and ('*param_1 = ' + want in body or '= (int)' + want in body):
            sc = this_scale(body) or 4
            acc, props = scan(body, sc)
            for k, t, _ in acc:
                out[(0, k)]['used_by'].add(name)
                if t: out[(0, k)]['types'].add(t)
    return out, slots


def ctor_defaults(cls, classes, funcs):
    """Constant stores in functions that install this class's first vtable."""
    c = classes[cls]
    want = f'vtable.{cls}.0;'
    d = {}
    for a, (name, body) in funcs.items():
        if want in body and name.startswith('fcn.') and ('*param_1 = ' + want in body or '= (int)' + want in body):
            sc = this_scale(body) or 4
            for m in re.finditer(r'(\*\s*\(\s*\w+\s*\*+\s*\)\s*\(\s*(?:\(int\)\s*)?param_1\s*\+\s*(?:0x[0-9a-f]+|\d+)\s*\)|\bparam_1\s*\[\s*(?:0x[0-9a-f]+|\d+)\s*\])\s*=\s*(-?0x[0-9a-f]+|-?\d+)\s*;', body):
                lhs = m.group(1)
                a1 = ACC_CAST.search(lhs)
                if a1:
                    k = num(a1.group(3)) * (1 if a1.group(2) else sc)
                else:
                    k = num(ACC_IDX.search(lhs).group(1)) * sc
                d.setdefault(k, m.group(2))
            d.setdefault('_ctor', []).append(hex(a))
    return d


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('rawdir'); ap.add_argument('classes_json')
    ap.add_argument('--class', dest='cls'); ap.add_argument('--json')
    ap.add_argument('--slots', action='store_true', help='also list vtable slots')
    a = ap.parse_args()
    classes, funcs = load(a.rawdir, a.classes_json)
    targets = [a.cls] if a.cls else sorted(classes)
    alljson = {}
    for cls in targets:
        out, slots = analyse(cls, classes, funcs)
        defs = ctor_defaults(cls, classes, funcs)
        rows = []
        for (sub, k), v in sorted(out.items()):
            rows.append(dict(sub=sub, off=k, types=sorted(v['types']), names=sorted(v['names']),
                             get=v['get'], set=v['set'], users=len(v['used_by']),
                             default=defs.get(k) if sub == 0 else None))
        alljson[cls] = dict(fields=rows, ctor=defs.get('_ctor', []),
                            slots=[dict(vt=vi, slot=si, addr=hex(ad), name=n, kind=k) for vi, si, ad, n, k in slots])
        if a.cls:
            print(f'{cls}: ctor {defs.get("_ctor")}  subobjects {[ (v["this_offset"], len(v["methods"])) for v in classes[cls]["vtables"]]}')
            for r in rows:
                print(f'  [{r["sub"]:#x}]+{r["off"]:#06x}  {",".join(r["types"]):<14} {" ".join(r["names"]):<34} '
                      f'get={",".join(r["get"])} set={",".join(r["set"])} users={r["users"]} def={r["default"]}')
            if a.slots:
                for s in alljson[cls]['slots']:
                    print(f'  slot vt{s["vt"]}[{s["slot"]}] {s["addr"]} {s["name"]} {s["kind"]}')
    if a.json:
        json.dump(alljson, open(a.json, 'w'), indent=1, default=list)


if __name__ == '__main__':
    main()
