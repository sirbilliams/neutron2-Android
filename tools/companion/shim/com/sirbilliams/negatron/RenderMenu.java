package com.sirbilliams.negatron;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import java.io.File;
import java.util.Arrays;
import javax.imageio.ImageIO;

/** Desktop check of the launcher menu (tools/companion/render-menu.sh): draws the real LauncherView
 *  pages (main screen 1920 x 1080 and bottom screen 1240 x 1080) from a game folder's save files
 *  (and the picture the game saves for the launcher, NEG_MENU_ART, if given). */
public final class RenderMenu {
    public static void main(String[] a) throws Exception {
        File game = new File(a[0]), out = new File(a[1]);
        out.mkdirs();
        File save = new File(game, "save");
        LauncherModel m = new LauncherModel();
        if (a.length > 2 && !a[2].equals("-")) {           // the folder the game saves its pictures in
            String[][] pics = { { "new", "museum.png" }, { "load", "loadsave.png" }, { "saves", "menu.png" },
                { "settings", "options.png" }, { "test", "inventory.png" }, { "quit", "credits.png" },
                { "continue", "loading.png" } };
            for (String[] k : pics) {
                File f = new File(a[2], k[1]);
                if (f.isFile()) m.pics.put(k[0], Bitmap.wrap(ImageIO.read(f)));
            }
            m.art = m.pics.get("saves");
        }
        for (LauncherPages.Slot slot : LauncherPages.slots(save)) if (slot.preview != null)
            m.pics.put(slot.number == 0 ? "continue" : "slot:" + slot.number, Bitmap.wrap(ImageIO.read(slot.preview)));
        m.reset(LauncherPages.main(LauncherPages.slots(save)[0], true));
        draw(m, out, "main");
        for (int k = 0; k < m.page().items.size(); k++)          // each option's picture
            if (m.select(k)) draw(m, out, "opt_" + m.page().items.get(k).id);
        m.select(1);
        if (a.length > 3) {                         // a stand-in for the player's box art
            m.boxArt = Bitmap.wrap(ImageIO.read(new File(a[3])));
            draw(m, out, "main_boxart");
            m.boxArt = null;
        }
        m.push(LauncherPages.newGame());
        draw(m, out, "new");
        m.pop();
        if (a.length > 4) m.background = Bitmap.wrap(ImageIO.read(new File(a[4])));   // a stand-in
        m.push(LauncherPages.settings(true, true, true, false, a.length > 4));
        draw(m, out, "settings");
        m.pop();
        m.push(LauncherPages.load(LauncherPages.slots(save)));
        m.page().sel = 2;
        draw(m, out, "load");
        m.pop();
        m.push(LauncherPages.controls(new ControlProfile()));
        draw(m, out, "controls");
        m.pop();
        m.push(LauncherPages.saves());
        draw(m, out, "saves");
        java.util.List<LauncherPages.Incoming> in = new java.util.ArrayList<>();
        for (int n : new int[] { 0, 1, 3, 7 }) {                 // an import of the folder's own saves
            LauncherPages.Incoming f = new LauncherPages.Incoming();
            f.name = LauncherPages.fileName(n);
            f.info = LauncherPages.slots(save)[n];
            f.replaces = f.info.present;
            in.add(f);
        }
        m.push(LauncherPages.importPage(in));
        m.page().sel = 3;
        draw(m, out, "import");
        m.page().sel = 0;
        draw(m, out, "import_cancel");
        m.pop();
        m.pop();
        String[] gams = new File(game, "gam").list();
        Arrays.sort(gams, String.CASE_INSENSITIVE_ORDER);
        m.push(LauncherPages.test());
        m.push(LauncherPages.levels(Arrays.asList(gams)));
        draw(m, out, "levels");
        m.pop();
        m.pop();
        m.push(LauncherPages.assets());
        m.page().sel = 1;
        draw(m, out, "assets");
        m.page().sel = 0;
        m.push(LauncherPages.explore(save, Arrays.asList(gams), "Level3.gam"));
        draw(m, out, "explore");
    }

    static void draw(LauncherModel m, File dir, String name) throws Exception {
        for (int b = 0; b < 2; b++) {
            int w = b == 1 ? 1240 : 1920, h = 1080;
            LauncherView v = new LauncherView(new Context(), m, b == 1);
            v.setTestSize(w, h);
            Bitmap bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
            v.drawForTest(new Canvas(bmp));
            ImageIO.write(bmp.img, "png", new File(dir, name + (b == 1 ? "_bottom" : "_top") + ".png"));
        }
    }
}
