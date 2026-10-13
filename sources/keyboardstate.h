#ifndef __MOG_KEYBOARDSTATE_H
#define __MOG_KEYBOARDSTATE_H

/* Keyboard state indexed by SDL_Scancode. It is updated only by key events
   (as SDL 1.2's SDL_GetKeyState array was), so game code that clears an
   entry to "consume" a key press keeps it cleared until the key is pressed
   again. */
#define KEYBOARD_SIZE SDL_SCANCODE_COUNT

unsigned char *keyboard_get_state(void);
void keyboard_event(const SDL_Event *event);

/* MoG.cfg stores the keys as SDL 1.2 key codes. */
SDL_Scancode keyboard_scancode_from_sdl12(int key);
int keyboard_scancode_to_sdl12(SDL_Scancode scancode);

bool IsAltPressed(void);
bool IsAltPressed2(void);

#endif
