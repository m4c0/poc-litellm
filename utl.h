#ifndef UTL_H
#define UTL_H

const char * utl_env(const char * name, const char * def) {
  char * env = getenv(name);
  return env ? strdup(env) : def;
}

#endif
