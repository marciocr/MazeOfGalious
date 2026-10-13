#include <unistd.h>
#include "filehandling.h"

// make (sub)directories including multiple subdirs
int mkdirp(const char *fqfn, mode_t mode) {
  char *t, str[STRLEN];
  struct stat stbuf;
  int len;

  t=(char *)fqfn;
  memset(str, '\0', STRLEN);

  while (*t) {
    if (*t == '/') {
      len = t-fqfn;
      if ((len < STRLEN) && (len > 0)) {
        strncpy(str, fqfn, len);
        if (stat(str, &stbuf) != 0)
          if (mkdir(str, mode) != 0) return(-1);
      }
    }
    t++;
  }

  return(0);
}

// where a user file lives
static void user_data_path(char *fname, const char *f) {
  const char *xdg = getenv("XDG_DATA_HOME");

  if (xdg != NULL && xdg[0] == '/')
    snprintf(fname, STRLEN-1, "%s/%s/%s", xdg, USERDATA_DIR, f);
  else
    snprintf(fname, STRLEN-1, "%s/.local/share/%s/%s", getenv("HOME"), USERDATA_DIR, f);
}

// new fopen()
FILE *f1open(const char *f, const char *m, const enum filetype t) {
  char fname[STRLEN];

  switch (t) {
    case GAMEDATA:
      // gamedata is read-only
      return(fopen(f, m));
      break;

    case USERDATA:
      // userdata is put in $XDG_DATA_HOME/mazeofgalious/ (by default
      // ~/.local/share/mazeofgalious/; inside the Flatpak, the app's own data
      // directory). Files not there yet are read from the old $HOME/.mog/.
      user_data_path(fname, f);
      if (m[0] == 'r' && access(fname, F_OK) != 0) {
        snprintf(fname, STRLEN-1, "%s/.%s/%s", getenv("HOME"), GAMENAME, f);
        return(fopen(fname, m));
      }
      // create subdirs if they don't exist
      mkdirp(fname, S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
      // open file
      return(fopen(fname, m));
      break;
  }

  // should not be reached
  return(NULL);
}

