#include <assert.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "minirent.h"

#include "crl.h"
#include "jsn.h"

static int read_msg(void) {
  printf("> "); fflush(stdout);

  char buf[10240];
  if (!fgets(buf, 10240, stdin)) return 1;
  buf[strlen(buf) - 1] = 0;

  if (0 == strlen(buf)) return read_msg();

  if (0 == strcmp(buf, ".")) return 0;

  if (0 == strncmp(buf, "tool ", 5)) {
    if (msg_head) {
      printf("cannot change tools after conversation started\n");
      return read_msg();
    }
    tll_load(buf + 5);
    return read_msg();
  }
  if (0 == strncmp(buf, "load ", 5)) {
    if (msg_load(buf + 5, 1)) printf("failed to load messages\n");
    return read_msg();
  }
  if (0 == strncmp(buf, "save ", 5)) {
    if (msg_save(buf + 5)) printf("failed to save messages\n");
    return read_msg();
  }

  if (0 == strcmp(buf, "a")) {
    char line[1024];
    char * ptr = buf;
    while (1) {
      printf(". "); fflush(stdout);

      if (!fgets(line, 1024, stdin)) return 1;
      if (0 == strcmp(line, ".\n")) break;
      for (int i = 0; line[i] && i < 1024 && ptr < buf + 10240; i++) *ptr++ = line[i];
    }
    *ptr = 0;
  }

  msg_t * msg = msg_alloc();
  msg->role = "user";
  msg->cont = strdup(buf);
  return 0;
}
static int cycle(void) {
  crl_fetch();

  const char * fini = wrt_msg->fini;

  if (!fini) {
    fprintf(stderr, "\nLLM ended without a concrete finish reason\n");
    return 0;
  }
  if (0 == strcmp(fini, "stop")) {
    if (read_msg()) return 0;
    return 1;
  }
  if (0 == strcmp(fini, "tool_calls")) {
    for (msg_tool_call_t * c = wrt_msg->calls; c; c = c->next) {
      tll_t * t = tll_find(c->name);
      assert(t && "tool not found"); // discard message and try again?

      *msg_alloc() = (msg_t) {
        .role = "tool",
        .call = strdup(c->id),
        .name = strdup(c->name),
        // expecting tools to return either a malloc'd string or a literal
        .cont = t->func(c->args),
      };
    }
    return 1;
  } else {
    fprintf(stderr, "finish reason = %s\n", fini);
    return 0;
  }
}

static int end() {
#ifndef _WIN32
  msg_save("/tmp/dudubot_last_session");
#endif
  tll_purge();
  return 0;
}

int main(int argc, char ** argv) {
  for (int i = 1; i < argc; i++) {
    if (0 != strcmp(argv[i], ".")) msg_load(argv[i], 0);
    else if (!cycle()) return end();
  }

  if (read_msg()) return 0;

  while (cycle()) {}

  return end();
}
