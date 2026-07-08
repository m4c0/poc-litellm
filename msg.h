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
    int len = c ? (int)(c - txt) : strlen(txt);
    fprintf(f, "  %.*s\n", (int)(c - txt), txt);
    txt = c;
  }
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
  }
  fclose(f);

  printf("saved in %s\n", name);
  return 0;
}
int msg_load(const char * name) {
  printf("would load from %s\n", name);
  return 0;
}

#endif
