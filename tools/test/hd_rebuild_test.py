#!/usr/bin/env python3
"""Regression checks for the private-artwork HD pack rebuild tool (Pillow)."""
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

from PIL import Image

spec = importlib.util.spec_from_file_location("rebuild_pack", Path(__file__).parents[1] / "hd/rebuild_pack.py")
hd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hd)


class RebuildTest(unittest.TestCase):
    def test_opaque_and_hidden_rgb(self):
        for alpha in (0, 255):
            out, _ = hd.resize(Image.new("RGBA", (4, 8), (20, 80, 130, alpha)), "omt/level3/1.png")
            self.assertEqual(out.size, (8, 16))
            self.assertEqual(out.getcolors(), [(128, (20, 80, 130, alpha))])

    def test_font_and_cutout(self):
        image = Image.new("RGBA", (8, 8), (255, 255, 255, 0))
        image.paste((30, 100, 200, 255), (2, 2, 6, 6))
        out, _ = hd.resize(image, "omt/alpha/1.png")
        self.assertEqual(out.tobytes(), image.resize((16, 16), Image.Resampling.NEAREST).tobytes())
        out, _ = hd.resize(image, "grn/character_0.png")
        self.assertEqual({color for count, color in out.getchannel("A").getcolors()}, {0, 255})
        self.assertTrue(all(abs(a - b) <= 1 for a, b in zip(out.getpixel((8, 8)), (30, 100, 200, 255))))

    def test_soft_alpha(self):
        out, _ = hd.resize(Image.new("RGBA", (4, 4), (70, 120, 200, 128)), "png/effect.png")
        self.assertEqual(out.getchannel("A").getextrema(), (128, 128))

    def test_archive_and_skip(self):
        with tempfile.TemporaryDirectory() as tmp:
            src, dst = Path(tmp) / "source.zip", Path(tmp) / "hd.zip"
            png = io.BytesIO()
            Image.new("RGB", (4, 4), (70, 120, 200)).save(png, format="PNG")
            with zipfile.ZipFile(src, "w") as archive:
                archive.writestr("tex/omt/level3/1.png", png.getvalue())
                archive.writestr("tex/png/Vulture.png", b"Paint Shop Pro")
            report = hd.build(src, dst)
            self.assertEqual(len(report["textures"]), 1)
            self.assertEqual(report["skipped"], [{"path": "png/vulture.png", "reason": "not a PNG"}])
            with zipfile.ZipFile(dst) as archive:
                self.assertIsNone(archive.testzip())
                self.assertIn("hd/omt/level3/1.png", archive.namelist())
                self.assertFalse(json.loads(archive.read("hd/rebuild-report.json"))["ai_enhanced"])
            with self.assertRaises(ValueError):
                hd.build(src, dst)

    def test_unsafe_and_duplicate_paths(self):
        for names in (("../omt/level3/1.png",), ("omt/level3/1.png", "omt/Level3/1.PNG")):
            with zipfile.ZipFile(io.BytesIO(), "w") as archive:
                for name in names:
                    archive.writestr(name, b"placeholder")
                with self.assertRaises(ValueError):
                    hd.textures(archive)


if __name__ == "__main__":
    unittest.main()
