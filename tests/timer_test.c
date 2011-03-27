#include <check.h>

#include "flm/flm.h"

#include "flm/core/private/monitor.h"
#include "flm/core/private/timer.h"

#include "test_utils.h"

/**
 * \example timer_test.c
 */

START_TEST(test_timer_create)
{
    flm_Monitor * monitor;
    flm_Timer * timer;

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor, NULL, NULL, 10)) == NULL) {
        fail ("Timer creation failed");
    }
}
END_TEST

START_TEST(test_timer_alloc_fail)
{
    flm_Monitor * monitor;

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    setTestAlloc (1);
    fail_if (flm_TimerNew (monitor, NULL, NULL, 10) != NULL);
}
END_TEST

START_TEST(test_timer_destruct)
{
    flm_Monitor * monitor;
    flm_Timer * timer;

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor, NULL, NULL, 10)) == NULL) {
        fail ("Timer creation failed");
    }
    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);
}
END_TEST

int _elapsed = 0;

void
_timer_handler (flm_Timer * timer,
                void * _state)
{
    (void) timer;

    fail_unless (_state == (void*)42);
    _elapsed++;
}

START_TEST(test_timer_cancel)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor,
                               _timer_handler,
                               (void *) 42,
                               2000)) == NULL) {
        fail ("Timer creation failed");
    }

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerCancel (timer);
    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff > 10);

    fail_if (_elapsed != 0);
}
END_TEST

START_TEST(test_timer_short)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor,
                               _timer_handler,
                               (void *) 42,
                               monitor->tm.res * 2)) == NULL) {
        fail ("Timer creation failed");
    }

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff < (monitor->tm.res * 1.90));
    fail_if (diff > (monitor->tm.res * 2.10));

    fail_if (_elapsed != 1);
}
END_TEST

START_TEST(test_timer_long)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor,
                               _timer_handler,
                               (void *) 42,
                               monitor->tm.res * 10)) == NULL) {
        fail ("Timer creation failed");
    }

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff < (monitor->tm.res * 9.5));
    fail_if (diff > (monitor->tm.res * 11.1));

    fail_if (_elapsed != 1);
}
END_TEST

START_TEST(test_timer_longer_than_wheel)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    flm__setMonitorDefaultTmSize (3);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor,
                               _timer_handler,
                               (void *) 42,
                               monitor->tm.res * 10)) == NULL) {
        fail ("Timer creation failed");
    }
    fail_unless (timer->wh.rounds == 3);

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff < (monitor->tm.res * 9.5));
    fail_if (diff > (monitor->tm.res * 11.1));

    fail_if (_elapsed != 1);
}
END_TEST

START_TEST(test_timer_reset)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor,
                               _timer_handler,
                               (void *) 42,
                               monitor->tm.res * 2)) == NULL) {
        fail ("Timer creation failed");
    }

    flm_TimerReset (timer, monitor->tm.res * 4);

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff < (monitor->tm.res * 3.90));
    fail_if (diff > (monitor->tm.res * 4.10));

    fail_if (_elapsed != 1);
}
END_TEST

START_TEST(test_timer_multiple)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;
    int                 count;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    for (count = 0; count < 100; count++) {
        usleep (2000);
        if ((timer = flm_TimerNew (monitor,
                                   _timer_handler,
                                   (void *) 42,
                                   monitor->tm.res * 2)) == NULL) {
            fail ("Timer creation failed");
        }
        flm_TimerRelease (timer);
    }

    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff < (monitor->tm.res * 1.50) + 200);
    fail_if (diff > (monitor->tm.res * 3.10) + 200);

    fail_if (_elapsed != count);
}
END_TEST

void
_timer_handler_reset (flm_Timer * timer,
                      void * _monitor)
{
    flm_Monitor * monitor = _monitor;

    if (_elapsed == 0) {
        flm_TimerReset (timer, monitor->tm.res * 2);
    }
    _elapsed++;
}

START_TEST(test_timer_reset_in_handler)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;
    int                 diff;

    fail_if (_elapsed != 0);

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((timer = flm_TimerNew (monitor,
                               _timer_handler_reset,
                               monitor, /* state */
                               monitor->tm.res * 2)) == NULL) {
        fail ("Timer creation failed");
    }

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_MonitorWait (monitor);

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm_TimerRelease (timer);
    flm_MonitorRelease (monitor);
    fail_unless (getAllocSum () == 0);

    diff = (((end.tv_sec * 1000) + end.tv_usec / 1000) -
            ((start.tv_sec * 1000) + start.tv_usec / 1000));

    fail_if (diff < (monitor->tm.res * 3.90));
    fail_if (diff > (monitor->tm.res * 4.10));

    fail_if (_elapsed != 2);
}
END_TEST

Suite *
timer_suite (void)
{
  Suite * s = suite_create ("timer");

  /* Core test case */
  TCase *tc_core = tcase_create ("Timer");
  tcase_add_test (tc_core, test_timer_create);
  tcase_add_test (tc_core, test_timer_alloc_fail);
  tcase_add_test (tc_core, test_timer_destruct);
  tcase_add_test (tc_core, test_timer_cancel);
  tcase_add_test (tc_core, test_timer_short);
  tcase_add_test (tc_core, test_timer_long);
  tcase_add_test (tc_core, test_timer_longer_than_wheel);
  tcase_add_test (tc_core, test_timer_multiple);
  tcase_add_test (tc_core, test_timer_reset);
  tcase_add_test (tc_core, test_timer_reset_in_handler);
  suite_add_tcase (s, tc_core);
  return s;
}
