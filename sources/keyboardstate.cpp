#include <string.h>
#include <SDL3/SDL.h>
#include "keyboardstate.h"

extern SDL_Scancode UP_KEY,DOWN_KEY,LEFT_KEY,RIGHT_KEY;
extern SDL_Scancode SWORD_KEY,WEAPON_KEY,ITEM_KEY,PAUSE_KEY;

#define SOURCE_KEYBOARD	1
#define SOURCE_GAMEPAD	2

/* Gamepad inputs, per slot: */
enum {
	PAD_LEFT, PAD_RIGHT, PAD_UP, PAD_DOWN,
	PAD_A, PAD_B, PAD_X, PAD_Y, PAD_BACK, PAD_START, PAD_SHOULDER,
	PAD_INPUTS
};

static unsigned char keyboard_state[KEYBOARD_SIZE];
static unsigned char key_sources[KEYBOARD_SIZE];	/* what is holding each key */
static unsigned char key_fresh[KEYBOARD_SIZE];		/* pressed since the last game cycle */
static unsigned char key_pending_release[KEYBOARD_SIZE];

int gamepad_device=0;

static SDL_Gamepad *gamepad_slots[MAX_GAMEPADS]={0,0,0,0};
static int context=KEYBOARD_CONTEXT_NONE;
static bool context_changed=false;
/* Inputs held when the context changed: they count only after a release. */
static unsigned char pad_latched[MAX_GAMEPADS][PAD_INPUTS];
static unsigned char pad_keys[KEYBOARD_SIZE];	/* keys the gamepads hold now */
/* Buttons pressed since the last game cycle, so a quick tap still counts: */
static unsigned char pad_tapped[MAX_GAMEPADS][PAD_INPUTS];


unsigned char *keyboard_get_state(void)
{
	return keyboard_state;
} /* keyboard_get_state */


/* A key goes down when its first source presses it and up when the last one
   releases it. A press and release between two game cycles still shows the
   key pressed for one cycle. */
static void keyboard_set(SDL_Scancode scancode,int source,bool down)
{
	unsigned char old_sources=key_sources[scancode];

	if (down) key_sources[scancode]|=source;
		 else key_sources[scancode]&=~source;

	if (old_sources==0 && key_sources[scancode]!=0) {
		keyboard_state[scancode]=1;
		key_fresh[scancode]=1;
		key_pending_release[scancode]=0;
	} else if (old_sources!=0 && key_sources[scancode]==0) {
		if (key_fresh[scancode]) key_pending_release[scancode]=1;
							else keyboard_state[scancode]=0;
	} /* if */
} /* keyboard_set */


static int gamepad_slot(SDL_JoystickID id)
{
	int i;

	for(i=0;i<MAX_GAMEPADS;i++) {
		if (gamepad_slots[i]!=0 && SDL_GetGamepadID(gamepad_slots[i])==id) return i;
	} /* for */

	return -1;
} /* gamepad_slot */


static int pad_input(Uint8 button)
{
	switch(button) {
	case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return PAD_LEFT;
	case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return PAD_RIGHT;
	case SDL_GAMEPAD_BUTTON_DPAD_UP: return PAD_UP;
	case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return PAD_DOWN;
	case SDL_GAMEPAD_BUTTON_SOUTH: return PAD_A;
	case SDL_GAMEPAD_BUTTON_EAST: return PAD_B;
	case SDL_GAMEPAD_BUTTON_WEST: return PAD_X;
	case SDL_GAMEPAD_BUTTON_NORTH: return PAD_Y;
	case SDL_GAMEPAD_BUTTON_BACK: return PAD_BACK;
	case SDL_GAMEPAD_BUTTON_START: return PAD_START;
	case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return PAD_SHOULDER;
	case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return PAD_SHOULDER;
	} /* switch */

	return -1;
} /* pad_input */


