package com.sirbilliams.negatron;

/**
 * port: JNI bridge to the dual-screen companion snapshot (game/src/port/Companion.cpp).
 * The natives live in libnegatron.so, which SDLActivity loads only for the game (not for the
 * model viewer); every call is guarded so a missing library just means "no data".
 */
final class Companion {
    private static boolean sBroken;

    private Companion() {}

    private static native String nativeSnapshot();
    private static native void nativeSetEnabled(boolean on);
    private static native int[] nativeMap();
    private static native int[] nativeIcon(int id);

    /** The latest snapshot as JSON, or null when the game library is not there. */
    static String snapshot() {
        if (sBroken) return null;
        try {
            return nativeSnapshot();
        } catch (UnsatisfiedLinkError e) {
            sBroken = true;
            return null;
        }
    }

    /** The level map: [gen, w, h, ARGB pixels...], or null. */
    static int[] map() {
        if (sBroken) return null;
        try {
            return nativeMap();
        } catch (UnsatisfiedLinkError e) {
            sBroken = true;
            return null;
        }
    }

    /** An icon the snapshot names: [w, h, ARGB pixels...], or null. */
    static int[] icon(int id) {
        if (sBroken) return null;
        try {
            return nativeIcon(id);
        } catch (UnsatisfiedLinkError e) {
            sBroken = true;
            return null;
        }
    }

    /** Starts / stops building snapshots on the game thread (off by default: no cost). */
    static void setEnabled(boolean on) {
        if (sBroken) return;
        try {
            nativeSetEnabled(on);
        } catch (UnsatisfiedLinkError e) {
            sBroken = true;
        }
    }
}
