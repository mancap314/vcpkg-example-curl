#include <curl/curl.h>
#include <stdio.h>

int main(void) {
  CURL *curl;

  CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
  if (result)
    return (int)result;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    /* follow redirection: */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    /* Perform the request, result gets the return code */
    result = curl_easy_perform(curl);
    /* Check for errors */
    if (result != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    /* Always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
