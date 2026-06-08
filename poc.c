#include <assert.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "crl.h"

int main(int argc, char ** argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <domain-name> <api-key>\n\n", *argv);
    fprintf(stderr, "Example: %s litellm.mycompany.com \"$API_KEY\"\n\n", *argv);
    return 1;
  }

  crl_host = argv[1];
  crl_tkn = argv[2];
  crl_fetch();

  const char * fini = wrt_msg->fini;

  if (!fini) {
    fputs("LLM ended without a concrete finish reason", stderr);
    return 1;
  }
  if (0 == strcmp(fini, "tool_calls")) {
    for (msg_tool_call_t * c = wrt_msg->calls; c->id; c++) {
      tll_t * t = tll_find(c->name);
      assert(t && "tool not found"); // discard message and try again?

      ttl_args_t args = {0};
      int argc = ttl_parse_args(t, c->args, &args);

      for (int i = 0; i < argc; i++) {
        printf("%s %s\n", args.list[i].name, args.list[i].value);
      }

      printf("%s %s %s %s\n", c->id, c->name, c->args, t->desc);
    }
    return 1;
  }

  crl_fetch();
}

