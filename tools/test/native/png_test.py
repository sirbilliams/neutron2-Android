"""Exercise the production C++ PNG decoder with synthetic art-free fixtures."""
import struct, zlib, pathlib, subprocess, sys, tempfile
probe = sys.argv[1]
def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
def png(w, h, depth=8, kind=6, raw=b'\0\x11\x22\x33\x44', extra=b'', interlace=0):
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, depth, kind, 0, 0, interlace))
            + extra + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))
with tempfile.TemporaryDirectory() as root:
    path = pathlib.Path(root) / 'image.png'
    def run(data, expected=None, error=None, budget=32):
        path.write_bytes(data)
        p = subprocess.run([probe, str(path), str(budget)], capture_output=True, text=True)
        if error:
            assert p.returncode == 2 and error in p.stderr, (p.returncode, p.stderr, error)
        else:
            assert p.returncode == 0, p.stderr
            assert tuple(map(int, p.stdout.split())) == expected, p.stdout
    def output(w, h, rgba): return w, h, zlib.crc32(rgba)
    run(png(1, 1), output(1, 1, b'\x11\x22\x33\x44'))
    # Palette, packed grayscale, 16-bit channels, RGB transparency and Adam7.
    run(png(1, 1, 1, 3, b'\0\x80', chunk(b'PLTE', b'\0\0\0\x10\x20\x30') + chunk(b'tRNS', b'\xff\x7f')),
        output(1, 1, b'\x10\x20\x30\x7f'))
    run(png(2, 1, 1, 0, b'\0\x40'), output(2, 1, b'\0\0\0\xff\xff\xff\xff\xff'))
    run(png(1, 1, 16, 6, b'\0\x11\x01\x22\x02\x33\x03\x44\x04'), output(1, 1, b'\x11\x22\x33\x44'))
    run(png(1, 1, 8, 2, b'\0\x11\x22\x33', chunk(b'tRNS', b'\0\x11\0\x22\0\x33')),
        output(1, 1, b'\x11\x22\x33\0'))
    run(png(1, 1, interlace=1), output(1, 1, b'\x11\x22\x33\x44'))
    # Multi-pass 8x8 Adam7 image, independent coordinate pattern.
    rgba = bytes(v for y in range(8) for x in range(8) for v in (x * 20, y * 20, 75, 255))
    passes = [(0,0,8,8),(4,0,8,8),(0,4,4,8),(2,0,4,4),(0,2,2,4),(1,0,2,2),(0,1,1,2)]
    raw = b''
    for ax, ay, dx, dy in passes:
        for y in range(ay, 8, dy):
            raw += b'\0' + b''.join(rgba[(y*8+x)*4:(y*8+x+1)*4] for x in range(ax, 8, dx))
    run(png(8, 8, raw=raw, interlace=1), output(8, 8, rgba))
    # All PNG row filters, with a known two-pixel row and an all-zero previous row.
    source = b'\x11\x22\x33\x44\x55\x66\x77\x88'
    for filt in range(5):
        encoded = bytes((v - (source[i-4] if i >= 4 else 0) * (filt in (1,4))
                         - ((source[i-4] if i >= 4 else 0)//2 if filt == 3 else 0)) % 256
                        for i, v in enumerate(source))
        run(png(2, 1, raw=bytes([filt])+encoded), output(2, 1, source))
    run(png(32768, 32768), error='working-memory budget')
    run(png(1, 1, raw=b'\5\x11\x22\x33\x44'), error='row filter')
    run(png(1, 1, depth=1, kind=6), error='colour type')
    run(png(1, 1, interlace=2), error='PNG method')
    run(png(1, 1)[:-12], error='IEND')
    bad = bytearray(png(1, 1)); bad[29] ^= 1
    run(bad, error='CRC')
    run(png(1, 1, 1, 3, b'\0\x80', chunk(b'PLTE', b'\0\0\0')), error='palette index')
    # A sparse file must fail before its advertised byte count is allocated/read.
    with path.open('wb') as f:
        f.write(png(1,1)); f.truncate(65 * 1024 * 1024)
    p = subprocess.run([probe, str(path), '192'], capture_output=True, text=True)
    assert p.returncode == 2 and 'compressed PNG memory limit' in p.stderr
print('Bounded HD PNG decoder regression checks passed')
