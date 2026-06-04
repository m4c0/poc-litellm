#ifndef MSG_H
#define MSG_H

typedef struct msg_tool_call_s {
  const char * id;
  const char * name;
  const char * args;
} msg_tool_call_t;
typedef struct msg_s {
  const char * role;
  const char * call;
  const char * name;
  const char * cont;
  msg_tool_call_t calls[10];
} msg_t;

msg_t msg_convo[10000] = {
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
  }
};

#endif
