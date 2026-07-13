#ifndef CRL_H
#define CRL_H

#include "rdr.h"
#include "utl.h"
#include "wrt.h"

const char * crl_url;

void crl_fetch(const char * session) {
  rdr_reset();
  wrt_reset();
  if (session) msg_save(session);

  CURL * curl = curl_easy_init();
  assert(curl);

  if (!crl_url) crl_url = utl_env("DUDUBOT_URL", "https://api.deepseek.com/chat/completions");

  curl_easy_setopt(curl, CURLOPT_URL, crl_url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, rdr_fn);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wrt_fn);
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  char * api_key = getenv("DUDUBOT_API_KEY");
  assert(api_key && "Missing environment 'DUDUBOT_API_KEY'");

  char auth[10240];
  snprintf(auth, sizeof(auth), "Authorization: Bearer %s", api_key);

  struct curl_slist * hdrs = NULL;
  hdrs = curl_slist_append(hdrs, "Content-type: application/json");
  hdrs = curl_slist_append(hdrs, auth);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) printf("err: %s\n", curl_easy_strerror(res));

  curl_slist_free_all(hdrs);

  curl_easy_cleanup(curl);

  wrt_flush();

  if (session) msg_save(session);
}

#endif
