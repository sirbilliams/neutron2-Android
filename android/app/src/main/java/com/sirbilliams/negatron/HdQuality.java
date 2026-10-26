package com.sirbilliams.negatron;

/** port: startup-only HD limits. Texture estimates exclude the original game and driver overhead. */
final class HdQuality {
    private HdQuality() {}
    static int valid(int preset) { return Math.max(0, Math.min(2, preset)); }
    static String name(int preset) { return new String[] { "LOW", "BALANCED", "HIGH" }[valid(preset)]; }
    static int textureMb(int preset) { return new int[] { 256, 512, 1024 }[valid(preset)]; }
    static int decodeMb(int preset) { return new int[] { 96, 192, 256 }[valid(preset)]; }
    static int scale(int preset) { return new int[] { 2, 4, 8 }[valid(preset)]; }
}
