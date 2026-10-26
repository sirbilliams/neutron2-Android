package com.sirbilliams.negatron;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Point;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.Display;
import android.view.InputDevice;
import android.widget.ScrollView;
import android.widget.TextView;
import java.io.*;
import java.util.Date;

/** port: live in-game diagnostics or explicitly dated last-run samples in the launcher. */
final class Diagnostics {
    static final String FILE = "diagnostics.txt";
    private Diagnostics() {}

    static String tail(File file, int limit) {
        if (file == null) return "Unavailable\n";
        try (RandomAccessFile in = new RandomAccessFile(file, "r")) {
            long length = in.length();
            in.seek(Math.max(0, length - limit));
            byte[] bytes = new byte[(int)Math.min(length, limit)];
            in.readFully(bytes);
            return new String(bytes, "UTF-8") + "\n";
        } catch (IOException | SecurityException ex) { return "No sample yet. Run the game first.\n"; }
    }

    static String report(Activity activity, boolean logs) {
        StringBuilder out = new StringBuilder("Neutron diagnostics\n");
        try {
            out.append("App: ").append(activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0).versionName).append('\n');
        } catch (android.content.pm.PackageManager.NameNotFoundException ignored) { }
        out.append("Device: ").append(Build.MANUFACTURER).append(' ').append(Build.MODEL)
                .append("\nAndroid: ").append(Build.VERSION.RELEASE).append(" (API ").append(Build.VERSION.SDK_INT).append(")\n");
        android.content.SharedPreferences prefs = activity.getSharedPreferences(CompanionPanel.PREFS, Activity.MODE_PRIVATE);
        out.append("HD rendering: ").append(prefs.getBoolean(SetupActivity.PREF_HD_ON, true) ? "ON" : "OFF")
                .append(" / ").append(HdQuality.name(prefs.getInt(SetupActivity.PREF_HD_QUALITY, 1))).append(" (next game launch)\n");
        Display current = activity.getWindowManager().getDefaultDisplay();
        out.append("Activity display: ").append(current != null ? current.getDisplayId() : -1).append('\n');
        DisplayManager manager = (DisplayManager)activity.getSystemService(Activity.DISPLAY_SERVICE);
        if (manager != null) for (Display display : manager.getDisplays()) {
            Point size = new Point(); display.getRealSize(size);
            out.append("Display ").append(display.getDisplayId()).append(": ").append(display.getName())
                    .append(' ').append(size.x).append('x').append(size.y).append(" state ").append(display.getState()).append('\n');
        }
        Display bottom = BottomScreen.find(activity, manager);
        out.append("Selected bottom display: ").append(bottom != null ? bottom.getDisplayId() : "none").append('\n');
        int pads = 0;
        for (int id : InputDevice.getDeviceIds()) {
            InputDevice device = InputDevice.getDevice(id);
            if (device == null) continue;
            int sources = device.getSources();
            if ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD
                    || (sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK) {
                out.append("Android controller: ").append(device.getName()).append('\n'); pads++;
            }
        }
        if (pads == 0) out.append("Android controllers: none detected\n");
        File base = activity.getExternalFilesDir(null);
        if (base != null) {
            File sample = new File(base, FILE);
            out.append("\nLAST RENDER SAMPLE");
            if (sample.isFile()) out.append(" — ").append(new Date(sample.lastModified()));
            out.append("\n(Refreshes during rendering; FPS includes menus/cut scenes.)\n").append(tail(sample, 16384));
            if (logs) {
                out.append("\nENGINE LOG\n").append(tail(new File(base, "omt-log.txt"), 120 * 1024));
                out.append("\nDISPLAY LOG\n").append(tail(new File(base, BottomScreen.LOG_FILE), 8 * 1024));
            }
        }
        return out.toString();
    }

    static void share(Activity activity) {
        Intent send = new Intent(Intent.ACTION_SEND);
        send.setType("text/plain");
        send.putExtra(Intent.EXTRA_SUBJECT, "Neutron diagnostics");
        send.putExtra(Intent.EXTRA_TEXT, report(activity, true));
        activity.startActivity(Intent.createChooser(send, "Share diagnostics"));
    }

    static void show(final Activity activity) {
        TextView text = new TextView(activity);
        text.setTextSize(14); text.setTextIsSelectable(true); text.setPadding(24, 16, 24, 16);
        ScrollView scroll = new ScrollView(activity); scroll.addView(text);
        AlertDialog dialog = new AlertDialog.Builder(activity).setTitle("Diagnostics").setView(scroll)
                .setPositiveButton("Close", null).setNeutralButton("Share report", (d, which) -> share(activity)).create();
        Handler handler = new Handler(Looper.getMainLooper());
        Runnable refresh = new Runnable() {
            @Override public void run() {
                if (!dialog.isShowing() || activity.isFinishing()) return;
                text.setText(report(activity, false)); handler.postDelayed(this, 2000);
            }
        };
        dialog.setOnDismissListener(d -> handler.removeCallbacks(refresh));
        dialog.show(); refresh.run();
    }
}
