#include <check.h>

#include "flm/flm.h"
#include "flm/core/private/monitor.h"

#include "test_utils.h"

START_TEST(test_monitor_create)
{
    flm_Monitor * monitor;

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    fail_unless (monitor->count == 0);
    fail_unless (monitor->tm.pos == 0);
}
END_TEST

START_TEST(test_monitor_alloc_fail)
{
    setTestAlloc (1);
    fail_if (flm_MonitorNew () != NULL);
    fail_unless (getAllocSum () == 0);

    setTestAlloc (2);
    fail_if (flm_MonitorNew () != NULL);
    fail_unless (getAllocSum () == 0);
}
END_TEST

int
_gettime_handler (int                   type,
                  struct timespec *     timespec)
{
    (void) type;
    (void) timespec;

    return (-1);
}

START_TEST(test_monitor_gettime_fail)
{
    setTestAlloc (0);
    flm__setClockGettime (_gettime_handler);
    fail_if (flm_MonitorNew () != NULL);
    fail_unless (getAllocSum () == 0);    
}
END_TEST

START_TEST(test_monitor_destruct)
{
    flm_Monitor * monitor;

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    flm_Release (FLM_OBJ (monitor));
    fail_unless (getAllocSum () == 0);
}
END_TEST

START_TEST(test_monitor_wait_nothing)
{
    flm_Monitor * monitor;

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if (flm_MonitorWait (monitor) == -1) {
        fail ("Monitor wait failed");
    }
    flm_Release (FLM_OBJ (monitor));
    fail_unless (getAllocSum () == 0);
}
END_TEST

START_TEST(test_monitor_wait)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }

    timer = flm_TimerNew (monitor, NULL, NULL, 1500);
    if (timer == NULL) {
        fail ("Timer creation failed");
    }
    fail_unless (monitor->count == 1);
    flm_Release (FLM_OBJ (timer));

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    if (flm_MonitorWait (monitor) == -1) {
        fail ("Monitor wait failed");
    }
    flm_Release (FLM_OBJ (monitor));

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    fail_if ((end.tv_sec - start.tv_sec) < 1);
    fail_unless (getAllocSum () == 0);
}
END_TEST

START_TEST(test_monitor_wait_gettime_fail)
{
    flm_Monitor *       monitor;
    flm_Timer *         timer;
    struct timeval      start;
    struct timeval      end;

    setTestAlloc (0);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }

    timer = flm_TimerNew (monitor, NULL, NULL, 1500);
    if (timer == NULL) {
        fail ("Timer creation failed");
    }
    fail_unless (monitor->count == 1);
    flm_Release (FLM_OBJ (timer));

    if (gettimeofday (&start, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    flm__setClockGettime (_gettime_handler);

    fail_unless (flm_MonitorWait (monitor) == -1);
    flm_Release (FLM_OBJ (monitor));

    if (gettimeofday (&end, NULL) == -1) {
        fail ("gettimeofday() failed");
    }

    fail_if ((end.tv_sec - start.tv_sec) < 1);
    fail_unless (getAllocSum () == 0);
}
END_TEST

void
_stream_read_handler (flm_Stream *  stream,
                      void *        state,
                      flm_Buffer *  buffer)
{
    fail_unless (state == (void *) 42);
    fail_unless (flm_BufferLength (buffer) == 1);
    fail_unless (flm_BufferContent (buffer)[0] == 'a');
    flm_Release (buffer);
    flm_IOClose (FLM_IO (stream));
    return ;
}

void
_stream_write_handler (flm_Stream *  stream,
                       void *        state,
                       size_t        size)
{
    fail_unless (state == (void *) 42);
    fail_unless (size == 1);
    flm_IOClose (FLM_IO (stream));
    return ;
}

START_TEST(test_monitor_wait_io)
{
    flm_Monitor *       monitor;
    flm_Stream *        read;
    flm_Stream *        write;
    int                 fds[2];

    setTestAlloc (0);
    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }

    if (pipe (fds) == -1) {
        fail ("Pipe creation failed");
    }

    read = flm_StreamNew (monitor, fds[0], (void *) 42);
    if (read == NULL) {
        fail ("Stream (read) creation failed");
    }

    flm_StreamOnRead (read, _stream_read_handler);

    fail_unless (monitor->count == 1);
    flm_Release (FLM_OBJ (read));

    write = flm_StreamNew (monitor, fds[1], (void *) 42);
    if (write == NULL) {
        fail ("Stream (write) creation failed");
    }

    flm_StreamOnWrite (write, _stream_write_handler);
    flm_StreamPrintf (write, "a");

    fail_unless (monitor->count == 2);
    flm_Release (FLM_OBJ (write));

    if (flm_MonitorWait (monitor) == -1) {
        fail ("Monitor wait failed");
    }
    flm_Release (FLM_OBJ (monitor));

    printf("SUM: %d\n", getAllocSum ());
    fail_unless (getAllocSum () == 0);
}
END_TEST

Suite *
monitor_suite (void)
{
  Suite * s = suite_create ("monitor");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_monitor_create);
  tcase_add_test (tc_core, test_monitor_alloc_fail);
  tcase_add_test (tc_core, test_monitor_gettime_fail);
  tcase_add_test (tc_core, test_monitor_destruct);
  tcase_add_test (tc_core, test_monitor_wait_nothing);
  tcase_add_test (tc_core, test_monitor_wait);
  tcase_add_test (tc_core, test_monitor_wait_gettime_fail);
  tcase_add_test (tc_core, test_monitor_wait_io);
  suite_add_tcase (s, tc_core);
  return s;
}
