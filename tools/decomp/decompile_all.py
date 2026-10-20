#!/usr/bin/env python3
"""Raw-decompile every function of Neutron2.exe with r2ghidra, named from the RTTI class map.

  python3 decompile_all.py Neutron2.exe classes.json outdir

Writes outdir/<Class>.c (methods found in that class's vtables, first owner wins) and
outdir/_free.c (everything else), plus outdir/functions.json (address -> name, class, size).
"""
import json, os, sys, re
import r2pipe

exe, classes_json, outdir = sys.argv[1:4]
os.makedirs(outdir, exist_ok=True)
classes = json.load(open(classes_json))['classes']

# Owner of each virtual method: the class with the fewest bases whose vtable holds it.
owner = {}
for c in sorted(classes, key=lambda c: len(c['bases'])):
    for vi, vt in enumerate(c['vtables']):
        for si, m in enumerate(vt['methods']):
            a = int(m['addr'], 16)
            if m['imported'] or a in owner: continue
            owner[a] = (c['name'], f"vt{vi}_{si}" if vi else f"v{si}")

r2 = r2pipe.open(exe, flags=['-2', '-e', 'scr.color=0', '-e', 'bin.demangle=true'])
r2.cmd('e r2ghidra.indent=4; e r2ghidra.casts=true; aaa')
for a, (cls, slot) in owner.items():
    r2.cmd(f'af @ {a}; afn {cls}__{slot} @ {a}')
funcs = r2.cmdj('aflj') or []
print(f'{len(funcs)} functions', flush=True)

files = {}
index = {}
for i, f in enumerate(sorted(funcs, key=lambda f: f['addr'])):
    a = f['addr']
    cls = owner.get(a, (None, None))[0]
    name = f['name']
    try:
        code = r2.cmd(f'pdg @ {a}')
    except Exception as e:
        code = f'/* decompile failed: {e} */\n'
    key = re.sub(r'[^A-Za-z0-9_]', '_', cls) if cls else '_free'
    fh = files.get(key)
    if fh is None:
        fh = files[key] = open(os.path.join(outdir, key + '.c'), 'w')
    fh.write(f'\n/* ==== {hex(a)}  {name}  size {f.get("size", 0)} ==== */\n{code}\n')
    index[hex(a)] = dict(name=name, cls=cls, size=f.get('size', 0))
    if i % 100 == 0:
        print(f'{i}/{len(funcs)}', flush=True)
for fh in files.values(): fh.close()
json.dump(index, open(os.path.join(outdir, 'functions.json'), 'w'), indent=0)
print('done', flush=True)
