package android.os;
/** Desktop shim (tools/companion). NEG_PANEL_CLOCK fixes the time for repeatable pictures. */
public final class SystemClock {
    public static long uptimeMillis() {
        String t = System.getenv("NEG_PANEL_CLOCK");
        return t != null ? Long.parseLong(t) : System.nanoTime() / 1000000L;
    }
}
