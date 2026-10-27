#!/usr/bin/env python3
"""Build a clean 2x resampled HD pack from an original texture export (Pillow).

No AI inference: never uses pixels from a potentially corrupted upscale.
Game artwork belongs in private output archives, never in the source repository.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import subprocess
import tempfile
import zipfile
import zlib

from PIL import Image

PNG = b"\x89PNG\r\n\x1a\n"
ROOTS = {"omt", "png", "grn"}


def textures(archive):
    """Accept the installer's optional single wrapper folder; reject aliases."""
    found = {}
    for entry in archive.infolist():
        if entry.is_dir():
            continue
        name = entry.filename
        parts = PurePosixPath(name).parts
        if "\\" in name or name.startswith("/") or ".." in parts:
            raise ValueError("unsafe archive path: " + name)
        index = 0 if parts and parts[0].lower() in ROOTS else 1
        if len(parts) <= index or parts[index].lower() not in ROOTS:
            continue
        if not name.lower().endswith(".png"):
            continue
        key = "/".join(parts[index:]).lower()
        if key in found:
            raise ValueError("duplicate texture: " + key)
        found[key] = entry
    if not found:
        raise ValueError("archive has no omt/png/grn textures")
    return found


def resize(image, key):
    image = image.convert("RGBA")
    size = (image.width * 2, image.height * 2)
    if key.startswith("omt/alpha/"):
        return image.resize(size, Image.Resampling.NEAREST), "nearest (font atlas)"
    alpha = image.getchannel("A")
    lo, hi = alpha.getextrema()
    if lo == hi:
        # Preserve hidden RGB even for originals whose alpha is entirely zero.
        out = image.convert("RGB").resize(size, Image.Resampling.LANCZOS).convert("RGBA")
        out.putalpha(lo)
        return out, "lanczos"
    out = image.convert("RGBa").resize(size, Image.Resampling.LANCZOS).convert("RGBA")
    # Preserve binary cutouts; avoid introducing semitransparent edges.
    binary = len(alpha.getcolors(257) or []) == 2 and lo == 0 and hi == 255
    if binary:
        out.putalpha(alpha.resize(size, Image.Resampling.NEAREST))
    # Hidden RGB is meaningful for some original opaque-material canvases.
    hidden = image.convert("RGB").resize(size, Image.Resampling.LANCZOS).convert("RGBA")
    hidden.putalpha(out.getchannel("A"))
    mask = out.getchannel("A").point(lambda a: 255 if a == 0 else 0)
    out.paste(hidden, mask=mask)
    return out, "premultiplied lanczos" + ("; nearest binary alpha" if binary else "")


def probe_image(probe, data, image):
    with tempfile.NamedTemporaryFile(suffix=".png") as temp:
        temp.write(data)
        temp.flush()
        result = subprocess.check_output([str(probe), temp.name, "96"], text=True)
    expected = [image.width, image.height, zlib.crc32(image.tobytes())]
    if list(map(int, result.split())) != expected:
        raise ValueError("production PNG decoder differs from Pillow")


def build(source, destination, probe=None):
    if source.resolve() == destination.resolve() or destination.exists():
        raise ValueError("output must be a new file")
    report = {"format": 1, "scale": 2, "ai_enhanced": False,
              "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
              "decoder_verified": bool(probe), "textures": [], "skipped": []}
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Stage beside the destination: failures never leave a seemingly complete pack.
    with tempfile.NamedTemporaryFile(dir=destination.parent, suffix=".zip") as staged:
        with zipfile.ZipFile(source) as original, zipfile.ZipFile(staged.name, "w", zipfile.ZIP_DEFLATED) as output:
            for key, entry in sorted(textures(original).items()):
                data = original.read(entry)
                if not data.startswith(PNG):
                    report["skipped"].append({"path": key, "reason": "not a PNG"})
                    continue
                with Image.open(io.BytesIO(data)) as src:
                    src.load()
                    before = src.convert("RGBA")
                result, method = resize(before, key)
                encoded = io.BytesIO()
                result.save(encoded, format="PNG")
                data = encoded.getvalue()
                with Image.open(io.BytesIO(data)) as check:
                    check.load()
                    if check.mode != "RGBA" or check.size != (before.width * 2, before.height * 2):
                        raise ValueError("invalid output dimensions or mode: " + key)
                    if check.tobytes() != result.tobytes():
                        raise ValueError("PNG write/read mismatch: " + key)
                    if before.getchannel("A").getextrema() == (255, 255) and check.getchannel("A").getextrema() != (255, 255):
                        raise ValueError("opaque source lost alpha: " + key)
                    if probe:
                        probe_image(probe, data, check)
                output.writestr("hd/" + key, data)
                report["textures"].append({"path": key, "size": list(result.size), "method": method,
                                            "rgba_crc32": zlib.crc32(result.tobytes())})
                if len(report["textures"]) % 250 == 0:
                    print("Validated", len(report["textures"]), "textures", flush=True)
            output.writestr("hd/rebuild-report.json", json.dumps(report, indent=2) + "\n")
            output.writestr("hd/README.txt", "Clean 2x resampled texture pack. No AI enhancement.\n"
                            "Install this ZIP through Settings > HD texture pack: install.\n"
                            "Keep HD rendering enabled. This replaces the damaged pack.\n"
                            "Font atlases use nearest-neighbor; other art uses Lanczos.\n"
                            "Original alpha is preserved; details are in rebuild-report.json.\n"
                            "Only use with your own copy of the game.\n")
        with zipfile.ZipFile(staged.name) as check:
            if check.testzip() is not None:
                raise ValueError("output archive CRC failure")
        # Exclusive creation also protects against a destination created mid-build.
        with destination.open("xb") as final, open(staged.name, "rb") as src:
            import shutil
            shutil.copyfileobj(src, final)
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--probe", type=Path, help="compiled tools/test/native/hd_png_probe")
    args = parser.parse_args()
    result = build(args.source, args.destination, args.probe.resolve() if args.probe else None)
    print(json.dumps({"written": len(result["textures"]), "skipped": result["skipped"],
                      "decoder_verified": result["decoder_verified"]}, indent=2))
