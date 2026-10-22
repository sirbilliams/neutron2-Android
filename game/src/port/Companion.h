/*  port: the dual-screen companion (not in the original). A read-only snapshot of the game state
    for a second screen (the AYN Thor's bottom display), as a small JSON string, plus a top-down
    picture of the level and the icons it refers to.

    The game thread calls CompanionTick once a frame (CViewPort::render_viewport); while the
    companion is enabled it rebuilds the snapshot at most 10 times a second under a mutex. The
    Android panel reads it over JNI (Companion.nativeSnapshot / nativeMap / nativeIcon) on its own
    thread. Nothing here writes game state.

    The map: when a level's terrain is there (C3DTerrain shapes, the polygons the physics terrain
    grid uses), it is drawn once seen from above into an ARGB image of at most 1024 x 1024: the
    floors (normal.y >= 0.7, the physics' own limit) coloured from their textures (a 32 x 32 box-
    filtered copy, sampled at the polygon's UVs) times their material colour, the highest floor
    winning, shaded by height and slope; ledges (height steps) and the walls' top edges outlined.
    Map axes (checked on Level3 against the player camera's screen projection): image x = world
    x, image y = -world z (north = +z is up), which keeps the map the right way round as seen from
    above. Jimmy's heading is his local +z axis rotated by his angles (CGameObject::v226), checked
    against his walking direction (yaw 290 walks towards +x).

    Enabled by the Android panel (Companion.nativeSetEnabled) or, on the desktop, by
    NEG_COMPANION_LOG=1 (logs the snapshot once a second) or NEG_COMPANION_DUMP=<dir> (writes
    <dir>/<level>_map.png once per level, and every 2 s <dir>/<level>_snap.json and
    <dir>/<level>_overlay.png: the map with the snapshot's points and Jimmy's arrow drawn on it,
    plus <dir>/icon_<id>.png for each icon used).

    JSON fields (missing / null = not known):
      t        seconds since the snapshot started
      level    the .gam file;  id  the game type's 4cc (e.g. "LEV3")
      mode     game mode (0 starting, 1 loading, 2 saving, 3 cut scene, 4 menu, 5 playing, 6 paused);
      cut      1 in a cut scene
      energy   neutron energy 0..100 (the HUD bar);  energy_max 100;  lives;  score
      gadget   current gadget = inventory page item (0 = none; C3DJimmy::ActivateMenuItem)
      gadgets  [[item, menu state, icon], ...] the inventory page items (state 0: not owned)
      items    [[pickup, count, icon], ...] the pickups page items Jimmy has (state != 0)
      tasks    [[name, state], ...] the story tasks (state 0 = not started; others are stages)
      caption  the caption being typed (C3DTextTrigger lines of the current group, so far)
      boss     {"name", "hp" 0..1, "bar" 1 while its bar is up} (C3DHerman)
      pickups / neutrons / checkpoints   [collected, total] in the level
      map      {"gen", "w", "h", "s" world units per pixel} the current map image (gen changes
               when it is rebuilt; 0 = none yet)
      jimmy    [x, y, hx, hy, height] in map pixels, hx/hy his heading (unit, map pixels)
      pts      [[kind, x, y, icon or label], ...] in map pixels: p pickup (icon), n neutron,
               c checkpoint, f friend, e other AI character, o objective (a C3DArrow showing),
               d door, x exit (label: the level it loads; kind X while its task holds it shut)
      picker   {"sel", "cards": [[name, wire icon, colour icon], ...]} while the Vox world picker is
               open (port/VoxPicker.h): the worlds left to right, sel the lit one (-1 none)
      icon     a screens.omt canvas id (-1 = none), fetched with Companion.nativeIcon        */
#pragma once

namespace neg {
void CompanionTick(float dt);			// once a frame, game thread
void CompanionSetEnabled(bool on);		// any thread
int  CompanionCopy(char *buf, int size);	// any thread: the latest snapshot (length, 0 if none)
}
