package com.sirbilliams.negatron;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

/**
 * port: the launcher's pages (SetupActivity builds them, LauncherView draws them) and the save-slot
 * listing. Plain Java, so tools/companion/render-menu.sh can draw the same pages on the desktop.
 */
final class LauncherPages {
    private LauncherPages() {}

    /** The game takes NEG_START=continue / load:<slot> (game/src/port/StartMode.h). */
    static final boolean GAME_LOADS = true;
    static final int SLOTS = 10;

    /** A saved game: its slot, the level it was saved in, the score and when. */
    static final class Slot {
        int number;                 // 1..10, 0 = the autosave
        String levelName = "", gam = "";
        long score = -1;
        long time;                  // ms
        boolean present;

        /** The level as the player knows it: the game's own name, else the .gam file. */
        String level() {
            if (levelName.length() > 0) return levelName;
            return gam.toLowerCase(Locale.ROOT).endsWith(".gam") ? gam.substring(0, gam.length() - 4) : gam;
        }

        String[] detail() {
            String title = number == 0 ? "AUTOSAVE" : "SLOT " + number;
            if (!present) return new String[] { title, "EMPTY" };
            ArrayList<String> d = new ArrayList<>();
            d.add(title);
            d.add("LEVEL  " + level().toUpperCase(Locale.ROOT));
            if (score >= 0) d.add("SCORE  " + score);
            if (time > 0) d.add("SAVED  " + new SimpleDateFormat("yyyy-MM-dd HH:mm", Locale.ROOT).format(new Date(time)));
            return d.toArray(new String[0]);
        }
    }

    /** The autosave (index 0) and slots 1..10, from save/slots.txt (written by the game: TAB-separated
     *  slot, level id, level name, gam, score, mtime; docs/controls.md), else from the .tsk headers. */
    static Slot[] slots(File saveDir) {
        Slot[] out = new Slot[SLOTS + 1];
        for (int n = 0; n <= SLOTS; n++) {
            out[n] = new Slot();
            out[n].number = n;
        }
        File list = new File(saveDir, "slots.txt");
        if (list.isFile()) {
            try (BufferedReader r = new BufferedReader(new InputStreamReader(new FileInputStream(list), "ISO-8859-1"))) {
                String line;
                while ((line = r.readLine()) != null) {
                    if (line.startsWith("#")) continue;
                    String[] f = line.split("\t", -1);
                    if (f.length < 6) continue;
                    int n = f[0].equalsIgnoreCase("autosave") ? 0 : parseInt(f[0], -1);
                    if (n < 0 || n > SLOTS) continue;
                    Slot s = out[n];
                    s.levelName = f[2].trim();
                    s.gam = f[3].trim();
                    s.score = parseInt(f[4], -1);
                    s.time = parseInt(f[5], 0) * 1000L;
                    s.present = true;
                }
            } catch (IOException e) {
                // as if empty
            }
            return out;
        }
        for (int n = 0; n <= SLOTS; n++) readTsk(saveDir, out[n]);
        return out;
    }

    private static int parseInt(String s, int def) {
        try {
            return Integer.parseInt(s.trim());
        } catch (NumberFormatException e) {
            return def;
        }
    }

    /** The game has not written slots.txt yet: the .tsk header (big-endian: version string,
     *  "DUMMY   " = empty; int level 4cc; gam string; task string; float health; short lives; int score). */
    private static void readTsk(File saveDir, Slot s) {
        File f = new File(saveDir, s.number == 0 ? "autosave.tsk" : "JimmyGame" + s.number + ".tsk");
        if (!f.isFile()) return;
        try (DataInputStream in = new DataInputStream(new FileInputStream(f))) {
            if (pascal(in).trim().equalsIgnoreCase("DUMMY")) return;
            in.readInt();
            s.gam = pascal(in);
            pascal(in);
            in.readFloat();
            in.readShort();
            s.score = in.readInt();
            s.time = f.lastModified();
            s.present = true;
        } catch (IOException e) {
            s.present = false;
        }
    }

    private static String pascal(DataInputStream in) throws IOException {
        int n = in.readUnsignedByte();
        byte[] b = new byte[n];
        in.readFully(b);
        StringBuilder sb = new StringBuilder();
        for (byte x : b) sb.append(x >= 32 && x < 127 ? (char) x : '?');
        return sb.toString();
    }

