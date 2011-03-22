#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_stream_create)
{
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;
    
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creatstreamn failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], NULL)) == NULL) {
        fail ("Stream creatstreamn failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], NULL)) == NULL) {
        fail ("Stream creatstreamn failed");
    }
}
END_TEST

static int nb_closed = 0;
static void
_close_handler (flm_Stream * _stream, void * _state)
{
    (void) _stream;
    (void) _state;
    nb_closed++;
}

START_TEST(test_stream_close)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creatstreamn failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], NULL)) == NULL) {
        fail ("Stream creatstreamn failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], NULL)) == NULL) {
        fail ("Stream creatstreamn failed");
    }

    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    flm_StreamClose (stream_in);
    flm_StreamRelease (stream_in);

    flm_StreamClose (stream_out);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_stream_shutdown)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creatstreamn failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], NULL)) == NULL) {
        fail ("Stream creatstreamn failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], NULL)) == NULL) {
        fail ("Stream creatstreamn failed");
    }

    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    flm_StreamShutdown (stream_in);
    flm_StreamRelease (stream_in);

    flm_StreamShutdown (stream_out);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_stream_alloc_fail)
{
    setTestAlloc (1);
    fail_if (flm_StreamNew (NULL, 0, NULL) != NULL);
    fail_unless (getAllocSum () == 0);
}
END_TEST

static int has_read = 0;
static void
_read_handler (flm_Stream * stream, void * state, flm_Buffer * buffer)
{
    flm_Stream * stream_in;

    stream_in = (flm_Stream *) state;
    has_read = 1;
    flm_BufferRelease (buffer);
    flm_StreamClose (stream);
    flm_StreamClose (stream_in);
}

START_TEST(test_stream_read)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creatstreamn failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Stream creatstreamn failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], stream_in)) == NULL) {
        fail ("Stream creatstreamn failed");
    }
    flm_StreamOnRead (stream_out, _read_handler);
    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    fail_if (write (fds[1], "a", 1) == -1);

    flm_StreamRelease (stream_in);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (has_read == 1);
    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

Suite *
stream_suite (void)
{
  Suite * s = suite_create ("stream");

  /* Stream test case */
  TCase *tc_core = tcase_create ("stream");

  tcase_add_test (tc_core, test_stream_create);
  tcase_add_test (tc_core, test_stream_close);
  tcase_add_test (tc_core, test_stream_shutdown);
  tcase_add_test (tc_core, test_stream_alloc_fail);
  tcase_add_test (tc_core, test_stream_read);
  suite_add_tcase (s, tc_core);

  return s;
}
