#include "utils.h"
#include <curl/curl.h>
#include <curl/easy.h>

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

int fetch_html(const char *url_to_get, struct MemoryStruct *chunk) {

  CURL *curl_handle;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify url to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url_to_get);

  /* Follow (max 10) redirection */
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10L);

  /* send all received data to this callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, mem_cb);

  /* we pass our `chunk` struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);

  /* some servers do not like requests that are made without a user-agent field,
   * so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  // Source - https://stackoverflow.com/a/62030603
  // Posted by ariia
  // Retrieved 2026-02-05, License - CC BY-SA 4.0

  char certificate_path[] = "ca-certificates/cacert.pem";

  curl_easy_setopt(curl_handle, CURLOPT_CAINFO, certificate_path);
  curl_easy_setopt(curl_handle, CURLOPT_CAPATH, certificate_path);

  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYSTATUS, 0L); // Disable OCSP
  curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

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

    printf("[INFO] %s: %zu bytes retrieved.\n", __func__, chunk->size);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /* we are done with libcurl, so clean it up */
  curl_global_cleanup();
  return res;
}

fetch_title_status fetch_title_from_html(lxb_char_t *html, size_t html_len,
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

defer_err:
  if (document)
    lxb_html_document_destroy(document);
  return ret;
}

int fetch_title_from_url(const char *url_to_get, char *title_buffer,
                         size_t title_buffer_len) {
  struct MemoryStruct chunk;
  chunk.memory = malloc(1); /* grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */
  int ret = EXIT_SUCCESS;

  int fetch_res = fetch_html(url_to_get, &chunk);
  if (fetch_res != CURLE_OK) {
    ret = EXIT_FAILURE;
    goto defer;
  }

  fetch_title_status fts =
      fetch_title_from_html((lxb_char_t *)chunk.memory, chunk.size,
                            (lxb_char_t *)title_buffer, title_buffer_len);
  if (fts != FT_SUCCESS) {
    fputs(fetch_title_message[fts], stderr);
    ret = EXIT_FAILURE;
    goto defer;
  }

defer:
  free(chunk.memory);
  return ret;
}
