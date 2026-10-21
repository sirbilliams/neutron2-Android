package com.sirbilliams.negatron;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
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

    /** Whether the game takes NEG_START=continue / load:<slot> yet (the game side of the menu). */
    static final boolean GAME_LOADS = false;
    static final int SLOTS = 10;

    /** A save file's header: the name, the level it was saved in, and when. */
    static final class Slot {
        int number;                 // 1..10, 0 = the autosave
        String name = "", level = "";
        long time;
        boolean present;

        String[] detail() {
            String title = number == 0 ? "AUTOSAVE" : "SLOT " + number;
            if (!present) return new String[] { title, "EMPTY" };
            return new String[] { title, "NAME   " + name, "LEVEL  " + level,
                    "SAVED  " + new SimpleDateFormat("yyyy-MM-dd HH:mm", Locale.ROOT).format(new Date(time)) };
        }
    }

    /** save/autosave.tsk (number 0) or save/JimmyGame<n>.tsk: a length-prefixed name, the level's
     *  four-letter code, its .gam and the file's own name (length-prefixed), then the game state. */
    static Slot readSlot(File saveDir, int number) {
        Slot s = new Slot();
        s.number = number;
        File f = new File(saveDir, number == 0 ? "autosave.tsk" : "JimmyGame" + number + ".tsk");
        if (!f.isFile()) return s;
        try (DataInputStream in = new DataInputStream(new FileInputStream(f))) {
            s.name = pascal(in).trim().replaceAll(" +", " ");
            in.readFully(new byte[4]);              // the level's four-letter code
            s.level = pascal(in);
            if (s.level.toLowerCase(Locale.ROOT).endsWith(".gam")) s.level = s.level.substring(0, s.level.length() - 4);
            s.time = f.lastModified();
            s.present = true;
        } catch (IOException e) {
            s.present = false;
        }
        return s;
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
                ? "Carry on from the autosave: " + autosave.level + "."
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

    static LauncherModel.Page load(List<Slot> slots) {
        LauncherModel.Page p = new LauncherModel.Page("load", "Load game");
        for (Slot s : slots) {
            LauncherModel.Item i = p.add("slot:" + s.number, s.present ? s.number + "  " + s.name : s.number + "  ---",
                    s.present ? "Load this game." : "Empty slot.");
            i.value = s.present ? shortLevel(s.level) : null;
            i.enabled = s.present && GAME_LOADS;
            i.detail = s.detail();
        }
        p.add("back", "Back", "Back to the main menu.");
        return p;
    }

    static List<Slot> slots(File saveDir) {
        List<Slot> out = new ArrayList<>();
        for (int n = 1; n <= SLOTS; n++) out.add(readSlot(saveDir, n));
        return out;
    }

    private static String shortLevel(String level) {
        return level.length() > 9 ? level.substring(0, 9) : level;
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
