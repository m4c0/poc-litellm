#ifndef ENC_H
#define ENC_H

#include "msg.h"
#include "utl.h"
#include "tll.h"

char * enc_txt;
char * enc_ptr;
char * enc_end;

void enc_sb_cat(const char * str) {
  for (; *str && enc_end > enc_ptr + 1; str++, enc_ptr++) *enc_ptr = *str;
}
void enc_sb_cat_str(const char * str) {
  enc_sb_cat("\"");
  for (; *str && enc_end > enc_ptr + 1; str++) {
    switch (*str) {
      case '\"': enc_sb_cat("\\\""); break;
      case '\\': enc_sb_cat("\\\\"); break;
      case '\n': enc_sb_cat("\\n" ); break;
      case '\r': enc_sb_cat("\\r" ); break;
      case '\t': enc_sb_cat("\\t" ); break;
      default: *enc_ptr++ = *str; break;
    }
  }
  enc_sb_cat("\"");
}
void enc_sb_cat_k(const char * k) {
  enc_sb_cat_str(k);
  enc_sb_cat(":");
}
void enc_sb_cat_kv(const char * k, const char * v) {
  enc_sb_cat_k(k);
  enc_sb_cat_str(v);
}
void enc_sb_cat_kv_comma(const char * k, const char * v) {
  enc_sb_cat_kv(k, v);
  enc_sb_cat(",");
}

const char * enc_model;
void enc_reset() {
  if (!enc_txt) {
    enc_txt = malloc(1024000);
    enc_end = enc_txt + 1024000;
  }
  enc_ptr = enc_txt;

  if (!enc_model) enc_model = utl_env("DUDUBOT_MODEL", "deepseek-v4-flash");

  enc_sb_cat("{");
  enc_sb_cat_kv_comma("model", enc_model);
  enc_sb_cat_k("stream");     enc_sb_cat("true");  enc_sb_cat(",");
  enc_sb_cat_k("max_tokens"); enc_sb_cat("10240"); enc_sb_cat(",");
  enc_sb_cat_k("tools");
  enc_sb_cat("[");

  for (tll_t * t = tll_head; t; t = t->next) {
    if (t != tll_head) enc_sb_cat(",");
    enc_sb_cat("{"); {
      enc_sb_cat_kv_comma("type", "function");
      enc_sb_cat_k("function");
      enc_sb_cat("{"); {
        enc_sb_cat_kv_comma("name", t->name);
        enc_sb_cat_kv_comma("description", t->desc);
        enc_sb_cat_k("parameters");
        enc_sb_cat("{"); {
          enc_sb_cat_kv_comma("type", "object");
          enc_sb_cat_k("required");
          enc_sb_cat("["); {
            for (const char ** r = t->reqs; *r; r++) {
              if (r != t->reqs) enc_sb_cat(",");
              enc_sb_cat_str(*r);
            }
          }
          enc_sb_cat("]");
          enc_sb_cat(",");
          enc_sb_cat_k("properties");
          enc_sb_cat("{"); {
            for (tll_prop_t * p = t->props; p->name; p++) {
              if (p != t->props) enc_sb_cat(",");
              enc_sb_cat_k(p->name);
              enc_sb_cat("{"); {
                enc_sb_cat_kv_comma("type", p->type);
                enc_sb_cat_kv("description", p->desc);
              }
              enc_sb_cat("}");
            }
          }
          enc_sb_cat("}");
        }
        enc_sb_cat("}");
      }
      enc_sb_cat("}");
    }
    enc_sb_cat("}");
  }

  enc_sb_cat("]"); // tools
  enc_sb_cat(",");
  enc_sb_cat_k("messages");
  enc_sb_cat("[");

  for (msg_t * m = msg_head; m; m = m->next) {
    if (m != msg_head) enc_sb_cat(",");
    enc_sb_cat("{"); {
      if (m->calls) {
        enc_sb_cat_k("tool_calls");
        enc_sb_cat("["); {
          for (msg_tool_call_t * c = m->calls; c->name; c++) {
            if (c != m->calls) enc_sb_cat(",");
            enc_sb_cat("{"); {
              enc_sb_cat_kv_comma("id", c->id);
              enc_sb_cat_kv_comma("type", "function");
              enc_sb_cat_k("function");
              enc_sb_cat("{"); {
                enc_sb_cat_kv_comma("name", c->name);
                enc_sb_cat_kv("arguments", c->args);
              }
              enc_sb_cat("}");
            }
            enc_sb_cat("}");
          }
        }
        enc_sb_cat("]");
        enc_sb_cat(",");
      }

      if (m->call) enc_sb_cat_kv_comma("tool_call_id",      m->call);
      if (m->name) enc_sb_cat_kv_comma("name",              m->name);
      if (m->cont) enc_sb_cat_kv_comma("content",           m->cont);
      if (m->reas) enc_sb_cat_kv_comma("reasoning_content", m->reas);
      enc_sb_cat_kv("role", m->role);
    }
    enc_sb_cat("}");
  }

  enc_sb_cat("]");

  enc_sb_cat("}");

  *enc_ptr = 0;
}
#endif
