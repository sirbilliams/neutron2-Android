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
        Intent i = new Intent(this, NegatronActivity.class);
        i.putExtra(NegatronActivity.EXTRA_LIB, lib);
        startActivity(i);
        finish();
    }

    /** Game files present: play the reconstructed game (work in progress) or open the model viewer. */
    private void buildChooser() {
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

        TextView note = new TextView(this);
        note.setTextColor(Color.rgb(220, 225, 235));
        note.setTextSize(15);
        note.setGravity(Gravity.CENTER);
        note.setPadding(0, dp(12), 0, dp(20));
        note.setText("The game is being rebuilt from the original; it doesn't play yet.");
        root.addView(note, wrap());

        Button play = new Button(this);
        play.setText("Play (work in progress)");
        play.setTextSize(18);
        play.setOnClickListener(v -> launch("negatron"));
        root.addView(play, wrap());

        Button viewer = new Button(this);
        viewer.setText("Model viewer");
        viewer.setTextSize(18);
        viewer.setOnClickListener(v -> launch("main"));
        LinearLayout.LayoutParams vl = wrap();
        vl.topMargin = dp(12);
        root.addView(viewer, vl);

        setContentView(root);
        play.requestFocus();        // the Thor's A button works without touching the screen
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

    private static void deleteTree(File f) {
        if (f == null || !f.exists()) return;
        File[] kids = f.listFiles();
        if (kids != null) for (File k : kids) deleteTree(k);
        //noinspection ResultOfMethodCallIgnored
        f.delete();
    }
}
