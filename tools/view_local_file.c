#include "../jsn.h"
#include "../tll_data.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

static const char * exec(const char * args) {
  char * json = jsn_decode(args);

  json_object_t * root = jsn_parse_object(json, strlen(json));
  assert(root && "invalid tool call args");

  const char * path = jsn_str(jsn_find_element(root, "path"));
  assert(path && "missing 'path' in 'view_local_file' arguments");

  fprintf(stderr, "view_local_file(%s)\n", path);

  if (0 == strcmp(path, ".")) {
    DIR * d = opendir(".");
    if (!d) return "listing files is not available. Try something else";

    long dt = telldir(d);

    int len = 0;
    struct dirent * ent;
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      len += ent->d_namlen + 2; // slash+n
    }

    char * buf = malloc(len + 1); // null-terminator
    buf[0] = 0;
    seekdir(d, dt);
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      if (*buf) strlcat(buf, "\n", len + 1);
      strlcat(buf, ent->d_name, len + 1);
    }

    closedir(d);
    return buf;
  }

  if (*path == '/' || *path == '\\')
    return "You cannot use absolute paths or paths starting with a '/'";

  char cwd[PATH_MAX];
  if (!getcwd(cwd, sizeof(cwd))) return "Tool failed to get CWD. Inform the user.";
  char real[PATH_MAX];
  if (!realpath(path, real)) return "Tool failed to resolve file. Inform the user.";

  if (0 != strncmp(cwd, real, strlen(cwd))) return "Access outside current directory is not permitted.";

  FILE * f = fopen(path, "rb");
  if (!f) return "Unknown file name";

  fseek(f, 0, SEEK_END);
  int sz = ftell(f);
  char * buf = malloc(sz);
  assert(buf);
  fseek(f, 0, SEEK_SET);
  assert(fread(buf, sz, 1, f));
  fclose(f);

  return buf;
}

void dudubot_tool(tll_t * t) {
  *t = (tll_t) {
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
