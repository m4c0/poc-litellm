#include "tll.h"

#include <assert.h>

int main(int argc, char ** argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <tool-name> <json>\n", argv[0]);
    return 1;
  }

  tll_dudubot_exe = "./dudubot";
  if (tll_load(argv[1])) return 1;

  puts(tll_exec("test-tool", argv[1], argv[2]));

  return 0;
}
