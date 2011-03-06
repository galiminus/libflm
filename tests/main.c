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

    Suite * bufferSuite = buffer_suite ();
    SRunner * bufferRunner = srunner_create (bufferSuite);

    Suite * containerSuite = container_suite ();
    SRunner * containerRunner = srunner_create (containerSuite);

    Suite * monitorSuite = monitor_suite ();
    SRunner * monitorRunner = srunner_create (monitorSuite);

    Suite * timerSuite = timer_suite ();
    SRunner * timerRunner = srunner_create (timerSuite);

    Suite * threadSuite = thread_suite ();
    SRunner * threadRunner = srunner_create (threadSuite);

    number_failed = 0;
    
    srunner_run_all (bufferRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (bufferRunner);
    srunner_free (bufferRunner);

    srunner_run_all (containerRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (containerRunner);
    srunner_free (containerRunner);

    printf (">> Switch to the Epoll backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_EPOLL);
    srunner_run_all (monitorRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (monitorRunner);

    printf (">> Switch to the select() backend\n");
    flm__setMonitorBackend (FLM__MONITOR_BACKEND_SELECT);
    srunner_run_all (monitorRunner, CK_NORMAL);
    number_failed += srunner_ntests_failed (monitorRunner);
    srunner_free (monitorRunner);

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

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
