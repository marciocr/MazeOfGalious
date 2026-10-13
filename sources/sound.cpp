#include <SDL3/SDL.h>
#include "sound.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "MOGtypes.h"

#define SFX_TRACKS	16


/* Paths: */ 
extern char *default_g_path;
extern char *default_s_path;
extern char **g_paths;
extern int n_g_paths,act_g_path;
extern char *g_path;
extern char **s_paths;
extern int n_s_paths,act_s_path;
extern char *s_path;
extern int music_volume;

bool sound_enabled=false;

static MIX_Mixer *mixer=0;
static MIX_Track *sfx_tracks[SFX_TRACKS];
static float sfx_gain=1.0F;

/* Music is a sequence of up to 3 files: each one plays once and the last one
   loops (e.g. an intro followed by the main theme). */ 
static MIX_Track *music_track=0;
static int music_position=0;
static bool music_paused=false;
static bool music_stopping=false;
static int music_applied_volume=-1;
bool music_loaded[3]={false,false,false};
MIX_Audio *music_sound[3]={0,0,0};
char music_files[3][128];		/* File names */ 
char music_realfiles[3][128];	/* File names including music directory */ 


/* Plays the file at music_position: once if another file follows it, or
   looping forever if it is the last one of the sequence. */ 
static void music_play_position(MIX_Track *track)
{
	SDL_PropertiesID props;
	bool last=!(music_position<2 && music_loaded[music_position+1]);

	MIX_SetTrackAudio(track,music_sound[music_position]);
	props=SDL_CreateProperties();
	SDL_SetNumberProperty(props,MIX_PROP_PLAY_LOOPS_NUMBER,last ? -1:0);
	MIX_PlayTrack(track,props);
	SDL_DestroyProperties(props);
} /* music_play_position */ 


/* Runs on the mixer thread, with the mixer locked, when a music file ends:
   moves on to the next one in the sequence. */ 
static void music_track_stopped(void *userdata,MIX_Track *track)
{
	if (music_stopping) return;
	if (!(music_position<2 && music_loaded[music_position+1])) return;

	music_position++;
	if (music_sound[music_position]!=0) music_play_position(track);
} /* music_track_stopped */ 


bool Sound_initialization(void)
{
	SDL_AudioSpec spec;
	int i,v;

	sound_enabled=true;
	fprintf (stderr, "Initializing SDL_mixer.\n");

	if (!MIX_Init()) {
		fprintf (stderr, "Unable to initialize SDL_mixer: %s\n", SDL_GetError());
		sound_enabled=false;
		fprintf (stderr, "Running the game without audio.\n");
		return false;
	} /* if */ 

	spec.format=SDL_AUDIO_S16;
	spec.channels=2;
	spec.freq=44100;
	mixer=MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec);
	if (mixer==0) {
		fprintf (stderr, "Unable to open audio: %s\n", SDL_GetError());
		sound_enabled=false;
		fprintf (stderr, "Running the game without audio.\n");
		MIX_Quit();
		return false;
	} /* if */ 

	MIX_GetMixerFormat(mixer,&spec);
	v=MIX_Version();
	fprintf (stderr, "    opened audio at %d Hz, %d channels\n", spec.freq, spec.channels);
	fprintf (stderr, "    running with SDL_mixer version: %d.%d.%d\n",
			 SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));

	for(i=0;i<SFX_TRACKS;i++) sfx_tracks[i]=MIX_CreateTrack(mixer);
	music_track=MIX_CreateTrack(mixer);
	MIX_SetTrackStoppedCallback(music_track,music_track_stopped,0);

	return true;
} /* Sound_init */ 


void Sound_release(void)
{
	int i;

	if (sound_enabled) {
		Sound_release_music();
		for(i=0;i<SFX_TRACKS;i++) {
			MIX_DestroyTrack(sfx_tracks[i]);
			sfx_tracks[i]=0;
		} /* for */ 
		MIX_DestroyTrack(music_track);
		music_track=0;
		MIX_DestroyMixer(mixer);
		mixer=0;
		MIX_Quit();
	} /* if */ 
	sound_enabled=false;
} /* Sound_release */ 


