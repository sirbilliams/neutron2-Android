package com.sirbilliams.negatron;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.view.View;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * port: draws the companion snapshot (see game/src/port/Companion.h for the fields) with plain
 * shapes and text: status and bars on the left, the task list below them, a top-down map on
 * the right (north up, centred on Jimmy). Laid out for the Thor's 1240 x 1080 bottom screen and
 * scaled to whatever size it gets. Dark, large text.
 */
final class CompanionView extends View {
    private static final int BG = Color.rgb(12, 14, 20);
    private static final int PANEL = Color.rgb(28, 32, 44);
    private static final int TEXT = Color.rgb(230, 232, 240);
    private static final int DIM = Color.rgb(140, 146, 160);
    private static final int ENERGY = Color.rgb(80, 200, 255);
    private static final int BOSS = Color.rgb(240, 90, 80);
    private static final int ACCENT = Color.rgb(255, 210, 70);
    private static final float MAP_RANGE = 3000f;   // world units from Jimmy to the map's edge

    private final Paint mFill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint mText = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Path mPath = new Path();
    private final RectF mRect = new RectF();
    private JSONObject mData;
    private String mLast;

    CompanionView(Context c) {
        super(c);
        setBackgroundColor(BG);
    }

    /** New snapshot (JSON) from the poll; redraws only when it changed. */
    void setSnapshot(String json) {
        if (json == null || json.isEmpty() || json.equals(mLast)) return;
        mLast = json;
        try {
            mData = new JSONObject(json);
        } catch (Exception e) {
            return;
        }
        invalidate();
    }

    @Override
    protected void onDraw(Canvas c) {
        float w = getWidth(), h = getHeight();
        float u = Math.min(w / 1240f, h / 1080f);         // layout unit: 1 px on the Thor's screen
        float pad = 24 * u;
        JSONObject d = mData;
        if (d == null) {
            text(c, "Waiting for the game...", pad, pad + 48 * u, 40 * u, DIM);
            return;
        }
        float colW = w * 0.5f - pad * 1.5f;
        float x = pad, y = pad;

        // header: level and mode
        String level = d.optString("level", "");
        String mode = modeName(d.optInt("mode", -1));
        text(c, level, x, y + 44 * u, 44 * u, TEXT);
        text(c, d.optString("id", "") + "   " + mode, x, y + 88 * u, 30 * u, d.optInt("cut", 0) == 1 ? ACCENT : DIM);
        y += 112 * u;

        // energy bar
        double energy = d.optDouble("energy", Double.NaN);
        double emax = d.optDouble("energy_max", 100);
        if (!Double.isNaN(energy)) {
            bar(c, x, y, colW, 44 * u, (float) (energy / emax), ENERGY, "Energy " + Math.round(energy), u);
            y += 60 * u;
        }
        text(c, "Score " + d.optInt("score", 0) + "     Lives " + d.optInt("lives", 0), x, y + 34 * u, 32 * u, TEXT);
        y += 52 * u;

        // boss
        JSONObject boss = d.optJSONObject("boss");
        if (boss != null && (boss.optInt("bar", 0) == 1 || boss.optDouble("hp", 1) < 1)) {
            bar(c, x, y, colW, 36 * u, (float) boss.optDouble("hp", 0), BOSS, boss.optString("name", "Boss"), u);
            y += 52 * u;
        }

        // gadget
        int gadget = d.optInt("gadget", 0);
        StringBuilder owned = new StringBuilder();
        JSONArray gadgets = d.optJSONArray("gadgets");
        for (int i = 0; gadgets != null && i < gadgets.length(); i++) {
            JSONArray g = gadgets.optJSONArray(i);
            if (g == null || g.optInt(1, 0) == 0) continue;
            int n = g.optInt(0, -1);
            owned.append(n == gadget ? " [" + n + "]" : " " + n);
        }
        text(c, "Gadget: " + (gadget == 0 ? "none" : "#" + gadget) + "    Have:" + (owned.length() == 0 ? " -" : owned.toString()),
                x, y + 32 * u, 30 * u, TEXT);
        y += 48 * u;

        // collected counts
        String counts = count(d, "pickups", "Pickups") + "   " + count(d, "neutrons", "Neutrons")
                + "   " + count(d, "checkpoints", "Checkpoints");
        text(c, counts, x, y + 28 * u, 26 * u, DIM);
        y += 48 * u;

        // caption
        String caption = d.optString("caption", "");
        if (!caption.isEmpty()) {
            float ch = 30 * u;
            String[] lines = caption.split("\n");
            float boxH = lines.length * ch * 1.25f + 20 * u;
            mFill.setColor(PANEL);
            mRect.set(x, y, x + colW, y + boxH);
            c.drawRoundRect(mRect, 12 * u, 12 * u, mFill);
            float ty = y + 10 * u + ch;
            for (String line : lines) {
                text(c, line, x + 14 * u, ty, ch, ACCENT);
                ty += ch * 1.25f;
            }
            y += boxH + 16 * u;
        }

        // tasks
        JSONArray tasks = d.optJSONArray("tasks");
        if (tasks != null && y < h - 60 * u) {
            text(c, "Tasks", x, y + 30 * u, 30 * u, DIM);
            y += 44 * u;
            float th = 26 * u;
            float tx = x;
            float top = y;
            for (int i = 0; i < tasks.length(); i++) {
                JSONArray t = tasks.optJSONArray(i);
                if (t == null) continue;
                int state = t.optInt(1, 0);
                if (y + th > h - pad) {                 // second column
                    if (tx > x) break;
                    tx = x + colW / 2;
                    y = top;
                }
                text(c, t.optString(0, "?") + "  " + state, tx, y + th, th, state != 0 ? TEXT : DIM);
                y += th * 1.3f;
            }
        }

        // map
        float mx = w * 0.5f + pad * 0.5f, my = pad, ms = Math.min(w - mx - pad, h - 2 * pad);
        drawMap(c, d, mx, my, ms, u);
    }

