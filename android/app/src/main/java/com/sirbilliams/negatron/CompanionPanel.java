package com.sirbilliams.negatron;

import android.app.Activity;
import android.app.Presentation;
import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Display;
import android.view.WindowManager;

/**
 * port: the bottom-screen companion panel (AYN Thor: its second display). Shows a
 * {@link CompanionView} in an android.app.Presentation on the first presentation display (or any
 * non-default display), polling the native snapshot 10 times a second. Off unless switched on
 * (remembered in the preferences); with no second display it simply shows nothing. The panel
 * never takes focus or touches, so the game keeps the input.
 */
final class CompanionPanel implements DisplayManager.DisplayListener {
    static final String PREFS = "negatron";
    static final String KEY_ON = "bottom_panel";

    private final Activity mActivity;
    private final DisplayManager mDisplays;
    private final SharedPreferences mPrefs;
    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private PanelPresentation mPresentation;
    private boolean mResumed;
    private boolean mEnabled;

    private final Runnable mPoll = new Runnable() {
        @Override
        public void run() {
            if (mPresentation == null) return;
            if (mPresentation.view != null) mPresentation.view.setSnapshot(Companion.snapshot());
            mHandler.postDelayed(this, 100);
        }
    };

    CompanionPanel(Activity activity) {
        mActivity = activity;
        mDisplays = (DisplayManager) activity.getSystemService(Context.DISPLAY_SERVICE);
        mPrefs = activity.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        mEnabled = mPrefs.getBoolean(KEY_ON, false);
    }

    boolean isEnabled() { return mEnabled; }

    boolean hasSecondDisplay() { return findDisplay() != null; }

    void setEnabled(boolean on) {
        mEnabled = on;
        mPrefs.edit().putBoolean(KEY_ON, on).apply();
        update();
    }

    void resume() {
        mResumed = true;
        if (mDisplays != null) mDisplays.registerDisplayListener(this, mHandler);
        update();
    }

    void pause() {
        mResumed = false;
        if (mDisplays != null) mDisplays.unregisterDisplayListener(this);
        update();
    }

    private Display findDisplay() {
        if (mDisplays == null) return null;
        Display[] shown = mDisplays.getDisplays(DisplayManager.DISPLAY_CATEGORY_PRESENTATION);
        if (shown != null && shown.length > 0) return shown[0];
        for (Display d : mDisplays.getDisplays()) {
            if (d.getDisplayId() != Display.DEFAULT_DISPLAY) return d;
        }
        return null;
    }

    private void update() {
        Display d = (mEnabled && mResumed) ? findDisplay() : null;
        if (mPresentation != null
                && (d == null || mPresentation.getDisplay().getDisplayId() != d.getDisplayId())) {
            hide();
        }
        if (d != null && mPresentation == null) show(d);
    }

    private void show(Display d) {
        PanelPresentation p = new PanelPresentation(mActivity, d);
        try {
            p.show();
        } catch (WindowManager.InvalidDisplayException | IllegalStateException e) {
            return;
        }
        mPresentation = p;
        Companion.setEnabled(true);
        mHandler.removeCallbacks(mPoll);
        mHandler.post(mPoll);
    }

    private void hide() {
        mHandler.removeCallbacks(mPoll);
        Companion.setEnabled(false);
        try {
            mPresentation.dismiss();
        } catch (RuntimeException e) {
            // the display went away
        }
        mPresentation = null;
    }

    @Override public void onDisplayAdded(int displayId) { update(); }
    @Override public void onDisplayRemoved(int displayId) { update(); }
    @Override public void onDisplayChanged(int displayId) { update(); }

    /** The Presentation window: one full-size CompanionView, never focused or touched. */
    static final class PanelPresentation extends Presentation {
        CompanionView view;

        PanelPresentation(Context outer, Display display) {
            super(outer, display);
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                    | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                    | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            view = new CompanionView(getContext());
            setContentView(view);
        }
    }
}
