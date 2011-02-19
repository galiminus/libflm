#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_buffer_create)
{
    if (flm_BufferNew ("", 0, NULL) == NULL) {
        fail ("Buffer creation failed");
    }
}
END_TEST

START_TEST(test_buffer_content)
{
    flm_Buffer * buffer;
    
    if ((buffer = flm_BufferNew ("test", 5, NULL)) == NULL) {
        fail ("Buffer creation failed");
    }
    if (strcmp (flm_BufferContent (buffer), "test")) {
        fail ("Invalid buffer content");
    }
}
END_TEST

START_TEST(test_buffer_length)
{
    flm_Buffer * buffer;
    
    if ((buffer = flm_BufferNew ("test", 5, NULL)) == NULL) {
        fail ("Buffer creation failed");
    }
    if (flm_BufferLength (buffer) == 4) {
        fail ("Invalid buffer content");
    }
}
END_TEST

int _has_freed;

void
_buffer_free_handler (void * content) {
    _has_freed = 1;
}

START_TEST(test_buffer_free)
{
    flm_Buffer * buffer;
    
    _has_freed = 0;
    if ((buffer = flm_BufferNew ("test", 5, _buffer_free_handler)) == NULL) {
        fail ("Buffer creation failed");
    }
    flm_Release (FLM_OBJ (buffer));
    fail_if(_has_freed == 0);
}
END_TEST

START_TEST(test_buffer_alloc_fail)
{
    flm_Buffer * buffer;

    setTestAlloc (1);
    fail_if (flm_BufferNew ("test", 5, NULL) != NULL);
}
END_TEST

Suite *
libflm_suite (void)
{
  Suite * s = suite_create ("buffer");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_buffer_create);
  tcase_add_test (tc_core, test_buffer_content);
  tcase_add_test (tc_core, test_buffer_length);
  tcase_add_test (tc_core, test_buffer_free);
  tcase_add_test (tc_core, test_buffer_alloc_fail);
  suite_add_tcase (s, tc_core);

  return s;
}

int
main (void)
{
    int number_failed;
    Suite * s = libflm_suite ();
    SRunner * sr = srunner_create (s);
    
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}