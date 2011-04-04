#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_thread_create)
{
    flm_Thread * thread;
    
    if ((thread = flm_ThreadNew ((void*)42)) == NULL) {
        fail ("Thread creation failed");
    }
}
END_TEST

START_TEST(test_thread_release)
{
    flm_Thread * thread;

    setTestAlloc (0);
    if ((thread = flm_ThreadNew ((void*)42)) == NULL) {
        fail ("Thread creation failed");
    }
    flm_ThreadRelease (thread);

    /**
     * Give the thread enough time to quit
     */
    sleep(1);

    fail_unless (getAllocSum () == 0);
}
END_TEST

START_TEST(test_thread_alloc_fail)
{
    setTestAlloc (1);
    fail_if (flm_ThreadNew (NULL) != NULL);
    fail_unless (getAllocSum () == 0);
}
END_TEST

int _called = 0;

void
_thread_call_handler (flm_Thread * thread,
                      flm_Monitor * monitor,
                      void * state,
                      void * params)
{
    (void) thread;
    (void) monitor;

    fail_unless (state == (void *)42);
    fail_unless (params == (void *)21);
    _called = 1;
}

START_TEST(test_thread_call)
{
    flm_Thread * thread;

    setTestAlloc (0);
    if ((thread = flm_ThreadNew ((void*)42)) == NULL) {
        fail ("Thread creation failed");
    }

    if (flm_ThreadCall (thread, _thread_call_handler, (void *) 21) == -1) {
        fail ("Thread call failed");
    }

    /**
     * Give the thread enough time to execute the callback
     */
    sleep(1);

    fail_if (_called != 1);

    flm_ThreadRelease (thread);

    /**
     * Give the thread enough time to quit
     */
    sleep(1);

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
  tcase_add_test (tc_core, test_thread_release);
  tcase_add_test (tc_core, test_thread_alloc_fail);
  tcase_add_test (tc_core, test_thread_call);
  suite_add_tcase (s, tc_core);

  return s;
}
