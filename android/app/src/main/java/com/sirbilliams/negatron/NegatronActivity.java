package com.sirbilliams.negatron;

import android.os.Bundle;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/**
 * Hosts the OMT engine. SDLActivity loads libSDL2.so and libmain.so, then calls SDL_main
 * (engine/sdl/OMediaSDLMain.cpp). Game files are read from Android/data/<package>/files/.
 */
public class NegatronActivity extends SDLActivity {

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
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
