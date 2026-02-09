#include <lexbor/core/types.h>
#include <lexbor/dom/interface.h>
#include <lexbor/html/interfaces/document.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <lexbor/html/html.h>
#include <lexbor/html/interface.h>

#define TITLE_BUFFER_LEN 256

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t mem_cb(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (NULL == mem->memory) {
    /* out of memory */
    fputs("Not enough memory (realloc() returned NULL)\n", stderr);
    return EXIT_FAILURE;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = '\0';

  return realsize;
}

typedef enum {
  FT_SUCCESS = 0,
  FT_CREATE_FAILED,
  FT_PARSING_FAILED,
  FT_NO_TITLE,
  FT_N_FETCH_TITLE_STATUS
} fetch_title_status;

const char fetch_title_message[FT_N_FETCH_TITLE_STATUS][30] = {
    [FT_SUCCESS] = "OK",
    [FT_CREATE_FAILED] = "Failed to create document",
    [FT_PARSING_FAILED] = "Failed to parse document",
    [FT_NO_TITLE] = "No title in document"};

fetch_title_status fetch_title(lxb_char_t *html, size_t html_len,
                               lxb_char_t *title_buffer,
                               size_t title_buffer_len) {
  lxb_status_t status;
  lxb_html_document_t *document;
  const lxb_char_t *title;
  size_t title_len;
  fetch_title_status ret = FT_SUCCESS;

  /* initialization */
  document = lxb_html_document_create();
  if (NULL == document) {
    ret = FT_CREATE_FAILED;
    goto defer_err;
  }

  /* parse html */
  status = lxb_html_document_parse(document, html, html_len);
  if (status != LXB_STATUS_OK) {
    ret = FT_PARSING_FAILED;
    goto defer_err;
  }

  title = lxb_html_document_title(document, &title_len);
  if (NULL == title) {
    ret = FT_NO_TITLE;
    goto defer_err;
  }

  if (title_len > title_buffer_len - 1) {
    printf("[WARNING] %s(): title has length %zu, will be truncated to length "
           "%zu\n",
           __func__, title_len, title_buffer_len);
    title_len = title_buffer_len - 1;
  }
  memcpy(title_buffer, (const lxb_char_t *)title, title_len);
  title_buffer[title_len] = '\0';

  lxb_html_document_destroy(document);
  return ret;

defer_err:
  if (document)
    lxb_html_document_destroy(document);
  return ret;
}

int main(void) {
  const char url_to_get[] = "https://www.example.com/";

  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1); /* grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify url to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url_to_get);

  /* send all received data to this callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, mem_cb);

  /* we pass our `chunk` struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers do not like requests that are made without a user-agent field,
   * so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  // Source - https://stackoverflow.com/a/62030603
  // Posted by ariia
  // Retrieved 2026-02-05, License - CC BY-SA 4.0

  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYSTATUS, 1);
  curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "ca-certificates/cacert.pem");
  curl_easy_setopt(curl_handle, CURLOPT_CAPATH, "ca-certificates/cacert.pem");

  /* get it */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  } else {
    /*
     * Now `chunk` points to a memory block of size `chunk.size` bytes and
     * contains the remote file. Do something with it
     */

    printf("%zu bytes retrieved:\n%s \n", chunk.size, chunk.memory);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /* we are done with libcurl, so clean it up */
  curl_global_cleanup();

  int ret = EXIT_SUCCESS;
  lxb_char_t title_buffer[TITLE_BUFFER_LEN];
  fetch_title_status fts = fetch_title((lxb_char_t *)chunk.memory, chunk.size,
                                       title_buffer, TITLE_BUFFER_LEN);
  if (fts != FT_SUCCESS) {
    fputs(fetch_title_message[fts], stderr);
    ret = EXIT_FAILURE;
  } else {
    printf("Title found: %s\n", title_buffer);
  }

  free(chunk.memory);
  return ret;
}
