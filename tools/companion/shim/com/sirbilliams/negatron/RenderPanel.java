package com.sirbilliams.negatron;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import java.io.File;
import java.nio.file.Files;
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;

/** Desktop check of the companion panel (tools/companion/render.sh): draws the real
 *  CompanionView with a dump from NEG_COMPANION_DUMP (snapshot JSON, map PNG, icon PNGs). */
public final class RenderPanel {
    public static void main(String[] a) throws Exception {
        if (a.length < 4) {
            System.err.println("usage: RenderPanel <snap.json> <map.png> <dump dir> <out.png> [far] [expanded]");
            System.exit(2);
        }
        final String json = new String(Files.readAllBytes(new File(a[0]).toPath()), "UTF-8");
        final File mapFile = new File(a[1]);
        final File dir = new File(a[2]);
        final int gen = new org.json.JSONObject(json).optJSONObject("map").optInt("gen", 0);
        CompanionView v = new CompanionView(new Context());
        v.setSource(new CompanionView.Source() {
            @Override public int[] map() { return load(mapFile, gen, true); }
            @Override public int[] icon(int id) { return load(new File(dir, "icon_" + id + ".png"), 0, false); }
        });
        v.setZoomFar(java.util.Arrays.asList(a).contains("far"));
        v.setSnapshot(json);
        v.setTestSize(1240, 1080);
        if (java.util.Arrays.asList(a).contains("expanded")) {
            java.lang.reflect.Method m = CompanionView.class.getDeclaredMethod("explore", boolean.class);
            m.setAccessible(true); m.invoke(v, true);
        }
        Bitmap out = Bitmap.createBitmap(1240, 1080, Bitmap.Config.ARGB_8888);
        v.drawForTest(new Canvas(out));
        ImageIO.write(out.img, "png", new File(a[3]));
    }

    static int[] load(File f, int gen, boolean map) {
        try {
            BufferedImage i = ImageIO.read(f);
            if (i == null) return null;
            int w = i.getWidth(), h = i.getHeight(), head = map ? 3 : 2;
            int[] r = new int[head + w * h];
            if (map) { r[0] = gen; r[1] = w; r[2] = h; } else { r[0] = w; r[1] = h; }
            i.getRGB(0, 0, w, h, r, head, w);
            return r;
        } catch (Exception e) {
            return null;
        }
    }
}
