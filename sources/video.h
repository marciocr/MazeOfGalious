#ifndef __MOG_VIDEO_H
#define __MOG_VIDEO_H

/* The game draws palette indices into an 8-bit SDL_Surface; every frame
   video_present() converts it through the palette into an OpenGL texture and
   draws it in the window, scaled and letterboxed to a 4:3 shape. */

bool video_init(int game_w,int game_h,bool fullscreen,int window_scale);
void video_release(void);

void video_present(SDL_Surface *screen);

void video_set_fullscreen(bool fullscreen);
bool video_get_fullscreen(void);
void video_set_window_scale(int scale);
int video_get_window_scale(void);
int video_max_window_scale(void);

SDL_Window *video_window(void);

#endif
