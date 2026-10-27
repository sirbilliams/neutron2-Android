package com.sirbilliams.negatron;

import java.io.*;
import java.util.zip.CRC32;

/** Optional sidecar; a full save CRC prevents stale thumbnails after import/replacement. */
final class SavePreviewInfo {
    long seconds = -1;
    File picture;
    static SavePreviewInfo read(File save) {
        SavePreviewInfo result = new SavePreviewInfo();
        File meta = new File(save.getPath() + ".preview");
        if (!save.isFile() || !meta.isFile() || meta.length() > 256) return result;
        try (BufferedReader r = new BufferedReader(new FileReader(meta))) {
            String line = r.readLine(); if (line == null) return result;
            String[] fields = line.trim().split("\\s+");
            if (fields.length != 4 || !fields[0].equals("1")) return result;
            long expected = Long.parseLong(fields[1]), seconds = Long.parseLong(fields[2]);
            int photo = Integer.parseInt(fields[3]);
            if (expected < 0 || expected > 0xffffffffL || seconds < 0 || seconds > 315360000L || photo < 0 || photo > 1) return result;
            CRC32 crc = new CRC32();
            try (InputStream in = new FileInputStream(save)) {
                byte[] buffer = new byte[8192]; int n;
                while ((n = in.read(buffer)) != -1) crc.update(buffer, 0, n);
            }
            if (crc.getValue() != expected) return result;
            result.seconds = seconds;
            File image = new File(save.getPath() + ".bmp");
            if (photo == 1 && image.isFile() && image.length() <= 512 * 288 * 4 + 1024) result.picture = image;
        } catch (IOException | NumberFormatException e) { /* a missing preview never blocks Continue */ }
        return result;
    }
    static String duration(long seconds) {
        return String.format(java.util.Locale.ROOT, "%d:%02d:%02d", seconds / 3600, (seconds / 60) % 60, seconds % 60);
    }
}
