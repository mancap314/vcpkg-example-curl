#include "utils.h"
#include <check.h>
#include <curl/curl.h>
#include <lexbor/html/html.h>
#include <lexbor/html/interface.h>
#include <stdlib.h>

void setup(void) {}
void teardown(void) {}

START_TEST(test_fetch_html_basic) {
  struct MemoryStruct chunk;
  chunk.memory = malloc(1); /* grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  const char url_to_get[] = "https://google.com/";
  int fetch_res = fetch_html(url_to_get, &chunk);

  free(chunk.memory);
  ck_assert_int_eq(fetch_res, CURLE_OK);
}
END_TEST

START_TEST(test_fetch_html_size) {
  struct MemoryStruct chunk;
  chunk.memory = malloc(1); /* grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  const char url_to_get[] = "https://stackoverflow.com/";
  int fetch_res = fetch_html(url_to_get, &chunk);

  free(chunk.memory);
  ck_assert_int_gt(chunk.size, 0);
}
END_TEST

START_TEST(test_fetch_title_from_html_basic) {
  lxb_char_t html[] = "<!DOCTYPE html>\n"
                      "<html>\n"
                      "\t<head>\n"
                      "\t\t<title>Page Title</title>\n"
                      "\t</head>\n"
                      "\t<body>\n"
                      "\t\t<h1>This is a Heading</h1>\n"
                      "\t\t<p>This is a paragraph.</p>\n"
                      "\t</body>\n"
                      "</html>\n";

  size_t html_size = sizeof(html);
  lxb_char_t title_buffer[256] = {0};
  size_t title_buffer_len = 256;

  int status =
      fetch_title_from_html(html, html_size, title_buffer, title_buffer_len);

  ck_assert_int_eq(status, FT_SUCCESS);
  ck_assert_str_eq(title_buffer, "Page Title");
}
END_TEST

Suite *basic_suite(void) {
  Suite *s;
  TCase *tc_fetch_html, *tc_fetch_title_from_html;

  s = suite_create("Basic utils functions");

  tc_fetch_html = tcase_create("fetch_html() function");
  tcase_add_checked_fixture(tc_fetch_html, setup, teardown);
  tcase_add_test(tc_fetch_html, test_fetch_html_basic);
  tcase_add_test(tc_fetch_html, test_fetch_html_size);
  suite_add_tcase(s, tc_fetch_html);

  tc_fetch_title_from_html = tcase_create("fetch_title_from_html() function");
  tcase_add_checked_fixture(tc_fetch_title_from_html, setup, teardown);
  tcase_add_test(tc_fetch_title_from_html, test_fetch_title_from_html_basic);
  suite_add_tcase(s, tc_fetch_title_from_html);

  return s;
}

int main(void) {
  puts("Running tests for utils library...");

  int number_failed;
  SRunner *sr;

  sr = srunner_create(basic_suite());
  srunner_run_all(sr, CK_VERBOSE);

  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