    static LauncherModel.Page main(Slot autosave, boolean test) {
        LauncherModel.Page p = new LauncherModel.Page("main", "Main menu");
        boolean canContinue = GAME_LOADS && autosave.present;
        LauncherModel.Item i = p.add("continue", "Continue", canContinue
                ? "Carry on from the autosave."
                : GAME_LOADS ? "No autosave yet: start a new game." : "Continue from the autosave: coming with the game's save / load.");
        i.enabled = canContinue;
        if (canContinue) i.detail = autosave.detail();
        p.add("new", "New game", "Start the story from the beginning. The autosave is replaced.");
        i = p.add("load", "Load game", GAME_LOADS ? "Pick one of the game's save slots."
                : "The game's save slots: coming with the game's save / load.");
        i.enabled = GAME_LOADS;
        p.add("settings", "Settings", "Widescreen, the bottom screen, the menu style.");
        if (test) p.add("test", "Test", "Jump to a level, the model viewer, share the log.");
        p.add("quit", "Quit", "Close the game.");
        return p;
    }

    static LauncherModel.Page newGame() {
        LauncherModel.Page p = new LauncherModel.Page("new", "New game");
        p.warning = "A new game wipes the autosave: Continue will start from the beginning.";
        p.add("new_yes", "Start a new game", "Wipe the autosave and start from the beginning.");
        p.add("back", "Cancel", "Back to the main menu.");
        p.sel = 1;                                  // the safe choice first
        return p;
    }

    static LauncherModel.Page load(Slot[] slots) {
        LauncherModel.Page p = new LauncherModel.Page("load", "Load game");
        for (int n = 1; n < slots.length; n++) {
            Slot s = slots[n];
            LauncherModel.Item i = p.add("slot:" + n, n + "  " + (s.present ? s.level() : "---"),
                    s.present ? "Load this game." : "Empty slot.");
            i.value = s.present && s.score >= 0 ? String.valueOf(s.score) : null;
            i.enabled = s.present && GAME_LOADS;
            i.detail = s.detail();
        }
        p.add("back", "Back", "Back to the main menu.");
        return p;
    }

    static LauncherModel.Page settings(boolean wide, boolean panel, boolean secondScreen, boolean original) {
        LauncherModel.Page p = new LauncherModel.Page("settings", "Settings");
        LauncherModel.Item i = p.add("wide", "Widescreen 16:9", "Fill a 16:9 screen; the HUD moves to the edges. Off: the original 4:3.");
        i.value = wide ? "ON" : "OFF";
        i = p.add("panel", "Dual screen", (secondScreen ? "" : "No second screen found. ")
                + "The bottom screen shows the level map, the tasks and the gadgets while you play "
                + "(on by default when there is a second screen). Also in game: hold Back.");
        i.value = panel ? "ON" : "OFF";
        i = p.add("style", "Menu style", "NEW: this menu. ORIGINAL: the game's own title menu, from the next start "
                + "(to come back here: hold Back in the game, then Menu style).");
        i.value = original ? "ORIGINAL" : "NEW";
        i = p.add("hd", "HD image pack", "Sharper pictures for the game: not available yet.");
        i.enabled = false;
        i.value = "LATER";
        p.add("back", "Back", "Back to the main menu.");
        return p;
    }

    static LauncherModel.Page test() {
        LauncherModel.Page p = new LauncherModel.Page("test", "Test");
        p.add("levels", "Jump to a level", "Play one level straight away, without its opening (NEG_TEST_PLAY).");
        p.add("viewer", "Model viewer", "Look at the game's models.");
        p.add("log", "Share log", "Send the last run's log (omt-log.txt) as text.");
        p.add("back", "Back", "Back to the main menu.");
        return p;
    }

    static LauncherModel.Page levels(List<String> gams) {
        LauncherModel.Page p = new LauncherModel.Page("levels", "Jump to a level");
        for (String g : gams) {
            p.add("level:" + g, g, "Play " + g + " from its start.");
            if (g.equalsIgnoreCase("Level5.gam")) p.sel = p.items.size() - 1;
        }
        p.add("back", "Back", "Back to the test page.");
        return p;
    }
}
