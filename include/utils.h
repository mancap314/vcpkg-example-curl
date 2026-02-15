#ifndef UTILS_H
#define UTILS_H

#include <lexbor/core/types.h>
#include <lexbor/dom/interface.h>
#include <lexbor/html/interfaces/document.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <lexbor/html/html.h>
#include <lexbor/html/interface.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

typedef enum {
  FT_SUCCESS = 0,
  FT_CREATE_FAILED,
  FT_PARSING_FAILED,
  FT_NO_TITLE,
  FT_N_FETCH_TITLE_STATUS
} fetch_title_status;

static const char fetch_title_message[FT_N_FETCH_TITLE_STATUS][30] = {
    [FT_SUCCESS] = "OK\n",
    [FT_CREATE_FAILED] = "Failed to create document\n",
    [FT_PARSING_FAILED] = "Failed to parse document\n",
    [FT_NO_TITLE] = "No title in document\n"};

static size_t mem_cb(void *contents, size_t size, size_t nmemb, void *userp);
int fetch_html(const char *url_to_get, struct MemoryStruct *chunk);
fetch_title_status fetch_title_from_html(lxb_char_t *html, size_t html_len,
                                         lxb_char_t *title_buffer,
                                         size_t title_buffer_len);
int fetch_title_from_url(const char *url_to_get, char *title_buffer,
                         size_t title_buffer_len);

#endif // UTILS_H
