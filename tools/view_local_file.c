#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"
#include "../utl.h"

#include <stdio.h>

static const char * exec(const char * json) {
  json_object_t * root = jsn_parse_object(json, strlen(json));
  assert(root && "invalid tool call args");

  const char * path = jsn_str(jsn_find_element(root, "path"));
  assert(path && "missing 'path' in 'view_local_file' arguments");

  fprintf(stderr, "view_local_file(%s)\n", path);

#if 0
  // Enable this to support listing current folder. This exposes more than what
  // the LLM should have access to (example: output files) and the
  // 'find_local_file' tool seems more suited for code navigation.
  //
  // LLMs might want to read "." to investigate the current repo as a whole,
  // but we can get a better mileage by telling the bot about the repo
  // directly. Either explicitly via instructions, or implicitly by asking it
  // to read a build file.

  if (0 == strcmp(path, ".")) {
    DIR * d = opendir(".");
    if (!d) return "listing files is not available. Try something else";

    str_bld_t * str = NULL;

    struct dirent * ent;
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      str_bld_cat(&str, ent->d_name);
    }

    closedir(d);
    return str_bld_flush(&str);
  }
#endif

  const char * res = utl_safe_to_read(path);
  if (res) return res;

  str_bld_t * str = NULL;

  char buf[10240];

  FILE * f = fopen(path, "rb");
  if (!f) return "Unknown file name";
  int line = 1;
  while (fgets(buf, sizeof(buf), f)) {
    int lcount = 0;
    for (char * c = buf; *c; c++) {
      if (*c < 0x20 && *c != '\n' && *c != '\t' && *c != '\r' && *c != '\f') return "Binary file";
      if (*c == '\n') lcount++;
    }
    assert(lcount == 1);
    char bb[20];
    snprintf(bb, 20, "%d: ", line++);
    str_bld_cat(&str, bb);
    str_bld_cat(&str, buf);
  }
  fclose(f);

  return str_bld_flush(&str);
}

EXPORT void dudubot_tool(tll_api_t * api) {
  *api->t = (tll_t) {
    .desc = "Reads the text contents of a file relative to the workspace directory.",
    .func = exec,
    .reqs = { "path" },
    .props = {{
      .name = "path",
      .type = "string",
      .desc = "The relative file path to read (e.g. 'src/index.js')",
    }},
  };
}
