#ifndef UTL_H
#define UTL_H

#ifdef _WIN32
#  include <direct.h>
#  include <windows.h>
#  define getcwd _getcwd
#  define realpath(P, R) _fullpath(R, P, MAX_PATH)
#else
#  include <unistd.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char * utl_env(const char * name, const char * def) {
  char * env = getenv(name);
  return env ? strdup(env) : def;
}

const char * utl_safe_to_read(const char * path) {
  if (*path == '/' || *path == '\\')
    return "You cannot use absolute paths or paths starting with a '/'";

  char cwd[PATH_MAX];
  if (!getcwd(cwd, sizeof(cwd))) return "Tool failed to get CWD. Inform the user.";
  char real[PATH_MAX];
  if (!realpath(path, real)) return "Tool failed to resolve file.";

  if (0 != strncmp(cwd, real, strlen(cwd))) return "Access outside current directory is not permitted.";

  struct stat st;
  if (0 != stat(path, &st)) return "File not found.";
  if (st.st_mode & S_IFDIR) return "This is a directory.";

  return NULL;
}

#endif
