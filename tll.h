#ifndef TLL_H
#define TLL_H

#include "tll_data.h"

#include <dlfcn.h>

tll_t * tll_head;

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

typedef void (*tll_fn_t)(tll_t * t);
int tll_load(const char * name) {
  // TODO: load relative to executable based on OS
  // TODO: handle extensions etc based on OS
  char buf[1024];
  snprintf(buf, sizeof(buf), "@rpath/lib%s.dylib", name);

  void * dl = dlopen(buf, RTLD_LOCAL | RTLD_NOW);
  if (!dl) {
    fprintf(stderr, "could not load tool library named: %s\n", buf);
    return 1;
  }

  tll_fn_t fn = dlsym(dl, "dudubot_tool");
  if (!fn) {
    fprintf(stderr, "tool library does not have 'dudubot_tool': %s\n", buf);
    return 1;
  }

  tll_t * t = tll_alloc();
  fn(t);

  // paranoid standardisation
  t->name = strdup(name);
  t->dl = dl;
  t->next = NULL;
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

#endif
