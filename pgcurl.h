#ifndef PGCURL_H
#define PGCURL_H

#define PGCURL_GET 1
#define PGCURL_PUT 2
#define PGCURL_POST 3
#define PGCURL_DELETE 4

/* libcurl specific */
#include <curl/curl.h>

/*PostgreSQL specific*/
#include "postgres.h"
#include <inttypes.h>
#include "access/heapam.h"
#include "access/htup.h"
#include "fmgr.h"
#include "funcapi.h"
#include "lib/stringinfo.h"
#include "utils/builtins.h"
#include "utils/datetime.h"
#include "utils/guc.h"
#include "utils/memutils.h"
#include "utils/lsyscache.h"

/* Per-backend global state. */
struct pgcurl_global
{
  /* context in which long-lived state is allocated */
  MemoryContext pg_ctxt;
};
 
struct help_struct {
  char *string;
  int size;
};

void _PG_init(void);
void _PG_fini(void);
Datum curl_do_actual_work(int, PG_FUNCTION_ARGS);
Datum pgcurl_get(PG_FUNCTION_ARGS);
Datum pgcurl_put(PG_FUNCTION_ARGS);
Datum pgcurl_delete(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(pgcurl_get);
PG_FUNCTION_INFO_V1(pgcurl_put);
PG_FUNCTION_INFO_V1(pgcurl_delete);
#endif
