package com.sirbilliams.negatron;

/** Persisted game-controller profile. Plain Java for desktop previews and tests. */
final class ControlProfile {
    static final int[] BUTTONS = {0, 1, 2, 3, 9, 10, 7, 8}; // SDL canonical IDs
    static final String[] NAMES = {"A", "B", "X", "Y", "L1", "R1", "L3", "R3"};
    static final String[] ACTIONS = {"Action / fire", "Jump / speed", "Brake", "Inventory", "HUD", "Turn around", "Headlight", "Unused button"};
    int movement = 0, camera = 20, speed = 100;
    boolean invert;
    final int[] map = BUTTONS.clone();
    static int valid(int n, int fallback, int lo, int hi) { return n < lo || n > hi ? fallback : n; }
    ControlProfile() {}
    ControlProfile(String csv, int movement, int camera, int speed, boolean invert) {
        this.movement = valid(movement, 0, 0, 35); this.camera = valid(camera, 20, 0, 35);
        this.speed = valid(speed, 100, 25, 250); this.invert = invert;
        if (csv == null) return;
        String[] parts = csv.split(",", -1); if (parts.length != 8) return;
        int[] result = new int[8]; int used = 0;
        try {
            for (int i = 0; i < 8; i++) {
                int n = Integer.parseInt(parts[i]); int k = index(n);
                if (k < 0 || (used & (1 << k)) != 0) return;
                used |= 1 << k; result[i] = n;
            }
        } catch (NumberFormatException e) { return; }
        System.arraycopy(result, 0, map, 0, 8);
    }
    static int index(int n) { for (int i = 0; i < 8; i++) if (BUTTONS[i] == n) return i; return -1; }
    String csv() {
        StringBuilder s = new StringBuilder();
        for (int n : map) { if (s.length() > 0) s.append(','); s.append(n); }
        return s.toString();
    }
    void step(String id, int dir) {
        if (id.equals("move_dead")) movement = Math.floorMod(movement + dir * 5, 40);
        else if (id.equals("camera_dead")) camera = Math.floorMod(camera + dir * 5, 40);
        else if (id.equals("camera_speed")) speed = 25 + Math.floorMod(speed - 25 + dir * 25, 250);
        else if (id.equals("invert_y")) invert = !invert;
        else if (id.startsWith("bind:")) {
            int action = Integer.parseInt(id.substring(5));
            if (action < 0 || action >= map.length) return;
            int next = BUTTONS[Math.floorMod(index(map[action]) + dir, 8)];
            int other = 0; while (map[other] != next) other++;
            map[other] = map[action]; map[action] = next;
        }
    }
}