/* a check to see if file is readable and greater than zero */
int file_check(char *fname)
{
	FILE *fp;

	if ((fp=fopen(fname,"r"))!=NULL) {
		if (fseek(fp,0L, SEEK_END)==0 && ftell(fp)>0) {
  			fclose(fp);
			return true;
		} /* if */ 
		/* either the file could not be read (==-1) or size was zero (==0) */ 
		fprintf(stderr,"ERROR in file_check(): the file %s is corrupted.\n", fname);
		fclose(fp);
		exit(1);
	} /* if */ 
	return false;
} /* file_check */ 


/* Finds file.(wav|ogg|mp3) in the current sound set, else in the default one. */ 
static bool Sound_find_file(char *file,char *fullname)
{
	int n_ext=6;
	char *ext[6]={".WAV",".OGG",".MP3",".wav",".ogg",".mp3"};
	int i;

	for(i=0;i<n_ext;i++) {
		sprintf(fullname,"%s%s%s",s_path,file,ext[i]);
		if (file_check(fullname)) return true;
	} /* for */ 
	for(i=0;i<n_ext;i++) {
		sprintf(fullname,"%s%s%s",default_s_path,file,ext[i]);
		if (file_check(fullname)) return true;
	} /* for */ 

	fullname[0]=0;
	return false;
} /* Sound_find_file */ 


SOUNDT Sound_create_sound(char *file,int flags)
{
	char name[256];
	SOUNDT s;

	if (sound_enabled) {
		if (!Sound_find_file(file,name)) {
			fprintf(stderr,"ERROR in Sound_create_sound(): Could not load sound file: %s%s.(wav|ogg|mp3)\n",s_path, file);
			exit(1);
		} /* if */ 
		s=MIX_LoadAudio(mixer,name,true);
		if (s==0) fprintf(stderr,"ERROR in Sound_create_sound(): %s: %s\n",name,SDL_GetError());
		return s;
	} else {
		return 0;
	} /* if */ 
} /* Sound_create_sound */ 


void Delete_sound(SOUNDT s)
{
	int i;

	if (sound_enabled && s!=0) {
		for(i=0;i<SFX_TRACKS;i++) {
			if (MIX_GetTrackAudio(sfx_tracks[i])==s) {
				MIX_StopTrack(sfx_tracks[i],0);
				MIX_SetTrackAudio(sfx_tracks[i],0);
			} /* if */ 
		} /* for */ 
		MIX_DestroyAudio(s);
	} /* if */ 
} /* Delete_sound */ 


void Sound_play(SOUNDT s)
{
	int i,oldest=0;
	Sint64 pos,oldest_pos=-1;

	if (!sound_enabled || s==0) return;

	/* A free track, or else the one that has been playing the longest: */ 
	for(i=0;i<SFX_TRACKS;i++) {
		if (!MIX_TrackPlaying(sfx_tracks[i])) {
			oldest=i;
			break;
		} /* if */ 
		pos=MIX_GetTrackPlaybackPosition(sfx_tracks[i]);
		if (pos>oldest_pos) {
			oldest_pos=pos;
			oldest=i;
		} /* if */ 
	} /* for */ 

	MIX_SetTrackAudio(sfx_tracks[oldest],s);
	MIX_SetTrackGain(sfx_tracks[oldest],sfx_gain);
	MIX_PlayTrack(sfx_tracks[oldest],0);
} /* Sound_play */ 


void Sound_set_sfx_volume(int volume)
{
	int i;

	if (volume<0) volume=0;
	if (volume>127) volume=127;
	sfx_gain=volume/127.0F;

	if (sound_enabled) {
		for(i=0;i<SFX_TRACKS;i++) MIX_SetTrackGain(sfx_tracks[i],sfx_gain);
	} /* if */ 
} /* Sound_set_sfx_volume */ 


