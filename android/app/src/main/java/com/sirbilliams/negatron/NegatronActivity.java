package com.sirbilliams.negatron;

import android.os.Bundle;
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

    @Override
    protected String[] getLibraries() {
        String lib = getIntent() != null ? getIntent().getStringExtra(EXTRA_LIB) : null;
        return new String[] { "SDL2", "negatron".equals(lib) ? "negatron" : "main" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Make sure the files folder exists so it shows up over USB for copying the game data.
        if (getExternalFilesDir(null) != null) {
            getExternalFilesDir(null).mkdirs();
        }
    }
}
