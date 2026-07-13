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

  if (0 == strcmp(buf, "."   )) return 0;
  if (0 == strcmp(buf, "end" )) return 1;
  if (0 == strcmp(buf, "exit")) return 1;
  if (0 == strcmp(buf, "quit")) return 1;

  if (0 == strncmp(buf, "tool ", 5)) {
    if (msg_head) printf("WARNING: changing tools after conversation started nukes token caching\n");
    tll_load(buf + 5);
    return read_msg();
  }
  if (0 == strncmp(buf, "load ", 5)) {
    if (msg_load(buf + 5)) printf("failed to load messages\n");
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
static int loop(const char * session) {
  crl_fetch(session);

  const char * fini = wrt_msg->fini;

  if (!fini) {
    fprintf(stderr, "\nLLM ended without a concrete finish reason\n");
    return 0;
  }
  if (0 == strcmp(fini, "stop")) return 1;
  if (0 == strcmp(fini, "tool_calls")) {
    for (msg_tool_call_t * c = wrt_msg->calls; c; c = c->next) {
      *msg_alloc() = (msg_t) {
        .role = "tool",
        .call = strdup(c->id),
        .name = strdup(c->name),
        // expecting tools to return either a malloc'd string or a literal
        .cont = tll_exec(c->id, c->name, c->args),
      };
    }
    return loop(session);
  }

  return 0;
}

static int end() {
  tll_purge();
  return 0;
}

int main(int argc, char ** argv) {
  for (int i = 1; i < argc; i++) {
    if (0 == strcmp(argv[i], "-")) {
      assert(i + 1 == argc && "stdin marker should be last");
      if (msg_load_file(stdin)) return 1;
      wrt_quiet = 1;
      loop(NULL);
      return end();
    }
    else if (0 == strcmp(argv[i], ".")) {
      assert(i + 1 == argc && "run marker should be last");
      loop(NULL);
    }
    else if (msg_load(argv[i])) return 1;
  }

  char session[PATH_MAX];
  snprintf(session, PATH_MAX, "%s/dudubot-%ld.chat", getenv("TMPDIR"), time(NULL));
  do {
    if (read_msg()) return 0;
  } while (loop(session));

  return end();
}