void Sound_obtain_file_name(char *file,char *fullname)
{
	if (!Sound_find_file(file,fullname)) {
		fprintf(stderr,"ERROR in Sound_obtain_file_name(): Could not find sound file: %s%s.(wav|ogg|mp3)\n",s_path, file);
		exit(1);
	} /* if */ 
} /* Sound_obtain_file_name */ 


/* Stops the music track and frees the loaded files, keeping the file names
   (and the position in the sequence) so music_recovery() can reload them. */ 
static void Sound_free_music_streams(void)
{
	int i;

	MIX_LockMixer(mixer);
	music_stopping=true;
	MIX_StopTrack(music_track,0);
	MIX_SetTrackAudio(music_track,0);
	music_stopping=false;
	MIX_UnlockMixer(mixer);

	for(i=0;i<3;i++) {
		if (music_sound[i]!=0) MIX_DestroyAudio(music_sound[i]);
		music_sound[i]=0;
	} /* for */ 
} /* Sound_free_music_streams */ 


/* Loads the files of the music sequence (music_files/music_loaded) and starts
   playing it from music_position. */ 
static void Sound_start_music(void)
{
	int i;

	for(i=0;i<3;i++) {
		if (music_loaded[i]) {
			Sound_obtain_file_name(music_files[i],music_realfiles[i]);
			/* Not predecoded: music is decoded on the fly while it plays. */ 
			music_sound[i]=MIX_LoadAudio(mixer,music_realfiles[i],false);
			if (music_sound[i]==0) fprintf(stderr,"ERROR loading music %s: %s\n",music_realfiles[i],SDL_GetError());
		} /* if */ 
	} /* for */ 

	music_applied_volume=-1;
	Sound_update();
	if (music_loaded[music_position] && music_sound[music_position]!=0) {
		music_play_position(music_track);
		if (music_paused) MIX_PauseTrack(music_track);
	} /* if */ 
} /* Sound_start_music */ 


void Sound_create_music(char *f1,char *f2,char *f3)
{
	char *f[3]={f1,f2,f3};
	int i;

	if (sound_enabled) {
		Sound_free_music_streams();

		for(i=0;i<3;i++) {
			if (f[i]!=0) {
				strcpy(music_files[i],f[i]);
				music_loaded[i]=true;
			} else {
				music_files[i][0]=0;
				music_loaded[i]=false;
			} /* if */ 
		} /* for */ 

		music_position=0;
		music_paused=false;
		Sound_start_music();
	} /* if */ 
} /* Sound_create_music */ 


void Sound_subst_music(char *f)
{
	char tmp[256];

	Sound_obtain_file_name(f,tmp);
//	my_SubstMusic(tmp);
} /* Sound_sust_music */ 


void Sound_subst_music_now(char *f)
{
	Sound_release_music();
	Sound_create_music(f,0,0);
} /* Sound_sust_music_now */ 


void Sound_release_music(void)
{
	int i;

	if (sound_enabled) {
		Sound_free_music_streams();
		for(i=0;i<3;i++) music_loaded[i]=false;
	} /* if */ 
} /* Sound_release_music */ 


void Sound_temporary_release_music(void)
{
	if (sound_enabled) Sound_free_music_streams();
} /* Sound_temporary_release_music */ 


void Sound_pause_music(void)
{
	music_paused=true;
	if (sound_enabled) MIX_PauseTrack(music_track);
} /* Sound_pause_music */ 


void Sound_unpause_music(void)
{
	music_paused=false;
	if (sound_enabled) MIX_ResumeTrack(music_track);
} /* Sound_unpause_music */ 


/* Reloads the music after the sound set changed: */ 
void music_recovery(void)
{	
	if (sound_enabled) Sound_start_music();
} /* music_recovery */ 


void Sound_update(void)
{
	int v=music_volume;

	if (v<0) v=0;
	if (v>127) v=127;

	if (sound_enabled && v!=music_applied_volume) {
		MIX_SetTrackGain(music_track,v/127.0F);
		music_applied_volume=v;
	} /* if */ 
} /* Sound_update */ 