void keyboard_event(const SDL_Event *event)
{
	if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
		int slot=gamepad_slot(event->gbutton.which);
		int input=pad_input(event->gbutton.button);

		if (slot>=0 && input>=0) pad_tapped[slot][input]=1;
		return;
	} /* if */

	if (event->type != SDL_EVENT_KEY_DOWN && event->type != SDL_EVENT_KEY_UP) return;
	if (event->key.scancode <= SDL_SCANCODE_UNKNOWN || event->key.scancode >= KEYBOARD_SIZE) return;
	if (event->key.repeat) return;

	keyboard_set(event->key.scancode,SOURCE_KEYBOARD,event->type == SDL_EVENT_KEY_DOWN);
} /* keyboard_event */


void keyboard_cycle_done(void)
{
	int i;

	for(i=0;i<KEYBOARD_SIZE;i++) {
		if (key_pending_release[i]) keyboard_state[i]=0;
		key_pending_release[i]=0;
		key_fresh[i]=0;
	} /* for */
} /* keyboard_cycle_done */


void keyboard_set_context(int c)
{
	if (c!=context) {
		context=c;
		context_changed=true;
	} /* if */
} /* keyboard_set_context */


static void read_gamepad(SDL_Gamepad *g,unsigned char *in)
{
	int v;

	v=SDL_GetGamepadAxis(g,SDL_GAMEPAD_AXIS_LEFTX);
	in[PAD_LEFT]=(v < -16000 || SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_DPAD_LEFT)) ? 1:0;
	in[PAD_RIGHT]=(v > 16000 || SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) ? 1:0;
	v=SDL_GetGamepadAxis(g,SDL_GAMEPAD_AXIS_LEFTY);
	in[PAD_UP]=(v < -16000 || SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_DPAD_UP)) ? 1:0;
	in[PAD_DOWN]=(v > 16000 || SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_DPAD_DOWN)) ? 1:0;
	in[PAD_A]=SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_SOUTH) ? 1:0;
	in[PAD_B]=SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_EAST) ? 1:0;
	in[PAD_X]=SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_WEST) ? 1:0;
	in[PAD_Y]=SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_NORTH) ? 1:0;
	in[PAD_BACK]=SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_BACK) ? 1:0;
	in[PAD_START]=SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_START) ? 1:0;
	in[PAD_SHOULDER]=(SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) ||
					  SDL_GetGamepadButton(g,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) ? 1:0;
} /* read_gamepad */


