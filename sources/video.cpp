#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "video.h"
#include "postfx.h"
#include "debug.h"

#ifndef MOG_VERSION_STRING
#define MOG_VERSION_STRING "Maze of Galious"
#endif

static SDL_Window *window=0;
static SDL_GLContext gl_context=0;
/* The frame as RGBA: at 640x400, and at 320x200 for the filters when it is
   doubled MSX art (see video_upload). */ 
static GLuint screen_tex=0,half_tex=0;
static Uint32 *rgba_buffer=0;
static int game_w=0,game_h=0;
/* The 640x400 image is shown on a 4:3 screen, as on the original monitors
   (pixels 1.2 times taller than wide): */ 
static int display_w=0,display_h=0;
static bool fullscreen=false;
static int window_scale=1;


void video_set_filter(int filter)
{
	postfx_set_filter(filter);
} /* video_set_filter */ 


int video_get_filter(void)
{
	return video_filter;
} /* video_get_filter */ 


SDL_Window *video_window(void)
{
	return window;
} /* video_window */ 


int video_max_window_scale(void)
{
	SDL_Rect r;
	int scale=1;

	if (window!=0 && SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(window),&r)) {
		while(display_w*(scale+1)<=r.w && display_h*(scale+1)<=r.h) scale++;
	} /* if */ 

	return scale;
} /* video_max_window_scale */ 


static void video_apply(void)
{
	SDL_SetWindowFullscreen(window,fullscreen);

	if (!fullscreen) {
		int max_scale=video_max_window_scale();

		if (window_scale>max_scale) window_scale=max_scale;
		if (window_scale<1) window_scale=1;
		SDL_SetWindowSize(window,display_w*window_scale,display_h*window_scale);
		SDL_SetWindowPosition(window,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);
	} /* if */ 

	SDL_SyncWindow(window);

	if (fullscreen) SDL_HideCursor();
			   else SDL_ShowCursor();
} /* video_apply */ 


static GLuint create_texture(int w,int h)
{
	GLuint tex;

	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D,tex);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D,0);

	return tex;
} /* create_texture */ 


bool video_init(int w,int h,bool fs,int scale)
{
	game_w=w;
	game_h=h;
	display_w=w;
	display_h=w*3/4;
	fullscreen=fs;
	window_scale=scale;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

	window=SDL_CreateWindow(MOG_VERSION_STRING,display_w*window_scale,display_h*window_scale,
							SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN);
	if (window==0) {
		output_debug_message("Couldn't create the window: %s\n",SDL_GetError());
		return false;
	} /* if */ 
	SDL_SetWindowMinimumSize(window,display_w/2,display_h/2);

	gl_context=SDL_GL_CreateContext(window);
	if (gl_context==0) {
		output_debug_message("Couldn't create the OpenGL context: %s\n",SDL_GetError());
		return false;
	} /* if */ 
	SDL_GL_MakeCurrent(window,gl_context);
	SDL_GL_SetSwapInterval(1);
	output_debug_message("OpenGL: %s / %s\n",(const char *)glGetString(GL_RENDERER),(const char *)glGetString(GL_VERSION));

	rgba_buffer=new Uint32[game_w*game_h];

	screen_tex=create_texture(game_w,game_h);
	half_tex=create_texture(game_w/2,game_h/2);

	if (!fullscreen) {
		int max_scale=video_max_window_scale();
		if (window_scale>max_scale) window_scale=max_scale;
	} /* if */ 
	video_apply();
	SDL_ShowWindow(window);

	return true;
} /* video_init */ 


void video_release(void)
{
	postfx_release();
	if (screen_tex!=0) glDeleteTextures(1,&screen_tex);
	if (half_tex!=0) glDeleteTextures(1,&half_tex);
	screen_tex=half_tex=0;
	delete []rgba_buffer;
	rgba_buffer=0;
	if (gl_context!=0) SDL_GL_DestroyContext(gl_context);
	gl_context=0;
	if (window!=0) SDL_DestroyWindow(window);
	window=0;
} /* video_release */ 


