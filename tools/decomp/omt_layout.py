#!/usr/bin/env python3
"""Reconstruct the 32-bit MSVC 6 memory layout of the OMT engine classes, so raw offsets in the
decompiled game ([obj+0x58]) can be translated back to member names.

clang provides field order and types (AST JSON of the real OMT headers); offsets are recomputed with
MSVC x86 rules and Visual C++ 6 STL container sizes (string/vector 16, list 12, map/set 16).

  python3 omt_layout.py <OMTClasses dir> layout.json            # dump
  python3 omt_layout.py --lookup layout.json OMediaElement 0x58  # which member is at +0x58
"""
import json, os, re, subprocess, sys, tempfile

VC6_STL = {  # name prefix -> (size, align)
    'std::basic_string': (16, 4), 'std::__cxx11::basic_string': (16, 4), 'basic_string': (16, 4),
    'std::vector': (16, 4), 'vector': (16, 4),
    'std::list': (12, 4), 'std::__cxx11::list': (12, 4), 'list': (12, 4),
    'std::map': (16, 4), 'std::multimap': (16, 4), 'std::set': (16, 4), 'std::multiset': (16, 4),
    'map': (16, 4), 'multimap': (16, 4), 'set': (16, 4), 'multiset': (16, 4),
    'std::_List_iterator': (4, 4), 'std::_Rb_tree_iterator': (4, 4), 'std::_Rb_tree_const_iterator': (4, 4),
    'std::_List_const_iterator': (4, 4), '__gnu_cxx::__normal_iterator': (4, 4),
}
BUILTIN = {'char': 1, 'signed char': 1, 'unsigned char': 1, 'bool': 1, 'short': 2, 'unsigned short': 2,
           'int': 4, 'unsigned int': 4, 'long': 4, 'unsigned long': 4, 'float': 4, 'double': 8,
           'long long': 8, 'unsigned long long': 8, 'wchar_t': 2, 'long double': 8}


def dump_ast(omt_dir):
    hdrs = []
    for root, _, fs in os.walk(omt_dir):
        if '/OS/MSWindows' in root or '/OS/MacOS' in root:
            continue
        hdrs += [os.path.join(root, f) for f in fs if f.endswith('.h')]
    incs = sorted({os.path.dirname(h) for h in hdrs})
    with tempfile.NamedTemporaryFile('w', suffix='.cpp', delete=False) as t:
        t.write('#define private public\n#define protected public\n')
        for h in sorted(hdrs):
            t.write(f'#include "{h}"\n')
        tu = t.name
    cmd = ['clang++', '-m32', '-std=gnu++98', '-fsyntax-only', '-w', '-fpermissive', '-D__linux__',
           '-Xclang', '-ast-dump=json', tu] + [f'-I{d}' for d in incs]
    gl4es = os.path.join(omt_dir, '..', '..', 'android', 'app', 'jni', 'gl4es', 'include')
    if os.path.isdir(gl4es):
        cmd.append('-I' + gl4es)
    out = subprocess.run(cmd, capture_output=True, text=True)
    if not out.stdout:
        sys.exit(out.stderr[:4000])
    return json.loads(out.stdout), omt_dir


