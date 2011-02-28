#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_thread_create)
{
    if (flm_ThreadNew (NULL, NULL) == NULL) {
        fail ("Thread creation failed");
    }
}
END_TEST

START_TEST(test_thread_content)
{
    flm_Thread * thread;
    
    if ((thread = flm_ThreadNew ((void*)42, NULL)) == NULL) {
        fail ("Thread creation failed");
    }
    fail_unless (flm_ThreadContent (thread) == ((void*)42));
}
END_TEST

int _has_freed;

void
_thread_free_handler (void * content) {
    _has_freed = 1;
}

START_TEST(test_thread_free)
{
    flm_Thread * thread;
    
    _has_freed = 0;
    if ((thread = flm_ThreadNew (NULL, _thread_free_handler)) == NULL) {
        fail ("Thread creation failed");
    }
    flm_Release (FLM_OBJ (thread));
    fail_if(_has_freed == 0);
}
END_TEST

START_TEST(test_thread_alloc_fail)
{
    flm_Thread * thread;

    setTestAlloc (1);
    fail_if (flm_ThreadNew (NULL, NULL) != NULL);
}
END_TEST

START_TEST(test_thread_destruct)
{
    flm_Thread * thread;

    setTestAlloc (0);
    if ((thread = flm_ThreadNew (NULL, NULL)) == NULL) {
        fail ("Thread creation failed");
    }
    flm_Release (FLM_OBJ (thread));
    fail_unless (getAllocSum () == 0);
}
END_TEST

Suite *
thread_suite (void)
{
  Suite * s = suite_create ("thread");

  /* Thread test case */
  TCase *tc_core = tcase_create ("Thread");

  tcase_add_test (tc_core, test_thread_create);
  tcase_add_test (tc_core, test_thread_content);
  tcase_add_test (tc_core, test_thread_free);
  tcase_add_test (tc_core, test_thread_alloc_fail);
  tcase_add_test (tc_core, test_thread_destruct);
  suite_add_tcase (s, tc_core);

  return s;
}
