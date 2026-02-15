#include "utils.h"
#include <stdlib.h>

#define TITLE_BUFFER_LEN (256)

int main(void) {
  const char url_to_get[] = "https://www.example.com/";
  char title_buffer[TITLE_BUFFER_LEN] = {0};
  int ret = fetch_title_from_url(url_to_get, title_buffer, TITLE_BUFFER_LEN);
  if (ret != EXIT_SUCCESS) {
    fprintf(stderr, "Failed to fetch title from %s\n", url_to_get);
    return EXIT_FAILURE;
  }
  fprintf(stdout, "Title of %s: %s\n", url_to_get, title_buffer);
  return EXIT_SUCCESS;
}
