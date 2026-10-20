#!/usr/bin/env python3
"""Convert the game's Granny 1.x .grn files into .grc files for game/granny/granny_compat.cpp.

  grn2grc.py <grn dir> <out dir> [--gxtr file.gxtr] [--wine-dir dir] [files...]

Skeletons, bone names and animation curves are read from the .grn files (grnfile.py).  What granny.dll
builds at load time - decoded textures, render-order vertex arrays, bone-binding inverse transforms,
weight lists, per-state triangle lists - is taken from the DLL itself: grnextract.exe is run once under
Wine over all files (or an existing .gxtr is reused).  Format: docs/granny-replacement.md.
"""
import os, struct, subprocess, sys, shutil, argparse

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import grnfile


# ------------------------------------------------------------------------------ .gxtr reader
class R:
    def __init__(self, b, p=0):
        self.b, self.p = b, p

    def u(self):
        v = struct.unpack_from('<I', self.b, self.p)[0]; self.p += 4; return v

    def raw(self, n):
        v = self.b[self.p:self.p + n]; self.p += n; return v


def read_gxtr(path):
    """-> (models {name: record}, offsets {(anim, model): [(has_bone, 64 bytes)] or None})"""
    b = open(path, 'rb').read()
    r = R(b)
    out = {}
    offsets = {}
    while r.p < len(b):
        tag = r.raw(4)
        if tag == b'GXOF':
            mname = r.raw(r.u()).decode('latin1').lower()
            aname = r.raw(r.u()).decode('latin1').lower()
            if r.u():
                offsets[(aname, mname)] = [(r.u(), r.raw(64)) for _ in range(r.u())]
            else:
                offsets[(aname, mname)] = None
            continue
        assert tag == b'GXTR', 'bad gxtr at %d' % r.p
        r.u()
        name = r.raw(r.u()).decode('latin1')
        rec = dict(ok=r.u())
        if rec['ok']:
            rec['textures'] = []
            for _ in range(r.u()):
                w, h, fmt, bpp, pitch, size = [r.u() for _ in range(6)]
                rec['textures'].append(dict(w=w, h=h, format=fmt, bpp=bpp, pitch=pitch, pixels=r.raw(size)))
            rec['states'] = []
            for _ in range(r.u()):
                st = dict(hdr=[r.u() for _ in range(8)], transform=r.raw(64), mesh=r.u(), index=r.u())
                st['streams'] = [[r.u() for _ in range(5)] for _ in range(6)]
                st['data'] = {}
                for k in (0, 3, 4, 5):
                    s = st['streams'][k]
                    if s[0]:
                        st['data'][k] = r.raw(s[3] * 3 * (2 if k == 5 else 4))
                rec['states'].append(st)
            rec['meshes'] = []
            for _ in range(r.u()):
                m = dict(bindings=[])
                for _ in range(r.u()):
                    bt, B, rawb = r.raw(12), r.raw(36), r.raw(0x7c)
                    m['bindings'].append(dict(bt=bt, B=B, raw=rawb))
                n = m['nrender'] = r.u()
                m['pos'], m['nrm'], m['uv'], m['col'] = r.raw(n * 12), r.raw(n * 12), r.raw(n * 12), r.raw(n * 12)
                m['nsource'] = r.u()
                m['weights'] = r.raw(4 * r.u())
                m['copies'] = r.raw(4 * r.u())
                m['normal_vertex'] = r.raw(4 * r.u())
                m['statelists'] = []
                for _ in range(r.u()):
                    c = r.u()
                    m['statelists'].append(r.raw(2 * c))
                rec['meshes'].append(m)
        out[name.lower()] = rec
    return out, offsets


# Animations are played on the model whose '<name>base.grn' is the longest case-insensitive prefix of the
# animation's name (jimmyfullrungun.grn -> jimmybase.grn), plus a few aliases (decided by name; granny
# binds by bone name).  granny's per-track offsets are extracted for these pairs.
ALIASES = {'nummyflail.grn': 'nummeybase.grn', 'numflyair.grn': 'nummeyflyerbase.grn', 'upperrun.grn': 'jimmybase.grn'}


