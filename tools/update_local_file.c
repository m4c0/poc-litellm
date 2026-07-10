
#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static const char * exec(const char * json) {
  json_object_t * root = jsn_parse_object(json, strlen(json));
  assert(root && "invalid tool call args");

  const char * path = jsn_str(jsn_find_element(root, "path"));
  assert(path && "missing 'path' in 'update_local_file' arguments");

  int line_start = jsn_atoi(jsn_find_element(root, "line_start"));
  assert(line_start && "missing 'line_start' in 'update_local_file' arguments");

  int line_end = jsn_atoi(jsn_find_element(root, "line_end"));
  assert(line_end && "missing 'line_end' in 'update_local_file' arguments");

  const char * original = jsn_str(jsn_find_element(root, "original"));
  assert(original && "missing 'original' in 'update_local_file' arguments");

  const char * replacement = jsn_str(jsn_find_element(root, "replacement"));
  assert(replacement && "missing 'replacement' in 'update_local_file' arguments");

  fprintf(stderr, "update_local_file(%s, %d, %d, [%s], [%s])\n",
      path, line_start, line_end, original, replacement);

  return "update failed";
}

void dudubot_tool(tll_t * t) {
  *t = (tll_t) {
    .desc =
      "Replaces a section of the text content of a file in the workspace "
      "directory. The text between line numbered 'line_start' and 'line_end' "
      "should match 'original'.",
    .func = exec,
    .reqs = { "path", "line_start", "line_end", "original", "replacement" },
    .props = {{
      .name = "path",
      .type = "string",
      .desc = "The relative file path to update (e.g. 'src/index.js')",
    }, {
      .name = "line_start",
      .type = "integer",
      .desc = "The line number of the first line to be replaced (inclusive)",
    }, {
      .name = "line_end",
      .type = "integer",
      .desc = "The line number of the last line to be replaced (inclusive)",
    }, {
      .name = "original",
      .type = "string",
      .desc = "Original content to be replaced.",
    }, {
      .name = "replacement",
      .type = "string",
      .desc = "Content to replaced the original text.",
    }},
  };
}
