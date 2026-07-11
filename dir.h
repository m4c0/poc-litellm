#ifndef DIR_H
#define DIR_H

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dirent.h>
#endif

#include <stdio.h>

typedef struct dir_s {
#ifdef _WIN32
#else
  DIR * dir;
  struct dirent * d;
#endif

  char fullpath[PATH_MAX];
  const char * path;
  const char * name;
  int is_dir;
} dir_t;

int dir_open(dir_t * d, const char * path) {
  d->path = path;

#ifdef _WIN32
  return 0;
#else
  d->dir = opendir(path);
  return d->dir;
#endif
}

int dir_read(dir_t * d) {
#ifdef _WIN32
#else
  do {
    d->d = readdir(d->dir);
    if (!d->d) return 0;
  } while (d->d->name[0] == '.');

  snprintf(d->fullpath, PATH_MAX, "%s/%s", d->path, d->d->name);

  struct stat st;
  if (stat(fullpath, &st) != 0) return 0;
  d->is_dir = S_ISDIR(st.st_mode);
  d->name = d->d->name;
#endif

  return 1;
}
void dir_close(dir_t * d) {
#ifdef _WIN32
#else
  closedir(d);
#endif
}

#endif
