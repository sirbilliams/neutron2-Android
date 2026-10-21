package com.sirbilliams.negatron;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * First screen. If the game files are readable it starts the engine straight away; otherwise it lets
 * the user pick the game's CD image (.iso), which is installed like the CD's setup does, or a zip of
 * the game's Program_Executable_Files folder, and puts the files into files/game, which the app owns. Files copied in over USB can be visible to
 * the app but not readable on some Android builds; files the app writes itself always are.
 */
public class SetupActivity extends Activity {

    private static final int PICK_ZIP = 1;
    private static final int PICK_BOXART = 2;
    private static final String[] GAME_DIRS = { "omt", "gam", "grn", "ase", "png", "dat", "save" };

    private TextView message;
    private Button choose;
    private ProgressBar progress;
    private final Handler ui = new Handler(Looper.getMainLooper());

    // ------------------------------------------------------------------ lifecycle

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        File base = getExternalFilesDir(null);
        if (base != null) base.mkdirs();

        if (findReadableData() != null) {
            buildChooser();
            return;
        }
        buildUi(base);
    }

    /** Directory holding omt/level1.omt that we can actually read, or null. */
    private File findReadableData() {
        File base = getExternalFilesDir(null);
        if (base == null) return null;
        File[] candidates = { new File(base, "game"), base, new File(base, "Program_Executable_Files") };
        for (File dir : candidates) {
            if (canRead(new File(new File(dir, "omt"), "level1.omt"))) return dir;
        }
        return null;
    }

    private static boolean canRead(File f) {
        try (InputStream in = new FileInputStream(f)) {
            return in.read() >= 0;
        } catch (IOException | SecurityException e) {
            return false;
        }
    }

    private void startGame() {
        buildChooser();
    }

    private void launch(String lib) {
        launch(lib, null, null);
    }

    /** Starts the engine: lib "negatron" (the game) or "main" (the model viewer); testPlay = a level
     *  to play straight away (NEG_TEST_PLAY); start = where the game starts (NEG_START: menu, new,
     *  continue, load:<slot>). */
    private void launch(String lib, String testPlay, String start) {
        hideBottom();
        Intent i = new Intent(this, NegatronActivity.class);
        i.putExtra(NegatronActivity.EXTRA_LIB, lib);
        if (testPlay != null) i.putExtra(NegatronActivity.EXTRA_TEST_PLAY, testPlay);
        if (start != null) i.putExtra(NegatronActivity.EXTRA_START, start);
        startActivity(i);
        finish();
    }

    // ------------------------------------------------------------------ port: the launcher menu

    /** port: the menu style, "new" (the launcher, default) or "original" (the game's own title menu). */
    static final String PREF_MENU_ORIGINAL = "menu_original";
    /** Intent extra: show the launcher even when the original menu is chosen. */
    static final String EXTRA_LAUNCHER = "launcher";
    /** The player's box art picture, copied into the app's private files (never shipped in the repo). */
    static final String BOX_ART = "boxart.img";

    private File boxArtFile() { return new File(getFilesDir(), BOX_ART); }

    /** Decodes a picture at most about 2048 pixels on its long side (null if it isn't one). */
    private static android.graphics.Bitmap decodeScaled(File f) {
        if (!f.isFile()) return null;
        android.graphics.BitmapFactory.Options o = new android.graphics.BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        android.graphics.BitmapFactory.decodeFile(f.getPath(), o);
        if (o.outWidth <= 0 || o.outHeight <= 0) return null;
        int sample = 1;
        while (Math.max(o.outWidth, o.outHeight) / sample > 2048) sample *= 2;
        o = new android.graphics.BitmapFactory.Options();
        o.inSampleSize = sample;
        try {
            return android.graphics.BitmapFactory.decodeFile(f.getPath(), o);
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    /** Settings -> Box art: copies the chosen picture into the app's files and shows it. */
    private void setBoxArt(Uri uri) {
        File tmp = new File(getFilesDir(), BOX_ART + ".tmp");
        try (InputStream in = getContentResolver().openInputStream(uri);
             OutputStream out = new FileOutputStream(tmp)) {
            if (in == null) throw new IOException("can't open the picture");
            byte[] buf = new byte[64 * 1024];
            int n;
            while ((n = in.read(buf)) > 0) out.write(buf, 0, n);
        } catch (IOException | SecurityException e) {
            tmp.delete();
            return;
        }
        android.graphics.Bitmap b = decodeScaled(tmp);
        if (b == null) {                                    // not a picture: keep the old one
            tmp.delete();
            return;
        }
        File f = boxArtFile();
        f.delete();
        if (!tmp.renameTo(f)) {
            tmp.delete();
            return;
        }
        if (mMenu != null) {
            mMenu.boxArt = b;
            if (mMenu.page() != null && mMenu.page().id.equals("settings")) mMenu.replace(settingsPage());
            else mMenu.changed();
        }
    }

    /** The game's main menu background, saved by the game on its first run (NEG_MENU_ART). */
    static final String MENU_ART = "launcher-art.png";

    private LauncherModel mMenu;
    private LauncherView mTop;
    private LauncherPresentation mBottom;
    private boolean mTestShown;
    private int mTouchItem = -1;

    private final Runnable mTick = new Runnable() {
        @Override
        public void run() {
            if (mTop == null) return;
            mTop.tick();
            if (mBottom != null && mBottom.view != null) mBottom.view.tick();
            ui.postDelayed(this, 50);
        }
    };

    private final Runnable mRevealTest = new Runnable() {
        @Override
        public void run() { revealTest(); }
    };

    private android.content.SharedPreferences prefs() {
        return getSharedPreferences(CompanionPanel.PREFS, MODE_PRIVATE);
    }

    /** Game files present: the launcher menu, or straight into the game's own title menu. */
    private void buildChooser() {
        boolean launcher = getIntent() != null && getIntent().getBooleanExtra(EXTRA_LAUNCHER, false);
        if (prefs().getBoolean(PREF_MENU_ORIGINAL, false) && !launcher) {
            launch("negatron", null, "menu");
            return;
        }
        mMenu = new LauncherModel();
        mMenu.art = android.graphics.BitmapFactory.decodeFile(new File(getExternalFilesDir(null), MENU_ART).getPath());
        mMenu.boxArt = decodeScaled(boxArtFile());
        mMenu.reset(mainPage());
        mTop = new LauncherView(this, mMenu, false);
        mTop.setFocusable(true);
        mTop.setFocusableInTouchMode(true);
        mTop.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, android.view.MotionEvent e) { return touch(e); }
        });
        setContentView(mTop);
        mTop.requestFocus();
        showBottom();
        ui.removeCallbacks(mTick);
        ui.post(mTick);
    }

    private File saveDir() {
        File data = findReadableData();
        return new File(data != null ? data : getExternalFilesDir(null), "save");
    }

    private LauncherModel.Page mainPage() {
        return LauncherPages.main(LauncherPages.slots(saveDir())[0], mTestShown);
    }

    private LauncherModel.Page settingsPage() {
        android.content.SharedPreferences p = prefs();
        boolean second = CompanionPanel.findDisplay(displays()) != null;
        return LauncherPages.settings(p.getBoolean(PREF_WIDESCREEN, true), CompanionPanel.wanted(p, second), second,
                p.getBoolean(PREF_MENU_ORIGINAL, false), boxArtFile().isFile());
    }

    private android.hardware.display.DisplayManager displays() {
        return (android.hardware.display.DisplayManager) getSystemService(DISPLAY_SERVICE);
    }

    /** The hidden test item (Select / Y on the pad, or a long press on the title). */
    private void revealTest() {
        if (mMenu == null || mTestShown) return;
        mTestShown = true;
        if (mMenu.depth() == 1) mMenu.replace(mainPage());
        else mMenu.changed();
    }

    private void choose() {
        LauncherModel.Item it = mMenu.selected();
        if (it == null || !it.enabled || it.hint) return;
        String id = it.id;
        android.content.SharedPreferences p = prefs();
        if (id.equals("continue")) launch("negatron", null, "continue");
        else if (id.equals("new")) mMenu.push(LauncherPages.newGame());
        else if (id.equals("new_yes")) launch("negatron", null, "new");
        else if (id.equals("load")) mMenu.push(LauncherPages.load(LauncherPages.slots(saveDir())));
        else if (id.startsWith("slot:")) launch("negatron", null, "load:" + id.substring(5));
        else if (id.equals("settings")) mMenu.push(settingsPage());
        else if (id.equals("test")) mMenu.push(LauncherPages.test());
        else if (id.equals("quit")) finish();
        else if (id.equals("wide")) {
            p.edit().putBoolean(PREF_WIDESCREEN, !p.getBoolean(PREF_WIDESCREEN, true)).apply();
            mMenu.replace(settingsPage());
        } else if (id.equals("panel")) {
            boolean second = CompanionPanel.findDisplay(displays()) != null;
            p.edit().putBoolean(CompanionPanel.KEY_ON, !CompanionPanel.wanted(p, second)).apply();
            mMenu.replace(settingsPage());
            showBottom();
        } else if (id.equals("style")) {
            p.edit().putBoolean(PREF_MENU_ORIGINAL, !p.getBoolean(PREF_MENU_ORIGINAL, false)).apply();
            mMenu.replace(settingsPage());
        } else if (id.equals("boxart")) {
            Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            i.addCategory(Intent.CATEGORY_OPENABLE);
            i.setType("image/*");
            try {
                startActivityForResult(i, PICK_BOXART);
            } catch (android.content.ActivityNotFoundException e) {
                // no picker on this device
            }
        } else if (id.equals("boxart_clear")) {
            boxArtFile().delete();
            mMenu.boxArt = null;
            mMenu.replace(settingsPage());
        } else if (id.equals("levels")) mMenu.push(LauncherPages.levels(levelFiles()));
        else if (id.startsWith("level:")) launch("negatron", id.substring(6), null);
        else if (id.equals("viewer")) launch("main");
        else if (id.equals("log")) shareLog();
        else if (id.equals("back")) back();
    }

    private void back() {
        if (!mMenu.pop()) return;
        if (mMenu.depth() == 1) mMenu.replace(mainPage());       // the autosave may have changed
    }

    /** The pad (d-pad / stick, A, B, Select / Y) and the keyboard drive the launcher. */
    @Override
    public boolean dispatchKeyEvent(android.view.KeyEvent e) {
        if (mMenu == null || mTop == null) return super.dispatchKeyEvent(e);
        int k = e.getKeyCode();
        int act;
        switch (k) {
            case android.view.KeyEvent.KEYCODE_DPAD_UP: act = 1; break;
            case android.view.KeyEvent.KEYCODE_DPAD_DOWN: act = 2; break;
            case android.view.KeyEvent.KEYCODE_DPAD_LEFT:
            case android.view.KeyEvent.KEYCODE_DPAD_RIGHT: act = 3; break;
            case android.view.KeyEvent.KEYCODE_DPAD_CENTER:
            case android.view.KeyEvent.KEYCODE_ENTER:
            case android.view.KeyEvent.KEYCODE_NUMPAD_ENTER:
            case android.view.KeyEvent.KEYCODE_BUTTON_A:
            case android.view.KeyEvent.KEYCODE_BUTTON_START: act = 4; break;
            case android.view.KeyEvent.KEYCODE_BUTTON_B:
            case android.view.KeyEvent.KEYCODE_ESCAPE: act = 5; break;
            case android.view.KeyEvent.KEYCODE_BACK: act = 6; break;
            case android.view.KeyEvent.KEYCODE_BUTTON_SELECT:
            case android.view.KeyEvent.KEYCODE_BUTTON_Y: act = 7; break;
            default: return super.dispatchKeyEvent(e);
        }
        if (e.getAction() != android.view.KeyEvent.ACTION_DOWN) return true;    // act on the press only
        boolean repeat = e.getRepeatCount() > 0;
        if (act == 1) mMenu.move(-1);
        else if (act == 2) mMenu.move(1);
        else if (repeat) return true;
        else if (act == 3) {                                    // left / right flip a setting
            LauncherModel.Item it = mMenu.selected();
            if (it != null && (it.id.equals("wide") || it.id.equals("panel") || it.id.equals("style"))) choose();
        } else if (act == 4) choose();
        else if (act == 5) back();
        else if (act == 6) {
            if (mMenu.depth() > 1) back();
            else finish();
        } else revealTest();
        return true;
    }

    /** Touch: tap a button to choose it; a long press on the title shows the test item. */
    private boolean touch(android.view.MotionEvent e) {
        float x = e.getX(), y = e.getY();
        switch (e.getActionMasked()) {
            case android.view.MotionEvent.ACTION_DOWN:
                mTouchItem = mTop.itemAt(x, y);
                if (mTouchItem >= 0) mMenu.select(mTouchItem);
                if (mTop.onHeader(x, y)) ui.postDelayed(mRevealTest, 800);
                return true;
            case android.view.MotionEvent.ACTION_UP:
                ui.removeCallbacks(mRevealTest);
                if (mTouchItem >= 0 && mTop.itemAt(x, y) == mTouchItem && mMenu.select(mTouchItem)) choose();
                mTouchItem = -1;
                return true;
            case android.view.MotionEvent.ACTION_CANCEL:
                ui.removeCallbacks(mRevealTest);
                mTouchItem = -1;
                return true;
            default:
                return true;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mTop != null) {
            showBottom();
            ui.removeCallbacks(mTick);
            ui.post(mTick);
        }
    }

    @Override
    protected void onPause() {
        ui.removeCallbacks(mTick);
        hideBottom();
        super.onPause();
    }

    /** On a dual-screen device (and the dual screen setting on) the bottom screen shows the page's
     *  options and the selected item's details / save-slot preview. */
    private void showBottom() {
        android.view.Display d = CompanionPanel.findDisplay(displays());
        boolean want = mMenu != null && d != null && CompanionPanel.wanted(prefs(), true);
        if (!want) { hideBottom(); return; }
        if (mBottom != null) return;
        LauncherPresentation b = new LauncherPresentation(this, d, mMenu);
        try {
            b.show();
        } catch (android.view.WindowManager.InvalidDisplayException | IllegalStateException ex) {
            return;
        }
        mBottom = b;
    }

    private void hideBottom() {
        if (mBottom == null) return;
        try {
            mBottom.dismiss();
        } catch (RuntimeException ex) {
            // the display went away
        }
        mBottom = null;
    }

    /** The bottom screen's window: one LauncherView, never focused or touched (the pad stays here). */
    static final class LauncherPresentation extends android.app.Presentation {
        LauncherView view;
        private final LauncherModel mModel;

        LauncherPresentation(android.content.Context outer, android.view.Display display, LauncherModel model) {
            super(outer, display);
            mModel = model;
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                    | android.view.WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
            view = new LauncherView(getContext(), mModel, true);
            setContentView(view);
        }
    }

    /** Test: the installed gam/*.gam files, sorted. */
    private List<String> levelFiles() {
        File data = findReadableData();
        File[] gams = data != null ? new File(data, "gam").listFiles() : null;
        List<String> names = new ArrayList<>();
        if (gams != null) {
            for (File f : gams) {
                String n = f.getName();
                if (f.isFile() && n.toLowerCase(Locale.ROOT).endsWith(".gam")) names.add(n);
            }
        }
        Collections.sort(names, String.CASE_INSENSITIVE_ORDER);
        if (names.isEmpty()) names.add("Level5.gam");
        return names;
    }

    /** Sends the last run's omt-log.txt (written by the engine next to the game folder) as text. */
    private void shareLog() {
        File f = new File(getExternalFilesDir(null), "omt-log.txt");
        String text;
        try (FileInputStream in = new FileInputStream(f)) {
            long len = f.length(), max = 200 * 1024;
            if (len > max) in.skip(len - max);              // the end of a long log matters most
            byte[] b = new byte[(int) Math.min(len, max)];
            int n = 0, r;
            while (n < b.length && (r = in.read(b, n, b.length - n)) > 0) n += r;
            text = new String(b, 0, n, "UTF-8");
        } catch (IOException e) {
            text = "No log yet (" + e.getMessage() + "). Run Play first.";
        }
        Intent i = new Intent(Intent.ACTION_SEND);
        i.setType("text/plain");
        i.putExtra(Intent.EXTRA_SUBJECT, "Negatron log");
        i.putExtra(Intent.EXTRA_TEXT, text);
        startActivity(Intent.createChooser(i, "Share log"));
    }

    // ------------------------------------------------------------------ UI

    private void buildUi(File base) {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER);
        root.setBackgroundColor(Color.rgb(20, 24, 40));
        int pad = dp(32);
        root.setPadding(pad, pad, pad, pad);

        TextView title = new TextView(this);
        title.setText("Jimmy Neutron vs. Jimmy Negatron");
        title.setTextColor(Color.rgb(255, 210, 90));
        title.setTextSize(26);
        title.setTypeface(Typeface.DEFAULT_BOLD);
        title.setGravity(Gravity.CENTER);
        root.addView(title, wrap());

        message = new TextView(this);
        message.setTextColor(Color.rgb(220, 225, 235));
        message.setTextSize(17);
        message.setGravity(Gravity.CENTER);
        message.setPadding(0, dp(20), 0, dp(24));
        message.setText("Game files needed.\n\nChoose the game's CD image (Jimmy Neutron vs. Jimmy "
                + "Negatron.iso): the game is installed from it, about 270 MB. A zip of an installed "
                + "game's Program_Executable_Files folder works too.");
        root.addView(message, wrap());

        progress = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        progress.setMax(1000);
        progress.setVisibility(View.GONE);
        LinearLayout.LayoutParams pl = new LinearLayout.LayoutParams(dp(420), ViewGroup.LayoutParams.WRAP_CONTENT);
        pl.bottomMargin = dp(20);
        root.addView(progress, pl);

        choose = new Button(this);
        choose.setText("Choose the game's .iso…");
        choose.setTextSize(18);
        choose.setOnClickListener(v -> pickZip());
        root.addView(choose, wrap());

        setContentView(root);
        choose.requestFocus();      // so the Thor's A button works without touching the screen
    }

    private int dp(int v) { return (int) (v * getResources().getDisplayMetrics().density + 0.5f); }

    private static LinearLayout.LayoutParams wrap() {
        return new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
    }

    private void pickZip() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("*/*");
        i.putExtra(Intent.EXTRA_MIME_TYPES, new String[] {
                "application/x-iso9660-image", "application/x-cd-image", "application/zip",
                "application/x-zip-compressed", "application/octet-stream" });
        startActivityForResult(i, PICK_ZIP);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == PICK_BOXART && resultCode == RESULT_OK && data != null && data.getData() != null) {
            setBoxArt(data.getData());
            return;
        }
        if (requestCode == PICK_ZIP && resultCode == RESULT_OK && data != null && data.getData() != null) {
            importZip(data.getData());
        }
    }

    // ------------------------------------------------------------------ import

    private void importZip(final Uri uri) {
        if (isIso(uri)) { importIso(uri); return; }
        choose.setEnabled(false);
        progress.setVisibility(View.VISIBLE);
        progress.setProgress(0);
        message.setText("Unpacking game files…");
        final long total = querySize(uri);

        new Thread(() -> {
            String error = null;
            File base = getExternalFilesDir(null);
            File tmp = new File(base, "game.import");
            File dest = new File(base, "game");
            try {
                deleteTree(tmp);
                int files = unzip(uri, tmp, total);
                if (!canRead(new File(new File(tmp, "omt"), "level1.omt"))) {
                    error = files == 0 && looksLikeInstaller(uri)
                            ? "That looks like the game's installer. Pick Negatron-GameFiles.zip instead "
                              + "(the installer keeps the files inside data1.cab)."
                            : "That zip doesn't contain the game files (omt/level1.omt is missing).";
                } else {
                    deleteTree(dest);
                    if (!tmp.renameTo(dest)) throw new IOException("could not move files into place");
                }
            } catch (IOException | SecurityException e) {
                error = "Couldn't unpack the zip: " + e.getMessage();
            } finally {
                deleteTree(tmp);
            }
            final String err = error;
            ui.post(() -> {
                if (err == null) {
                    startGame();
                } else {
                    progress.setVisibility(View.GONE);
                    message.setText(err + "\n\nChoose another file to try again.");
                    choose.setEnabled(true);
                    choose.requestFocus();
                }
            });
        }).start();
    }

    // ------------------------------------------------------------------ import from the CD image

    private boolean isIso(Uri uri) {
        try (ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri, "r")) {
            return pfd != null && GameImporter.isIso(pfd.getFd());
        } catch (IOException | RuntimeException | UnsatisfiedLinkError e) {
            return false;
        }
    }

    private void importIso(final Uri uri) {
        choose.setEnabled(false);
        progress.setVisibility(View.VISIBLE);
        progress.setProgress(0);
        message.setText("Installing from the CD image…");

        new Thread(() -> {
            String error;
            File base = getExternalFilesDir(null);
            File tmp = new File(base, "game.import");
            File cab = new File(base, "game.import-cab");
            File dest = new File(base, "game");
            try (ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri, "r")) {
                deleteTree(tmp);
                deleteTree(cab);
                if (pfd == null) throw new IOException("can't open the file");
                final int[] last = { -1 };
                error = GameImporter.importIso(pfd.getFd(), tmp.getPath(), cab.getPath(), (stage, done, total) -> {
                    int p = total > 0 ? (int) (stage * 500 + done * 500 / total) : stage * 500;
                    if (p != last[0]) {
                        last[0] = p;
                        final String what = stage == 0 ? "Copying the installer off the CD image…" : "Unpacking the game files…";
                        ui.post(() -> { progress.setProgress(p); message.setText(what); });
                    }
                    return false;
                });
                if (error == null) {
                    if (!canRead(new File(new File(tmp, "omt"), "level1.omt"))) {
                        error = "The CD image was unpacked but the game files are missing (omt/level1.omt).";
                    } else {
                        deleteTree(dest);
                        if (!tmp.renameTo(dest)) throw new IOException("could not move files into place");
                    }
                }
            } catch (IOException | SecurityException e) {
                error = "Couldn't read the CD image: " + e.getMessage()
                        + "\nIf it is on a cloud drive, copy it to the device first.";
            } finally {
                deleteTree(tmp);
                deleteTree(cab);
            }
            final String err = error;
            ui.post(() -> {
                if (err == null) {
                    startGame();
                } else {
                    progress.setVisibility(View.GONE);
                    message.setText(err + "\n\nChoose another file to try again.");
                    choose.setEnabled(true);
                    choose.requestFocus();
                }
            });
        }).start();
    }

    private long querySize(Uri uri) {
        try (Cursor c = getContentResolver().query(uri, new String[] { OpenableColumns.SIZE }, null, null, null)) {
            if (c != null && c.moveToFirst() && !c.isNull(0)) return c.getLong(0);
        } catch (RuntimeException ignored) { }
        return -1;
    }

    /** Unpacks the game folders from the zip into dir. Returns the number of game files written. */
    private int unzip(Uri uri, File dir, final long total) throws IOException {
        int written = 0;
        final long[] read = { 0 };
        final long[] lastUi = { 0 };
        InputStream raw = getContentResolver().openInputStream(uri);
        if (raw == null) throw new IOException("can't open the file");
        InputStream counting = new FilterInputStream(raw) {
            @Override public int read() throws IOException { int b = super.read(); if (b >= 0) tick(1); return b; }
            @Override public int read(byte[] b, int off, int len) throws IOException {
                int n = super.read(b, off, len); if (n > 0) tick(n); return n;
            }
            private void tick(long n) {
                read[0] += n;
                if (total > 0 && read[0] - lastUi[0] > 512 * 1024) {
                    lastUi[0] = read[0];
                    final int p = (int) Math.min(1000, read[0] * 1000 / total);
                    ui.post(() -> progress.setProgress(p));
                }
            }
        };
        byte[] buf = new byte[256 * 1024];
        try (ZipInputStream zip = new ZipInputStream(counting)) {
            ZipEntry e;
            while ((e = zip.getNextEntry()) != null) {
                String rel = gamePath(e.getName());
                if (rel == null || e.isDirectory()) continue;
                File out = new File(dir, rel);
                if (!out.getCanonicalPath().startsWith(dir.getCanonicalPath() + File.separator)) continue;  // zip-slip
                File parent = out.getParentFile();
                if (parent != null) parent.mkdirs();
                try (OutputStream os = new BufferedOutputStream(new FileOutputStream(out), 256 * 1024)) {
                    int n;
                    while ((n = zip.read(buf)) > 0) os.write(buf, 0, n);
                }
                written++;
            }
        }
        return written;
    }

    /** "Program_Executable_Files/omt/level1.omt" -> "omt/level1.omt"; null for anything else. */
    private static String gamePath(String name) {
        String n = name.replace('\\', '/');
        String low = n.toLowerCase(Locale.ROOT);
        // The game reads its built-in data (text, tables, defaults) from the player's own
        // Neutron2.exe; the source tree has none of it. Other programs and DLLs aren't needed.
        if (low.equals("neutron2.exe") || low.endsWith("/neutron2.exe")) return "Neutron2.exe";
        if (low.endsWith(".exe") || low.endsWith(".dll")) return null;
        String[] parts = n.split("/");
        for (int i = 0; i < parts.length - 1; i++) {
            for (String d : GAME_DIRS) {
                if (parts[i].equalsIgnoreCase(d)) {
                    StringBuilder sb = new StringBuilder(d);           // normalise folder case
                    for (int k = i + 1; k < parts.length; k++) sb.append('/').append(parts[k]);
                    return sb.toString();
                }
            }
        }
        return null;
    }

    private boolean looksLikeInstaller(Uri uri) {
        try (ZipInputStream zip = new ZipInputStream(getContentResolver().openInputStream(uri))) {
            ZipEntry e;
            int checked = 0;
            while ((e = zip.getNextEntry()) != null && checked++ < 200) {
                if (e.getName().toLowerCase(Locale.ROOT).endsWith("data1.cab")) return true;
            }
        } catch (IOException | RuntimeException ignored) { }
        return false;
    }

    /** port: the 16:9 widescreen setting (on by default; NegatronActivity passes it as NEG_WIDESCREEN). */
    static final String PREF_WIDESCREEN = "widescreen";

    private static void deleteTree(File f) {
        if (f == null || !f.exists()) return;
        File[] kids = f.listFiles();
        if (kids != null) for (File k : kids) deleteTree(k);
        //noinspection ResultOfMethodCallIgnored
        f.delete();
    }
}
