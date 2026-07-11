#ifdef _WIN32
#  define _CRT_SECURE_NO_WARNINGS
#  define _CRT_NONSTDC_NO_WARNINGS
#  include <process.h>
#else
#  include <unistd.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int run(char ** args) {
  assert(args && args[0]);
#ifdef _WIN32
  if (0 == _spawnvp(_P_WAIT, args[0], (const char * const *)args)) {
    return 0;
  }
#else
  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    abort();
  } else if (pid > 0) {
    int sl = 0;
    assert(0 <= waitpid(pid, &sl, 0));
    if (WIFEXITED(sl)) return WEXITSTATUS(sl);
  }
#endif
  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}
#define RUN(...) do { char * args[] = { __VA_ARGS__, 0 }; if (run(args)) return 1; } while (0)

#ifdef __APPLE__
#  define LIB "lib"
#  define SO ".dylib"
#elif _WIN32
#  define LIB ""
#  define SO ".dll"
#else
#  define LIB "lib"
#  define SO ".so"
#endif
#define TOOL(X) RUN("clang", "-shared", "-g", "-o", LIB X SO, "tools/"X".c")

int main() {
  RUN("clang", "-g", "-o", "dudubot", "dudubot.c", "-lcurl", "-rpath", "@executable_path",
#ifdef _WIN32
      "-D_CRT_SECURE_NO_WARNINGS", "-D_CRT_NONSTDC_NO_WARNINGS",
#endif
      getenv("CFLAGS"));

  TOOL("find_local_file");
  TOOL("update_local_file");
  TOOL("view_local_file");

  return 0;
}
