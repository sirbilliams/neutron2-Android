/*  port: how the game starts (NEG_START) and the save-slot list for the Android launcher.

    What the original does (Neutron2.exe, CEditor::UpdateGame 0x4a5c70; the manual, p. 6-7):
    there is no menu before the game. The installer writes save\StartLevel.txt = "autosave", so
    every start CONTINUES the autosave (save\autosave.tsk, written at the checkpoints); when there
    is none (the shipped autosave.tsk is a "DUMMY" placeholder) a NEW GAME starts: level1d.gam,
    the title / logo screens and the start-up story ("for first time players, the game begins
    automatically"); loading level1d.gam deletes the autosaves (CEditor::LoadGame 0x4a8590).
    The game's Main Menu is the PDA that ESC opens (C2DInGameMenu page 3: Load/Save, Pick Ups,
    Options, Credits, Resume, Exit); its Load/Save page has New (score screen, then a new game,
    autosaves wiped), Load (save\JimmyGame<n>.tsk, n = 1..10), Save (copies the autosave into the
    slot) and Delete.

    NEG_START (environment; read once at start-up; NEG_TEST_PLAY wins over it):
        menu      (default, also when unset or unknown) the original start, and when a saved game
                  was continued, the original Main Menu (the PDA, as ESC opens it) is opened as
                  soon as Jimmy is playing, so the player can Resume / Load / start New.
        original  exactly the original start (continue the autosave, else a new game), no menu.
        new       a new game: level1d.gam (the original deletes the autosaves when it loads it).
        continue  the autosave; a new game when there is none (as the original).
        load:<n>  save slot n (1..10, save\JimmyGame<n>.tsk), as the Load/Save page's Load does;
                  the original start when the slot is empty.

    save\slots.txt (written by the game at start-up and whenever a slot or the autosave is
    written or deleted; missing = the game has not run yet: no saves to show). UTF-8 text, '\n'
    line ends, lines starting with '#' are comments, then one line per saved game, TAB-separated:
        <slot>  <level id>  <level name>  <gam file>  <score>  <mtime>
    slot: "autosave" or 1..10 (the NEG_START=load:<n> number); level id: the 4cc ('LV3E');
    level name: the Load/Save page's name for it ("negatron lab b"; empty when it has none);
    gam file: "Level3e.gam"; score: integer; mtime: the file's modification time, Unix seconds.
    Empty slots and DUMMY placeholders are not listed.                                         */
#pragma once

namespace neg {
// CEditor start-up, after save\StartLevel.txt was read into level_name (80 chars) and before
// NEG_TEST_PLAY: applies NEG_START. Returns the task file to restore when level_name is
// "autosave" ("autosave.tsk", or "JimmyGame<n>.tsk" for load:<n>).
const char *StartModeChoose(char *level_name);
// End of CEditor::LoadGame (a level has loaded).
void StartModeLevelLoaded(void);
// Every frame (CEditor::UpdateGame): opens the Main Menu once for NEG_START=menu.
void StartModeFrame(void);
// Writes save\slots.txt (see above).
void WriteSaveSlotList(void);
// The Load/Save page's name of a level id (C2DInGameMenu_c.cpp), NULL when it has none.
const char *SaveLevelName(int level);
}
