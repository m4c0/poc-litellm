#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
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

    str_bld_t * str = NULL;

    struct dirent * ent;
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      str_bld_cat(&str, ent->d_name);
    }

    closedir(d);
    return str_bld_flush(&str);
  }

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

  char * buf = malloc(st.st_size + 1);

  FILE * f = fopen(path, "rb");
  if (!f) return "Unknown file name";
  assert(fread(buf, st.st_size, 1, f));
  fclose(f);

  for (int i = 0; i < st.st_size; i++) {
    if (buf[i] < 0x20 && buf[i] != '\n' && buf[i] != '\t' && buf[i] != '\r' && buf[i] != '\f') {
      free(buf);
      return "Binary file";
    }
  }

  buf[st.st_size] = 0;
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
