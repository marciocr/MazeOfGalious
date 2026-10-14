#ifndef __MOG_KEYBOARDSTATE_H
#define __MOG_KEYBOARDSTATE_H

/* Keyboard state indexed by SDL_Scancode. It is updated only on press and
   release (as SDL 1.2's SDL_GetKeyState array was), so game code that clears
   an entry to "consume" a key press keeps it cleared until the key is pressed
   again. Gamepads feed the same array, as presses of the keys they stand for. */
#define KEYBOARD_SIZE SDL_SCANCODE_COUNT

/* Gamepads live in fixed slots so the chosen one ("GAMEPAD 2") survives other
   controllers being plugged or unplugged. */
#define MAX_GAMEPADS 4

/* What the gamepads are pressing, set by the game from its current screen: */
enum {
	KEYBOARD_CONTEXT_NONE,	/* gamepads ignored (key redefinition) */
	KEYBOARD_CONTEXT_TEXT,	/* typing a password: any gamepad, d-pad/stick = arrows only,
							   A/Start = return, B/Back = escape */
	KEYBOARD_CONTEXT_MENU,	/* any gamepad: d-pad/stick = arrows and the direction keys,
							   A/Start = confirm (space, return, sword), B/Back = escape */
	KEYBOARD_CONTEXT_GAME_MENU,	/* the in-game options menu: as MENU, but Start = escape
							   (back to the game) so it can't pick QUIT GAME */
	KEYBOARD_CONTEXT_GAME	/* the chosen gamepad: d-pad/stick = direction keys, A = sword,
							   B = weapon, X = up (jump), Y = item, Start = pause, Back = escape */
};

unsigned char *keyboard_get_state(void);
void keyboard_event(const SDL_Event *event);

/* Called by the main loop: keyboard_update_gamepads() before each game cycle,
   keyboard_cycle_done() after it. */
void keyboard_update_gamepads(void);
void keyboard_cycle_done(void);
void keyboard_set_context(int context);

/* Gamepad slot used to play (-1 = keyboard only); the keyboard always works. */
extern int gamepad_device;

/* Hotplug: SDL_EVENT_GAMEPAD_ADDED/REMOVED (SDL also sends ADDED for the
   controllers already connected at startup). */
void keyboard_gamepad_added(SDL_JoystickID id);
void keyboard_gamepad_removed(SDL_JoystickID id);
bool keyboard_gamepad_connected(int slot);
void keyboard_close_gamepads(void);

/* MoG.cfg stores the keys as SDL 1.2 key codes. */
SDL_Scancode keyboard_scancode_from_sdl12(int key);
int keyboard_scancode_to_sdl12(SDL_Scancode scancode);

bool IsAltPressed(void);
bool IsAltPressed2(void);

#endif
