#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"
#include "../utl.h"

#include <stdio.h>

static const char * exec(const char * json) {
  json_object_t * root = jsn_parse_object(json, strlen(json));
  assert(root && "invalid tool call args");

  const char * path = jsn_str(jsn_find_element(root, "path"));
  assert(path && "missing 'path' in 'append_lines' arguments");

  int line_p = jsn_atoi(jsn_find_element(root, "line"));
  //assert(line_p && "missing 'line' in 'append_lines' arguments");

  const char * text = jsn_str(jsn_find_element(root, "text"));
  assert(text && "missing 'text' in 'append_lines' arguments");

  fprintf(stderr, "append_lines(%s, %d, %s)\n", path, line_p, text);

  const char * res = utl_safe_to_read(path);
  if (res) return res;

  str_bld_t * str = NULL;

  char buf[10240];

  FILE * f = fopen(path, "rb");
  if (!f) return "Unknown file name";
  int line = 1;
  if (0 == line_p) {
    str_bld_cat(&str, text);
    str_bld_cat(&str, "\n");
  }
  while (fgets(buf, sizeof(buf), f)) {
    int lcount = 0;
    for (char * c = buf; *c; c++) {
      if (*c < 0x20 && *c != '\n' && *c != '\t' && *c != '\r' && *c != '\f') return "Binary file";
      if (*c == '\n') lcount++;
    }
    assert(lcount == 1);
    str_bld_cat(&str, buf);
    if (line == line_p) {
      str_bld_cat(&str, text);
      str_bld_cat(&str, "\n");
    }
    line++;
  }
  fclose(f);

  char * s = str_bld_flush(&str);

  f = fopen(path, "wb");
  if (!f) return "failed to update file";
  fputs(s, f);
  fclose(f);

  free(s);

  return "file updated";
}

EXPORT void dudubot_tool(tll_api_t * api) {
  *api->t = (tll_t) {
    .desc =
      "Appends a new section of text in a file in the workspace directory. "
      "The new text will be appended after the line informed as parameter and "
      "may contain newlines. Example - if view_local_file('file.txt') "
      "returns this:\n"
      "1: lorem\n"
      "2: dolor\n"
      "Then append_lines('file.txt', line=1, text='ipsum') followed by "
      "another view_local_file('file.txt') yields:\n"
      "1: lorem\n"
      "2: ipsum\n"
      "3: dolor\n"
      "Or an append_lines('file.txt', line=0, text='sit\\namet') yields: "
      "1: sit\n"
      "2: amet\n"
      "3: lorem\n"
      "4: dolor\n",
    .func = exec,
    .reqs = { "path", "line", "text" },
    .props = {{
      .name = "path",
      .type = "string",
      .desc = "The relative file path to update (e.g. 'src/index.js')",
    }, {
      .name = "line",
      .type = "integer",
      .desc = "The line number after which the text will be appended (or zero for beginning of file)",
    }, {
      .name = "text",
      .type = "string",
      .desc = "Text to be appended",
    }},
  };
}
