#ifndef TLL_VIEW_LOCAL_FILE_H
#define TLL_VIEW_LOCAL_FILE_H
#include "jsn.h"

static char * tll_view_local_file(char * args) {
  char * json = jsn_decode(args);

  json_object_t * root = jsn_parse_object(json, strlen(json));
  assert(root && "invalid tool call args");

  const char * path = jsn_str(jsn_find_element(root, "path"));
  assert(path && "missing 'path' in 'view_local_file' arguments");

  fprintf(stderr, "view_local_file(%s)\n", path);

  if (0 == strcmp(path, ".")) {
    DIR * d = opendir(".");
    assert(d);

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
      if (*buf) strlcat(buf, "\\n", len + 1);
      strlcat(buf, ent->d_name, len + 1);
    }

    closedir(d);
    return buf;
  }

  assert(*path != '/');
  for (const char * c = path; *c; c++) assert(*c != '/');

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

#endif
