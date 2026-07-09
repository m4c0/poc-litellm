#ifndef MSG_H
#define MSG_H

#include "str.h"
#include "tll.h"

typedef struct msg_tool_call_s {
  const char * id;
  char * name;
  char * args;

  struct msg_tool_call_s * next;
} msg_tool_call_t;
typedef struct msg_s {
  const char * role;
  const char * call;
  const char * name;
  const char * fini;
  const char * cont;
  const char * reas;
  msg_tool_call_t * calls;

  struct msg_s * next;
} msg_t;

msg_t * msg_head = NULL;

void msg_purge() {
  msg_t * m = msg_head;
  while (m) {
    msg_t * mm = m;
    m = m->next;
    free(mm);
  }
}
msg_t * msg_alloc() {
  if (msg_head == NULL) {
    return msg_head = calloc(sizeof(msg_t), 1);
  }
  msg_t * m = msg_head;
  while (m->next) m = m->next;
  return m->next = calloc(sizeof(msg_t), 1);
}
msg_tool_call_t * msg_alloc_call(msg_t * msg) {
  if (msg->calls == NULL) {
    return msg->calls = calloc(sizeof(msg_tool_call_t), 1);
  }
  msg_tool_call_t * m = msg->calls;
  while (m->next) m = m->next;
  return m->next = calloc(sizeof(msg_tool_call_t), 1);
}

void msg_print_indented(FILE * f, const char * txt) {
  while (txt && *txt) {
    const char * c = strchr(txt, '\n');
    if (c == txt) {
      fprintf(f, "\n");
      txt++;
      continue;
    }
    int len = c ? (int)(c - txt) : strlen(txt);
    fprintf(f, "  %.*s\n", len, txt);
    txt = c ? c + 1 : NULL;
  }
  fprintf(f, ".\n");
}

int msg_save(const char * name) {
  FILE * f = fopen(name, "wb");

  for (tll_t * t = tll_head; t; t = t->next) fprintf(f, "tool %s\n", t->name);
  if (tll_head) fprintf(f, "\n");

  for (msg_t * m = msg_head; m; m = m->next) {
    fprintf(f, "role %s\n", m->role);
    if (m->call) fprintf(f, "call %s\n", m->call);
    if (m->name) fprintf(f, "name %s\n", m->name);
    if (m->fini) fprintf(f, "fini %s\n", m->fini);
    if (m->cont) {
      fprintf(f, "cont\n");
      msg_print_indented(f, m->cont);
    }
    if (m->reas) {
      fprintf(f, "reas\n");
      msg_print_indented(f, m->reas);
    }
    for (msg_tool_call_t * t = m->calls; t; t = t->next) {
      fprintf(f, "calls %s %s\n  %s\n.\n", t->id, t->name, t->args);
    }
    fprintf(f, "\n");
  }
  fclose(f);

  printf("saved in %s\n", name);
  return 0;
}
int msg_load(const char * name, int purge) {
  FILE * f = fopen(name, "rb"); 
  assert(f);

  if (purge) msg_purge();

  char buf[102400];
  msg_t * m = NULL;
  msg_tool_call_t * t = NULL;
  const char ** tgt = NULL;
  str_bld_t * tst = NULL;
  while (fgets(buf, sizeof(buf), f)) {
    buf[strlen(buf) - 1] = 0;

    if (strncmp(buf, "tool ", 5) == 0) {
      assert(!msg_head && "tools can only be defined in an empty conversation");
      if (tll_load(buf + 5)) return 1;
      continue;
    }

    if (strncmp(buf, "role ", 5) == 0) {
      m = msg_alloc();
      m->role = strdup(buf + 5);
      t = NULL;
      assert(!tgt);
      continue;
    }
    if (strncmp(buf, "call ", 5) == 0) { assert(!tgt); m->call = strdup(buf + 5); continue; }
    if (strncmp(buf, "name ", 5) == 0) { assert(!tgt); m->name = strdup(buf + 5); continue; }
    if (strncmp(buf, "fini ", 5) == 0) { assert(!tgt); m->fini = strdup(buf + 5); continue; }

    if (strcmp(buf, "cont") == 0) { tgt = &m->cont; continue; }
    if (strcmp(buf, "reas") == 0) { tgt = &m->reas; continue; }

    if (strncmp(buf, "calls ", 6) == 0) {
      if (!t) t = msg_alloc_call(m);

      char * id = buf + 6;

      char * name = strchr(id, ' ');
      assert(name);
      *name++ = 0;

      t->id = strdup(id);
      t->name = strdup(name);

      tgt = &t->args;
      continue;
    }

    if (!tgt) {
      if (strlen(buf) == 0) continue; // ignore empty line outside multilines
      fprintf(stderr, "invalid command: %s\n", buf);
      return 1;
    }
    if (strlen(buf) == 0) {
      assert(tgt);
      str_bld_cat(&tst, "\n");
      continue;
    }
    if (strcmp(buf, ".") == 0) {
      *tgt = str_bld_flush(tst);
      tgt = NULL;
      tst = NULL;
      continue;
    }
    if (strncmp(buf, "  ", 2) != 0) {
      fprintf(stderr, "invalid indentation: [%s]\n", buf);
      return 1;
    }
    assert(tgt);
    str_bld_cat(&tst, buf + 2);
    str_bld_cat(&tst, "\n");
  }
  fclose(f);

  printf("loaded from %s\n", name);
  return 0;
}

#endif
