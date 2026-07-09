#ifndef STR_H
#define STR_H

#include <stdlib.h>
#include <string.h>

typedef struct str_bld_s {
  char * str;
  int len;
  struct str_bld_s * next;
} str_bld_t;

void str_bld_cat(str_bld_t ** t, const char * str) {
  if (!*t) {
    *t = malloc(sizeof(str_bld_t));
    (*t)->str = strdup(str);
    (*t)->len = strlen(str);
    return;
  }

  str_bld_t * it = *t;
  while (it->next) it = it->next;

  it->next = malloc(sizeof(str_bld_t));
  it->next->str = strdup(str);
  it->next->len = strlen(str);
}
char * str_bld_flush(str_bld_t ** t) {
  if (!*t) return NULL;

  int len = 1;
  for (str_bld_t * it = *t; it; it = it->next) len += it->len;

  char * str = malloc(len);
  str[len - 1] = 0;

  char * ptr = str;
  str_bld_t * it = *t;
  while (it) {
    strcpy(ptr, it->str);
    ptr += it->len;

    str_bld_t * n = it->next;
    free(it->str);
    free(it);
    it = n;
  }

  *t = NULL;
  return str;
}

#endif
