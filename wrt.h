#ifndef WRT_H
#define WRT_H

#include "jsn.h"
#include "msg.h"

char json_buf[10240];
char * json_ptr;
msg_t * wrt_msg;

static json_array_element_t * to_arr(json_value_t * v) {
  json_array_t * arr = v ? json_value_as_array(v) : NULL;
  return arr ? arr->start : NULL;
}

static str_bld_t * wrt_cont;
static str_bld_t * wrt_reas;

static void wrt_cat(char * dst, const char * src, int n) {
  int len = strlen(dst);
  dst += len;
  n -= len;

  for (; *src && n > 0; n--, src++, dst++) *dst = *src;
}

enum {
  rsn_start,
  rsn_reasoning,
  rsn_output,
} rsn = rsn_start;
static void process_json() {
  struct json_object_s * obj = jsn_parse_object(json_buf, json_ptr - json_buf);
  assert(obj && "Root value should be an object");

  json_array_element_t * arr = to_arr(jsn_find_element(obj, "choices"));
  assert(arr && "Expecting to have 'choices'");

  obj = json_value_as_object(arr->value);
  assert(obj && "First choice should be an object");

  const char * str = jsn_str(jsn_find_element(obj, "finish_reason"));
  if (str) {
    wrt_msg->fini = strdup(str);
    fprintf(stderr, "\nfinish reason: %s\n", str);
    return;
  }
 
  obj = json_value_as_object(jsn_find_element(obj, "delta"));
  assert(obj && "Delta should be an object");

  arr = to_arr(jsn_find_element(obj, "tool_calls"));
  if (arr) {
    while (arr) {
      json_object_t * obj = json_value_as_object(arr->value);
      assert(obj && "Tool calls must be an object");

      int idx = atoi(json_value_as_number(jsn_find_element(obj, "index"))->number);

      msg_tool_call_t * call = wrt_msg->calls;
      for (int i = 0; i <= idx; i++, call = call->next) {
        if (!call) call = msg_alloc_call(wrt_msg);
      }

      call = wrt_msg->calls;
      for (int i = 0; i < idx; i++, call = call->next) {}

      if (!call->name) call->name = calloc(1024, 1);
      if (!call->args) call->args = calloc(1024, 1);

      const char * id = jsn_str(jsn_find_element(obj, "id"));
      if (id) call->id = strdup(id);

      json_object_t * fn = json_value_as_object(jsn_find_element(obj, "function"));
      if (fn) {
        const char * name = jsn_str(jsn_find_element(fn, "name"));
        if (name) wrt_cat(call->name, name, 1024);

        const char * args = jsn_str(jsn_find_element(fn, "arguments"));
        if (args) wrt_cat(call->args, args, 1024);
      }
      arr = arr->next;
    }
  }

  str = jsn_str(jsn_find_element(obj, "reasoning_content"));
  if (str) {
    if (rsn != rsn_reasoning) {
      fprintf(stderr, "\nTHINKING: ");
      rsn = rsn_reasoning;
    }
    fprintf(stderr, "%s", str);
    str_bld_cat(&wrt_reas, str);
    return;
  }

  str = jsn_str(jsn_find_element(obj, "content"));
  if (str) {
    if (rsn != rsn_output) {
      fprintf(stderr, "\nASSISTANT ");
      rsn = rsn_output;
    }
    fprintf(stderr, "%s", str);
    str_bld_cat(&wrt_cont, str);
    return;
  }
}

enum {
  fsm_data_0, fsm_data_1, fsm_data_2, fsm_data_3, fsm_data_4, fsm_data_5,
  fsm_done_0, fsm_done_1, fsm_done_2, fsm_done_3, fsm_done_4, fsm_done_5, fsm_done_6, fsm_done_7, fsm_done_8,
  fsm_dump,
  fsm_json,
  fsm_after_eol,
} fsm = fsm_data_0;

static void chk(char c, char e) {
  if (c == e) {
    fsm++;
    return;
  }
  putc(c, stderr);
  fsm = fsm_dump;
}
static void pump(char c) {
  switch (fsm) {
    case fsm_dump: putc(c, stderr); break;

    case fsm_data_0:
      if (c == '{') {
        putc(c, stderr);
        fsm = fsm_dump;
        break;
      }
      chk(c, 'd');
      break;
    case fsm_data_1: chk(c, 'a'); break;
    case fsm_data_2: chk(c, 't'); break;
    case fsm_data_3: chk(c, 'a'); break;
    case fsm_data_4: chk(c, ':'); break;
    case fsm_data_5: chk(c, ' '); break;

    case fsm_done_0: {
      if (c == '{') {
        json_ptr = json_buf;
        *json_ptr++ = c;
        fsm = fsm_json;
      } else {
        chk(c, '[');
      }
      break;
    }
    case fsm_done_1: chk(c, 'D'); break;
    case fsm_done_2: chk(c, 'O'); break;
    case fsm_done_3: chk(c, 'N'); break;
    case fsm_done_4: chk(c, 'E'); break;
    case fsm_done_5: chk(c, ']'); break;
    case fsm_done_6: chk(c, '\n'); break;
    case fsm_done_7: chk(c, '\n'); break;
    case fsm_done_8: {
      assert(0 && "Unexpected data after [DONE]");
      break;
    }

    case fsm_json: {
      assert(json_ptr < json_buf + sizeof(json_buf) - 1);
      *json_ptr++ = c;
      if (c == '\n') fsm++;
      break;
    }

    case fsm_after_eol: {
      if (c != '\n') {
        fsm = fsm_data_0; // continue same JSON
        break;
      }

      *json_ptr = 0;
      process_json();

      fsm = fsm_data_0; // prepare JSON
      break;
    }
  }
}

size_t wrt_fn(char * data, size_t sz, size_t n, void * ptr) {
  for (int i = 0; i < sz * n; i++) pump(data[i]);
  return n;
}

void wrt_reset() {
  wrt_msg = msg_alloc();
  wrt_msg->role = "assistant";

  fsm = fsm_data_0;
}

void wrt_flush() {
  wrt_msg->cont = str_bld_flush(wrt_cont);
  wrt_msg->reas = str_bld_flush(wrt_reas);

  wrt_cont = NULL;
  wrt_reas = NULL;
}

#endif
