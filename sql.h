#ifndef SQL_H
#define SQL_H

#include "sqlite3.h"

#include <assert.h>

#define SQL_SCHEMA \
  "CREATE TABLE IF NOT EXISTS convo ( " \
  "  id   INT NOT NULL PRIMARY KEY,   " \
  "  json TEXT                        " \
  ")"
#define SQL_UPSERT_CONVO "INSERT OR REPLACE INTO convo (id, json) VALUES (?, ?)"

static void sql_check(int rc, sqlite3 * db, const char * msg) {
  if (rc == SQLITE_OK) return;
  fprintf(stderr, "%s: %s\n%s", msg, sqlite3_errstr(rc), sqlite3_errmsg(db));
  exit(1);
}
#define SQL_(x) sql_check((x), db, #x)

static int sql_id = -1;
static void sql_update_convo(const char * convo) {
  sqlite3 * db;
  assert(SQLITE_OK == sqlite3_open("poc.sqlite", &db));

  sqlite3_stmt * stmt;
  SQL_(sqlite3_prepare_v2(db, SQL_SCHEMA, -1, &stmt, NULL));
  assert(SQLITE_DONE == sqlite3_step(stmt));
  sqlite3_finalize(stmt);

  if (sql_id == -1) sql_id = time(NULL);

  SQL_(sqlite3_prepare_v2(db, SQL_UPSERT_CONVO, -1, &stmt, NULL));
  sqlite3_bind_int(stmt, 1, sql_id);
  sqlite3_bind_text(stmt, 2, convo, -1, NULL);
  assert(SQLITE_DONE == sqlite3_step(stmt));
  sqlite3_finalize(stmt);

  sqlite3_close(db);
}

#endif
