#!/usr/bin/env python3
"""Census of .gam level files: which factory classes each holds (load order, count), and whether the port
has each class (a hand-written New_<Class> in game/src; the generated weak stubs in gen/ are ignored).
Reads the player's own game files; prints only class IDs and names, copies nothing.

usage: python3 tools/decomp/gamcensus.py /home/claude/decomp/factory.json game/src <game>/gam/Level3.gam [...]
It scans bytes for the 4-char IDs, so an ID inside a string can show up: level1d "has" TRIG x2 that
are really text (LoadGame creates no CTrigger there; CTrigger 0x4b8ea0 itself is not written yet).
Counts are halved because each object also stores its ID as a property.
"""
import json, os, re, struct, sys
from collections import Counter, OrderedDict

fac = json.load(open(sys.argv[1]))
src = sys.argv[2]
have = set()
for root, _, files in os.walk(src):
    if '/gen' in root: continue
    for f in files:
        if f.endswith('.cpp'):
            for m in re.finditer(r'\bNew_(C\w+)\s*\(', open(os.path.join(root, f), errors='ignore').read()):
                have.add(m.group(1))
# OMT streams are big-endian on disk? Try both and keep the reading that finds the most IDs.
by_val = {}
for e in fac:
    by_val[int(e['idval'], 16)] = e
for path in sys.argv[3:]:
    data = open(path, 'rb').read()
    best = None
    for fmt in ('>I', '<I'):
        hits = []
        for i in range(0, len(data) - 3):
            v = struct.unpack_from(fmt, data, i)[0]
            if v in by_val:
                hits.append((i, by_val[v]))
        if best is None or len(hits) > len(best[1]):
            best = (fmt, hits)
    fmt, hits = best
    gt = by_val.get(struct.unpack_from(fmt, data, 0)[0])
    num = struct.unpack_from(fmt, data, 4)[0]
    cnt = Counter(e['id'] for _, e in hits[1:])
    order = OrderedDict()
    for _, e in hits[1:]:
        order.setdefault(e['id'], e)
    print(f'== {os.path.basename(path)}: game type {gt["id"] if gt else "?"} ({gt["cls"] if gt else ""}), '
          f'{num} objects, endian {fmt}')
    missing = []
    for k, e in order.items():
        n = cnt[k] // 2 if cnt[k] > 1 else cnt[k]
        ok = e['cls'] in have
        if not ok:
            missing.append(e['cls'])
        print(f'  {k} x{n:<3} {e["cls"]:<28} size {e["size"]:<6} ctor {e["ctor"]}  {"ok" if ok else "MISSING"}')
    print('  missing:', ' '.join(missing) or '-')
