#include "stdio.h"
#include "string.h"
#include <SDL3/SDL.h>
#include "keyboardstate.h"
#include "sound.h"
#include "video.h"
#include "postfx.h"
#include "MOGtypes.h"
#include "drawing.h"
#include "bitmaps.h"
#include "sprites.h"
#include "tiles.h"
#include "mog.h"

/* OPTIONS screen (STATE 20), reached from the title screen: global audio,
   video and controls settings, saved in MoG.cfg. */

extern int STATE,SUBSTATE;
extern unsigned char old_keyboard[KEYBOARD_SIZE];
extern SDL_Scancode UP_KEY,DOWN_KEY,LEFT_KEY,RIGHT_KEY;
extern SDL_Scancode SWORD_KEY,WEAPON_KEY,ITEM_KEY,PAUSE_KEY;
extern int music_volume,sfx_volume;
extern SOUNDT S_select;

enum {PAGE_MAIN, PAGE_AUDIO, PAGE_VIDEO, PAGE_CONTROLS};

static int page=PAGE_MAIN;
static int selected=0;
static int hold_cycles=0;

/* Key redefinition (STATE 10) returns here, and restores these on cancel: */
static int redefine_return_state=2;
static SDL_Scancode redefine_backup[8];


/* Volumes are 0..127 in MoG.cfg and shown as 0..100: */
static int volume_percent(int v)
{
	return (v*100+63)/127;
} /* volume_percent */


static int volume_from_percent(int p)
{
	if (p<0) p=0;
	if (p>100) p=100;
	return (p*127+50)/100;
} /* volume_from_percent */


static bool pressed(unsigned char *keyboard,SDL_Scancode k)
{
	return keyboard[k] && !old_keyboard[k];
} /* pressed */


/* Left/right: a press, then repeats while held. */
static int left_right(unsigned char *keyboard)
{
	bool l=keyboard[SDL_SCANCODE_LEFT] || keyboard[LEFT_KEY];
	bool r=keyboard[SDL_SCANCODE_RIGHT] || keyboard[RIGHT_KEY];

	if (l==r) {
		hold_cycles=0;
		return 0;
	} /* if */

	hold_cycles++;
	if (hold_cycles==1 || (hold_cycles>10 && (hold_cycles%2)==0)) return l ? -1:1;
	return 0;
} /* left_right */


static int n_items(void)
{
	switch(page) {
	case PAGE_MAIN: return 4;		/* AUDIO, VIDEO, CONTROLS, BACK */
	case PAGE_AUDIO: return 3;		/* MUSIC, SFX, BACK */
	case PAGE_VIDEO: return 4;		/* FULLSCREEN, WINDOW SIZE, FILTER, BACK */
	case PAGE_CONTROLS: return 3;	/* DEVICE, REDEFINE KEYS, BACK */
	} /* switch */
	return 1;
} /* n_items */


static void device_name(int device,char *buf)
{
	if (device<0) strcpy(buf,"KEYBOARD");
			 else sprintf(buf,"GAMEPAD %i",device+1);
} /* device_name */


static void go_back(void)
{
	if (page==PAGE_MAIN) {
		guardar_configuracion("MoG.cfg");
		STATE=2;
		SUBSTATE=0;
	} else {
		selected=page-1;	/* the page's entry in the main page */
		page=PAGE_MAIN;
	} /* if */
} /* go_back */


void start_key_redefinition(int return_state)
{
	redefine_return_state=return_state;
	redefine_backup[0]=UP_KEY;
	redefine_backup[1]=DOWN_KEY;
	redefine_backup[2]=LEFT_KEY;
	redefine_backup[3]=RIGHT_KEY;
	redefine_backup[4]=SWORD_KEY;
	redefine_backup[5]=WEAPON_KEY;
	redefine_backup[6]=ITEM_KEY;
	redefine_backup[7]=PAUSE_KEY;
	STATE=10;
	SUBSTATE=0;
} /* start_key_redefinition */


