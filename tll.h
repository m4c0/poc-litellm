#ifndef TLL_H
#define TLL_H

typedef struct tll_prop_s {
  const char * name;
  const char * type;
  const char * desc;
} tll_prop_t;
typedef struct tll_s {
  const char * name;
  const char * desc;
  tll_prop_t props[10];
  const char * reqs[10];
} tll_t;

tll_t tll_list[] = {
  {
    .name = "view_local_file",
    .desc = "Reads the text contents of a file relative to the workspace directory.",
    .reqs = { "path" },
    .props = {{
      .name = "path",
      .type = "string",
      .desc = "The relative file path to read (e.g. 'src/index.js')",
    }},
  },
  {}
};

tll_t * tll_find(const char * name) {
  for (tll_t * t = tll_list; t->name; t++) {
    if (0 != strcmp(name, t->name)) continue;
    return t;
  }
  return NULL;
}

#endif
