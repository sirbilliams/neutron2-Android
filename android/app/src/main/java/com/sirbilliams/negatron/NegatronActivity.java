package com.sirbilliams.negatron;

import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/**
 * Hosts the OMT engine. SDLActivity loads libSDL2.so and libmain.so (viewer) or libnegatron.so
 * (the reconstructed game), then calls SDL_main
 * (engine/sdl/OMediaSDLMain.cpp). Game files are read from Android/data/<package>/files/.
 */
public class NegatronActivity extends SDLActivity {

    /** Intent extra: which native app to run, "main" (the model viewer) or "negatron" (the game). */
    static final String EXTRA_LIB = "lib";
    /** Intent extra: a level to play straight away without its opening (the NEG_TEST_PLAY test hook). */
    static final String EXTRA_TEST_PLAY = "test_play";

    @Override
    protected String[] getLibraries() {
        String lib = getIntent() != null ? getIntent().getStringExtra(EXTRA_LIB) : null;
        return new String[] { "SDL2", "negatron".equals(lib) ? "negatron" : "main" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Set (or clear: the process can outlive a run) the test hook before the engine starts.
        String test = getIntent() != null ? getIntent().getStringExtra(EXTRA_TEST_PLAY) : null;
        try {
            if (test != null) Os.setenv("NEG_TEST_PLAY", test, true);
            else Os.unsetenv("NEG_TEST_PLAY");
        } catch (ErrnoException e) {
            // no test hook then
        }
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Make sure the files folder exists so it shows up over USB for copying the game data.
        if (getExternalFilesDir(null) != null) {
            getExternalFilesDir(null).mkdirs();
        }
    }
}
