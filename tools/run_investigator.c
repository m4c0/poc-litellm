#include "../jsn.h"
#include "../str.h"
#include "../tll_data.h"
#include "../utl.h"

#include <stdio.h>

#ifdef _WIN32
#  define popen _popen
#  define pclose _pclose
#endif

static const char * dudubot_exe;

static const char * exec(tll_call_t t) {
  const char * prompt = jsn_str(jsn_find_element(t.json, "prompt"));
  assert(prompt && "missing 'prompt' in 'run_investigator' arguments");

  char cmd[10240];
  snprintf(cmd, 10240, "%s -", dudubot_exe);
  FILE * job = popen(cmd, "r+");

  fputs("tool find_local_file\n", job);
  fputs("tool view_local_file\n", job);
  fputs("role user\n", job);
  fputs("cont\n", job);
  
  while (prompt && *prompt) {
    char * c = strchr(prompt, '\n');
    if (c) fprintf(job, "  %.*s", (int)(++c - prompt), prompt);
    else fprintf(job, "  %s\n", prompt);
    prompt = c;
  }

  fputs(".\n", job);
  fputs("end\n", job);

  str_bld_t * res = NULL;
  char buf[1024];
  while (fgets(buf, 1024, job)) str_bld_cat(&res, buf);

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
