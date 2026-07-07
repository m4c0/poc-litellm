#include <assert.h>
#include <curl/curl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "crl.h"
#include "jsn.h"

static char * view_local_file(const char * path) {
  fprintf(stderr, "view_local_file(%s)\n", path);

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

    char * buf = malloc(len + 1); // null-terminator
    buf[0] = 0;
    seekdir(d, dt);
    while ((ent = readdir(d))) {
      if (ent->d_name[0] == '.') continue;
      if (*buf) strlcat(buf, "\\n", len + 1);
      strlcat(buf, ent->d_name, len + 1);
    }

    closedir(d);
    return buf;
  }

  assert(*path != '/');
  for (const char * c = path; *c; c++) assert(*c != '/');

  FILE * f = fopen(path, "rb");
  if (!f) return "Unknown file name";

  fseek(f, 0, SEEK_END);
  int sz = ftell(f);
  char * buf = malloc(sz);
  assert(buf);
  fseek(f, 0, SEEK_SET);
  assert(fread(buf, sz, 1, f));
  fclose(f);

  int esz = 1;
  for (const char * c = buf; *c; c++) {
    switch (*c) {
      case '"':
      case '\\':
      case '\n': esz += 2; break;
      default:   esz += 1; break;
    }
  }
  char * enc = malloc(esz);
  for (char * c = buf, * e = enc; *c; c++, e++) {
    switch (*c) {
      case '"':  *e++ = '\\'; *e = '"';  break;
      case '\n': *e++ = '\\'; *e = 'n';  break;
      case '\\': *e++ = '\\'; *e = '\\'; break;
      default:   *e = *c; break;
    }
  }
  enc[esz - 1] = 0;

  free(buf);
  return enc;
}

static int read_msg(msg_t * msg) {
  printf("> "); fflush(stdout);

  char buf[10240];
  if (!fgets(buf, 10240, stdin)) return 1;

  if (0 == strcmp(buf, "a\n")) {
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

  msg->role = "user";
  msg->cont = calloc(10240, 1);
  wrt_esccat(msg->cont, buf, 10240);
  return 0;
}
static int cycle() {
  crl_fetch();

  const char * fini = wrt_msg->fini;

  if (!fini) {
    fputs("LLM ended without a concrete finish reason", stderr);
    return 0;
  }
  if (0 == strcmp(fini, "stop")) {
    if (read_msg(wrt_msg + 1)) return 0;
    return 1;
  }
  if (0 == strcmp(fini, "tool_calls")) {
    msg_t * tool = wrt_msg + 1;
    for (msg_tool_call_t * c = wrt_msg->calls; c && c->id; c++) {
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
    return 1;
  } else {
    fprintf(stderr, "finish reason = %s\n", fini);
    return 0;
  }
}

int main(int argc, char ** argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <domain-name> <api-key>\n\n", *argv);
    fprintf(stderr, "Example: %s litellm.mycompany.com \"$API_KEY\"\n\n", *argv);
    return 1;
  }

  crl_host = argv[1];
  crl_tkn = argv[2];
  read_msg(msg_convo);
  while (cycle()) {}
}
