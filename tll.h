#ifndef TLL_H
#define TLL_H

#ifdef _WIN32
#  include <windows.h>
#  define dlclose FreeLibrary
#  define dlopen(X, F) LoadLibrary(X)
#  define dlsym(H, N) ((void *)GetProcAddress(H, N))
#else
#  include <dlfcn.h>
#  include <limits.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsn.h"
#include "tll_data.h"

tll_t * tll_head;
const char * tll_dudubot_exe;

void tll_init() {
  assert(!tll_dudubot_exe && "tll_init called twice");

#if _WIN32
  char exe[PATH_MAX];
  int len = GetModuleFileName(NULL, exe, PATH_MAX);
  if (len) tll_dudubot_exe = strdup(exe);
#else
  Dl_info addr;
  int res = dladdr(&tll_init, &addr);
  if (res) tll_dudubot_exe = strdup(addr.dli_fname);
#endif

  assert(tll_dudubot_exe && "tll_init failed to infer executable location");
}

void tll_purge() {
  tll_t * m = tll_head;
  while (m) {
    tll_t * mm = m;
    m = m->next;
    dlclose(mm->dl);
    free(mm);
  }
}
tll_t * tll_alloc() {
  if (tll_head == NULL) {
    return tll_head = calloc(sizeof(tll_t), 1);
  }
  tll_t * m = tll_head;
  while (m->next) m = m->next;
  return m->next = calloc(sizeof(tll_t), 1);
}

static void * tll__find_tool(const char * name) {
  // TODO: load from exe/tools/blah.so
  char buf[PATH_MAX];
#if _WIN32
  snprintf(buf, sizeof(buf), "%s.dll", name);
#elif __APPLE__
  snprintf(buf, sizeof(buf), "@rpath/lib%s.dylib", name);
#else
  snprintf(buf, sizeof(buf), "lib%s.so", name);
#endif

  return dlopen(buf, RTLD_LOCAL | RTLD_NOW);
}

typedef void (*tll_fn_t)(tll_api_t * t);
int tll_load(const char * name) {
  if (!tll_dudubot_exe) tll_init();

  void * dl = tll__find_tool(name);
  if (!dl) {
    fprintf(stderr, "could not load tool library named: %s\n", name);
    return 1;
  }

  tll_fn_t fn = dlsym(dl, "dudubot_tool");
  if (!fn) {
    fprintf(stderr, "tool library does not have 'dudubot_tool': %s\n", name);
    return 1;
  }

  tll_api_t api = {
    .magic = TLL_API_MAGIC_IN,
    .t = tll_alloc(),
    .dudubot_exe = tll_dudubot_exe,
  };
  fn(&api);

  if (api.magic != TLL_API_MAGIC_OUT) return log_error("invalid version magic from tool");

  // paranoid standardisation
  api.t->name = strdup(name);
  api.t->dl = dl;
  api.t->next = NULL;
  fprintf(stderr, "loaded tool %s\n", name);
  return 0;
}

tll_t * tll_find(const char * name) {
  for (tll_t * t = tll_head; t; t = t->next) {
    if (0 != strcmp(name, t->name)) continue;
    return t;
  }
  return NULL;
}

const char * tll_exec(const char * id, const char * name, const char * args) {
  tll_t * t = tll_find(name);
  if (!t) return "Tool not found";

  json_object_t * root = jsn_parse_object(args, strlen(args));
  if (!root) return "Invalid tool call args";

  tll_call_t ct = {
    .json = root,
  };
  return t->func(ct);
}

#endif