void end_key_redefinition(bool cancel)
{
	if (cancel) {
		UP_KEY=redefine_backup[0];
		DOWN_KEY=redefine_backup[1];
		LEFT_KEY=redefine_backup[2];
		RIGHT_KEY=redefine_backup[3];
		SWORD_KEY=redefine_backup[4];
		WEAPON_KEY=redefine_backup[5];
		ITEM_KEY=redefine_backup[6];
		PAUSE_KEY=redefine_backup[7];
	} else {
		guardar_configuracion("MoG.cfg");
	} /* if */
	STATE=redefine_return_state;
	SUBSTATE=0;
} /* end_key_redefinition */


void start_options(void)
{
	page=PAGE_MAIN;
	selected=0;
	hold_cycles=0;
	STATE=20;
	SUBSTATE=0;
} /* start_options */


static void item(const char *text,int row,int n,BYTE *screen,int dx,int dy)
{
	if (n==selected) B_rectangle(screen,TILE_SIZE_X*5,TILE_SIZE_Y*row,TILE_SIZE_X*30,TILE_SIZE_Y,dx,47);
	tile_print((char *)text,TILE_SIZE_X*6,TILE_SIZE_Y*row,screen,dx,dy);
} /* item */


static void centered(const char *text,int row,BYTE *screen,int dx,int dy)
{
	tile_print((char *)text,TILE_SIZE_X*((40-(int)strlen(text))/2),TILE_SIZE_Y*row,screen,dx,dy);
} /* centered */


