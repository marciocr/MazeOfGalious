#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "keyboardstate.h"
#include "sound.h"
#include "video.h"

#include "bitmaps.h"
#include "sprites.h"
#include "tiles.h"

#include "mog.h"

#include "debug.h"

extern void DebugReport(void);

#ifndef MOG_VERSION_STRING
#define MOG_VERSION_STRING "Maze of Galious"
#endif

int SCREEN_X = 640;
int SCREEN_Y = 400;
int COLOUR_DEPTH = 8;
int window_scale = 2;

#define TRANSPARANT_COLOR (0)

bool fullscreen = false;

// Redrawing constant
int REDRAWING_PERIOD = 40;

// Frames per second counter
float frames_per_sec = 0.0;
int frames_per_sec_tmp = 0;
int init_time = 0;

// Paths
char *default_g_path = "graphics/original/";
char *default_s_path = "sound/original/";
char **g_paths = 0;
int n_g_paths = 0, act_g_path = 0;
char *g_path = 0;
char **s_paths = 0;
int n_s_paths = 0, act_s_path = 0;
char *s_path = 0;

extern int music_volume, sfx_volume;
extern int fighting_demon;


/* Pantalla: */ 

SDL_Surface *screen;

void Render(SDL_Surface *surface);
SDL_Surface* initializeSDL(void);
void finalizeSDL();

int main(int argc, char** argv)
{

	int time, act_time;
	SDL_Event event;
    bool quit = false;
	bool redraw;

	screen = initializeSDL();

	if (screen == 0) {
		return 0;
	}

	time= init_time = SDL_GetTicks();

	GameInit(SCREEN_X, SCREEN_Y);

	SDL_FillSurfaceRect(screen, NULL, 0);
	while (!quit) {
	
		while(SDL_PollEvent(&event)) {
			keyboard_event(&event);

            switch(event.type) {
			
                // grab keyboard events
                case SDL_EVENT_KEY_DOWN:
					if (event.key.repeat) break;

                    // default quit: F12
                    if (event.key.scancode == SDL_SCANCODE_F12) {
                        quit = true;
                    } /* if */ 
										

                    if (event.key.scancode == SDL_SCANCODE_RETURN && IsAltPressed2()) {
						fullscreen = !fullscreen;
						video_set_fullscreen(fullscreen);
                    }
				
					// Change graphic set with either F10 or 9
					if (event.key.scancode == SDL_SCANCODE_F10 || event.key.scancode == SDL_SCANCODE_9) {

						act_g_path++;
						if (act_g_path >= n_g_paths) {
							act_g_path = 0;
						}
						
						g_path = g_paths[act_g_path];
						ReleaseGraphics();
						ReloadGraphics(SCREEN_X, SCREEN_Y);
						guardar_configuracion("MoG.cfg");
						
						if (fighting_demon != 0) {
							redo_demonintro(fighting_demon, 0, SCREEN_X, SCREEN_Y);
						}
					} 

					// Change sound set with either F11 or 0
					if (event.key.scancode == SDL_SCANCODE_F11 || event.key.scancode == SDL_SCANCODE_0) {

						act_s_path++;
						if (act_s_path >= n_s_paths) {
							act_s_path = 0;
						}
						
						s_path = s_paths[act_s_path];
						ReleaseSound(false);
						ReloadSound();
						music_recovery();
						guardar_configuracion("MoG.cfg");
						SetSFXVolume(sfx_volume);
					}

					if (event.key.scancode == SDL_SCANCODE_D) {
						write_debug_report("debug-report.txt");
					} 
                    break;

                case SDL_EVENT_GAMEPAD_ADDED:
                    keyboard_gamepad_added(event.gdevice.which);
                    break;

                case SDL_EVENT_GAMEPAD_REMOVED:
                    keyboard_gamepad_removed(event.gdevice.which);
                    break;

                // quit game and close window 
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
            }
        }

		Sound_update();
		
		// game main loop
		act_time = SDL_GetTicks();
		redraw = false;
        if (!quit && act_time - time >= REDRAWING_PERIOD) {
            int max_frame_step = 10;
            do {
                time += REDRAWING_PERIOD;
                if ((act_time - time) > 10 * REDRAWING_PERIOD) {
                    time = act_time;
				}

	
				// render graphics
				keyboard_update_gamepads();
				Render(screen);
				keyboard_cycle_done();
				redraw = true;
				
                act_time = SDL_GetTicks();
                max_frame_step--;
            } while (act_time - time >= REDRAWING_PERIOD && max_frame_step > 0);

        }
		if (redraw) {
			video_present(screen);
			frames_per_sec_tmp++;
		}
        if ((act_time - init_time) >= 1000) {
            frames_per_sec = frames_per_sec_tmp;
            frames_per_sec_tmp = 0;
            init_time = act_time;
        }
		
		SDL_Delay(1);		
	}

	finalizeSDL();

	return 0;
}


SDL_Surface* initializeSDL(void)
{
	SDL_Surface *screen;

	output_debug_message( "Initializing SDL.\n");

	if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD)) {
		output_debug_message( "Couldn't initialize SDL: %s\n", SDL_GetError());
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 0;
	}

    output_debug_message( "SDL video driver used: %s\n", SDL_GetCurrentVideoDriver());
	output_debug_message( "SDL audio driver used: %s\n", SDL_GetCurrentAudioDriver());

	Sound_initialization();

	if (!video_init(SCREEN_X, SCREEN_Y, fullscreen, window_scale)) {
		fprintf(stderr, "Couldn't open the window: %s\n", SDL_GetError());
		return 0;
	}

	/* The game draws palette indices into this surface (see render.cpp). */ 
	screen = SDL_CreateSurface(SCREEN_X, SCREEN_Y, SDL_PIXELFORMAT_INDEX8);
	if (screen == 0 || SDL_CreateSurfacePalette(screen) == 0) {
		output_debug_message( "Couldn't create the %ix%i screen surface: %s\n", SCREEN_X, SCREEN_Y, SDL_GetError());
		return 0;
	}

	return screen;
}

void finalizeSDL()
{
	ReleaseSound(true);
	ReleaseGraphics();
	Sound_release();
	keyboard_close_gamepads();
	SDL_DestroySurface(screen);
	video_release();
	SDL_Quit();
}