void keyboard_update_gamepads(void)
{
	unsigned char keys[KEYBOARD_SIZE];
	unsigned char in[PAD_INPUTS];
	int i,j;

	memset(keys,0,sizeof(keys));
	SDL_UpdateGamepads();

	for(i=0;i<MAX_GAMEPADS;i++) {
		if (gamepad_slots[i]==0) {
			memset(pad_latched[i],0,PAD_INPUTS);
			memset(pad_tapped[i],0,PAD_INPUTS);
			continue;
		} /* if */

		read_gamepad(gamepad_slots[i],in);
		for(j=0;j<PAD_INPUTS;j++) {
			/* A new press releases the latch: */
			if (pad_tapped[i][j]) pad_latched[i][j]=0;
			in[j]|=pad_tapped[i][j];
			pad_tapped[i][j]=0;
			if (context_changed) pad_latched[i][j]=in[j];
			pad_latched[i][j]&=in[j];
			if (pad_latched[i][j]) in[j]=0;
		} /* for */

		if (context==KEYBOARD_CONTEXT_MENU || context==KEYBOARD_CONTEXT_GAME_MENU) {
			unsigned char confirm=in[PAD_A];
			unsigned char back=in[PAD_B] | in[PAD_BACK];

			if (context==KEYBOARD_CONTEXT_MENU) confirm|=in[PAD_START];
										   else back|=in[PAD_START];
			keys[SDL_SCANCODE_LEFT]|=in[PAD_LEFT];
			keys[SDL_SCANCODE_RIGHT]|=in[PAD_RIGHT];
			keys[SDL_SCANCODE_UP]|=in[PAD_UP];
			keys[SDL_SCANCODE_DOWN]|=in[PAD_DOWN];
			keys[LEFT_KEY]|=in[PAD_LEFT];
			keys[RIGHT_KEY]|=in[PAD_RIGHT];
			keys[UP_KEY]|=in[PAD_UP];
			keys[DOWN_KEY]|=in[PAD_DOWN];
			keys[SDL_SCANCODE_SPACE]|=confirm;
			keys[SDL_SCANCODE_RETURN]|=confirm;
			keys[SWORD_KEY]|=confirm;
			keys[SDL_SCANCODE_ESCAPE]|=back;
		} else if (context==KEYBOARD_CONTEXT_TEXT) {
			keys[SDL_SCANCODE_LEFT]|=in[PAD_LEFT];
			keys[SDL_SCANCODE_RIGHT]|=in[PAD_RIGHT];
			keys[SDL_SCANCODE_UP]|=in[PAD_UP];
			keys[SDL_SCANCODE_DOWN]|=in[PAD_DOWN];
			keys[SDL_SCANCODE_RETURN]|=in[PAD_A] | in[PAD_START];
			keys[SDL_SCANCODE_ESCAPE]|=in[PAD_B] | in[PAD_BACK];
			keys[SDL_SCANCODE_TAB]|=in[PAD_SHOULDER];
		} else if (context==KEYBOARD_CONTEXT_GAME && i==gamepad_device) {
			keys[LEFT_KEY]|=in[PAD_LEFT];
			keys[RIGHT_KEY]|=in[PAD_RIGHT];
			keys[UP_KEY]|=in[PAD_UP] | in[PAD_X];
			keys[DOWN_KEY]|=in[PAD_DOWN];
			keys[SWORD_KEY]|=in[PAD_A];
			keys[WEAPON_KEY]|=in[PAD_B];
			keys[ITEM_KEY]|=in[PAD_Y];
			keys[PAUSE_KEY]|=in[PAD_START];
			keys[SDL_SCANCODE_ESCAPE]|=in[PAD_BACK];
			keys[SDL_SCANCODE_TAB]|=in[PAD_SHOULDER];
		} /* if */
	} /* for */
	context_changed=false;

	keys[SDL_SCANCODE_UNKNOWN]=0;
	for(i=0;i<KEYBOARD_SIZE;i++) {
		if (keys[i]!=pad_keys[i]) {
			pad_keys[i]=keys[i];
			keyboard_set((SDL_Scancode)i,SOURCE_GAMEPAD,keys[i]!=0);
		} /* if */
	} /* for */
} /* keyboard_update_gamepads */


void keyboard_gamepad_added(SDL_JoystickID id)
{
	int i;

	for(i=0;i<MAX_GAMEPADS;i++) {
		if (gamepad_slots[i]!=0 && SDL_GetGamepadID(gamepad_slots[i])==id) return;
	} /* for */

	for(i=0;i<MAX_GAMEPADS;i++) {
		if (gamepad_slots[i]==0) {
			gamepad_slots[i]=SDL_OpenGamepad(id);
			/* Buttons already held don't count until released: */
			if (gamepad_slots[i]!=0) {
				SDL_UpdateGamepads();
				read_gamepad(gamepad_slots[i],pad_latched[i]);
			} /* if */
			return;
		} /* if */
	} /* for */
} /* keyboard_gamepad_added */


void keyboard_gamepad_removed(SDL_JoystickID id)
{
	int i;

	for(i=0;i<MAX_GAMEPADS;i++) {
		if (gamepad_slots[i]!=0 && SDL_GetGamepadID(gamepad_slots[i])==id) {
			SDL_CloseGamepad(gamepad_slots[i]);
			gamepad_slots[i]=0;
		} /* if */
	} /* for */
} /* keyboard_gamepad_removed */


bool keyboard_gamepad_connected(int slot)
{
	return slot>=0 && slot<MAX_GAMEPADS && gamepad_slots[slot]!=0;
} /* keyboard_gamepad_connected */


void keyboard_close_gamepads(void)
{
	int i;

	for(i=0;i<MAX_GAMEPADS;i++) {
		if (gamepad_slots[i]!=0) SDL_CloseGamepad(gamepad_slots[i]);
		gamepad_slots[i]=0;
	} /* for */
} /* keyboard_close_gamepads */


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
