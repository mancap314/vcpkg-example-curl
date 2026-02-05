#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

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
  free(chunk.memory);

  /* we are done with libcurl, so clean it up */
  curl_global_cleanup();

  return EXIT_SUCCESS;
}