void video_set_fullscreen(bool fs)
{
	fullscreen=fs;
	video_apply();
} /* video_set_fullscreen */ 


bool video_get_fullscreen(void)
{
	return fullscreen;
} /* video_get_fullscreen */ 


void video_set_window_scale(int scale)
{
	window_scale=scale;
	video_apply();
} /* video_set_window_scale */ 


int video_get_window_scale(void)
{
	return window_scale;
} /* video_get_window_scale */ 


/* True if every 2x2 block of the frame is one colour: the original MSX
   graphics drawn at double size, which the pixel art filters must see at
   their real resolution to do anything. */ 
static bool video_doubled(SDL_Surface *screen)
{
	int x,y;

	for(y=0;y<game_h;y+=2) {
		Uint8 *a=(Uint8 *)screen->pixels+y*screen->pitch;
		Uint8 *b=a+screen->pitch;

		for(x=0;x<game_w;x+=2) {
			if (a[x]!=a[x+1] || a[x]!=b[x] || a[x]!=b[x+1]) return false;
		} /* for */ 
	} /* for */ 

	return true;
} /* video_doubled */ 


/* Converts the 8-bit screen through its palette into a texture, bottom row
   first (OpenGL's order, as the filters expect). Returns the texture and
   its size. */ 
static GLuint video_upload(SDL_Surface *screen,int *tex_w,int *tex_h)
{
	SDL_Palette *palette=SDL_GetSurfacePalette(screen);
	Uint32 lut[256];
	Uint8 *rgba;
	int i,x,y,step=1;
	GLuint tex=screen_tex;

	for(i=0;i<256;i++) {
		rgba=(Uint8 *)&lut[i];
		if (palette!=0 && i<palette->ncolors) {
			rgba[0]=palette->colors[i].r;
			rgba[1]=palette->colors[i].g;
			rgba[2]=palette->colors[i].b;
		} else {
			rgba[0]=rgba[1]=rgba[2]=0;
		} /* if */ 
		rgba[3]=255;
	} /* for */ 

	if (postfx_active() && video_doubled(screen)) {
		step=2;
		tex=half_tex;
	} /* if */ 
	*tex_w=game_w/step;
	*tex_h=game_h/step;

	for(y=0;y<*tex_h;y++) {
		Uint8 *src=(Uint8 *)screen->pixels+y*step*screen->pitch;
		Uint32 *dst=rgba_buffer+(*tex_h-1-y)*(*tex_w);

		for(x=0;x<*tex_w;x++) dst[x]=lut[src[x*step]];
	} /* for */ 

	glBindTexture(GL_TEXTURE_2D,tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,*tex_w,*tex_h,GL_RGBA,GL_UNSIGNED_BYTE,rgba_buffer);
	glBindTexture(GL_TEXTURE_2D,0);

	return tex;
} /* video_upload */ 


void video_present(SDL_Surface *screen)
{
	int window_w=game_w,window_h=game_h,w,h,tex_w,tex_h;
	GLuint tex;

	tex=video_upload(screen,&tex_w,&tex_h);

	SDL_GetWindowSizeInPixels(window,&window_w,&window_h);

	/* Where the image goes in the window: 4:3, centred, black bars around it. */ 
	if (window_w*display_h>window_h*display_w) {
		w=window_h*display_w/display_h;
		h=window_h;
	} else {
		w=window_w;
		h=window_w*display_h/display_w;
	} /* if */ 

	glViewport(0,0,window_w,window_h);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	if (postfx_active()) {
		postfx_draw(tex,tex_w,tex_h,(window_w-w)/2,(window_h-h)/2,w,h);
	} else {
		glViewport((window_w-w)/2,(window_h-h)/2,w,h);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,tex);
		glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(-1,-1);
		glTexCoord2f(1,0); glVertex2f(1,-1);
		glTexCoord2f(1,1); glVertex2f(1,1);
		glTexCoord2f(0,1); glVertex2f(-1,1);
		glEnd();
		glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_2D);
	} /* if */ 

	SDL_GL_SwapWindow(window);
} /* video_present */ 
