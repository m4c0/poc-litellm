#include <assert.h>
#include <curl/curl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "crl.h"
#include "jsn.h"

static char * view_local_file(const char * path) {
  if (0 == strcmp(path, ".")) {
    DIR * d = opendir(".");
    assert(d);

    long dt = telldir(d);

    int len = 0;
    struct dirent * ent;
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      len += ent->d_namlen + 2; // slash+n
    }
    printf("%d\n", len);

    char * buf = malloc(len + 1); // null-terminator
    buf[0] = 0;
    seekdir(d, dt);
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      if (*buf) strlcat(buf, "\\n", len + 1);
      strlcat(buf, ent->d_name, len + 1);
    }
    printf("%s\n", buf);

    closedir(d);
    return buf;
  }
  return 0;
}

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
    msg_t * tool = wrt_msg + 1;
    for (msg_tool_call_t * c = wrt_msg->calls; c->id; c++) {
      tll_t * t = tll_find(c->name);
      assert(t && "tool not found"); // discard message and try again?

      assert(0 == strcmp(c->name, "view_local_file"));

      char * json = jsn_decode(c->args);

      json_object_t * root = jsn_parse_object(json, strlen(json));
      assert(root && "invalid tool call args");

      const char * path = jsn_str(jsn_find_element(root, "path"));
      assert(path && "missing 'path' in 'view_local_file' arguments");

      *tool++ = (msg_t) {
        .role = "tool",
        .call = strdup(c->id),
        .name = "view_local_file",
        .cont = view_local_file(path),
      };
    }
  }

  crl_fetch();
}

