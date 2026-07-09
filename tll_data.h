#ifndef TLL_DATA_H
#define TLL_DATA_H

typedef struct tll_prop_s {
  const char * name;
  const char * type;
  const char * desc;
} tll_prop_t;
typedef struct tll_s {
  const char * name;
  const char * desc;
  const char * (*func)(const char *);
  tll_prop_t props[10];
  const char * reqs[10];

  struct tll_s * next;
} tll_t;

#endif