class Layout:
    def __init__(self, ast, root):
        self.records = {}      # name -> record decl (definitions in OMT headers)
        self.typedefs = {}
        self.enums = set()
        self.root = os.path.abspath(root)
        self._file = ''
        self._walk(ast, '')
        self.cache = {}

    def _walk(self, node, scope):
        for n in node.get('inner', []) or []:
            # clang's JSON only names the file when it changes: carry the last one forward
            f = n.get('loc', {}).get('file') or n.get('range', {}).get('begin', {}).get('file')
            if f: self._file = f
            n['_file'] = self._file
            k = n.get('kind')
            if k == 'NamespaceDecl':
                self._walk(n, scope + n.get('name', '') + '::')
            elif k == 'CXXRecordDecl' and n.get('completeDefinition') and n.get('name'):
                full = n['name']
                if full not in self.records or self._in_omt(n):
                    self.records[full] = n
            elif k == 'TypedefDecl':
                self.typedefs[n['name']] = n['type'].get('desugaredQualType', n['type']['qualType'])
            elif k == 'EnumDecl' and n.get('name'):
                self.enums.add(n['name'])

    def _in_omt(self, n):
        f = n.get('_file', '')
        return bool(f) and os.path.abspath(f).startswith(self.root)

    def type_info(self, t):
        t = t.strip()
        t = re.sub(r'\b(const|volatile|struct|class|enum|union)\b', '', t).strip()
        if t.endswith('*') or t.endswith('&') or '(*)' in t:
            return 4, 4
        m = re.match(r'^(.*)\[(\d+)\]$', t)
        if m:
            s, a = self.type_info(m.group(1)); return s * int(m.group(2)), a
        for k, v in VC6_STL.items():
            if t.startswith(k + '<') or t == k:
                return v
        if t in BUILTIN:
            s = BUILTIN[t]; return s, s
        if t in self.enums:
            return 4, 4
        if t in self.typedefs and self.typedefs[t] != t:
            return self.type_info(self.typedefs[t])
        base = t.split('<')[0].split('::')[-1]
        if base in self.records:
            l = self.layout(base); return l['size'], l['align']
        if t in ('omt_RGBAPixel',):
            return 4, 4
        raise KeyError(t)

    def polymorphic(self, name):
        r = self.records.get(name)
        if not r: return False
        for n in r.get('inner', []) or []:
            if n.get('kind') in ('CXXMethodDecl', 'CXXDestructorDecl') and n.get('virtual'):
                return True
        return any(self.polymorphic(self._bname(b)) for b in r.get('bases', []))

    def _bname(self, b):
        return b['type']['qualType'].split('<')[0].split('::')[-1]

    def layout(self, name):
        if name in self.cache: return self.cache[name]
        r = self.records[name]
        off, align = 0, 1
        fields, bases = [], []
        blist = [self._bname(b) for b in r.get('bases', [])]
        poly_bases = [b for b in blist if self.polymorphic(b)]
        # MSVC: polymorphic bases first (the first one provides the vfptr), then the rest
        order = poly_bases + [b for b in blist if b not in poly_bases]
        if self.polymorphic(name) and not poly_bases:
            fields.append(dict(name='__vfptr', offset=0, size=4, type='void**')); off, align = 4, 4
        for b in order:
            if b not in self.records:
                continue
            bl = self.layout(b)
            if bl['size'] == 1 and not bl['fields']:      # empty base
                continue
            off = (off + bl['align'] - 1) // bl['align'] * bl['align']
            bases.append(dict(name=b, offset=off)); off += bl['size']; align = max(align, bl['align'])
        for n in r.get('inner', []) or []:
            if n.get('kind') != 'FieldDecl': continue
            t = n['type'].get('desugaredQualType', n['type']['qualType'])
            try:
                s, a = self.type_info(t)
            except KeyError:
                s, a = self.type_info(n['type']['qualType'])
            off = (off + a - 1) // a * a
            fields.append(dict(name=n['name'], offset=off, size=s, type=n['type']['qualType'])); off += s
            align = max(align, a)
        size = max(1, (off + align - 1) // align * align)
        res = dict(size=size, align=align, bases=bases, fields=fields)
        self.cache[name] = res
        return res

    def flat(self, name, base_off=0, path=''):
        l = self.layout(name)
        out = []
        for b in l['bases']:
            out += self.flat(b['name'], base_off + b['offset'], path + b['name'] + '::')
        for f in l['fields']:
            out.append(dict(offset=base_off + f['offset'], size=f['size'], name=path + f['name'], type=f['type']))
        return sorted(out, key=lambda x: x['offset'])


if __name__ == '__main__':
    if sys.argv[1] == '--lookup':
        d = json.load(open(sys.argv[2])); cls = d[sys.argv[3]]
        want = int(sys.argv[4], 0)
        for f in cls['flat']:
            if f['offset'] <= want < f['offset'] + max(f['size'], 1):
                print(f"{sys.argv[3]}+{hex(want)} = {f['name']} ({f['type']}, at +{hex(f['offset'])})")
        sys.exit(0)
    ast, root = dump_ast(sys.argv[1])
    L = Layout(ast, root)
    out = {}
    bad = []
    for name, r in L.records.items():
        if not name.startswith('OMedia') or not L._in_omt(r):
            continue
        try:
            l = L.layout(name)
            out[name] = dict(size=l['size'], bases=l['bases'], flat=L.flat(name))
        except KeyError as e:
            bad.append((name, str(e)))
    json.dump(out, open(sys.argv[2], 'w'), indent=0)
    print(f'{len(out)} OMT classes laid out; {len(bad)} skipped')
    for b in bad[:15]: print('  skipped', b)
