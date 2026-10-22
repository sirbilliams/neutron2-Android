/*  port: the Asset Viewer's level explorer (Bill's request; not in the original).

    NEG_ASSET_VIEW=<level.gam> (environment, set by the launcher's Asset Viewer -> "Explore a
    level"; wins over NEG_START and NEG_TEST_PLAY): the game starts with that level, loaded by the
    game's own loader (CEditor::LoadGame), and then, instead of playing it, freezes it and gives a
    free-fly camera:
        left stick      move forward / back, strafe          (keys: W S A D)
        right stick     look                                 (keys: arrows)
        L2 / R2, L1 / R1  down / up                          (keys: Q / E, PageDown / PageUp)
        A               speed: SLOW / NORMAL / FAST          (key: Tab)
        Y               the level's fog on / off             (key: F)
        X               Jimmy shown / hidden                 (key: J)
        Start           back to where the explorer started   (key: Home)
        B / Back        leave (the game ends; the app goes back to the launcher)  (keys: Esc, Backspace)
    No collision, no gravity: nothing of the level runs (no physics, no object updates: AI, cut
    scenes, triggers, level exits and autosaves are all still); only the level's texture
    animations and the sky keep going. A small overlay shows the level's name, the position and
    the speed. Nothing changes when NEG_ASSET_VIEW is unset.

    NEG_ASSET_VIEW=models: the model viewer. The small Template.gam level is loaded as the stage,
    all its objects hidden, no sky or fog, a light of our own; one model at a time is shown by the
    game's own loaders: .grn as a C3DGrannyObj (the bind pose), .ase as a C3DASEObj (the texture
    its *BITMAP names, png\<name>.png), and each 3D shape of an .omt database. It turns slowly.
        up / down       previous / next model (held: repeats)       (keys: arrows)
        L1 / R1         10 back / on                                (keys: PageUp / PageDown)
        left / right    previous / next shape of an .omt            (keys: arrows)
        sticks          turn around it (stops the turning)          (keys: W A S D)
        L2 / R2         zoom out / in                               (keys: Q / E)
        A               turning on / off; Start: reset the view; B / Back: leave
    NEG_MODEL=<file> (desktop tests): the model to start with.

    save\levels.txt (written by the game at every start, for the launcher's level list): one line
    per gam\*.gam, TAB-separated: <gam file> <display name> (the bottom-screen panel's names,
    Companion.cpp; empty when the level has none).                                            */
#pragma once

namespace neg {
const char *AssetViewLevel(void);		// NEG_ASSET_VIEW's level (.gam), or NULL
bool AssetViewOn(void);					// the explorer runs: the level is loaded and frozen
void AssetViewLevelLoaded(const char *name);	// end of CEditor::LoadGame
bool AssetViewFrame(float dt);			// CViewPort::render_viewport: true = the explorer took the frame
void WriteLevelList(void);				// save\levels.txt (see above)
}
