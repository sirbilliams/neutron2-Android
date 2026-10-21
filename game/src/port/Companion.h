/*  port: the dual-screen companion (not in the original). A read-only snapshot of the game state
    for a second screen (the AYN Thor's bottom display), as a small JSON string.

    The game thread calls CompanionTick once a frame (CViewPort::render_viewport); while the
    companion is enabled it rebuilds the snapshot at most 10 times a second under a mutex. The
    Android panel reads it over JNI (Companion.nativeSnapshot) on its own thread. Nothing here
    writes game state.

    Enabled by the Android panel (Companion.nativeSetEnabled) or, on the desktop, by
    NEG_COMPANION_LOG=1, which also logs the snapshot once a second.

    JSON fields (missing / null = not known):
      t        seconds since the snapshot started
      level    the .gam file;  id  the game type's 4cc (e.g. "LV3E")
      mode     game mode (0 starting, 1 loading, 2 saving, 3 cut scene, 4 menu, 5 playing, 6 paused);
      cut      1 in a cut scene
      energy   neutron energy 0..100 (the HUD bar);  energy_max 100;  lives;  score
      gadget   current gadget number (0 = none)
      gadgets  [[item, menu state], ...] the inventory page items (state 0: not shown / not owned)
      tasks    [[name, state], ...] the story tasks
      caption  the caption being typed (C3DTextTrigger lines of the current group, so far)
      boss     {"name", "hp" 0..1, "bar" 1 while its bar is up} (C3DHerman)
      pickups / neutrons / checkpoints   [collected, total] in the level
      jimmy    [x, z, yaw degrees]
      pts      [[kind, x, z], ...] for the map: p pickup, n neutron, c checkpoint, f friend,
               e other AI character (collected / hidden ones are left out)                  */
#pragma once

namespace neg {
void CompanionTick(float dt);			// once a frame, game thread
void CompanionSetEnabled(bool on);		// any thread
int  CompanionCopy(char *buf, int size);	// any thread: the latest snapshot (length, 0 if none)
}
