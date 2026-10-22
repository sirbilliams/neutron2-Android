/*  port: a gamepad picker for Vox, the museum's world-select terminal (not in the original).

    In the museum (Level5d) the question-mark terminal ('helpo', a C3DHelpSound) calls the trigger
    MAINMENU, which opens the PDA main menu with the world pictures (C2DInGameMenu items at y 251:
    VR tank, pyramid, King Carl ...). The original drives that page with the Windows mouse
    pointer, which the Thor does not show. While that page is open after MAINMENU:

      - left / right (d-pad, left stick, arrow keys) move the highlight from picture to picture: the
        page's pointer position becomes the chosen picture's centre, so the original rollover,
        info and click code run unchanged (A clicks, as the action button always did);
      - B (or Backspace) closes the menu as RESUME does and steps Jimmy back out of the terminal's
        radius, so walking in again re-opens it;
      - each picture gets its world's name below it (the HELPMENUITEM texts of dat\menu.dat);
      - the companion panel shows the worlds as cards (VoxPickerCards).

    The original behaviour stays reachable: moving the pointer (touch, mouse, right stick) hands
    the page back to the pointer until left / right is pressed again, and NEG_VOX_PICKER=0 turns
    the picker off. Nothing changes on the other menu pages or on the main menu opened with Esc.          */
#pragma once

class C2DInGameMenu;

namespace neg {
void VoxPickerOpened(void);								// the MAINMENU trigger opened the main menu
bool VoxPickerFrame(C2DInGameMenu *m);					// HandleMenuInput, first; true when it closed the menu
bool VoxPickerPointer(short &x, short &y);				// HandleMenuInput: the chosen picture's centre
														// instead of the pointer (false: use the pointer)
// For the companion (game thread): the pictures shown, while the picker is on. Each card: name,
// wire picture and colour picture (screens.omt canvas ids). Returns the count; *sel the lit one.
struct VoxCard { char name[32]; int wire, colour; };
int VoxPickerCards(VoxCard *cards, int max, int *sel);
}
