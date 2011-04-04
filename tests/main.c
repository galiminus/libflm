#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <check.h>

#include "test.h"

/**
 * To change backends
 */
#include "flm/core/private/monitor.h"

int
main (void)
{
    int number_failed;

    Suite * allocSuite = alloc_suite ();
    SRunner * allocRunner = srunner_create (allocSuite);

    Suite * bufferSuite = buffer_suite ();
    SRunner * bufferRunner = srunner_create (bufferSuite);

    Suite * monitorSuite = monitor_suite ();
    SRunner * monitorRunner = srunner_create (monitorSuite);

    Suite * epollSuite = epoll_suite ();
    SRunner * epollRunner = srunner_create (epollSuite);

    Suite * selectSuite = select_suite ();
    SRunner * selectRunner = srunner_create (selectSuite);

    Suite * timerSuite = timer_suite ();
    SRunner * timerRunner = srunner_create (timerSuite);

    Suite * threadSuite = thread_suite ();
    SRunner * threadRunner = srunner_create (threadSuite);

    Suite * threadPoolSuite = thread_pool_suite ();
    SRunner * threadPoolRunner = srunner_create (threadPoolSuite);

    Suite * ioSuite = io_suite ();
    SRunner * ioRunner = srunner_create (ioSuite);

    Suite * streamSuite = stream_suite ();
    SRunner * streamRunner = srunner_create (streamSuite);

    Suite * tcpServerSuite = tcp_server_suite ();
    SRunner * tcpServerRunner = srunner_create (tcpServerSuite);

    number_failed = 0;
    
    srunner_run_all (allocRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (allocRunner);
    srunner_free (allocRunner);

    srunner_run_all (bufferRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (bufferRunner);
    srunner_free (bufferRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (ioRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (ioRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (ioRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (ioRunner);
    srunner_free (ioRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (streamRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (streamRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (streamRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (streamRunner);
    srunner_free (streamRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (tcpServerRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (tcpServerRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (tcpServerRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (tcpServerRunner);
    srunner_free (tcpServerRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (monitorRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (monitorRunner);

    srunner_run_all (epollRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (epollRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (monitorRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (monitorRunner);
    srunner_free (monitorRunner);

    srunner_run_all (selectRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (selectRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (timerRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (timerRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (timerRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (timerRunner);
    srunner_free (timerRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (threadRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (threadRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (threadRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (threadRunner);
    srunner_free (threadRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (threadPoolRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (threadPoolRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (threadPoolRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (threadPoolRunner);
    srunner_free (threadPoolRunner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
