#ifndef __BRAIN_SDL_SOUND
#define __BRAIN_SDL_SOUND

#include <SDL3_mixer/SDL_mixer.h>

typedef MIX_Audio * SOUNDT;

bool Sound_initialization(void);
void Sound_release(void);

SOUNDT Sound_create_sound(char *file,int flags);
void Delete_sound(SOUNDT s);
void Sound_play(SOUNDT s);
void Sound_set_sfx_volume(int volume);

void Sound_create_music(char *f1,char *f2,char *f3);
void Sound_subst_music(char *f);
void Sound_subst_music_now(char *f);
void Sound_temporary_release_music(void);
void Sound_release_music(void);
void Sound_pause_music(void);
void Sound_unpause_music(void);

void music_recovery(void);

/* Called once per main loop iteration: applies music_volume changes. */ 
void Sound_update(void);

#endif
