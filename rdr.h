#ifndef RDR_H
#define RDR_H

typedef struct rdr_tool_prop_s {
  const char * name;
  const char * type;
  const char * desc;
} rdr_tool_prop_t;
typedef struct rdr_tool_s {
  const char * name;
  const char * desc;
  rdr_tool_prop_t props[10];
  const char * reqs[10];
  char * json;
} rdr_tool_t;

typedef struct rdr_tool_call_s {
  const char * id;
  const char * name;
  const char * args;
} rdr_tool_call_t;
typedef struct rdr_msg_s {
  const char * role;
  const char * call;
  const char * name;
  const char * cont;
  rdr_tool_call_t calls[10];
} rdr_msg_t;

rdr_tool_t rdr_tools[] = {
  {
    .name = "view_local_file",
    .desc = "Reads the text contents of a file relative to the workspace directory.",
    .reqs = { "path" },
    .props = {{
      .name = "path",
      .type = "string",
      .desc = "The relative file path to read (e.g. 'src/index.js')",
    }},
  },
  {}
};

rdr_msg_t rdr_msgs[] = {
  {
    .role = "user",
    .cont = "I want to find dead code in the current repository",
  }, {
    .role = "assistant",
    .calls = {{
      .id = "chatcmpl-tool-970c946da77d4697851ec2343f21c77d",
      .name = "view_local_file",
      .args = "{\\\"path\\\":\\\".\\\"}",
    }},
  }, {
    .role = "assistant",
    .cont = "Let me start by exploring the repository structure to understand the codebase.",
  }, {
    .role = "tool",
    .call = "chatcmpl-tool-970c946da77d4697851ec2343f21c77d",
    .name = "view_local_file",
    .cont = "main.c\\nmicroui.h",
  }, {
  }
};

char * rdr_txt;
char * rdr_end;
char * rdr_ptr;
static size_t rdr_fn(char * data, size_t sz, size_t n, void * ptr) {
  assert(sz == 1 && "Expecting libcurl to pass size=1 as documented");

  int i;
  for (i = 0; *rdr_ptr && i < n; i++, rdr_ptr++, data++) *data = *rdr_ptr;
  return i;
}

void rdr_sb_cat(const char * str) {
  strncat(rdr_ptr, str, rdr_end - rdr_ptr);
  rdr_ptr += strlen(str);
}
void rdr_sb_cat_str(const char * str) {
  rdr_sb_cat("\"");
  rdr_sb_cat(str);
  rdr_sb_cat("\"");
}
void rdr_sb_cat_k(const char * k) {
  rdr_sb_cat_str(k);
  rdr_sb_cat(":");
}
void rdr_sb_cat_kv(const char * k, const char * v) {
  rdr_sb_cat_k(k);
  rdr_sb_cat_str(v);
}
void rdr_sb_cat_kv_comma(const char * k, const char * v) {
  rdr_sb_cat_kv(k, v);
  rdr_sb_cat(",");
}

void rdr_reset() {
  if (!rdr_txt) {
    rdr_txt = malloc(1024000);
    rdr_end = rdr_txt + 1024000;
  }
  rdr_ptr = rdr_txt;

  rdr_sb_cat("{");
  rdr_sb_cat_kv_comma("model", "deepseek-v4-pro");
  rdr_sb_cat_k("stream");     rdr_sb_cat("true");  rdr_sb_cat(",");
  rdr_sb_cat_k("max_tokens"); rdr_sb_cat("10240"); rdr_sb_cat(",");
  rdr_sb_cat_k("tools");
  rdr_sb_cat("[");

  for (rdr_tool_t * t = rdr_tools; t->name; t++) {
    rdr_sb_cat("{"); {
      rdr_sb_cat_kv_comma("type", "function");
      rdr_sb_cat_k("function");
      rdr_sb_cat("{"); {
        rdr_sb_cat_kv_comma("name", t->name);
        rdr_sb_cat_kv_comma("description", t->desc);
        rdr_sb_cat_k("parameters");
        rdr_sb_cat("{"); {
          rdr_sb_cat_kv_comma("type", "object");
          rdr_sb_cat_k("required");
          rdr_sb_cat("["); {
            for (const char ** r = t->reqs; *r; r++) {
              if (r != t->reqs) rdr_sb_cat(",");
              rdr_sb_cat_str(*r);
            }
          }
          rdr_sb_cat("]");
          rdr_sb_cat(",");
          rdr_sb_cat_k("properties");
          rdr_sb_cat("{"); {
            for (rdr_tool_prop_t * p = t->props; p->name; p++) {
              if (p != t->props) rdr_sb_cat(",");
              rdr_sb_cat_k(p->name);
              rdr_sb_cat("{"); {
                rdr_sb_cat_kv_comma("type", p->type);
                rdr_sb_cat_kv("description", p->desc);
              }
              rdr_sb_cat("}");
            }
          }
          rdr_sb_cat("}");
        }
        rdr_sb_cat("}");
      }
      rdr_sb_cat("}");
    }
    rdr_sb_cat("}");
  }

  rdr_sb_cat("]"); // tools
  rdr_sb_cat(",");
  rdr_sb_cat_k("messages");
  rdr_sb_cat("[");

  for (rdr_msg_t * m = rdr_msgs; m->role; m++) {
    if (m != rdr_msgs) rdr_sb_cat(",");
    rdr_sb_cat("{"); {
      if (m->calls->id) {
        rdr_sb_cat_k("tool_calls");
        rdr_sb_cat("["); {
          for (rdr_tool_call_t * c = m->calls; c->name; c++) {
            if (c != m->calls) rdr_sb_cat(",");
            rdr_sb_cat("{"); {
              rdr_sb_cat_kv_comma("id", c->id);
              rdr_sb_cat_kv_comma("type", "function");
              rdr_sb_cat_k("function");
              rdr_sb_cat("{"); {
                rdr_sb_cat_kv_comma("name", c->name);
                rdr_sb_cat_kv("arguments", c->args);
              }
              rdr_sb_cat("}");
            }
            rdr_sb_cat("}");
          }
        }
        rdr_sb_cat("]");
        rdr_sb_cat(",");
      }

      if (m->call) rdr_sb_cat_kv_comma("tool_call_id", m->call);
      if (m->name) rdr_sb_cat_kv_comma("name",         m->name);
      if (m->cont) rdr_sb_cat_kv_comma("content",      m->cont);
      rdr_sb_cat_kv("role", m->role);
    }
    rdr_sb_cat("}");
  }

  rdr_sb_cat("]");

  rdr_sb_cat("}");

  puts(rdr_txt);
  rdr_ptr = rdr_txt;
}

#endif
