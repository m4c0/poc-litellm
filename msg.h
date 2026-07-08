#ifndef MSG_H
#define MSG_H

typedef struct msg_tool_call_s {
  const char * id;
  char * name;
  char * args;
} msg_tool_call_t;
typedef struct msg_s {
  const char * role;
  const char * call;
  const char * name;
  const char * fini;
  char * cont;
  char * reas;
  msg_tool_call_t * calls;
} msg_t;

msg_t msg_convo[10000] = {0};

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
    txt = c;
  }
  fprintf(f, ".\n");
}

int msg_save(const char * name) {
  FILE * f = fopen(name, "wb");
  for (msg_t * m = msg_convo; m->role; m++) {
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
    fprintf(f, "\n");
  }
  fclose(f);

  printf("saved in %s\n", name);
  return 0;
}
int msg_load(const char * name) {
  FILE * f = fopen(name, "rb"); 
  assert(f);

  char buf[102400];
  msg_t * m = NULL;
  char * tgt = NULL;
  unsigned tln = 0;
  while (fgets(buf, sizeof(buf), f)) {
    buf[strlen(buf) - 1] = 0;

    if (strncmp(buf, "role ", 5) == 0) {
      m = m ? m + 1 : msg_convo;
      m->role = strdup(buf + 5);
      tgt = NULL;
      continue;
    }
    if (strncmp(buf, "call ", 5) == 0) { tgt = NULL; m->call = strdup(buf + 5); continue; }
    if (strncmp(buf, "name ", 5) == 0) { tgt = NULL; m->name = strdup(buf + 5); continue; }
    if (strncmp(buf, "fini ", 5) == 0) { tgt = NULL; m->fini = strdup(buf + 5); continue; }

    if (strcmp(buf, "cont") == 0) { tln = 102400; tgt = m->cont = calloc(tln, 1); continue; }
    if (strcmp(buf, "reas") == 0) { tln = 102400; tgt = m->reas = calloc(tln, 1); continue; }

    if (!tgt) {
      if (strlen(buf) == 0) continue; // ignore empty line outside multilines
      fprintf(stderr, "invalid command: %s\n", buf);
      return 1;
    }
    if (strlen(buf) == 0) {
      assert(tln);
      strncat(tgt++, "\n", tln--);
      continue;
    }
    if (strcmp(buf, ".") == 0) {
      tgt = NULL;
      tln = 0;
      continue;
    }
    if (strncmp(buf, "  ", 2) != 0) {
      fprintf(stderr, "invalid indentation: [%s]\n", buf);
      return 1;
    }
    int len = strlen(buf) - 2;
    assert(tln >= len + 1);
    strncpy(tgt, buf + 2, tln);
    tgt += len;
    tln -= len;
  }
  fclose(f);

  printf("loaded from %s\n", name);
  return 0;
}

#endif
