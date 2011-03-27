#include <check.h>
#include <errno.h>

#include "flm/flm.h"
#include "flm/core/private/epoll.h"
#include "flm/core/private/monitor.h"

#include "test_utils.h"

int
_epollCreateHandler (int _count)
{
    (void) _count;

    errno = 42;
    return (-1);
}

START_TEST(test_epoll_create_fail)
{
    flm__setEpollCreateHandler (_epollCreateHandler);

    setTestAlloc (0);
    fail_unless (flm__EpollNew () == NULL);
    fail_unless (getAllocSum () == 0);
    fail_unless (flm_Error () == FLM_ERR_ERRNO);
    fail_unless (errno == 42);
}
END_TEST

START_TEST(test_epoll_alloc_fail)
{
    int baseFD;

    baseFD = getFDCount ();

    setTestAlloc (1);
    fail_unless (flm__EpollNew () == NULL);
    fail_unless (getAllocSum () == 0);
    fail_unless (flm_Error () == FLM_ERR_NOMEM);
    fail_unless (getFDCount () == baseFD);

    setTestAlloc (2);
    fail_unless (flm__EpollNew () == NULL);
    fail_unless (getAllocSum () == 0);
    fail_unless (flm_Error () == FLM_ERR_NOMEM);
    fail_unless (getFDCount () == baseFD);

    setTestAlloc (3);
    fail_unless (flm__EpollNew () == NULL);
    fail_unless (getAllocSum () == 0);
    fail_unless (flm_Error () == FLM_ERR_NOMEM);
    fail_unless (getFDCount () == baseFD);
}
END_TEST

int
_epollWaitHandler (int                  _epfd,
                   struct epoll_event * _events,
                   int                  _maxevents,
                   int                  _timeout)
{
    (void) _epfd;
    (void) _events;
    (void) _maxevents;
    (void) _timeout;

    errno = 42;
    return (-1);
}

START_TEST(test_epoll_wait_fail)
{
    flm_Monitor *       monitor;
    flm_Stream *        read;
    flm_Stream *        write;
    int                 fds[2];

    flm__setEpollWaitHandler (_epollWaitHandler);

    setTestAlloc (0);
    if ((monitor = (flm_Monitor *)flm__EpollNew ()) == NULL) {
        fail ("Monitor creation failed");
    }

    if (pipe (fds) == -1) {
        fail ("Pipe creation failed");
    }
    
    read = flm_StreamNew (monitor, fds[0], (void *) 42);
    if (read == NULL) {
        fail ("Stream (read) creation failed");
    }

    fail_unless (monitor->io.count == 1);
    flm_StreamRelease (read);

    write = flm_StreamNew (monitor, fds[1], (void *) 42);
    if (write == NULL) {
        fail ("Stream (write) creation failed");
    }

    flm_StreamPrintf (write, "a");

    fail_unless (monitor->io.count == 2);
    flm_StreamRelease (write);

    fail_unless (flm_MonitorWait (monitor) == -1);
    flm_MonitorRelease (monitor);

    fail_unless (getAllocSum () == 0);
}
END_TEST

static bool _again = false;
int
_epollWaitHandlerAgain (int                  _epfd,
                        struct epoll_event * _events,
                        int                  _maxevents,
                        int                  _timeout)
{
    (void) _epfd;
    (void) _events;
    (void) _maxevents;
    (void) _timeout;

    if (_again) {
        errno = 42;
    }
    else {
        errno = EAGAIN;
        _again = true;
    }
    return (-1);
}

START_TEST(test_epoll_wait_fail_eagain)
{
    flm_Monitor *       monitor;
    flm_Stream *        read;
    flm_Stream *        write;
    int                 fds[2];

    flm__setEpollWaitHandler (_epollWaitHandlerAgain);

    setTestAlloc (0);
    if ((monitor = (flm_Monitor *)flm__EpollNew ()) == NULL) {
        fail ("Monitor creation failed");
    }

    if (pipe (fds) == -1) {
        fail ("Pipe creation failed");
    }
    
    read = flm_StreamNew (monitor, fds[0], (void *) 42);
    if (read == NULL) {
        fail ("Stream (read) creation failed");
    }

    fail_unless (monitor->io.count == 1);
    flm_StreamRelease (read);

    write = flm_StreamNew (monitor, fds[1], (void *) 42);
    if (write == NULL) {
        fail ("Stream (write) creation failed");
    }

    flm_StreamPrintf (write, "a");

    fail_unless (monitor->io.count == 2);
    flm_StreamRelease (write);

    fail_unless (flm_MonitorWait (monitor) == -1);
    flm_MonitorRelease (monitor);

    fail_unless (_again == true);
    fail_unless (getAllocSum () == 0);
}
END_TEST

Suite *
epoll_suite (void)
{
  Suite * s = suite_create ("epoll");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_epoll_create_fail);
  tcase_add_test (tc_core, test_epoll_alloc_fail);
  tcase_add_test (tc_core, test_epoll_wait_fail);
  tcase_add_test (tc_core, test_epoll_wait_fail_eagain);
  suite_add_tcase (s, tc_core);
  return s;
}
