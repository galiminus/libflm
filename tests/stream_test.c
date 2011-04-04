#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        fail ("Monitor creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], NULL)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], NULL)) == NULL) {
        fail ("Stream creation failed");
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
        fail ("Monitor creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], NULL)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], NULL)) == NULL) {
        fail ("Stream creation failed");
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
        fail ("Monitor creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], NULL)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], NULL)) == NULL) {
        fail ("Stream creation failed");
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
    fail_unless (flm_BufferLength (buffer) == 1);
    fail_unless (flm_BufferContent (buffer)[0] == 'a');
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
        fail ("Monitor creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], stream_in)) == NULL) {
        fail ("Stream creation failed");
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

static int nb_read = 0;
static void
_big_read_handler (flm_Stream * stream, void * state, flm_Buffer * buffer)
{
    flm_Stream * stream_in;
    size_t i;

    stream_in = (flm_Stream *) state;
    nb_read += flm_BufferLength (buffer);
    for (i = 0; i < flm_BufferLength (buffer); i++) {
        fail_unless (flm_BufferContent (buffer)[i] == 'a');
    }
    flm_BufferRelease (buffer);

    if (nb_read == 20000) {
        flm_StreamClose (stream);
        flm_StreamClose (stream_in);
    }
}

START_TEST(test_stream_big_read)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;
    size_t i;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], stream_in)) == NULL) {
        fail ("Stream creation failed");
    }
    flm_StreamOnRead (stream_out, _big_read_handler);
    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    for (i = 0; i < 1000; i++) {
        fail_if (write (fds[1], "aaaaaaaaaaaaaaaaaaaa", 20) == -1);
    }

    flm_StreamRelease (stream_in);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_read == 20000);
    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_stream_printf)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;
    size_t i;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], stream_in)) == NULL) {
        fail ("Stream creation failed");
    }
    flm_StreamOnRead (stream_out, _big_read_handler);
    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    for (i = 0; i < 1000; i++) {
        flm_StreamPrintf (stream_in, "%saaaaaaaaaa", "aaaaaaaaaa");
    }

    flm_StreamRelease (stream_in);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_read == 20000);
    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_stream_push_buffer)
{
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;
    flm_Buffer * buffer;
    size_t i;

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], stream_in)) == NULL) {
        fail ("Stream creation failed");
    }
    flm_StreamOnRead (stream_out, _big_read_handler);
    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    for (i = 0; i < 1000; i++) {
        buffer = flm_BufferNew ("aaaaaaaaaaaaaaaaaaaa", 20, NULL);
        flm_StreamPushBuffer (stream_in, buffer, 0, 20);
        flm_BufferRelease (buffer);
    }

    flm_StreamRelease (stream_in);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_read == 20000);
    fail_unless (nb_closed == 2);
    fail_unless (getAllocSum () == 0);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

START_TEST(test_stream_push_file)
{
    int raw_file;
    flm_File * file;
    int baseFD;
    int fds[2];
    flm_Monitor * monitor;
    flm_Stream * stream_in;
    flm_Stream * stream_out;
    size_t i;

    int error;

    /**
     * Create the file
     */
    raw_file = open ("/tmp/_libflm_testfile", O_CREAT|O_TRUNC|O_WRONLY);
    for (i = 0; i < 1000; i++) {
        error = write (raw_file, "aaaaaaaaaaaaaaaaaaaa", 20);
    }
    close (raw_file);

    setTestAlloc (0);

    baseFD = getFDCount ();
    fail_if (pipe (fds) == -1);

    if ((monitor = flm_MonitorNew ()) == NULL) {
        fail ("Monitor creation failed");
    }
    if ((stream_in = flm_StreamNew (monitor, fds[1], (void *)42)) == NULL) {
        fail ("Stream creation failed");
    }
    if ((stream_out = flm_StreamNew (monitor, fds[0], stream_in)) == NULL) {
        fail ("Stream creation failed");
    }
    flm_StreamOnRead (stream_out, _big_read_handler);
    flm_StreamOnClose (stream_out, _close_handler);
    flm_StreamOnClose (stream_in, _close_handler);

    file = flm_FileOpen ("", "/tmp/_libflm_testfile", "r");
    flm_StreamPushFile (stream_in, file, 0, 0);
    flm_FileRelease (file);

    flm_StreamRelease (stream_in);
    flm_StreamRelease (stream_out);

    flm_MonitorWait (monitor);
    flm_MonitorRelease (monitor);

    fail_unless (nb_read == 20000);
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
  tcase_add_test (tc_core, test_stream_big_read);
  tcase_add_test (tc_core, test_stream_printf);
  tcase_add_test (tc_core, test_stream_push_buffer);

  tcase_set_timeout(tc_core, 30);
  tcase_add_test (tc_core, test_stream_push_file);

  suite_add_tcase (s, tc_core);

  return s;
}
