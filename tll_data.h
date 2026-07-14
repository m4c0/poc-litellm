#ifndef TLL_DATA_H
#define TLL_DATA_H

typedef struct tll_call_s {
  struct json_object_s * json;
} tll_call_t;

typedef struct tll_prop_s {
  const char * name;
  const char * type;
  const char * desc;
} tll_prop_t;
typedef struct tll_s {
  const char * name;
  const char * desc;
  const char * (*func)(tll_call_t);
  tll_prop_t props[10];
  const char * reqs[10];

  void * dl;
  struct tll_s * next;
} tll_t;

typedef struct tll_api_s {
  tll_t * t;
  const char * dudubot_exe;
} tll_api_t;

#endif
