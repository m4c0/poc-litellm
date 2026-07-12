#include "../dir.h"
#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"

#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

static void find(const char * path, const char * filename, str_bld_t ** out) {
  dir_t dir;
  if (!dir_open(&dir, path)) return;

  while (dir_read(&dir)) {
    if (dir.is_dir) {
      find(dir.fullpath, filename, out);
    } else if (strcmp(dir.name, filename) == 0) {
      str_bld_cat(out, dir.fullpath + 2);
      str_bld_cat(out, "\n");
    }
  }

  dir_close(&dir);
}

static const char * exec(tll_call_t t) {
  const char * filename = jsn_str(jsn_find_element(t.json, "filename"));
  assert(filename && "missing 'filename' in 'find_local_file' arguments");

  fprintf(stderr, "find_local_file(%s)\n", filename);

  if (strchr(filename, '*')) return "Glob searching is not supported";

  str_bld_t * str = NULL;
  find(".", filename, &str);

  return str ? str_bld_flush(&str) : "File not found";
}

EXPORT void dudubot_tool(tll_api_t * api) {
  *api->t = (tll_t) {
    .desc =
      "Finds the path of a file in the current repository given a filename. "
      "Example: searching for 'MyCode.java' would be the semantically equivalent of 'find . -name MyCode.java'. ",
    .func = exec,
    .reqs = { "filename" },
    .props = {{
      .name = "filename",
      .type = "string",
      .desc = "Basename of the file to be searched.",
    }},
  };
}
