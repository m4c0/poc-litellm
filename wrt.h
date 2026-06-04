#ifndef WRT_H
#define WRT_H

#include "json.h"
#include "msg.h"

static char json_buf[10240];
static char * json_ptr;
static msg_t * wrt_msg;

static struct json_value_s * find_element(struct json_object_s * obj, const char * el) {
  struct json_object_element_s * it = obj->start;
  while (it) {
    if (0 != strcmp(el, it->name->string)) {
      it = it->next;
      continue;
    }
    break;
  }
  return it ? it->value : NULL;
}

static json_array_element_t * to_arr(json_value_t * v) {
  json_array_t * arr = v ? json_value_as_array(v) : NULL;
  return arr ? arr->start : NULL;
}
static const char * to_str(struct json_value_s * v) {
  struct json_string_s * str = v ? json_value_as_string(v) : NULL;
  return str ? str->string : NULL;
}

enum {
  rsn_start,
  rsn_reasoning,
  rsn_output,
} rsn = rsn_start;
static void process_json() {
  struct json_value_s * root = json_parse(json_buf, json_ptr - json_buf);
  struct json_object_s * obj = json_value_as_object(root);
  assert(obj && "Root value should be an object");

  json_array_element_t * arr = to_arr(find_element(obj, "choices"));
  assert(arr && "Expecting to have 'choices'");

  obj = json_value_as_object(arr->value);
  assert(obj && "First choice should be an object");

  const char * str = to_str(find_element(obj, "finish_reason"));
  if (str) {
    fprintf(stderr, "\nfinish reason: %s\n", str);
    free(root);
    return;
  }
 
  obj = json_value_as_object(find_element(obj, "delta"));
  assert(obj && "Delta should be an object");

  arr = to_arr(find_element(obj, "tool_calls"));
  if (arr) {
    fprintf(stderr, "\nTOOL CALLS\n");
    while (arr) {
      json_object_t * obj = json_value_as_object(arr->value);
      assert(obj && "Tool calls must be an object");

      const char * id = to_str(find_element(obj, "id"));
      assert(id && "Expecting an ID for a tool call");

      json_object_t * fn = json_value_as_object(find_element(obj, "function"));
      assert(fn && "Expecting 'function' as a tool call");

      const char * name = to_str(find_element(fn, "name"));
      assert(name && "Expecting a name for a tool call");

      const char * args = to_str(find_element(fn, "arguments"));
      assert(args && "Expecting arguments for a tool call");

      fprintf(stderr, "  %s\n", id);
      fprintf(stderr, "  => %s with %s\n", name, args);
      arr = arr->next;
    }
  }

  str = to_str(find_element(obj, "reasoning_content"));
  if (str) {
    if (rsn != rsn_reasoning) {
      fprintf(stderr, "\nTHINKING: ");
      rsn = rsn_reasoning;
    }
    fprintf(stderr, "%s", str);
    free(root);
    return;
  }

  str = to_str(find_element(obj, "content"));
  if (str) {
    if (rsn != rsn_output) {
      fprintf(stderr, "\nASSISTANT ");
      rsn = rsn_output;
    }
    fprintf(stderr, "%s", str);
    free(root);
    return;
  }

  fprintf(stderr, "\nunknown delta\n");
  free(root);
}

enum {
  fsm_data_0, fsm_data_1, fsm_data_2, fsm_data_3, fsm_data_4, fsm_data_5,
  fsm_done_0, fsm_done_1, fsm_done_2, fsm_done_3, fsm_done_4, fsm_done_5, fsm_done_6, fsm_done_7, fsm_done_8,
  fsm_json,
  fsm_after_eol,
} fsm = fsm_data_0;

static void chk(char c, char e) {
  if (c == e) return;
  fprintf(stderr, "ERROR: Expecting %c got %c\n\n", e, c);
  exit(1);
}
static void pump(char c) {
  switch (fsm) {
    case fsm_data_0: chk(c, 'd'); fsm++; break;
    case fsm_data_1: chk(c, 'a'); fsm++; break;
    case fsm_data_2: chk(c, 't'); fsm++; break;
    case fsm_data_3: chk(c, 'a'); fsm++; break;
    case fsm_data_4: chk(c, ':'); fsm++; break;
    case fsm_data_5: chk(c, ' '); fsm++; break;

    case fsm_done_0: {
      if (c == '{') {
        json_ptr = json_buf;
        *json_ptr++ = c;
        fsm = fsm_json;
      } else {
        chk(c, '[');
        fsm++;
      }
      break;
    }
    case fsm_done_1: chk(c, 'D'); fsm++; break;
    case fsm_done_2: chk(c, 'O'); fsm++; break;
    case fsm_done_3: chk(c, 'N'); fsm++; break;
    case fsm_done_4: chk(c, 'E'); fsm++; break;
    case fsm_done_5: chk(c, ']'); fsm++; break;
    case fsm_done_6: chk(c, '\n'); fsm++; break;
    case fsm_done_7: {
      chk(c, '\n');
      fsm++;

      enc_reset();
      puts(enc_txt);

      break;
    }
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
  for (wrt_msg = msg_convo; wrt_msg->role; wrt_msg++) {}

  wrt_msg->role = "assistant";

  fsm = fsm_data_0;
}

#endif
