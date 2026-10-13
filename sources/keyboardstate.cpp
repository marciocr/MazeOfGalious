#include <SDL3/SDL.h>
#include "keyboardstate.h"

static unsigned char keyboard_state[KEYBOARD_SIZE];


unsigned char *keyboard_get_state(void)
{
	return keyboard_state;
} /* keyboard_get_state */


void keyboard_event(const SDL_Event *event)
{
	if (event->type != SDL_EVENT_KEY_DOWN && event->type != SDL_EVENT_KEY_UP) return;
	if (event->key.scancode <= SDL_SCANCODE_UNKNOWN || event->key.scancode >= KEYBOARD_SIZE) return;

	keyboard_state[event->key.scancode] = (event->type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
} /* keyboard_event */


/* SDL 1.2 key codes (SDL_keysym.h) of the keys that have a fixed scancode on
   a US layout; letters and digits are handled separately. */
static const struct {
	int key;
	SDL_Scancode scancode;
} sdl12_keys[] = {
	{8, SDL_SCANCODE_BACKSPACE}, {9, SDL_SCANCODE_TAB}, {13, SDL_SCANCODE_RETURN},
	{19, SDL_SCANCODE_PAUSE}, {27, SDL_SCANCODE_ESCAPE}, {32, SDL_SCANCODE_SPACE},
	{39, SDL_SCANCODE_APOSTROPHE}, {44, SDL_SCANCODE_COMMA}, {45, SDL_SCANCODE_MINUS},
	{46, SDL_SCANCODE_PERIOD}, {47, SDL_SCANCODE_SLASH}, {59, SDL_SCANCODE_SEMICOLON},
	{61, SDL_SCANCODE_EQUALS}, {91, SDL_SCANCODE_LEFTBRACKET}, {92, SDL_SCANCODE_BACKSLASH},
	{93, SDL_SCANCODE_RIGHTBRACKET}, {96, SDL_SCANCODE_GRAVE}, {127, SDL_SCANCODE_DELETE},
	{256, SDL_SCANCODE_KP_0}, {257, SDL_SCANCODE_KP_1}, {258, SDL_SCANCODE_KP_2},
	{259, SDL_SCANCODE_KP_3}, {260, SDL_SCANCODE_KP_4}, {261, SDL_SCANCODE_KP_5},
	{262, SDL_SCANCODE_KP_6}, {263, SDL_SCANCODE_KP_7}, {264, SDL_SCANCODE_KP_8},
	{265, SDL_SCANCODE_KP_9}, {266, SDL_SCANCODE_KP_PERIOD}, {267, SDL_SCANCODE_KP_DIVIDE},
	{268, SDL_SCANCODE_KP_MULTIPLY}, {269, SDL_SCANCODE_KP_MINUS}, {270, SDL_SCANCODE_KP_PLUS},
	{271, SDL_SCANCODE_KP_ENTER}, {272, SDL_SCANCODE_KP_EQUALS},
	{273, SDL_SCANCODE_UP}, {274, SDL_SCANCODE_DOWN}, {275, SDL_SCANCODE_RIGHT},
	{276, SDL_SCANCODE_LEFT}, {277, SDL_SCANCODE_INSERT}, {278, SDL_SCANCODE_HOME},
	{279, SDL_SCANCODE_END}, {280, SDL_SCANCODE_PAGEUP}, {281, SDL_SCANCODE_PAGEDOWN},
	{282, SDL_SCANCODE_F1}, {283, SDL_SCANCODE_F2}, {284, SDL_SCANCODE_F3},
	{285, SDL_SCANCODE_F4}, {286, SDL_SCANCODE_F5}, {287, SDL_SCANCODE_F6},
	{288, SDL_SCANCODE_F7}, {289, SDL_SCANCODE_F8}, {290, SDL_SCANCODE_F9},
	{291, SDL_SCANCODE_F10}, {292, SDL_SCANCODE_F11}, {293, SDL_SCANCODE_F12},
	{294, SDL_SCANCODE_F13}, {295, SDL_SCANCODE_F14}, {296, SDL_SCANCODE_F15},
	{300, SDL_SCANCODE_NUMLOCKCLEAR}, {301, SDL_SCANCODE_CAPSLOCK}, {302, SDL_SCANCODE_SCROLLLOCK},
	{303, SDL_SCANCODE_RSHIFT}, {304, SDL_SCANCODE_LSHIFT}, {305, SDL_SCANCODE_RCTRL},
	{306, SDL_SCANCODE_LCTRL}, {307, SDL_SCANCODE_RALT}, {308, SDL_SCANCODE_LALT},
	{309, SDL_SCANCODE_RGUI}, {310, SDL_SCANCODE_LGUI}, {311, SDL_SCANCODE_LGUI},
	{312, SDL_SCANCODE_RGUI}, {313, SDL_SCANCODE_MODE},
};


SDL_Scancode keyboard_scancode_from_sdl12(int key)
{
	unsigned int i;

	if (key >= 'a' && key <= 'z') return (SDL_Scancode)(SDL_SCANCODE_A + (key - 'a'));
	if (key >= '1' && key <= '9') return (SDL_Scancode)(SDL_SCANCODE_1 + (key - '1'));
	if (key == '0') return SDL_SCANCODE_0;

	for (i = 0; i < SDL_arraysize(sdl12_keys); i++) {
		if (sdl12_keys[i].key == key) return sdl12_keys[i].scancode;
	} /* for */

	return SDL_SCANCODE_UNKNOWN;
} /* keyboard_scancode_from_sdl12 */


int keyboard_scancode_to_sdl12(SDL_Scancode scancode)
{
	unsigned int i;

	if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) return 'a' + (scancode - SDL_SCANCODE_A);
	if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) return '1' + (scancode - SDL_SCANCODE_1);
	if (scancode == SDL_SCANCODE_0) return '0';

	for (i = 0; i < SDL_arraysize(sdl12_keys); i++) {
		if (sdl12_keys[i].scancode == scancode) return sdl12_keys[i].key;
	} /* for */

	return 0;
} /* keyboard_scancode_to_sdl12 */


bool IsAltPressed()
{
	return ((SDL_GetModState() & SDL_KMOD_ALT) != 0);
} /* IsAltPressed */


bool IsAltPressed2()
{
	return ((SDL_GetModState() & SDL_KMOD_ALT) != 0);
}
