#include "stdio.h"
#include <SDL3/SDL.h>

#include "loadpcx.h"
#include "filehandling.h"

bool pcx_load_palette(const char *nombre,SDL_Color colors[256])
{
	FILE *fp;
	unsigned char pal[768];
	int i;

	fp=f1open(nombre,"rb",GAMEDATA);
	if (fp==NULL) return false;

	/* Cabecera: PCX de 8 bits por pixel. */ 
	if (fgetc(fp)!=10 || fgetc(fp)!=5 || fgetc(fp)!=1 || fgetc(fp)!=8) {
		fclose(fp);
		return false;
	} /* if */ 

	/* La paleta son los ultimos 768 bytes, precedidos por un 12: */ 
	if (fseek(fp,-769,SEEK_END)!=0 || fgetc(fp)!=12 || fread(pal,1,768,fp)!=768) {
		fclose(fp);
		return false;
	} /* if */ 
	fclose(fp);

	for(i=0;i<256;i++) {
		colors[i].r=pal[i*3];
		colors[i].g=pal[i*3+1];
		colors[i].b=pal[i*3+2];
		colors[i].a=255;
	} /* for */ 

	return true;
} /* pcx_load_palette */ 
