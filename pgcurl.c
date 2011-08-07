/*
 * PostgreSQL functions to interface with libcurl.
 * Copyright (c) 2011 Hannu Valtonen <hannu.valtonen@ohmu.fi>
 * See the file COPYING for distribution terms.
 */

#include "pgcurl.h"
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

static struct pgcurl_global globals;
 
void _PG_init(void)
{
  MemoryContext old_ctxt;
  globals.pg_ctxt = AllocSetContextCreate(TopMemoryContext,
					  "pgcurl global context",
					  ALLOCSET_SMALL_MINSIZE,
					  ALLOCSET_SMALL_INITSIZE,
					  ALLOCSET_SMALL_MAXSIZE);
  old_ctxt = MemoryContextSwitchTo(globals.pg_ctxt);
}
 
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct help_struct *pooh = (struct help_struct *)userp;
 
  if(size*nmemb < 1)
    return 0;
 
  if(pooh->size) {
    *(char *)ptr = pooh->string[0]; /* copy one single byte */ 
    pooh->string++;                 /* advance pointer */ 
    pooh->size--;                /* less data left */ 
    return 1;                        /* we return 1 byte at a time! */ 
  }
  return 0;                          /* no more data left to deliver */ 
}

static size_t
write_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct help_struct *mem = (struct help_struct *)data;
 
  mem->string = repalloc(mem->string, mem->size + realsize + 1);
  if (mem->string == NULL)
    elog(ERROR, "not enough memory to realloc)\n");
 
  memcpy(&(mem->string[mem->size]), ptr, realsize);
  mem->size += realsize;
  mem->string[mem->size] = 0;
 
  return realsize;
}

Datum pgcurl_put(PG_FUNCTION_ARGS)
{
  return curl_do_actual_work(PGCURL_PUT, fcinfo);
}

Datum pgcurl_get(PG_FUNCTION_ARGS)
{
  return curl_do_actual_work(PGCURL_GET, fcinfo);
}

Datum pgcurl_delete(PG_FUNCTION_ARGS)
{
  return curl_do_actual_work(PGCURL_DELETE, fcinfo);
}

Datum curl_do_actual_work(int method_type, PG_FUNCTION_ARGS)
{
  CURL *curl;
  CURLcode res;
  StringInfoData buf;
  text *url_text = PG_GETARG_TEXT_P(0), *payload_text;
  char *url, *payload;
  struct help_struct read_chunk, write_chunk;
 
  read_chunk.string = palloc(1);  /* will be grown as needed by the realloc above */ 
  read_chunk.size = 0;    /* no data at this point */ 
  url = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(url_text)));

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "pgcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); /* Consider making this configurable */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); /* Consider making this configurable */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&read_chunk);
    if (method_type == PGCURL_PUT) {
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
      payload_text = PG_GETARG_TEXT_P(0);
      payload = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(payload_text)));

      write_chunk.string = payload;
      write_chunk.size = strlen(payload);

      curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
      curl_easy_setopt(curl, CURLOPT_READDATA, &write_chunk);
      curl_easy_setopt(curl, CURLOPT_INFILESIZE, write_chunk.size);
     }

    if (method_type == PGCURL_POST) {
      curl_easy_setopt(curl, CURLOPT_POST, 1);
    }

    if (method_type == PGCURL_DELETE) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    res = curl_easy_perform(curl);
    if (res != 0)
      elog(WARNING, "PGCurl error: %s, URL: %s\n", curl_easy_strerror(res), url);
    curl_easy_cleanup(curl);
  }

  initStringInfo(&buf);
  if(read_chunk.string) {
    appendStringInfo(&buf, "%s", read_chunk.string);
    pfree(read_chunk.string);
  }
  PG_RETURN_DATUM(DirectFunctionCall1(textin, CStringGetDatum(buf.data)));
}
