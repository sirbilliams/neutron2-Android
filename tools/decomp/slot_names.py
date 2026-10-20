#!/usr/bin/env python3
"""Name virtual slots from the debug strings their implementations log.

The game logs most methods by name ("InitObject()", "+LoadNewLevel()", ...). For every class and
vtable slot, the first "Name(" string in the implementation names the slot. Slots of the
CGameObject vtable family (CGameObject 265 slots, CLocalGameObject 286, ...) are merged across all
classes, so an override in C3DJimmy that logs "Jump()" names the base slot too.

    slot_names.py raw/ classes.json [--family CGameObject] [--json out.json]
"""
import json, re, sys, argparse
from collections import defaultdict, Counter
sys.path.insert(0, __import__('os').path.dirname(__file__))
from fields import load

NAME = re.compile(r'"[+\-]?(?:[A-Za-z0-9_]+::)?([A-Z][A-Za-z0-9_]{2,})\((?:[^"]{0,20})\)?"')


def family_of(cls, vt, classes):
    """Which root class's slots this vtable continues: the base (or the class itself) with the
    smallest vtable whose slot 0..n-1 layout is a prefix by count and owner name."""
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('rawdir'); ap.add_argument('classes_json')
    ap.add_argument('--family', default='CGameObject')
    ap.add_argument('--json')
    a = ap.parse_args()
    classes, funcs = load(a.rawdir, a.classes_json)
    root = classes[a.family]
    nroot = len(root['vtables'][0]['methods'])
    votes = defaultdict(Counter)
    impl = defaultdict(set)
    for cname, c in classes.items():
        if cname != a.family and a.family not in c.get('bases', []):
            continue
        for vt in c['vtables']:
            if len(vt['methods']) < nroot:
                continue
            # the family vtable: the one that holds the root's slot-0 thunk/dtor position. For classes
            # deriving from the root through a secondary base, it's the vtable with >= nroot slots
            # and the root's subobject; for the root and its primary chain it's vtable 0.
            if cname != a.family and c['vtables'].index(vt) == len(c['vtables']) - 1 and cname not in ('CLocalGameObject', 'C2DInGameMenu', 'CTaskList'):
                # last vtable is the element family for multi-base classes
                if len(c['vtables']) > 1:
                    continue
            for si, m in enumerate(vt['methods']):
                ad = int(m['addr'], 16)
                impl[si].add((cname, m['addr']))
                f = funcs.get(ad)
                if not f:
                    continue
                for nm in NAME.findall(f[1])[:1]:
                    votes[si][nm] += 1
    out = {}
    for si in sorted(impl):
        best = votes[si].most_common(3)
        out[si] = dict(name=best[0][0] if best else None, votes=best, nimpl=len({x[1] for x in impl[si]}))
        print(f'{si:3d} 0x{si*4:03x} {str(out[si]["name"]):<28} impls={out[si]["nimpl"]:<3} {best}')
    if a.json:
        json.dump(out, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
