#ifndef DIR_H
#define DIR_H

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dirent.h>
#  include <limits.h>
#  include <sys/stat.h>
#endif

#include <stdio.h>

typedef struct dir_s {
#ifdef _WIN32
  HANDLE h;
  WIN32_FIND_DATA fd;
  int first;
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
  char pat[PATH_MAX];
  snprintf(pat, PATH_MAX, "%s/*.*", path);
  d->h = FindFirstFile(pat, &d->fd);
  d->first = d->h != INVALID_HANDLE_VALUE;
  return d->first;
#else
  d->dir = opendir(path);
  return d->dir ? 1 : 0;
#endif
}

int dir_read(dir_t * d) {
#ifdef _WIN32
  do {
    if (d->first) d->first = 0;
    else if (!FindNextFile(d->h, &d->fd)) return 0;
  } while (d->fd.cFileName[0] == '.');

  snprintf(d->fullpath, PATH_MAX, "%s/%s", d->path, d->fd.cFileName);
  d->name = d->fd.cFileName;
  d->is_dir = (d->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  do {
    d->d = readdir(d->dir);
    if (!d->d) return 0;
  } while (d->d->d_name[0] == '.');

  snprintf(d->fullpath, PATH_MAX, "%s/%s", d->path, d->d->d_name);

  struct stat st;
  if (stat(d->fullpath, &st) != 0) return 0;
  d->is_dir = S_ISDIR(st.st_mode);
  d->name = d->d->d_name;
#endif

  return 1;
}
void dir_close(dir_t * d) {
#ifdef _WIN32
  FindClose(d->h);
#else
  closedir(d->dir);
#endif
}

#endif
