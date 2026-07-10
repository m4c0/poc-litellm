#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"
#include "../utl.h"

#include <stdio.h>

static const char * exec(const char * json) {
  json_object_t * root = jsn_parse_object(json, strlen(json));
  assert(root && "invalid tool call args");

  const char * path = jsn_str(jsn_find_element(root, "path"));
  assert(path && "missing 'path' in 'delete_lines' arguments");

  int line_start = jsn_atoi(jsn_find_element(root, "line_start"));
  assert(line_start && "missing 'line_start' in 'delete_lines' arguments");

  int line_end = jsn_atoi(jsn_find_element(root, "line_end"));
  assert(line_end && "missing 'line_end' in 'delete_lines' arguments");

  fprintf(stderr, "delete_lines(%s, %d, %d)\n", path, line_start, line_end);

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
    if (line < line_start || line > line_end) str_bld_cat(&str, buf);
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

void dudubot_tool(tll_t * t) {
  *t = (tll_t) {
    .desc =
      "Removes a section of the text content from a file in the workspace "
      "directory. The text between lines numbered 'line_start' and 'line_end' "
      "(inclusive) will be removed. Example - if view_local_file('file.txt') "
      "returns this:\n"
      "1: lorem\n"
      "2: ipsum\n"
      "3: dolor\n"
      "4: sit amet\n"
      "Then delete_lines('file.txt', line_start=2, line_end=3) follow by "
      "another view_local_file('file.txt') yields:\n"
      "1: lorem\n"
      "2: sit amet\n",
    .func = exec,
    .reqs = { "path", "line_start", "line_end" },
    .props = {{
      .name = "path",
      .type = "string",
      .desc = "The relative file path to update (e.g. 'src/index.js')",
    }, {
      .name = "line_start",
      .type = "integer",
      .desc = "The line number of the first line to be removed (inclusive)",
    }, {
      .name = "line_end",
      .type = "integer",
      .desc = "The line number of the last line to be removed (inclusive)",
    }},
  };
}
