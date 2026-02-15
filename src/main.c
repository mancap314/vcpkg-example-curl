#include "utils.h"

#define TITLE_BUFFER_LEN (256)

int main(int argc, char *argv[argc + 1]) {
  char usage[] = "CLI tool to fetch title of a web page\n"
                 "usage: ./fetch_title https://your.url/\n";

  if (argc < 2) {
    fprintf(stderr, "[ERROR] You must specify the url as first argument:\n%s\n",
            usage);
    return EXIT_FAILURE;
  }

  char title_buffer[TITLE_BUFFER_LEN] = {0};
  int ret = fetch_title_from_url(argv[1], title_buffer, TITLE_BUFFER_LEN);
  if (ret != EXIT_SUCCESS) {
    fprintf(stderr, "Failed to fetch title from %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  fprintf(stdout, "Title of %s: %s\n", argv[1], title_buffer);
  return EXIT_SUCCESS;
}
