#ifndef JSN_H
#define JSN_H
#include "json.h"

static json_value_t * jsn_last_root;

static json_object_t * jsn_parse_object(const char * str, int len) {
  if (jsn_last_root) free(jsn_last_root);

  json_value_t * root = json_parse(str, strlen(str));
  return root ? json_value_as_object(root) : NULL;
}

static json_value_t * jsn_find_element(json_object_t * obj, const char * el) {
  for (json_object_element_t * it = obj->start; it; it = it->next) 
    if (0 == strcmp(el, it->name->string)) return it->value;

  return NULL;
}

static const char * jsn_str(json_value_t * v) {
  json_string_t * str = v ? json_value_as_string(v) : NULL;
  return str ? str->string : NULL;
}

#endif
