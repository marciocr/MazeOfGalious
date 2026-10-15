#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NAME_MAX	256

static int compare_paths(const void *a,const void *b)
{
	return strcmp(*(char * const *)a,*(char * const *)b);
} /* compare_paths */ 

/* Paths: */ 

extern char **g_paths;
extern int n_g_paths,act_g_path;
extern char *g_path;
extern char **s_paths;
extern int n_s_paths,act_s_path;
extern char *s_path;


void init_paths(void) {

  int i,j;
  DIR *dd;
  struct dirent *direntry;
  
  if (g_paths==NULL) {	  
	  char **tmp_g_paths; 

	  n_g_paths=0;
	  
	  if((dd=opendir("graphics"))==NULL) {
	    perror("ERROR: Could not open directory \"graphics\"");
	    exit(1);
	  }
	  if ((direntry=readdir(dd))==NULL) {
	    perror("ERROR: Could not read directory \"graphics\"");
	    exit(1);
	  }
	  
	  while(direntry != NULL) {

	    // fprintf(stderr,"DIR: %s\n",direntry->d_name);
	    
	    if (strcmp(direntry->d_name,".")!=0 &&
		strcmp(direntry->d_name,"..")!=0) {
	      if (n_g_paths==0) {
		i=0;
		n_g_paths=1;
		g_paths=new char *[1];
		g_paths[i]=new char[NAME_MAX];
		sprintf(g_paths[i++],"graphics/%s/",direntry->d_name);
	      } else {
		tmp_g_paths=g_paths;
		n_g_paths++;
		g_paths=new char *[i+1];
		for(j=0;j<i;j++) {
		  g_paths[j]=tmp_g_paths[j];
		} /* for */ 
		delete[] tmp_g_paths;
		g_paths[i]=new char[NAME_MAX];
		sprintf(g_paths[i++],"graphics/%s/",direntry->d_name);
	      } /* if */ 
	    } /* if */ 
	    
	    direntry=readdir(dd);
	  } /* while */ 
	  closedir(dd);
	  
	  if (n_g_paths==0) {
	    fprintf(stderr,"ERROR: No directories for graphics found in graphics/* \n");
	    exit(1);
	  }
	  qsort(g_paths,n_g_paths,sizeof(char *),compare_paths);
  } /* if */ 


  if (s_paths==NULL) {	  
	  char **tmp_s_paths; 

	  n_s_paths=0;
	  
	  if((dd=opendir("sound"))==NULL) {
	    perror("ERROR: Could not open directory \"sound\"");
	    exit(1);
	  }
	  if ((direntry=readdir(dd))==NULL) {
	    perror("ERROR: Could not read directory \"sound\"");
	    exit(1);
	  }
	  
	  while(direntry != NULL) {

	    // fprintf(stderr,"DIR: %s\n",direntry->d_name);
	    
	    if (strcmp(direntry->d_name,".")!=0 &&
		strcmp(direntry->d_name,"..")!=0) {
	      if (n_s_paths==0) {
		i=0;
		n_s_paths=1;
		s_paths=new char *[1];
		s_paths[i]=new char[NAME_MAX];
		sprintf(s_paths[i++],"sound/%s/",direntry->d_name);
	      } else {
		tmp_s_paths=s_paths;
		n_s_paths++;
		s_paths=new char *[i+1];
		for(j=0;j<i;j++) {
		  s_paths[j]=tmp_s_paths[j];
		} /* for */ 
		delete[] tmp_s_paths;
		s_paths[i]=new char[NAME_MAX];
		sprintf(s_paths[i++],"sound/%s/",direntry->d_name);
	      } /* if */ 
	    } /* if */ 
	    
	    direntry=readdir(dd);
	  } /* while */ 
	  closedir(dd);
	  
	  if (n_s_paths==0) {
	    fprintf(stderr,"ERROR: No directories for sound found in sound/* \n");
	    exit(1);
	  }
	  /* readdir() order is arbitrary: sort so F10/F11 cycle in a fixed order. */ 
	  qsort(s_paths,n_s_paths,sizeof(char *),compare_paths);
  } /* if */ 

} /* init_paths */
