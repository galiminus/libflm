#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_io_create)
{
    int fds[2];
    flm_Monitor * monitor;
    flm_IO * io_in;
    flm_IO * io_out;
    
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((io_out = flm_IONew (monitor, fds[0], NULL)) == NULL) {
        fail ("Io creation failed");
    }
    if ((io_in = flm_IONew (monitor, fds[1], NULL)) == NULL) {
        fail ("Io creation failed");
    }
}
END_TEST

static int nb_closed = 0;
static void
_close_handler (flm_IO * _io, void * _state)
{
    (void) _io;
    (void) _state;
    nb_closed++;
}

START_TEST(test_io_close)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_IO * io_in;
    flm_IO * io_out;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((io_out = flm_IONew (monitor, fds[0], NULL)) == NULL) {
        fail ("Io creation failed");
    }
    if ((io_in = flm_IONew (monitor, fds[1], NULL)) == NULL) {
        fail ("Io creation failed");
    }

    flm_IOOnClose (io_out, _close_handler);
    flm_IOOnClose (io_in, _close_handler);

    flm_IOClose (io_in);
    flm_IORelease (io_in);

    flm_IOClose (io_out);
    flm_IORelease (io_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_io_shutdown)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_IO * io_in;
    flm_IO * io_out;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((io_out = flm_IONew (monitor, fds[0], NULL)) == NULL) {
        fail ("Io creation failed");
    }
    if ((io_in = flm_IONew (monitor, fds[1], NULL)) == NULL) {
        fail ("Io creation failed");
    }

    flm_IOOnClose (io_out, _close_handler);
    flm_IOOnClose (io_in, _close_handler);

    flm_IOShutdown (io_in);
    flm_IORelease (io_in);

    flm_IOShutdown (io_out);
    flm_IORelease (io_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_io_alloc_fail)
{
    setTestAlloc (1);
    fail_if (flm_IONew (NULL, 0, NULL) != NULL);
    fail_unless (getAllocSum () == 0);
}
END_TEST

static int has_read = 0;
static void
_read_handler (flm_IO * io, void * state)
{
    flm_IO * io_in;

    io_in = (flm_IO *) state;
    has_read = 1;
    flm_IOClose (io);
    flm_IOClose (io_in);
}

START_TEST(test_io_read)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_IO * io_in;
    flm_IO * io_out;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((io_in = flm_IONew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Io creation failed");
    }
    if ((io_out = flm_IONew (monitor, fds[0], io_in)) == NULL) {
        fail ("Io creation failed");
    }
    flm_IOOnRead (io_out, _read_handler);
    flm_IOOnClose (io_out, _close_handler);
    flm_IOOnClose (io_in, _close_handler);

    fail_if (write (fds[1], "a", 1) == -1);

    flm_IORelease (io_in);
    flm_IORelease (io_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (has_read == 1);
    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

Suite *
io_suite (void)
{
  Suite * s = suite_create ("io");

  /* Io test case */
  TCase *tc_core = tcase_create ("io");

  tcase_add_test (tc_core, test_io_create);
  tcase_add_test (tc_core, test_io_close);
  tcase_add_test (tc_core, test_io_shutdown);
  tcase_add_test (tc_core, test_io_alloc_fail);
  tcase_add_test (tc_core, test_io_read);
  suite_add_tcase (s, tc_core);

  return s;
}
