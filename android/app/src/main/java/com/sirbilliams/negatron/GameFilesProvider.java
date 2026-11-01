package com.sirbilliams.negatron;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayDeque;

/**
 * Shares the installed game files, read-only, with other sirbilliams apps signed with the same key
 * (Neutron Arena imports from here). Android 11+ forbids reading another app's Android/data, so
 * the files go through this provider. Guarded by a signature-level read permission.
 *
 *   content://<applicationId>.gamefiles/manifest   query: columns path, size of every game file
 *   content://<applicationId>.gamefiles/<path>     openFile(uri, "r") for one file of the game folder
 *
 * Nothing is returned while the game isn't installed (no omt/level1.omt).
 */
public class GameFilesProvider extends ContentProvider {
    @Override public boolean onCreate() { return true; }

    /** The game folder (same candidates as SetupActivity.findReadableData), or null. */
    private File gameDir() {
        if (getContext() == null) return null;
        File base = getContext().getExternalFilesDir(null);
        if (base == null) return null;
        File[] candidates = { new File(base, "game"), base, new File(base, "Program_Executable_Files") };
        for (File dir : candidates) {
            if (new File(new File(dir, "omt"), "level1.omt").isFile()) return dir;
        }
        return null;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        MatrixCursor c = new MatrixCursor(new String[] { "path", "size" });
        File root = gameDir();
        if (root == null) return c;
        try {
            String rootPath = root.getCanonicalPath();
            ArrayDeque<File> todo = new ArrayDeque<>();
            todo.add(root);
            while (!todo.isEmpty()) {
                File[] list = todo.poll().listFiles();
                if (list == null) continue;
                for (File f : list) {
                    // skip symlinks and anything resolving outside the game folder
                    if (!f.getCanonicalPath().startsWith(rootPath + File.separator)) continue;
                    if (f.isDirectory()) todo.add(f);
                    else if (f.isFile()) c.addRow(new Object[] { f.getCanonicalPath().substring(rootPath.length() + 1), f.length() });
                }
            }
        } catch (IOException | SecurityException ignored) { }
        return c;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        if (!"r".equals(mode)) throw new SecurityException("read-only");
        File root = gameDir();
        String rel = uri.getPath();
        if (root == null || rel == null || rel.length() < 2) throw new FileNotFoundException("game not installed");
        try {
            File f = new File(root, rel.substring(1)).getCanonicalFile();
            if (!f.getPath().startsWith(root.getCanonicalPath() + File.separator) || !f.isFile())
                throw new FileNotFoundException(rel);
            return ParcelFileDescriptor.open(f, ParcelFileDescriptor.MODE_READ_ONLY);
        } catch (IOException e) {
            throw new FileNotFoundException(rel);
        }
    }

    @Override public String getType(Uri uri) { return "application/octet-stream"; }
    @Override public Uri insert(Uri uri, ContentValues values) { throw new UnsupportedOperationException("read-only"); }
    @Override public int delete(Uri uri, String s, String[] a) { throw new UnsupportedOperationException("read-only"); }
    @Override public int update(Uri uri, ContentValues v, String s, String[] a) { throw new UnsupportedOperationException("read-only"); }
}
