#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int run(char ** args) {
  assert(args && args[0]);

  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    abort();
  } else if (pid > 0) {
    int sl = 0;
    assert(0 <= waitpid(pid, &sl, 0));
    if (WIFEXITED(sl)) return WEXITSTATUS(sl);
  }

  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}
#define RUN(...) do { char * args[] = { __VA_ARGS__, 0 }; if (run(args)) return 1; } while (0)

#ifdef __APPLE__
#  define SO "dylib"
#elif _WIN32
#  define SO "dll"
#else
#  define SO "so"
#endif

int main() {
  RUN("clang", "-g", "-o", "dudubot", "dudubot.c", "-lcurl");

  RUN("clang", "-shared", "-g", "-o", "libview_local_file."SO, "tools/view_local_file.c");

  return 0;
}