void OptionsCycle(BYTE *screen,int dx,int dy,unsigned char *keyboard)
{
	char tmp[80],tmp2[40];
	int change;
	bool confirm,up,down;

	memset(screen,0,dx*dy);

	/* Input: */
	up=pressed(keyboard,SDL_SCANCODE_UP) || pressed(keyboard,UP_KEY);
	down=pressed(keyboard,SDL_SCANCODE_DOWN) || pressed(keyboard,DOWN_KEY);
	confirm=pressed(keyboard,SDL_SCANCODE_SPACE) || pressed(keyboard,SDL_SCANCODE_RETURN);
	change=left_right(keyboard);

	if (up && selected>0) {
		selected--;
		Sound_play(S_select);
	} /* if */
	if (down && selected<n_items()-1) {
		selected++;
		Sound_play(S_select);
	} /* if */

	if (pressed(keyboard,SDL_SCANCODE_ESCAPE) || (confirm && selected==n_items()-1)) {
		go_back();
		if (STATE!=20) return;
		confirm=false;
		change=0;
	} /* if */

	switch(page) {
	case PAGE_MAIN:
		if (confirm) {
			page=selected+1;
			selected=0;
			hold_cycles=0;
			Sound_play(S_select);
			change=0;
		} /* if */
		break;

	case PAGE_AUDIO:
		if (selected==0 && change!=0) {
			music_volume=volume_from_percent(volume_percent(music_volume)+change*5);
		} /* if */
		if (selected==1 && change!=0) {
			sfx_volume=volume_from_percent(volume_percent(sfx_volume)+change*5);
			SetSFXVolume(sfx_volume);
			Sound_play(S_select);
		} /* if */
		break;

	case PAGE_VIDEO:
		if (selected==0 && (confirm || (change!=0 && hold_cycles==1))) {
			video_set_fullscreen(!video_get_fullscreen());
		} /* if */
		if (selected==1 && (change!=0 || confirm) && hold_cycles<=1) {
			int scale=video_get_window_scale()+(change<0 ? -1:1);
			int max_scale=video_max_window_scale();

			if (scale<1) scale=max_scale;
			if (scale>max_scale) scale=1;
			video_set_window_scale(scale);
		} /* if */
		if (selected==2 && (change!=0 || confirm) && hold_cycles<=1) {
			int filter=video_get_filter()+(change<0 ? -1:1);

			if (filter<0) filter=N_FILTERS-1;
			if (filter>=N_FILTERS) filter=FILTER_OFF;
			video_set_filter(filter);	/* OFF if the GPU can't run it */
		} /* if */
		break;

	case PAGE_CONTROLS:
		if (selected==0 && (change!=0 || confirm) && hold_cycles<=1) {
			/* -1 = keyboard, 0..MAX_GAMEPADS-1 = gamepads: */
			gamepad_device+=(change<0 ? -1:1);
			if (gamepad_device<-1) gamepad_device=MAX_GAMEPADS-1;
			if (gamepad_device>=MAX_GAMEPADS) gamepad_device=-1;
			Sound_play(S_select);
		} /* if */
		if (selected==1 && confirm) {
			start_key_redefinition(20);
			return;
		} /* if */
		break;
	} /* switch */

	/* Drawing: */
	switch(page) {
	case PAGE_MAIN:
		centered("OPTIONS",3,screen,dx,dy);
		item("AUDIO",7,0,screen,dx,dy);
		item("VIDEO",9,1,screen,dx,dy);
		item("CONTROLS",11,2,screen,dx,dy);
		item("BACK",14,3,screen,dx,dy);
		break;

	case PAGE_AUDIO:
		centered("AUDIO",3,screen,dx,dy);
		sprintf(tmp,"MUSIC VOLUME:      %3i",volume_percent(music_volume));
		item(tmp,7,0,screen,dx,dy);
		sprintf(tmp,"SFX VOLUME:        %3i",volume_percent(sfx_volume));
		item(tmp,9,1,screen,dx,dy);
		item("BACK",14,2,screen,dx,dy);
		break;

	case PAGE_VIDEO:
		centered("VIDEO",3,screen,dx,dy);
		sprintf(tmp,"FULLSCREEN:        %s",video_get_fullscreen() ? "ON":"OFF");
		item(tmp,7,0,screen,dx,dy);
		sprintf(tmp,"WINDOW SIZE:       %iX",video_get_window_scale());
		item(tmp,9,1,screen,dx,dy);
		sprintf(tmp,"FILTER:            %s",postfx_filter_name(video_get_filter()));
		item(tmp,11,2,screen,dx,dy);
		item("BACK",14,3,screen,dx,dy);
		break;

	case PAGE_CONTROLS:
		centered("CONTROLS",3,screen,dx,dy);
		device_name(gamepad_device,tmp2);
		sprintf(tmp,"DEVICE:      %s",tmp2);
		item(tmp,7,0,screen,dx,dy);
		if (gamepad_device>=0) {
			if (keyboard_gamepad_connected(gamepad_device)) {
				tile_print("CONNECTED",TILE_SIZE_X*19,TILE_SIZE_Y*8,screen,dx,dy);
			} else {
				tile_print("NOT CONNECTED:",TILE_SIZE_X*19,TILE_SIZE_Y*8,screen,dx,dy);
				tile_print("USING THE KEYBOARD",TILE_SIZE_X*19,TILE_SIZE_Y*9,screen,dx,dy);
			} /* if */
		} /* if */
		item("REDEFINE KEYS",11,1,screen,dx,dy);
		item("BACK",14,2,screen,dx,dy);
		if (gamepad_device>=0) {
			centered("GAMEPAD:  A SWORD   B WEAPON",17,screen,dx,dy);
			centered("X JUMP   Y ITEMS",18,screen,dx,dy);
			centered("START PAUSE   BACK MENU",19,screen,dx,dy);
		} /* if */
		centered("THE KEYBOARD ALWAYS WORKS",21,screen,dx,dy);
		break;
	} /* switch */

	if (page==PAGE_MAIN) {
		centered("UP:DOWN SELECT   SPACE OK",23,screen,dx,dy);
	} else {
		centered("LEFT:RIGHT CHANGE   ESC BACK",23,screen,dx,dy);
	} /* if */
} /* OptionsCycle */
