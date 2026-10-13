#ifndef __LOADPCX_HEADER
#define __LOADPCX_HEADER

/* Reads the 256-colour palette at the end of an 8-bit PCX file, at full
   8-bit precision (Bitmap::B_loadpcx keeps only the top 6 bits). */ 
bool pcx_load_palette(const char *nombre,SDL_Color colors[256]);


#endif
