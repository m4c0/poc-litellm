#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"
#include "../utl.h"

#include <stdio.h>

static const char * dudubot_exe;

static const char * exec(tll_call_t t) {
  const char * prompt = jsn_str(jsn_find_element(t.json, "prompt"));
  assert(prompt && "missing 'prompt' in 'run_investigator' arguments");

  FILE * job = popen(dudubot_exe, "r+");

  fputs("tool find_local_file\n", job);
  fputs("tool view_local_file\n", job);
  fputs("a\n", job);
  fputs(prompt, job);
  fputs("\n.\n", job);
  fputs("exit\n", job);

  str_bld_t * res = NULL;
  char buf[1024];
  while (fgets(buf, 1024, job)) {
    fprintf(stderr, "  agent: %s", buf);
    str_bld_cat(&res, buf);
  }
  fprintf(stderr, "\n");

  pclose(job);

  return str_bld_flush(&res);
}

EXPORT void dudubot_tool(tll_api_t * api) {
  *api->t = (tll_t) {
    .desc =
      "Runs an agent capable of investigating the current codebase.\n",
    .func = exec,
    .reqs = { "prompt" },
    .props = {{
      .name = "prompt",
      .type = "string",
      .desc = "Prompt to be sent to the agent",
    }},
  };
  dudubot_exe = api->dudubot_exe;
}