    private void drawMap(Canvas c, JSONObject d, float mx, float my, float ms, float u) {
        mFill.setColor(PANEL);
        mRect.set(mx, my, mx + ms, my + ms);
        c.drawRoundRect(mRect, 16 * u, 16 * u, mFill);
        JSONArray jim = d.optJSONArray("jimmy");
        if (jim == null) {
            text(c, "No map", mx + 20 * u, my + 50 * u, 30 * u, DIM);
            return;
        }
        float jx = (float) jim.optDouble(0, 0), jz = (float) jim.optDouble(1, 0);
        float cx = mx + ms / 2, cy = my + ms / 2, k = (ms / 2) / MAP_RANGE;
        c.save();
        c.clipRect(mRect);
        JSONArray pts = d.optJSONArray("pts");
        if (pts != null) {
            for (int i = 0; i < pts.length(); i++) {
                JSONArray p = pts.optJSONArray(i);
                if (p == null) continue;
                float px = cx + ((float) p.optDouble(1, 0) - jx) * k;
                float py = cy - ((float) p.optDouble(2, 0) - jz) * k;
                String kind = p.optString(0, "");
                int col;
                float r = 9 * u;
                switch (kind) {
                    case "p": col = ACCENT; break;
                    case "n": col = ENERGY; r = 6 * u; break;
                    case "c": col = Color.rgb(120, 230, 120); break;
                    case "f": col = Color.rgb(200, 150, 255); break;
                    case "e": col = BOSS; break;
                    default: col = DIM;
                }
                mFill.setColor(col);
                c.drawCircle(px, py, r, mFill);
            }
        }
        // Jimmy: an arrow along his heading (yaw in degrees; the sign convention is a guess)
        double yaw = Math.toRadians(jim.optDouble(2, 0));
        float s = 22 * u;
        float fx = (float) Math.sin(yaw), fy = (float) -Math.cos(yaw);
        mPath.reset();
        mPath.moveTo(cx + fx * s, cy + fy * s);
        mPath.lineTo(cx - fx * s * 0.6f - fy * s * 0.6f, cy - fy * s * 0.6f + fx * s * 0.6f);
        mPath.lineTo(cx - fx * s * 0.6f + fy * s * 0.6f, cy - fy * s * 0.6f - fx * s * 0.6f);
        mPath.close();
        mFill.setColor(Color.WHITE);
        c.drawPath(mPath, mFill);
        c.restore();
        text(c, "N", mx + ms / 2 - 8 * u, my + 30 * u, 24 * u, DIM);
    }

    private void bar(Canvas c, float x, float y, float w, float h, float frac, int color, String label, float u) {
        if (Float.isNaN(frac)) frac = 0;
        frac = Math.max(0, Math.min(1, frac));
        mFill.setColor(PANEL);
        mRect.set(x, y, x + w, y + h);
        c.drawRoundRect(mRect, h / 3, h / 3, mFill);
        mFill.setColor(color);
        mRect.set(x, y, x + w * frac, y + h);
        c.drawRoundRect(mRect, h / 3, h / 3, mFill);
        text(c, label, x + 12 * u, y + h * 0.72f, h * 0.62f, Color.BLACK);
    }

    private void text(Canvas c, String s, float x, float y, float size, int color) {
        mText.setColor(color);
        mText.setTextSize(size);
        c.drawText(s, x, y, mText);
    }

    private static String count(JSONObject d, String key, String label) {
        JSONArray a = d.optJSONArray(key);
        if (a == null) return label + " ?";
        return label + " " + a.optInt(0, 0) + "/" + a.optInt(1, 0);
    }

    private static String modeName(int m) {
        switch (m) {
            case 0: return "starting";
            case 1: return "loading";
            case 2: return "saving";
            case 3: return "cut scene";
            case 4: return "menu";
            case 5: return "playing";
            case 6: return "paused";
            default: return "";
        }
    }
}