def group(files):
    """-> {model file: [animation files]}"""
    bases = {f[:-len('base.grn')].lower(): f for f in files if f.lower().endswith('base.grn')}
    groups = {b: [] for b in bases.values()}
    for f in files:
        if f in groups:
            continue
        low = f.lower()
        alias = ALIASES.get(low) or ('nummeybase.grn' if low.startswith('nummy') else None)
        target = next((b for b in groups if b.lower() == alias), None) if alias else None
        if not target:
            best = max((p for p in bases if low.startswith(p)), key=len, default=None)
            target = bases[best] if best else None
        if target:
            groups[target].append(f)
    return groups


def run_extract(grn_dir, files, gxtr, wine_dir):
    """run grnextract.exe under Wine over all files (and the model/animation pairs) -> gxtr"""
    os.makedirs(wine_dir, exist_ok=True)
    exe = os.path.join(wine_dir, 'grnextract.exe')
    if not os.path.exists(exe):
        subprocess.check_call(['i686-w64-mingw32-gcc', '-O2', '-o', exe, os.path.join(HERE, 'grnextract.c')])
    dll = os.path.join(wine_dir, 'granny.dll')
    if not os.path.exists(dll):
        shutil.copy(os.path.join(grn_dir, '..', 'granny.dll'), dll)
    env = dict(os.environ, WINEDEBUG='-all', WINEARCH='win32', WINEPREFIX=os.environ.get('WINEPREFIX', '/root/.wine32'))
    wpath = lambda p: 'Z:' + os.path.abspath(p).replace('/', '\\')
    lst = os.path.join(wine_dir, 'extract-list.txt')
    with open(lst, 'w') as fp:
        for f in files:
            fp.write('model %s\n' % wpath(os.path.join(grn_dir, f)))
    args = ['wine', exe, wpath(gxtr), '-f', wpath(lst)]
    for attempt in range(3):    # (one run out of many has been seen to crash inside Wine; a rerun is fine)
        r = subprocess.run(args, cwd=wine_dir, env=env, capture_output=True, text=True)
        if r.returncode == 0:
            return r.stderr
        print('grnextract failed (attempt %d): %s' % (attempt + 1, r.stderr.strip()[-500:]), file=sys.stderr)
    raise SystemExit('grnextract failed')


# ------------------------------------------------------------------------------ .grc writer
class W:
    def __init__(self):
        self.parts = []

    def u(self, v):
        self.parts.append(struct.pack('<I', v & 0xffffffff))

    def i(self, v):
        self.parts.append(struct.pack('<i', v))

    def d(self, v):
        self.parts.append(struct.pack('<d', v))

    def f(self, vals):
        self.parts.append(struct.pack('<%df' % len(vals), *vals))

    def raw(self, b):
        self.parts.append(bytes(b))

    def s(self, txt):
        b = (txt or '').encode('latin1')
        self.u(len(b)); self.raw(b)

    def data(self):
        return b''.join(self.parts)


def f32bits(vals):
    """floats from the .grn are already float32; repack them bit-exactly"""
    return struct.pack('<%df' % len(vals), *vals)


