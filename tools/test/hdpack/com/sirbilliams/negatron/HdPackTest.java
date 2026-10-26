package com.sirbilliams.negatron;

import java.io.*;
import java.nio.file.*;
import java.util.zip.*;

/** Synthetic fixtures only. Exercises the production installer, no game files or Android SDK. */
public final class HdPackTest {
    private static void check(boolean ok, String message) {
        if (!ok) throw new AssertionError(message);
    }
    private static byte[] zip(String... paths) throws IOException {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        try (ZipOutputStream zip = new ZipOutputStream(bytes)) {
            for (String path : paths) {
                zip.putNextEntry(new ZipEntry(path));
                zip.write(new byte[] {1, 2, 3}); // Import copies bytes; native decoder validates PNGs.
                zip.closeEntry();
            }
        }
        return bytes.toByteArray();
    }
    private static HdPack.Result install(File data, byte[] bytes) {
        return HdPack.install(new ByteArrayInputStream(bytes), data, -1, p -> {});
    }
    private static void oldPack(File data) throws IOException {
        Path path = data.toPath().resolve("hd/png/old.png");
        Files.createDirectories(path.getParent());
        Files.write(path, new byte[] {42});
    }
    private static void oldIntact(File data) throws IOException {
        check(Files.readAllBytes(data.toPath().resolve("hd/png/old.png"))[0] == 42, "old pack lost");
        check(!new File(data, "hd.import").exists(), "staging folder leaked");
    }
    public static void main(String[] args) throws Exception {
        File data = Files.createTempDirectory("hdpack-test-").toFile();
        try {
            check(HdPack.packPath("/") == null, "slash path crashes");
            check(HdPack.packPath("") == null, "empty path");
            check(HdPack.packPath("wrap/omt/../123.png") == null, "parent segment accepted");
            check(HdPack.packPath("/png/x.png") == null, "absolute path accepted");
            check(HdPack.packPath("png//x.png") == null, "empty segment accepted");
            check("omt/screens/12.png".equals(HdPack.packPath("Pack\\OMT\\Screens\\12.PNG")), "wrapped Windows path");
            check("grn/Jimmy_0.png".equals(HdPack.packPath("grn/Jimmy_0.PNG")), "extension normalization");

            oldPack(data);
            check(install(data, zip("README.txt")).error != null, "empty pack accepted");
            oldIntact(data);
            check(install(data, zip("png/New.png", "wrap/png/new.PNG")).error != null, "case collision accepted");
            oldIntact(data);
            byte[] broken = zip("png/new.png");
            broken[30 + "png/new.png".length()] ^= 0x7f; // corrupt compressed payload / CRC
            check(install(data, broken).error != null, "corrupt archive accepted");
            oldIntact(data);

            File noSpace = new File(data.getPath()) {
                @Override public long getUsableSpace() { return 0; }
            };
            check(install(noSpace, zip("png/new.png")).error != null, "expanded-space check missing");
            oldIntact(data);

            File stage = new File(data, "hd.import");
            check(stage.mkdir(), "stage creation");
            File failedRename = new File(stage.getPath()) {
                @Override public boolean renameTo(File target) { return false; }
            };
            try {
                HdPack.publish(failedRename, new File(data, "hd"), new File(data, "hd.previous"));
                throw new AssertionError("swap failure accepted");
            } catch (IOException expected) { /* old pack must have been restored */ }
            HdPack.deleteTree(stage);
            oldIntact(data);

            check(new File(data, "hd").renameTo(new File(data, "hd.previous")), "simulate interrupted swap");
            check(HdPack.count(data) == 1, "interrupted swap recovery");
            oldIntact(data);

            HdPack.Result result = install(data, zip("wrap/omt/ScreenS/123.PNG", "png/New.PNG", "grn/Jimmy_0.PNG", "manifest.tsv"));
            check(result.error == null, "valid pack: " + result.error);
            check(result.omt == 1 && result.png == 1 && result.grn == 1 && result.skipped == 1, "counts");
            check(HdPack.count(data) == 3, "installed count");
            check(!new File(data, "hd/png/old.png").exists(), "replacement retained stale picture");
            check(!new File(data, "hd.previous").exists(), "successful swap retained backup");
            check(HdPack.remove(data) && HdPack.count(data) == 0, "remove");
            System.out.println("HD pack regression checks passed");
        } finally { HdPack.deleteTree(data); }
    }
}
