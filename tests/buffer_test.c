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

START_TEST(test_buffer_printf)
{
    flm_Buffer * buffer;

    if ((buffer = flm_BufferPrintf ("TEST %d %s ", 42, "coucou")) == NULL) {
        fail ("Buffer creation failed");
    }
    fail_unless (strcmp (flm_BufferContent (buffer), "TEST 42 coucou ") == 0);
}
END_TEST

START_TEST(test_buffer_content)
{
    flm_Buffer * buffer;
    
    if ((buffer = flm_BufferNew ("test", 5, NULL)) == NULL) {
        fail ("Buffer creation failed");
    }
    fail_unless (strcmp (flm_BufferContent (buffer), "test") == 0);
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
    (void) content;
    _has_freed = 1;
}

START_TEST(test_buffer_free)
{
    flm_Buffer * buffer;
    
    setTestAlloc (0);
    _has_freed = 0;
    if ((buffer = flm_BufferNew ("test", 5, _buffer_free_handler)) == NULL) {
        fail ("Buffer creation failed");
    }
    flm_Release (FLM_OBJ (buffer));
    fail_if(_has_freed == 0);
    fail_unless (getAllocSum () == 0);
}
END_TEST

START_TEST(test_buffer_alloc_fail)
{
    setTestAlloc (1);
    fail_if (flm_BufferNew ("test", 5, NULL) != NULL);
    fail_unless (getAllocSum () == 0);

    setTestAlloc (1);
    fail_if (flm_BufferPrintf ("test") != NULL);
    fail_unless (getAllocSum () == 0);

    setTestAlloc (2);
    fail_if (flm_BufferPrintf ("test") != NULL);
    fail_unless (getAllocSum () == 0);
}
END_TEST

START_TEST(test_buffer_destruct)
{
    flm_Buffer * buffer;

    setTestAlloc (0);
    if ((buffer = flm_BufferNew ("test", 5, NULL)) == NULL) {
        fail ("Buffer creation failed");
    }
    flm_Release (FLM_OBJ (buffer));
    fail_unless (getAllocSum () == 0);

    if ((buffer = flm_BufferNew ("test", 5, NULL)) == NULL) {
        fail ("Buffer creation failed");
    }
    flm_Release (FLM_OBJ (buffer));
    fail_unless (getAllocSum () == 0);
}
END_TEST

Suite *
buffer_suite (void)
{
  Suite * s = suite_create ("buffer");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_buffer_create);
  tcase_add_test (tc_core, test_buffer_printf);
  tcase_add_test (tc_core, test_buffer_content);
  tcase_add_test (tc_core, test_buffer_length);
  tcase_add_test (tc_core, test_buffer_free);
  tcase_add_test (tc_core, test_buffer_alloc_fail);
  tcase_add_test (tc_core, test_buffer_destruct);
  suite_add_tcase (s, tc_core);

  return s;
}