def convert(path, ex, warn):
    g = grnfile.GrnFile(path)
    tn = g.tool_nodes()
    models = g.models()
    skels = g.skeletons()
    anims = g.animations()
    w = W()
    has_model = bool(ex and ex.get('ok') and models and models[0]['skeleton'])
    has_anim = bool(anims)
    w.raw(b'GRC1'); w.u(1); w.u((1 if has_model else 0) | (2 if has_anim else 0))
    if has_model:
        mo = models[0]
        sk = skels[mo['skeleton'] - 1]
        names = [tn[k - 1] if 0 < k <= len(tn) else '' for k in mo['bone_nodes']]
        w.u(len(sk))
        for i, b in enumerate(sk):
            w.s(names[i] if i < len(names) else '')
            w.i(b['parent'] if i else 0)
            w.raw(b['raw'][4:0x44])
        w.u(len(ex['textures']))
        for t in ex['textures']:
            if not t['pixels']:
                # a material without an image (only in animation files that carry meshes, e.g.
                # seagullmove.grn): granny still queues it, as format 0 with uninitialised width and height
                w.u(0); w.u(0); w.u(t['format']); w.u(t['bpp']); w.u(t['pitch'])
                w.u(0)
                continue
            w.u(t['w']); w.u(t['h']); w.u(t['format']); w.u(t['bpp']); w.u(t['pitch'])
            w.u(len(t['pixels'])); w.raw(t['pixels'])
        meshes = ex['meshes']
        w.u(len(meshes))
        for mi, m in enumerate(meshes):
            fb = mo['meshes'][mi]['bindings'] if mi < len(mo['meshes']) else []
            if len(fb) != len(m['bindings']):
                warn('%s mesh %d: %d bindings in the file, %d in granny' % (path, mi, len(fb), len(m['bindings'])))
            w.u(len(m['bindings']))
            for k, b in enumerate(m['bindings']):
                w.u(fb[k]['bone'] if k < len(fb) else 0)
                w.raw(b['bt']); w.raw(b['B'])
            w.u(m['nrender'])
            for key in ('pos', 'nrm', 'uv', 'col'):
                w.raw(m[key])
            w.u(m['nsource'])
            w.u(len(m['weights']) // 4); w.raw(m['weights'])
            w.u(len(m['copies']) // 4); w.raw(m['copies'])
            w.u(len(m['normal_vertex']) // 4); w.raw(m['normal_vertex'])
            sts = [s for s in ex['states'] if s['mesh'] == mi]
            if len(sts) != len(m['statelists']):
                warn('%s mesh %d: %d states, %d index lists' % (path, mi, len(sts), len(m['statelists'])))
            w.u(len(sts))
            for s in sts:
                cookie = s['hdr'][2]
                w.u(cookie - 1 if cookie else 0xffffffff)
                w.u(s['hdr'][6]); w.u(s['hdr'][7])
                tri = s['data'].get(5, b'')
                if s['index'] < len(m['statelists']) and m['statelists'][s['index']] != tri:
                    warn('%s mesh %d state %d: index list differs from the rendering state' % (path, mi, s['index']))
                w.u(len(tri) // 6); w.raw(tri)
                if len(tri) % 4:
                    w.raw(b'\0\0')
    if has_anim:
        tracks = anims[0]['tracks']
        dur = 0.0
        for t in tracks:
            if t['kind'] == 'raw':
                dur = max(dur, t['keys'][-1][0] if t['keys'] else 0.0)
            elif t['knot_arrays'][0]:
                dur = max(dur, t['knot_arrays'][0][-1])
        w.d(dur)
        w.u(len(tracks))
        for t in tracks:
            w.s(tn[t['node'] - 1] if 0 < t['node'] <= len(tn) else '')
            if t['kind'] == 'raw':
                w.u(0); w.u(len(t['keys']))
                for k in t['keys']:
                    w.raw(f32bits(k))
                continue
            w.u(1)
            for k in t['knot_arrays']:
                w.u(len(k)); w.raw(f32bits(k))
            for c in t['curves']:
                w.u(c['degree']); w.u(c['knots']); w.raw(f32bits(c['cps']))
    return w.data(), has_model, has_anim


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('grn_dir'); ap.add_argument('out_dir')
    ap.add_argument('--gxtr', help='reuse/keep the extracted DLL data here')
    ap.add_argument('--wine-dir', default=os.path.join(os.path.expanduser('~'), '.cache', 'grn2grc-wine'))
    ap.add_argument('files', nargs='*')
    a = ap.parse_args()
    files = a.files or sorted(f for f in os.listdir(a.grn_dir) if f.lower().endswith('.grn'))
    os.makedirs(a.out_dir, exist_ok=True)
    gxtr = a.gxtr or os.path.join(a.out_dir, 'extract.gxtr')
    if not (a.gxtr and os.path.exists(a.gxtr)):
        log = run_extract(a.grn_dir, files, gxtr, a.wine_dir)
        for l in log.splitlines():
            if 'OpenModel failed' not in l:
                print('grnextract:', l)
    ex, _ = read_gxtr(gxtr)
    warnings = []
    nm = na = 0
    for f in files:
        data, hm, ha = convert(os.path.join(a.grn_dir, f), ex.get(f.lower()), warnings.append)
        nm += hm; na += ha
        open(os.path.join(a.out_dir, os.path.splitext(f)[0].lower() + '.grc'), 'wb').write(data)
    for x in warnings:
        print('warning:', x)
    print('%d files: %d with a model, %d with an animation, %d warnings' % (len(files), nm, na, len(warnings)))


if __name__ == '__main__':
    main()
